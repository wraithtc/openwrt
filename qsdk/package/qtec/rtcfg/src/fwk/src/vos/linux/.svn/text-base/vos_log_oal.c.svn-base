#include "vos_log_oal.h"

/** OS dependent logging functions go in this file.
 */
void oalLog_init(void)
{
   openlog(NULL, 0, LOG_DAEMON);
   return;
}

void oalLog_syslog(VosLogLevel level, const char *buf)
{
   syslog(level, buf);
   return;
}

void oalLog_cleanup(void)
{
   closelog();
   return;
}
