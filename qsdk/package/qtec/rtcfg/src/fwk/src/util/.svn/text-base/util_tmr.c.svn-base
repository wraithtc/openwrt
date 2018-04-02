#include "fwk.h"



/** Internal event timer structure
 */
typedef struct util_timer_event
{
   struct util_timer_event *next;      /**< pointer to the next timer. */
   UtilTimestamp            expireTms; /**< Timestamp (in the future) of when this
                                       *   timer event will expire. */
   UtilEventHandler         func;      /**< handler func to call when event expires. */
   void *                  ctxData;   /**< context data to pass to func */
   char name[UTIL_EVENT_TIMER_NAME_LENGTH]; /**< name of this timer */
} UtilTimerEvent;


/** Internal timer handle. */
typedef struct
{
   UtilTimerEvent *events;     /**< Singly linked list of events */
   UINT32         numEvents;  /**< Number of events in this handle. */
} UtilTimerHandle;


VOS_RET_E utilTmr_init(void **tmrHandle)
{

   (*tmrHandle) = VOS_MALLOC_FLAGS(sizeof(UtilTimerHandle), ALLOC_ZEROIZE);
   if ((*tmrHandle) == NULL)
   {
      vosLog_error("could not malloc mem for tmrHandle");
      return VOS_RET_RESOURCE_EXCEEDED;
   }

   return VOS_RET_SUCCESS;
}


void utilTmr_cleanup(void **handle)
{
   UtilTimerHandle *tmrHandle = (UtilTimerHandle *) handle;
   UtilTimerEvent *tmrEvent;

   while ((tmrEvent = tmrHandle->events) != NULL)
   {
      tmrHandle->events = tmrEvent->next;
      VOS_MEM_FREE_BUF_AND_NULL_PTR(tmrEvent);
   }

   VOS_MEM_FREE_BUF_AND_NULL_PTR((*handle));

   return;
}

/** This macro will evaluate TRUE if a is earlier than b */
#define IS_EARLIER_THAN(a, b) (((a)->sec < (b)->sec) || \
                               (((a)->sec == (b)->sec) && ((a)->nsec < (b)->nsec)))

VOS_RET_E utilTmr_set(void *handle, UtilEventHandler func, void *ctxData, UINT32 ms, const char *name)
{
   UtilTimerHandle *tmrHandle = (UtilTimerHandle *) handle;
   UtilTimerEvent *currEvent, *prevEvent, *newEvent;

   /*
    * First verify there is not a duplicate event.
    * (The original code first deleted any existing timer,
    * which is a "side-effect", bad style, but maybe tr69c requires
    * that functionality?)
    */
   if (utilTmr_isEventPresent(handle, func, ctxData))
   {
      vosLog_error("There is already an event func %p ctxData %p",
                   func, ctxData);
      return VOS_RET_INVALID_ARGUMENTS;
   }

   /* make sure name is not too long */
   if ((name != NULL) && (strlen(name) >= UTIL_EVENT_TIMER_NAME_LENGTH))
   {
      vosLog_error("name of timer event is too long, max %d", UTIL_EVENT_TIMER_NAME_LENGTH);
      return VOS_RET_INVALID_ARGUMENTS;
   }


   /*
    * Allocate a structure for the timer event.
    */
   newEvent = VOS_MALLOC_FLAGS(sizeof(UtilTimerEvent), ALLOC_ZEROIZE);
   if (newEvent == NULL)
   {
      vosLog_error("malloc of new timer event failed");
      return VOS_RET_RESOURCE_EXCEEDED;
   }

   /* fill in fields of new event timer structure. */
   newEvent->func = func;
   newEvent->ctxData = ctxData;

   utilTms_get(&(newEvent->expireTms));
   utilTms_addMilliSeconds(&(newEvent->expireTms), ms);

   if (name != NULL)
   {
      UTIL_SNPRINTF(newEvent->name, sizeof(newEvent->name), "%s", name);
   }


   /* 
    * Now we just need to insert it in the correct place in the timer handle.
    * We just insert the events in absolute order, i.e. smallest expire timer
    * at the head of the queue, largest at the end of the queue.  If the
    * modem is up long enough where timestamp rollover is an issue (139 years!)
    * utilTmr_executeExpiredEvents and utilTmr_getTimeToNextEvent will have to
    * be careful about where they pick the next timer to expire.
    */
   if (tmrHandle->numEvents == 0)
   {
      tmrHandle->events = newEvent;
   }
   else 
   {
      currEvent = tmrHandle->events;

      if (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms)))
      {
         /* queue at the head */
         newEvent->next = currEvent;
         tmrHandle->events = newEvent;
      }
      else
      {
         UBOOL8 done = FALSE;

         while (!done)
         {
            prevEvent = currEvent;
            currEvent = currEvent->next;

            if ((currEvent == NULL) ||
                (IS_EARLIER_THAN(&(newEvent->expireTms), &(currEvent->expireTms))))
            {
               newEvent->next = prevEvent->next;
               prevEvent->next = newEvent;
               done = TRUE;
            }
         }
      }
   }

   tmrHandle->numEvents++;

   vosLog_debug("added event %s, expires in %ums (at %u.%03u), func=%p data=%p count=%d",
                newEvent->name,
                ms,
                newEvent->expireTms.sec,
                newEvent->expireTms.nsec/NSECS_IN_MSEC,
                func,
                ctxData,
                tmrHandle->numEvents);

   return VOS_RET_SUCCESS;
}  


