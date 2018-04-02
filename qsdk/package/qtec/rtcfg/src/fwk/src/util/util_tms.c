#include "fwk.h"
#include "util_tms_oal.h"


void utilTms_get(UtilTimestamp *tms)
{
   oalTms_get(tms);
}

void utilTms_delta(const UtilTimestamp *newTms,
                  const UtilTimestamp *oldTms,
                  UtilTimestamp *deltaTms)
{
   if (newTms->sec >= oldTms->sec)
   {
      if (newTms->nsec >= oldTms->nsec)
      {
         /* no roll-over in the sec and nsec fields, straight subtract */
         deltaTms->nsec = newTms->nsec - oldTms->nsec;
         deltaTms->sec = newTms->sec - oldTms->sec;
      }
      else
      {
         /* no roll-over in the sec field, but roll-over in nsec field */
         deltaTms->nsec = (NSECS_IN_SEC - oldTms->nsec) + newTms->nsec;
         deltaTms->sec = newTms->sec - oldTms->sec - 1;
      }
   }
   else
   {
      if (newTms->nsec >= oldTms->nsec)
      {
         /* roll-over in the sec field, but no roll-over in the nsec field */
         deltaTms->nsec = newTms->nsec - oldTms->nsec;
         deltaTms->sec = (MAX_UINT32 - oldTms->sec) + newTms->sec + 1; /* +1 to account for time spent during 0 sec */
      }
      else
      {
         /* roll-over in the sec and nsec fields */
         deltaTms->nsec = (NSECS_IN_SEC - oldTms->nsec) + newTms->nsec;
         deltaTms->sec = (MAX_UINT32 - oldTms->sec) + newTms->sec;
      }
   }
}

UINT32 utilTms_deltaInMilliSeconds(const UtilTimestamp *newTms,
                                  const UtilTimestamp *oldTms)
{
   UtilTimestamp deltaTms;
   UINT32 ms;

   utilTms_delta(newTms, oldTms, &deltaTms);

   if (deltaTms.sec > MAX_UINT32 / MSECS_IN_SEC)
   {
      /* the delta seconds is larger than the UINT32 return value, so return max value */
      ms = MAX_UINT32;
   }
   else
   {
      ms = deltaTms.sec * MSECS_IN_SEC;

      if ((MAX_UINT32 - ms) < (deltaTms.nsec / NSECS_IN_MSEC))
      {
         /* overflow will occur when adding the nsec, return max value */
         ms = MAX_UINT32;
      }
      else
      {
         ms += deltaTms.nsec / NSECS_IN_MSEC;
      }
   }

   return ms;
}


void utilTms_addMilliSeconds(UtilTimestamp *tms, UINT32 ms)
{
   UINT32 addSeconds;
   UINT32 addNano;

   addSeconds = ms / MSECS_IN_SEC;
   addNano = (ms % MSECS_IN_SEC) * NSECS_IN_MSEC;

   tms->sec += addSeconds;
   tms->nsec += addNano;

   /* check for carry-over in nsec field */
   if (tms->nsec > NSECS_IN_SEC)
   {
      /* we can't have carried over by more than 1 second */
      tms->sec++;
      tms->nsec -= NSECS_IN_SEC;
   }

   return;
}


VOS_RET_E utilTms_getXSIDateTime(UINT32 t, char *buf, UINT32 bufLen)
{
   return (oalTms_getXSIDateTime(t, buf, bufLen));
}

VOS_RET_E utilTms_getDaysHoursMinutesSeconds(UINT32 t, char *buf, UINT32 bufLen)
{
    UINT32 days, hours, minutes, seconds;
    SINT32 r;
    VOS_RET_E ret=VOS_RET_SUCCESS;

    days = t / SECS_IN_DAY;
    t -= (days * SECS_IN_DAY);

    hours = t / SECS_IN_HOUR;
    t -= (hours * SECS_IN_HOUR);

    minutes = t / SECS_IN_MINUTE;
    t -= (minutes * SECS_IN_MINUTE);

    seconds = t;

    memset(buf, 0, bufLen);
    r = snprintf(buf, bufLen-1, "%dD %dH %dM %dS", days, hours, minutes, seconds);
    if (r >= bufLen)
    {
        ret = VOS_RET_RESOURCE_EXCEEDED;
    }

    return ret;
}

