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

/*Time Delay Set*/
#define WAIT_RX 1000
#define WAIT_EVERY 100

enum Machine{
	Initial_OK = 1


};




/*Reference
	Ref : https://www.xanthium.in/Serial-Port-Programming-using-Win32-API
	Ev-Mask : https://blog.csdn.net/targusyoona/article/details/9624203
	All Functions: https://nosleep.pixnet.net/blog/post/83847684-%E7%A8%8B%E5%BC%8F%E9%96%8B%E7%99%BC-%7C-kernel32-%E2%80%8Bcomm.c
	Tutorial Source Multiple Thread :https://www.itdaan.com/tw/2de00f652c01c938d5f9aaabe4cb246c
	
	OverLap:https://ffyy99.pixnet.net/blog/post/28789992

	Tutorial:https://www.itread01.com/p/872935.html

*/


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

LONG OnReceiveEvent(void);

/**
 * Rx WaitEvent callback
 * Do ReadFile Fucntion 
 * 
 * 
 */

DWORD ThreadProcEvent(LPVOID pParam);
/**
 * Thread - Create for waitEvent
 * 
 * 
 * 
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