void utilTmr_cancel(void *handle, UtilEventHandler func, void *ctxData)
{
   UtilTimerHandle *tmrHandle = (UtilTimerHandle *) handle;
   UtilTimerEvent *currEvent, *prevEvent;

   if ((currEvent = tmrHandle->events) == NULL)
   {
      vosLog_debug("no events to delete (func=%p data=%p)", func, ctxData);
      return;
   }

   if (currEvent->func == func && currEvent->ctxData == ctxData)
   {
      /* delete from head of the queue */
      tmrHandle->events = currEvent->next;
      currEvent->next = NULL;
   }
   else
   {
      UBOOL8 done = FALSE;

      while ((currEvent != NULL) && (!done))
      {
         prevEvent = currEvent;
         currEvent = currEvent->next;

         if (currEvent != NULL && currEvent->func == func && currEvent->ctxData == ctxData)
         {
            prevEvent->next = currEvent->next;
            currEvent->next = NULL;
            done = TRUE;
         }
      }
   }

   if (currEvent != NULL)
   {
      tmrHandle->numEvents--;

      vosLog_debug("canceled event %s, count=%d", currEvent->name, tmrHandle->numEvents);

      VOS_MEM_FREE_BUF_AND_NULL_PTR(currEvent);
   }
   else
   {
      vosLog_debug("could not find requested event to delete, func=%p data=%p count=%d",
                   func, ctxData, tmrHandle->numEvents);
   }

   return;
}


VOS_RET_E utilTmr_getTimeToNextEvent(const void *handle, UINT32 *ms)
{
   UtilTimerHandle *tmrHandle = (UtilTimerHandle *) handle;
   UtilTimerEvent *currEvent;
   UtilTimestamp nowTms;

   utilTms_get(&nowTms);
   currEvent = tmrHandle->events;

   if (currEvent == NULL)
   {
      *ms = MAX_UINT32;
      return VOS_RET_NO_MORE_INSTANCES;
   }

   /* this is the same code as in dumpEvents, integrate? */
   if (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms))
   {
      /*
       * the next event is past due (nowTms is later than currEvent),
       * so time to next event is 0.
       */
      *ms = 0;
   }
   else
   {
      /*
       * nowTms is earlier than currEvent, so currEvent is still in
       * the future.  
       */
      (*ms) = utilTms_deltaInMilliSeconds(&(currEvent->expireTms), &nowTms);
   }

   return VOS_RET_SUCCESS;
}


UINT32 utilTmr_getNumberOfEvents(const void *handle)
{
   const UtilTimerHandle *tmrHandle = (const UtilTimerHandle *) handle;

   return (tmrHandle->numEvents);
}


void utilTmr_executeExpiredEvents(void *handle)
{
   UtilTimerHandle *tmrHandle = (UtilTimerHandle *) handle;
   UtilTimerEvent *currEvent;
   UtilTimestamp nowTms;

   utilTms_get(&nowTms);
   currEvent = tmrHandle->events;

   while ((currEvent != NULL) && (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms)))
   {
      /*
       * first remove the currEvent from the tmrHandle because
       * when we execute the callback function, it might call the
       * utilTmr API again.
       */
      tmrHandle->events = currEvent->next;
      currEvent->next = NULL;
      tmrHandle->numEvents--;

      vosLog_debug("executing timer event %s func %p",
                   currEvent->name, currEvent->func);

      /* call the function */
      (*currEvent->func)(currEvent->ctxData);

      /* free the event struct */
      VOS_FREE(currEvent);

      currEvent = tmrHandle->events;
   }

   return;
}


