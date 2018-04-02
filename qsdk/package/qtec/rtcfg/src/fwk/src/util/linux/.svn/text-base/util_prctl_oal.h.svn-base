/*
 * This header file contains all the functions exported by the
 * OS Adaptation Layer (OAL).  The OAL functions live under the
 * directory with the name of the OS, e.g. linux, ecos.
 * The Make system will automatically compile the appropriate files
 * and link them in with the final executable based on the TARGET_OS
 * variable, which is set by make menuconfig.
 */

#ifndef __UTIL_PRCTL_OAL_H__
#define __UTIL_PRCTL_OAL_H__

#include "fwk.h"


/* in oal_prctl.c */
extern VOS_RET_E oal_spawnProcess(const SpawnProcessInfo *spawnInfo, SpawnedProcessInfo *procInfo);
extern VOS_RET_E oal_collectProcess(const CollectProcessInfo *collectInfo, SpawnedProcessInfo *procInfo);
extern VOS_RET_E oal_terminateProcessGracefully(SINT32 pid);
extern VOS_RET_E oal_terminateProcessForcefully(SINT32 pid);
extern VOS_RET_E oal_signalProcess(SINT32 pid, SINT32 sig);
extern int oal_getPidByName(const char *name);


#endif /* __UTIL_PRCTL_OAL_H__ */
