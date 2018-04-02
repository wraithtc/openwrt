#ifndef __UTIL_SHELL_H__
#define __UTIL_SHELL_H__


#undef system
#define system  UTIL_DO_SYSTEM_ACTION
#define UTIL_DO_SYSTEM_ACTION(fmt, ...)    UTIL_doSystemAction(__FUNCTION__, fmt, ##__VA_ARGS__)
#define UTIL_COMM_SYSTEM_EX(pszCmd, pszResult, iCount)	UTIL_commSystemEx(pszCmd, pszResult, iCount)

int UTIL_doSystemAction(const char *from, const char *fmt, ...);
int UTIL_commSystemEx(const char *pszCmd, char * pszResult, int iCount);


#endif /* __UTIL_SHELL_H__*/