VOS_RET_E utilTmr_getTimeRemaining(const void *handle, UtilEventHandler func, void *ctxData, UINT32 *ms)
{
   const UtilTimerHandle *tmrHandle = (const UtilTimerHandle *)handle;
   UtilTimerEvent *tmrEvent;
   UtilTimestamp   nowTms;
   UINT32 msRem = 0;
   VOS_RET_E ret = VOS_RET_SUCCESS;

   tmrEvent = tmrHandle->events;
   while (tmrEvent != NULL)
   {
      if ((tmrEvent->func == func) && (tmrEvent->ctxData == ctxData))
      {
          ret = VOS_RET_SUCCESS;
          utilTms_get(&nowTms);
          if ( IS_EARLIER_THAN(&nowTms, &tmrEvent->expireTms) )
          {
              msRem = utilTms_deltaInMilliSeconds(&tmrEvent->expireTms, &nowTms);
          }
          else
          {
              msRem = 0;
          }
          break;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }

   *ms = msRem;

   return ret;
}



UBOOL8 utilTmr_isEventPresent(const void *handle, UtilEventHandler func, void *ctxData)
{
   const UtilTimerHandle *tmrHandle = (const UtilTimerHandle *) handle;
   UtilTimerEvent *tmrEvent;
   UBOOL8 found=FALSE;

   tmrEvent = tmrHandle->events;

   while ((tmrEvent != NULL) && (!found))
   {
      if (tmrEvent->func == func && tmrEvent->ctxData == ctxData)
      {
         found = TRUE;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }

   return found;
}

void utilTmr_dumpEvents(const void *handle)
{
   const UtilTimerHandle *tmrHandle = (const UtilTimerHandle *) handle;
   UtilTimerEvent *currEvent;
   UtilTimestamp nowTms;
   UINT32 expires;

   vosLog_debug("dumping %d events", tmrHandle->numEvents);
   utilTms_get(&nowTms);

   currEvent = tmrHandle->events;

   while (currEvent != NULL)
   {

      /* this is the same code as in getTimeToNextEvent, integrate? */
      if (IS_EARLIER_THAN(&(currEvent->expireTms), &nowTms))
      {
         /*
          * the currentevent is past due (nowTms is later than currEvent),
          * so expiry time is 0.
          */
         expires = 0;
      }
      else
      {
         /*
          * nowTms is earlier than currEvent, so currEvent is still in
          * the future.  
          */
         expires = utilTms_deltaInMilliSeconds(&(currEvent->expireTms), &nowTms);
      }


      vosLog_debug("event %s expires in %ums (at %u.%03u) func=%p data=%p",
                   currEvent->name,
                   expires,
                   currEvent->expireTms.sec,
                   currEvent->expireTms.nsec/NSECS_IN_MSEC,
                   currEvent->func,
                   currEvent->ctxData);

      currEvent = currEvent->next;
   }

   return;
}


VOS_RET_E utilTmr_replaceIfSooner(void *handle, UtilEventHandler func, void *ctxData, UINT32 ms, const char *name)
{
   UtilTimestamp nowTms;
   const UtilTimerHandle *tmrHandle = (const UtilTimerHandle *) handle;
   UtilTimerEvent *tmrEvent;
   UBOOL8 found=FALSE;

   tmrEvent = tmrHandle->events;

   while ((tmrEvent != NULL) && (!found))
   {
      if (tmrEvent->func == func && tmrEvent->ctxData == ctxData)
      {
         found = TRUE;
      }
      else
      {
         tmrEvent = tmrEvent->next;
      }
   }
   if (found && tmrEvent)
   {
      /* find out the expire time of this event.  If it's sooner then the one in the 
       * timer list, then replace the one in list with this one.
       */
      utilTms_get(&nowTms);
      utilTms_addMilliSeconds(&nowTms, ms);
      if (IS_EARLIER_THAN(&nowTms, &(tmrEvent->expireTms)))
      {
         utilTmr_cancel((void*)tmrHandle, func, (void*)NULL);
      }
      else
      {
         return VOS_RET_SUCCESS;
      }
   } /* found */
   return(utilTmr_set(handle, func, ctxData, ms, name));
}


//

UBOOL8 utilMsgTmr_isEventPresent(const void *handle, UtilEventHandler func, void *ctxData)
{
    return utilTmr_isEventPresent(handle, func, ctxData);
}


VOS_RET_E utilMsgTmr_set(void *msgHandle, void *handle, UtilEventHandler func, void *ctxData, UINT32 ms, const char *name)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    RegisterDelayedMsgBody delayTime;
    
    ret=  utilTmr_set(handle, func, ctxData, ms, name);
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("Fail to set timer !");
        return ret;
    }
    
    //send VOS_MSG_REGISTER_DELAYED_MSG to smd
    delayTime.delayMs = ms;
    vosLog_debug("send VOS_MSG_REGISTER_DELAYED_MSG to smd");
    ret = UTIL_sendRequestToSmd(msgHandle, VOS_MSG_REGISTER_DELAYED_MSG, (UINT32)ctxData, (void *)&delayTime, sizeof(RegisterDelayedMsgBody));
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("Fail to send VOS_MSG_REGISTER_DELAYED_MSG(ret=%d)", ret);
    }

    return ret;
}


void utilMsgTmr_cancel(void *msgHandle, void *handle, UtilEventHandler func, void *ctxData)
{
    VOS_RET_E ret = VOS_RET_SUCCESS;
    
    utilTmr_cancel(handle, func, ctxData);

    //send VOS_MSG_REGISTER_DELAYED_MSG to smd
    vosLog_debug("send VOS_MSG_UNREGISTER_DELAYED_MSG to smd");
    ret = UTIL_sendRequestToSmd(msgHandle, VOS_MSG_UNREGISTER_DELAYED_MSG, (UINT32)ctxData, NULL, 0);
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("Fail to send VOS_MSG_REGISTER_DELAYED_MSG");
    }
}


void utilMsgTmr_executeExpiredEvents(void *handle)
{
    return utilTmr_executeExpiredEvents(handle);
}

