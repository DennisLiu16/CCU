/*OverLapped Reference
    https://blog.csdn.net/wowocpp/article/details/80609894
*/

/*Private Include*/
#include "CCU.h"
/*Private Define*/
#define DEFAULT_COM 5

/*Global Var*/
unsigned char TxBuffer[32];
unsigned char RxBuffer[256];
/*Handle*/
HANDLE hComm;
HANDLE hThreadEvent;
/*Overlapped Vars*/
OVERLAPPED Eol={0};
OVERLAPPED Wol={0};
OVERLAPPED Rol={0};

DWORD dwThreadID;
bool bEventRun;
bool fStopMsg;

int Machine_State;


/*Main Function*/
int main(void){

    /*Variable*/
    int State;
    /*Initial Serial*/
    State = Initial_Serial(DEFAULT_COM);
    if(SETTING_OK == State){
        /*Initial Succeeded*/
        /*Wait Thread Running*/
        
        if(UCC_Initial(hComm)){
            
           
        
        }
        else
        {
            /* failed */
        }
        
        

    }
    else if(State > 0){
        CloseHandle(hComm);
    }
system("PAUSE");
CloseHandle(hThreadEvent);
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
    hComm = CreateFile( com_num,                          // COM number              
                        GENERIC_READ | GENERIC_WRITE,     // Read & Write
                        0,                                // No Sharing
                        NULL ,                            // None Security
                        OPEN_EXISTING,                    // Open existing port only
                        OVERLAPPED_FLAG,                  // Overlapped I/O
                        NULL   );                         // Null for Comm Devices
    
    if(hComm == INVALID_HANDLE_VALUE){
        printf("COM%d Initial Failed,Please Confirm Port Number\r\n",com);
        return CREATE_ERROR;
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

        Ok = SetCommState(hComm,&hcomPara);
        if(!Ok) {
            printf("Set Gerneral Part Failed\r\n");
            return Ok;
        }

        Ok = SetupComm(hComm,Insize,Outsize);
        

        if(!Ok) {
            printf("Set Queue Part Failed\r\n");
            return Ok;
        }


        Ok = SetCommTimeouts(hComm,&timeouts);
        if(!Ok) {
            printf("Set TimeOut Part Failed\r\n");
            return Ok;
        }


        Ok = SetCommMask(hComm,EVFLAG);
        if(!Ok) {
            printf("Set Event Part Failed\r\n");
            return Ok;
        }

        // DWORD mask;
        // GetCommMask(Comm,&mask);
        // printf("%x\r\n",mask);

        PurgeComm(hComm,IOCLEAR);

        printf("All Setting Done!\r\n");

        DWORD dwParam;

        hThreadEvent = CreateThread(NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE) ThreadProcEvent,
                                    &dwParam,
                                    0,
                                    &dwThreadID
                        );

        if(hThreadEvent == INVALID_HANDLE_VALUE){
            printf("Wait_Thread Create Failed\r\n");
            return THREAD_CR_ERROR;
        }
        
        bEventRun = TRUE;
        Machine_State = Initial_OK;
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
    /*§ï¼g*/
    DWORD byte;
    if(!Request(handle,SC_C_GET_CCU_VERSIONINFO)){
        printf("Please Check AccessPort ,Req : SC_C_GET_CCU_VERSIONINFO\r\n");
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
    char test [] = {0x34,0x35,0x36,0x37,0x38,0x39};
    //return WriteFile(hComm,test,strlen(test),&byte,NULL);
    return WriteFile(hComm,TxBuffer,length,&byte,&Wol);
}

/*OverLapped RX CallBack*/
LONG OnReceiveEvent(void){

    DWORD dwRes;
    DWORD dwRead;
    DWORD dwErrors;
    COMSTAT Rcs;
    fStopMsg = true;

    BOOL bResult ;
    memset(RxBuffer,0,sizeof(RxBuffer));

    ClearCommError( hComm,
                    &dwErrors,
                    &Rcs );

    bResult = ReadFile(hComm,RxBuffer,Rcs.cbInQue,&dwRead,&Rol);

    /*Determine Leave or Wait*/
    if(bResult){
        /*Success*/
        printf("Direct Success Rx\r\n");
    }
    else{
        if(GetLastError() == ERROR_IO_PENDING){
            printf("Before Get overlapped result\r\n");
            GetOverlappedResult(hComm,&Rol,&dwRead,TRUE);
        }

        
/*        
        Rol.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
        dwRes = WaitForSingleObject(Rol.hEvent,WAIT_RX);

     
        switch(dwRes){
            case WAIT_OBJECT_0:
                bResult = GetOverlappedResult(hComm,&Rol,&dwRead,TRUE);

                if(!bResult)
                    printf("WaitCom For Read failed\r\n");

                else{
                    printf("WaitCom For Read success dwRead=%d,Rcs.cbInQue=%d",dwRead,Rcs.cbInQue);
                    printf("Data:\r\n");
                    for(int i = 0;i < Rcs.cbInQue;i++)
                        printf("%x ",RxBuffer[i]);
                    printf("\r\n"); 
                }
                //Clean Buffer
                memset(RxBuffer,0,sizeof(RxBuffer));
                break;

            case WAIT_TIMEOUT:
                printf("WaitCom TimeOut\r\n");
                break;

        }
        SetEvent(Rol.hEvent)
*/
    }
    printf("Receive Direct Success,Data:\r\n");
        for(int i = 0;i < Rcs.cbInQue;i++)
            printf("%x ",RxBuffer[i]);
        printf("\r\n");
        printf("\r\n");
        fStopMsg = false;
        return 0;

}

/*Event Detecter - SubThread*/
DWORD ThreadProcEvent(LPVOID pParam){

    DWORD dwEvtMask;
    DWORD dwRes;
    Wol.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    Eol.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

    while(bEventRun){
        /*Req Part*/
        WaitCommEvent(hComm,&dwEvtMask,&Wol);
        if(GetLastError()== ERROR_IO_PENDING){
            DWORD dwWrite;
            //GetOverlappedResult(hComm,&Wol,&dwWrite,TRUE);
            if(dwEvtMask&EV_TXEMPTY == EV_TXEMPTY){
                printf("Send Finished\r\n");
            }
        }



        WaitCommEvent(hComm,&dwEvtMask,&Eol);
        if(GetLastError()== ERROR_IO_PENDING){

            //dwRes = WaitForSingleObject(Eol.hEvent,INFINITE);
            DWORD dwRead;
            GetOverlappedResult(hComm,&Eol,&dwRead,TRUE);
            if(dwEvtMask&EV_RXFLAG == EV_RXFLAG){
                printf("dwEvtMask:%x\r\n",dwEvtMask);
                printf("In EV_RXFLAG\r\n");
                OnReceiveEvent();
            }
            /*
            switch(dwRes){
            
                case WAIT_OBJECT_0:
                {
                    if(dwEvtMask&EV_RXFLAG == EV_RXFLAG){
                        printf("dwEvtMask:%x\r\n",dwEvtMask);
                        printf("In EV_RXFLAG\r\n");
                        OnReceiveEvent();
                    }

                    else if(dwEvtMask&EV_RXCHAR == EV_RXCHAR){
                        OnReceiveEvent();
                    }

                    break;
                }
                //TimeOut

            }
            */
        }
        else{
            printf("Get Something Error:%d\r\n",GetLastError());
        }
        
    }

    return true;


}


