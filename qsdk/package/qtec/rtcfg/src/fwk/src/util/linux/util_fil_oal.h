/*
 * This header file contains all the functions exported by the
 * OS Adaptation Layer (OAL).  The OAL functions live under the
 * directory with the name of the OS, e.g. linux, ecos.
 * The Make system will automatically compile the appropriate files
 * and link them in with the final executable based on the TARGET_OS
 * variable, which is set by make menuconfig.
 */

#ifndef __UTIL_FIL_OAL_H__
#define __UTIL_FIL_OAL_H__

#include "fwk.h"


extern UBOOL8 oalFil_isFilePresent(const char *filename);
extern SINT32 oalFil_getSize(const char *filename);
extern VOS_RET_E oalFil_copyToBuffer(const char *filename, UINT8 *buf, UINT32 *bufSize);
extern VOS_RET_E oalFil_writeToProc(const char *procFilename, const char *s);
extern VOS_RET_E oalFil_writeBufferToFile(const char *filename, const UINT8 *buf,
                                       UINT32 bufLen);
extern VOS_RET_E oalFil_writeToFileEnd(const char *filename, const char *s);                                       
extern VOS_RET_E oalFil_removeDir(const char *dirname);
extern VOS_RET_E oalFil_makeDir(const char *dirname);

#endif /* __UTIL_FIL_OAL_H__ */
