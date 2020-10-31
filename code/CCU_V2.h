/*System Include*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <windows.h>

/*UCC Define*/
#define MKWORD(lb,hb)           (WORD)(((WORD)(hb))<<8) | (WORD)(lb)
#define DCNC_APP_SC 16
/*UCC Function Define*/
#define SC_C_GET_CCU_VERSIONINFO   MKWORD (6,DCNC_APP_SC)

/*Private Define */
#define EVFLAG     		EV_RXCHAR|EV_RXFLAG|EV_CTS|EV_DSR|EV_RLSD|EV_BREAK|EV_ERR|EV_RING
#define IOCLEAR    		PURGE_RXABORT|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_TXCLEAR
#define OVERLAPPED_FLAG FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED
/*HANDLE ERROR TYPE*/
#define SETTING_OK 		     0
#define CREATE_ERROR	    -1
#define THREAD_CR_ERROR     -2

char* 
memcat(char *dest, size_t dest_len, const char *src, size_t src_len);
/**
 *   To Solve The Problem of 0x0 inside the command
 * 
 *   Example
 * 		#define Limit = 5   //Supposed what you want the buffer is
 * 
 * 	  char a[Limit] = {0x01,0x0,0x13};
 *    char b[] = {0x11,0x15};
 * 	  memcat(a,sizeof(a),b,size(b));

 * 		a would be what you want;
 * 
 */

int Initial_Serial(int com);
/**
 * 	The Function to Initial Serial
 * 
 * 	Input : COM Number
 * 	OutPut : Error Number
 * 
 * 
 */

char* CCT(int COM_NUM);

/*
		Convert COM to TCHAR

		func: Convert int to TCHAR
*/

int UCC_Initial(HANDLE handle);
/**
 * 	Deal with UCC Initial e.g set get version, set config...
 * 
 * 
 * 
 */

bool Request(HANDLE handle ,WORD Event);

/**
 *	Write to UCC Command 
 * 	According to the Event you want to activate
 * 
 * 	HANDLE : the handle type object
 *  WORD   : the Request function,define above
 */

BOOL Version_Detect_Send(void);
/**
 *  Detect Version 
 * 
 * 
 */

int Read_Thread(void);
/**
 * Read Thread
 * 
 * 
 * 
 */