/*
 * This header file contains all the functions exported by the
 * OS Adaptation Layer (OAL).  The OAL functions live under the
 * directory with the name of the OS, e.g. linux, ecos.
 * The Make system will automatically compile the appropriate files
 * and link them in with the final executable based on the TARGET_OS
 * variable, which is set by make menuconfig.
 */

#ifndef __UTIL_IFADDR6_OAL_H__
#define __UTIL_IFADDR6_OAL_H__

#include "fwk.h"


/* in util_ifAddr6_oal.c */
extern VOS_RET_E oal_getIfAddr6(const char *ifname, UINT32 addrIdx,
                      char *ipAddr, UINT32 *ifIndex, UINT32 *prefixLen, UINT32 *scope, UINT32 *ifaFlags);


#endif /* __UTIL_IFADDR6_OAL_H__ */

