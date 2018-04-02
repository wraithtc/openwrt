#include <unistd.h>
#include <time.h>
#include "util_tms_oal.h"


/** OS dependent timestamp functions go in this file.
 */
void oalTms_get(UtilTimestamp *tms)
{
   struct timespec ts;
   SINT32 rc;

   if (tms == NULL)
   {
      return;
   }

   rc = clock_gettime(CLOCK_MONOTONIC, &ts);
   if (rc == 0)
   {
      tms->sec = ts.tv_sec;
      tms->nsec = ts.tv_nsec;
   }
   else
   {
      vosLog_error("clock_gettime failed, set timestamp to 0");
      tms->sec = 0;
      tms->nsec = 0;
   }
}


VOS_RET_E oalTms_getXSIDateTime(UINT32 t, char *buf, UINT32 bufLen)
{
	int          c;
   time_t       now;
	struct tm   *tmp;

   if (t == 0)
   {
      now = time(NULL);
   }
   else
   {
      now = t;
   }

	tmp = localtime(&now);
   memset(buf, 0, bufLen);
	c = strftime(buf, bufLen, "%Y-%m-%dT%H:%M:%S%z", tmp);
   if ((c == 0) || (c+1 > bufLen))
   {
      /* buf was not long enough */
      return VOS_RET_RESOURCE_EXCEEDED;
   }

	/* fix missing : in time-zone offset-- change -500 to -5:00 */
   buf[c+1] = '\0';
   buf[c] = buf[c-1];
   buf[c-1] = buf[c-2];
   buf[c-2]=':';

   return VOS_RET_SUCCESS;
}

