/*
 * This header file contains all the functions exported by the
 * OS Adaptation Layer (OAL).  The OAL functions live under the
 * directory with the name of the OS, e.g. linux, ecos.
 * The Make system will automatically compile the appropriate files
 * and link them in with the final executable based on the TARGET_OS
 * variable, which is set by make menuconfig.
 */

#ifndef __UTIL_TMS_OAL_H__
#define __UTIL_TMS_OAL_H__

#include "fwk.h"


/* in oal_timestamp.c */
void oalTms_get(UtilTimestamp *tms);
VOS_RET_E oalTms_getXSIDateTime(UINT32 t, char *buf, UINT32 bufLen);


#endif /* __UTIL_TMS_OAL_H__ */
