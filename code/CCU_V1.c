/*Private Include*/
#include "CCU.h"
/*Private Define*/
#define DEFAULT_COM 5

/*Global Var*/
unsigned char TxBuffer[32];
unsigned char RxBuffer[256];
/*Handle*/
HANDLE Comm;
/*Overlapped Vars*/
OVERLAPPED Eol={0};
OVERLAPPED Wol={0};
OVERLAPPED Rol={0};

DWORD dwThreadID;
bool ComStopMsg;

/*Main Function*/
int main(void){

    /*Variable*/
    int State;
    /*Initial Serial*/
    State = Initial_Serial(DEFAULT_COM);
    if(SETTING_OK == State){
        /*Initial Succeeded*/
        /*open thread hold waitcomm*/

        if(UCC_Initial(Comm)){
            
           
        
        }
        else
        {
            /* failed */
        }
        

    }
    else if(State > 0){
        CloseHandle(Comm);
    }
    
system("PAUSE");
return 0;

    

}



/* Private Sys Functions*/

/*memcat*/
char * 
memcat(char *dest, size_t dest_len, const char *src, size_t src_len){
    memcpy(dest+dest_len, src, src_len);
    return dest;
}

/*Initial_Serial*/

int Initial_Serial(int com){
    char com_num[] = "";
    strcpy(com_num,CCT(com));
    Comm = CreateFile(  com_num,                          // COM number              
                        GENERIC_READ | GENERIC_WRITE,     // Read & Write
                        0,                                // No Sharing
                        NULL ,                            // None Security
                        OPEN_EXISTING,                    // Open existing port only
                        FILE_FLAG_OVERLAPPED,             // Non Overlapped I/O
                        NULL   );                         // Null for Comm Devices
    
    if(Comm == INVALID_HANDLE_VALUE){
        printf("COM%d Initial Failed,Please Confirm Port Number\r\n",com);
        return -1;
    }
    else{

        printf("opening serial port successful\r\n");  
        int Ok = 0;
        DCB hcomPara = {0};
		hcomPara.DCBlength = sizeof(hcomPara);

        //General Setting
        hcomPara.BaudRate = CBR_115200;
        hcomPara.ByteSize = 8;
        hcomPara.StopBits = ONESTOPBIT;
        hcomPara.Parity = (BYTE)PARITY_NONE;

        //Advance Setting
        hcomPara.XonLim = 1024;
        hcomPara.XoffLim = 1024;
        hcomPara.XonChar = 0x11;
        hcomPara.XoffChar = 0x13;

        hcomPara.EofChar = 0x1A;    //BreakChar will set by dafault
        hcomPara.EvtChar = 0x1A;
        hcomPara.ErrorChar = 0x0;
        
        //about HandShake - need to add
        /*ref: https://www.cs.colorado.edu/~main/cs1300-old/include/ddk/ntddser.h*/

        hcomPara.fRtsControl = RTS_CONTROL_HANDSHAKE;
        hcomPara.fOutxCtsFlow = true;
        hcomPara.fOutxDsrFlow = false;
        hcomPara.fOutX = false;
        hcomPara.fInX = false;

        //Init TimeOut Struture
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout         = -1; 
        timeouts.ReadTotalTimeoutConstant    = 500; 
        timeouts.ReadTotalTimeoutMultiplier  = -1; 
        timeouts.WriteTotalTimeoutConstant   = 500; 
        timeouts.WriteTotalTimeoutMultiplier = 0;

        //Set Wait Mask
        DWORD Insize = 4096;
		DWORD Outsize = 2048;

        Ok =  SetCommState(Comm,&hcomPara);
        if(!Ok) {
            printf("Set Gerneral Part Failed\r\n");
            return Ok;
        }

        Ok = SetupComm(Comm,Insize,Outsize);
        

        if(!Ok) {
            printf("Set Queue Part Failed\r\n");
            return Ok;
        }


        Ok = SetCommTimeouts(Comm,&timeouts);
        if(!Ok) {
            printf("Set TimeOut Part Failed\r\n");
            return Ok;
        }


        Ok = SetCommMask(Comm,EVFLAG);
        if(!Ok) {
            printf("Set Event Part Failed\r\n");
            return Ok;
        }

        // DWORD mask;
        // GetCommMask(Comm,&mask);
        // printf("%x\r\n",mask);

        PurgeComm(Comm,IOCLEAR);

        printf("All Setting Done!\r\n");
        return SETTING_OK;
    }

}

/*Covert COM to TCHAR TYPE*/

char* CCT(int COM_NUM){

    //need to confirm return char array has a '\0'
    static char start[6] = "COM";
    char COM_CHAR[1];
    *COM_CHAR = COM_NUM + '0';
    strncat(start,COM_CHAR,1);
    return start;
}

int UCC_Initial(HANDLE handle){
    
    if(!Request(handle,SC_C_GET_CCU_VERSIONINFO)){
        printf("Please Check AccessPort\r\n , SC_C_GET_CCU_VERSIONINFO");
        return -1;
    }
    printf("Get Version Command Send Ok\r\n");
    /*Other Config Setting*/


    return true;
    
}

bool Request(HANDLE handle ,WORD Event){
    memset(TxBuffer,'\0',sizeof(TxBuffer));
    int length = 0;
    unsigned char Req_header[] = {0x1B,0x24,0x0D,0x03};

    switch(Event){
        case SC_C_GET_CCU_VERSIONINFO:
        {
            unsigned char sub_header[] = {0x02,0x0};
            unsigned char FnId [] = {(BYTE)Event,(BYTE)(Event >> 8)};
            unsigned char dontKnowWhy[] = {0x98};

            char* header_array[] = {Req_header,sub_header,FnId,dontKnowWhy};
            int header_length[] = {sizeof(Req_header),sizeof(sub_header),sizeof(FnId),sizeof(dontKnowWhy)};
            

            /*put cmd into TxBuffer*/
            for(int i = 0 ; i < sizeof(header_length)/sizeof(int) ; i++){
                memcat(TxBuffer,length,*(header_array+i),header_length[i]);
                length+=header_length[i];
            }

        break;
        }

    }

    /*Write To Driver*/
    DWORD byte;
    
    return WriteFile(Comm,TxBuffer,length,&byte,NULL);
}

