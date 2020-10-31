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

DWORD dwThreadID;
bool bEventRun;
bool fStopRead = 0;

int main(void){
    Initial_Serial(DEFAULT_COM);

    Version_Detect_Send();

    system("PAUSE");
    fStopRead = 1;
    CloseHandle(hThreadEvent);

    CloseHandle(hComm);
    return 0;



}

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
                                    (LPTHREAD_START_ROUTINE) Read_Thread,
                                    &dwParam,
                                    0,
                                    &dwThreadID
                        );

        if(hThreadEvent == INVALID_HANDLE_VALUE){
            printf("Wait_Thread Create Failed\r\n");
            return THREAD_CR_ERROR;
        }

        
        bEventRun = TRUE;

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

BOOL Version_Detect_Send(void){

    printf("WriteFile Start\r\n");
    DWORD byte;
    char cmd [] ={0x1B,0x24,0x0D,0x03,0x02,0x0,0x06,0x10,0x98};
    char test[] = {0x33,0x34,0x35,0x1A,0x0D,0x0A};

    OVERLAPPED olWrite;
    olWrite.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);


    if(WriteFile(hComm,cmd,sizeof(cmd),&byte,&olWrite)==FALSE){
        if(GetLastError() != ERROR_IO_PENDING){
            printf("Write File Error \r\n");
        }

        else{
            if(GetOverlappedResult(hComm,&olWrite,&byte,TRUE) == FALSE){
                // do smthing
            }

        }
    }

    return true;
}

int Read_Thread(void){
    
    
    
    DWORD byte;
    DWORD MASK;
    
    OVERLAPPED olWaite;
    OVERLAPPED olRead;
    
    
    while(bEventRun && !fStopRead){

        memset(&olWaite,0,sizeof(olWaite)); 
        olWaite.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL); 
        WaitCommEvent(hComm,&MASK,&olWaite);

        if(GetOverlappedResult(hComm,&olWaite,&byte,TRUE)==FALSE){
            if(GetLastError()!=ERROR_IO_PENDING){
                printf("Wait Event Wrong \r\n");
            }
            else{
                DWORD dwErrors;
                COMSTAT Rcs;
                memset(&Rcs,0,sizeof(Rcs));
                ClearCommError(hComm,&dwErrors,&Rcs);
            }
        }

        memset(&olRead,0,sizeof(olRead));

        olRead.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
        DWORD dwRead;

        if(ReadFile(hComm,RxBuffer,sizeof(RxBuffer),&dwRead,&olRead) ==FALSE){
            
            if(GetLastError() != ERROR_IO_PENDING){
                printf("Read Event Wrong\r\n");
            }
            else{
                if(GetOverlappedResult(hComm,&olRead,&dwRead,TRUE) == FALSE){
                    printf("Hello\r\n");
                }
                else if(dwRead == 0){
                    printf("Nothing To Read \r\n");
                }
                else{
                    printf("byte:%d \r\n\r\n",byte);
                    printf("Read Data: ");
                    for(int i =0;i<dwRead;i++)printf("0x%02X ",RxBuffer[i]);
                    printf("\r\n");
                }
                
            }
        }


        
    }
    printf("Leave\r\n");
    return 0;

}
