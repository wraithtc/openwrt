/*
 * This header file contains all the functions exported by the
 * OS Adaptation Layer (OAL).  The OAL functions live under the
 * directory with the name of the OS, e.g. linux, ecos.
 * The Make system will automatically compile the appropriate files
 * and link them in with the final executable based on the TARGET_OS
 * variable, which is set by make menuconfig.
 */

#ifndef __UTIL_STRCONV_OAL_H__
#define __UTIL_STRCONV_OAL_H__

#include "fwk.h"


/* in oal_strconv.c */
VOS_RET_E oal_strtol(const char *str, char **endptr, SINT32 base, SINT32 *val);
VOS_RET_E oal_strtoul(const char *str, char **endptr, SINT32 base, UINT32 *val);
VOS_RET_E oal_strtol64(const char *str, char **endptr, SINT32 base, SINT64 *val);
VOS_RET_E oal_strtoul64(const char *str, char **endptr, SINT32 base, UINT64 *val);


#endif /* __UTIL_STRCONV_OAL_H__ */
