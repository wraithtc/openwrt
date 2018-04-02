
#include "QtBase.h"
#include "NetworkDetect.h"

#ifdef WIN32
#include <Wininet.h>
#include <Tapi.h>
#include <TCHAR.H>

/****Related Data Type Definitions: ****/
//Device setting information
typedef struct tagDEVCFGDR{
	DWORD dwSize;
	DWORD dwVersion;
	WORD fwOptions;
	WORD wWaitBong;
} DEVCFGHDR;

typedef struct  tagDEVCFG{
	DEVCFGHDR	dfgHdr;
	COMMCONFIG	commconfig;
}DEVCFG, *PDEVCFG, FAR* LPDEVCFG;  
/*********end***********/

BOOL  IsModemConnect();
QtResult GetRate(DWORD &dwRate);

QtResult QtCheckNetwork(BOOL& bModem, DWORD& dwSpeed)
{
	bModem = IsModemConnect();
	if(bModem)
		return GetRate(dwSpeed);
	return QT_OK;
}


QtResult GetRate(DWORD &dwRate)
{
	LPVARSTRING lpVarString = NULL;
	LPVOID lpDeviceConfig = NULL;
	DWORD dwSizeofDevConfig = 0;
	DWORD dwSizeofVarString = sizeof(VARSTRING)+1000;

	lpVarString = (LPVARSTRING) malloc(dwSizeofVarString);
	QT_ASSERTE(lpVarString);

	lpVarString->dwTotalSize = dwSizeofVarString;
	if(lineGetDevConfig(0, lpVarString,(LPTSTR)_T("comm/datamodem"))==0)
	{
		dwSizeofDevConfig = lpVarString->dwStringSize;
		lpDeviceConfig = (LPVOID) malloc(dwSizeofDevConfig+1);
		QT_ASSERTE(lpDeviceConfig);
		memcpy(lpDeviceConfig, ((LPBYTE)lpVarString+lpVarString->dwStringOffset), dwSizeofDevConfig);
		dwRate = ((LPDEVCFG)lpDeviceConfig)->commconfig.dcb.BaudRate;
		free(lpDeviceConfig);
		return QT_OK;
	}
	free(lpVarString);
	return QT_ERROR_FAILURE;
}

BOOL IsModemConnect()
{
	DWORD dwFlag = 0;
	InternetGetConnectedState(&dwFlag,0);
	if(dwFlag & INTERNET_CONNECTION_MODEM)
		return TRUE;
	return FALSE;
}
#else
QtResult QtCheckNetwork(BOOL& bModem, DWORD& dwSpeed)
{
	return QT_ERROR_NOT_IMPLEMENTED;
}
#endif

#ifdef __MODEM_TEST__
int main(int argc, char** argv)
{
	BOOL bModem = FALSE;
	DWORD dwSpeed = 0;
	QtCheckNetwork(bModem, dwSpeed);
	return 0;
}
#endif


