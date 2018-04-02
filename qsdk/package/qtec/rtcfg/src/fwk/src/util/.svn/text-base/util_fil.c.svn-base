#include "fwk.h"
#include "util_fil_oal.h"


UBOOL8 utilFil_isFilePresent(const char *filename)
{
   return (oalFil_isFilePresent(filename));
}


SINT32 utilFil_getSize(const char *filename)
{
   return (oalFil_getSize(filename));
}


VOS_RET_E utilFil_copyToBuffer(const char *filename, UINT8 *buf, UINT32 *bufSize)
{
   return (oalFil_copyToBuffer(filename, buf, bufSize));
}

VOS_RET_E utilFil_writeToProc(const char *procFilename, const char *s)
{
   return (oalFil_writeToProc(procFilename, s));
}

VOS_RET_E utilFil_writeBufferToFile(const char *filename, const UINT8 *buf,
                                UINT32 bufLen)
{
   return (oalFil_writeBufferToFile(filename, buf, bufLen));
}

VOS_RET_E utilFil_writeToFileEnd(const char *filename, const char *s)
{
    return (oalFil_writeToFileEnd(filename, s));
}

VOS_RET_E utilFil_removeDir(const char *dirname)
{
   return (oalFil_removeDir(dirname));
}

VOS_RET_E utilFil_makeDir(const char *dirname)
{
   return (oalFil_makeDir(dirname));
}

UINT32 utilFil_scaleSizetoKB(long nblks, long blkSize)
{

	return (nblks * (long long) blkSize + KILOBYTE/2 ) / KILOBYTE;

}
