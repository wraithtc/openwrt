/*
 * This header file contains all the functions exported by the
 * OS Adaptation Layer (OAL).  The OAL functions live under the
 * directory with the name of the OS, e.g. linux, ecos.
 * The Make system will automatically compile the appropriate files
 * and link them in with the final executable based on the TARGET_OS
 * variable, which is set by make menuconfig.
 */

#ifndef __VOS_LOG_OAL_H__
#define __VOS_LOG_OAL_H__

#include "fwk.h"


extern void oalLog_init(void);
extern void oalLog_syslog(VosLogLevel level, const char *buf);
extern void oalLog_cleanup(void);


#endif /* __VOS_LOG_OAL_H__ */
