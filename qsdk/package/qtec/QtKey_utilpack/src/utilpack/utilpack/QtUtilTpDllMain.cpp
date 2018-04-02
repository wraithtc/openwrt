 
#include "QtBase.h"

#if !defined(WIN32) || !defined(_USRDLL)
 #error ERROR: This file is only for Win32 DLL!
#endif // !_USRDLL

/*
HANDLE g_hInstDll;
*/
BOOL g_bRunTimeLoad = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	g_hInstDll = hinstDLL;

	switch(fdwReason)
    {
	case DLL_PROCESS_ATTACH :
		if(lpReserved && *(static_cast< DWORD* >(lpReserved)) == 0) //is run time load
			g_bRunTimeLoad = TRUE;
		DisableThreadLibraryCalls(hinstDLL);
		QT_INFO_TRACE("DllMain DLL_PROCESS_ATTACH, handle = " << hinstDLL << " reserved = " << lpReserved << " reserved value = " << (lpReserved ? *(static_cast< DWORD* >(lpReserved)) : 0));
		break;

	case DLL_THREAD_ATTACH :
		QT_INFO_TRACE("DllMain DLL_THREAD_ATTACH , handle = " << hinstDLL << " reserved = " << lpReserved);
		break;

	case DLL_THREAD_DETACH :
		QT_INFO_TRACE("DllMain DLL_THREAD_DETACH , handle = " << hinstDLL << " reserved = " << lpReserved);
		break;

	case DLL_PROCESS_DETACH :
		QT_INFO_TRACE("DllMain DLL_PROCESS_DETACH disable, handle = " << hinstDLL << " reserved = " << lpReserved);
		break;

	default :
		QT_ASSERTE(FALSE);
		break;
	}
    return TRUE;
}
