#include <stdio.h>
#include <stdarg.h>
#include "fwk.h"


int UTIL_doSystemAction(const char *from, const char *fmt, ...)
{
    int ret = 0;
    int len = 0;
    char buf[256] = {0};
    char *cmd = NULL;
    char *allocBuf = NULL;
    va_list paraList;

    va_start(paraList, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, paraList);
    va_end(paraList);

    if (len < sizeof(buf))
    {
        cmd = buf;
    }
    else
    {
        len++;
        allocBuf = (char *)VOS_MALLOC_FLAGS(len, ALLOC_ZEROIZE);
        va_start(paraList, fmt);
        vsnprintf(allocBuf, len, fmt, paraList);
        va_end(paraList);

        cmd = allocBuf;
    }

    vosLog_notice("%s -- %s", from, cmd);

#ifndef DESKTOP_LINUX
#undef system
    ret = system(cmd);
#endif

    VOS_FREE(allocBuf);
    return ret;
}

/************************************************************
Function:      int UTIL_commSystemEx(const char *pszCmd,char * pszResult,int iCount)
Description:   获取shell命令输出信息
Input:         const char *pszCmd,              要执行的命令
                   char * pszResult,                    执行的输出结果
                   int iCount                                当前buffer的长度
Output:
Return:        false ,       失败;
                    true,     成功;
Others:		LiZhimin@20130520.
************************************************************/
int UTIL_commSystemEx(const char *pszCmd,char * pszResult,int iCount)
{

    FILE   *stream;

    //memset( pszResult, '\0', sizeof(pszResult) );
    memset( pszResult, '\0', iCount );
    stream = popen( pszCmd, "r" );
    if (!stream)
    {
        pclose( stream );
        return 0;
    }

    iCount = fread( pszResult, sizeof(char), iCount, stream);
    if (!(iCount))
    {
        pclose( stream );
        return 0;
    }

    pclose( stream );
    return 1;
}



