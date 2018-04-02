#include "fwk.h"
#include "vos_msg_oal.h"


#ifdef DESKTOP_LINUX
UINT16 desktopFakePid = 30;
#endif


#define MSG_WAIT_TIMEOUT    (2 * SECS_IN_MINUTE * MSECS_IN_SEC)


/* message API functions go here */

VOS_RET_E vosMsg_init(VosEntityId eid, void **msgHandle)
{
   return oalVosMsg_init(eid, msgHandle);
}


void vosMsg_cleanup(void **msgHandle)
{
   VosMsgHandle *handle = (VosMsgHandle *) *msgHandle;
   VosMsgHeader *msg;

   /* free any queued up messages */
   while ((msg = handle->putBackQueue) != NULL)
   {
      handle->putBackQueue = msg->next;
      VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);
   }

   oalVosMsg_cleanup(msgHandle);
}


VOS_RET_E vosMsg_send(void *msgHandle, const VosMsgHeader *buf)
{
   VosMsgHandle *handle = (VosMsgHandle *) msgHandle;

#ifdef DESKTOP_LINUX
   if (handle->standalone)
   {
      /* just pretend to have sent the message */
      return VOS_RET_SUCCESS;
   }
#endif   

   return oalVosMsg_send(handle->commFd, buf);
}


VOS_RET_E vosMsg_sendReply(void *msgHandle, const VosMsgHeader *msg, VOS_RET_E retCode)
{
   VosMsgHandle *handle = (VosMsgHandle *) msgHandle;
   VosMsgHeader replyMsg = EMPTY_MSG_HEADER;

   replyMsg.dst = msg->src;
   replyMsg.src = msg->dst;
   replyMsg.type = msg->type;

   replyMsg.flags_request = 0;
   replyMsg.flags_response = 1;
   replyMsg.flags_bounceIfNotRunning = msg->flags_bounceIfNotRunning;
   /* do we want to copy any other flags? */

   replyMsg.wordData = retCode;

   return oalVosMsg_send(handle->commFd, &replyMsg);
}


static VOS_RET_E sendAndGetReply(void *msgHandle, const VosMsgHeader *buf, UINT32 *timeout)
{
   VosMsgHandle *handle = (VosMsgHandle *) msgHandle;
   VosMsgType sentType;
   VosMsgHeader *replyMsg=NULL;
   UBOOL8 doReceive=TRUE;
   VOS_RET_E ret;
   UINT32 saveTimeout = 0;
   UINT32 msgWaitTime = 0;

    if (timeout)
    {
        saveTimeout = *timeout;
        msgWaitTime = *timeout;
    }

#ifdef DESKTOP_LINUX
   if (handle->standalone)
   {
       VosMsgHeader *msg = (VosMsgHeader *) buf;

      /*
       * Standalone mode occurs during unittests.
       * Pretend to send out the message and get a successful reply.
       */
      if ((msg->type == VOS_MSG_START_APP) || (msg->type == VOS_MSG_RESTART_APP))
      {
         /* For the START_APP and RESTART_APP messages, the expected return value is the pid. */
         return desktopFakePid++;
      }
      else
      {
         return VOS_RET_SUCCESS;
      }
   }
#endif   

   /* remember what msg type we sent out. */
   sentType = buf->type;

   ret = oalVosMsg_send(handle->commFd, buf);
   if (ret != VOS_RET_SUCCESS)
   {
       vosLog_error("Send msg 0x%x wordData %u from %u to %u failed",
                    buf->type, buf->wordData, buf->src, buf->dst);
       return ret;
   }

   while (doReceive)
   {
      if (NULL == timeout)
      {
          msgWaitTime = MSG_WAIT_TIMEOUT;
      }
      
      ret = oalVosMsg_receive(handle->commFd, &replyMsg, &msgWaitTime);
      if (ret != VOS_RET_SUCCESS)
      {
         doReceive = FALSE;
         if (VOS_RET_TIMED_OUT == ret)
         {
            vosLog_error("Reply msg 0x%x wordData %u from %u to %u timeout",
                         buf->type, buf->wordData, buf->src, buf->dst);
            
            if (NULL == timeout)
            {
                doReceive = TRUE;
            }
         }
         else
         {
            vosLog_error("error during get of reply, ret=%d", ret);
         }

         VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);
      }
      else
      {
         if (replyMsg->type == sentType)
         {
            ret = (VOS_RET_E)replyMsg->wordData;
            doReceive = FALSE;
            VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);
         }
         else
         {
            if (VOS_MSG_REREQUEST == replyMsg->type)
            {
                if (replyMsg->src == buf->dst)
                {
                    VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);

                    if (timeout)
                    {
                        *timeout = saveTimeout;
                    }
                    sendAndGetReply(msgHandle, buf, timeout);
                    return ret;
                }

                VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);
                continue;
            }
             /* we got a mesage, but it was not the reply we were expecting.
             * Could be an event msg.  Push it back on the put-back queue and
             * keep trying to get the message we really want.
             */
            vosMsg_putBack(msgHandle, &replyMsg);
            replyMsg = NULL;
         }
      }
   }

   vosMsg_requeuePutBacks(msgHandle);

   if (timeout)
   {
       *timeout = msgWaitTime;
   }

   return ret;
}

static VOS_RET_E sendAndGetReplyBuf(void *msgHandle, const VosMsgHeader *buf, VosMsgHeader **replyBuf, UINT32 *timeout)
{
   VosMsgHandle *handle = (VosMsgHandle *) msgHandle;
   VosMsgType sentType;
   VosMsgHeader *replyMsg=NULL;
   UBOOL8 doReceive=TRUE;
   VOS_RET_E ret;
   UINT32 saveTimeout = 0;
   UINT32 msgWaitTime = 0;
   
    if (timeout)
    {
        saveTimeout = *timeout;
        msgWaitTime = *timeout;
    }

   /* remember what msg type we sent out. */
   sentType = buf->type;

   ret = oalVosMsg_send(handle->commFd, buf);
   if (ret != VOS_RET_SUCCESS)
   {
      vosLog_error("Send msg 0x%x wordData %u from %u to %u failed",
                   buf->type, buf->wordData, buf->src, buf->dst);
      return ret;
   }

   while (doReceive)
   {
      if (NULL == timeout)
      {
          msgWaitTime = MSG_WAIT_TIMEOUT;
      }
      
      ret = oalVosMsg_receive(handle->commFd, &replyMsg, &msgWaitTime);
      if (ret != VOS_RET_SUCCESS)
      {
         doReceive = FALSE;
         if (VOS_RET_TIMED_OUT == ret)
         {
            vosLog_error("Reply msg 0x%x wordData %u from %u to %u timeout",
                         buf->type, buf->wordData, buf->src, buf->dst);
            
            if (NULL == timeout)
            {
                doReceive = TRUE;
            }
         }
         else
         {
            vosLog_error("error during get of reply, ret=%d", ret);
         }

         VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);
      }
      else
      {
         if (replyMsg->type == sentType)
         {
            if ((!buf->flags_srpc) || (replyMsg->flags_srpc && (buf->wordData == replyMsg->wordData)))
            {
                *replyBuf = replyMsg;
                 doReceive = FALSE;
            }
            else
            {
                vosLog_error("Unmatched srpc msg %u from %u to %u, expect srpc msg %u",
                             replyMsg->wordData, replyMsg->src, replyMsg->dst, buf->wordData);
                VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);
            }
         }
         else
         {
            if (VOS_MSG_REREQUEST == replyMsg->type)
            {
                if (replyMsg->src == buf->dst)
                {
                    VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);

                    if (timeout)
                    {
                        *timeout = saveTimeout;
                    }
                    sendAndGetReplyBuf(msgHandle, buf, replyBuf, timeout);
                    return ret;
                }

                VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);
                continue;
            }

            /* we got a mesage, but it was not the reply we were expecting.
             * Could be an event msg.  Push it back on the put-back queue and
             * keep trying to get the message we really want.
             */
            vosMsg_putBack(msgHandle, &replyMsg);
            replyMsg = NULL;
         }
      }
   }

   vosMsg_requeuePutBacks(msgHandle);

   if (timeout)
   {
       *timeout = msgWaitTime;
   }

   return ret;
}


VOS_RET_E vosMsg_sendAndGetReply(void *msgHandle, const VosMsgHeader *buf)
{
   return (sendAndGetReply(msgHandle, buf, NULL));
}


VOS_RET_E vosMsg_sendAndGetReplyWithTimeout(void *msgHandle,
                                         const VosMsgHeader *buf,
                                         UINT32 timeoutMilliSeconds)
{
   UINT32 timeout = timeoutMilliSeconds;

   return (sendAndGetReply(msgHandle, buf, &timeout));
}


VOS_RET_E vosMsg_sendAndGetReplyBuf(void *msgHandle, const VosMsgHeader *buf, VosMsgHeader **replyBuf)
{
   return (sendAndGetReplyBuf(msgHandle, buf, replyBuf, NULL)); 
}


VOS_RET_E vosMsg_sendAndGetReplyBufWithTimeout(void *msgHandle, const VosMsgHeader *buf, VosMsgHeader **replyBuf, UINT32 timeoutMilliSeconds)
{
   UINT32 timeout = timeoutMilliSeconds;

   return (sendAndGetReplyBuf(msgHandle, buf, replyBuf, &timeout)); 
}


VOS_RET_E vosMsg_receive(void *msgHandle, VosMsgHeader **buf)
{
   VosMsgHandle *handle = (VosMsgHandle *) msgHandle;

#ifdef DESKTOP_LINUX
   if (handle->standalone)
   {
      /*
       * Hmm, this is a tricky situation.  Caller has told us to block until
       * we get a message, but since smd is not running, we will never get
       * a message.  Return INTERNAL_ERROR and let caller handle it?
       */
      vosLog_error("cannot receive msg while in standalone (unittest) mode");
      *buf = NULL;
      return VOS_RET_INTERNAL_ERROR;
   }
#endif   

   vosMsg_receivePutBack(msgHandle, buf);
   if (*buf)
   {
      return VOS_RET_SUCCESS;
   }

   return oalVosMsg_receive(handle->commFd, buf, NULL);
}


VOS_RET_E vosMsg_receiveWithTimeout(void *msgHandle, VosMsgHeader **buf, UINT32 timeoutMilliSeconds)
{
   VosMsgHandle *handle = (VosMsgHandle *) msgHandle;
   UINT32 timeout = timeoutMilliSeconds;

#ifdef DESKTOP_LINUX
   if (handle->standalone)
   {
      *buf = NULL;
      return VOS_RET_TIMED_OUT;
   }
#endif   

   vosMsg_receivePutBack(msgHandle, buf);
   if (*buf)
   {
      return VOS_RET_SUCCESS;
   }

   return oalVosMsg_receive(handle->commFd, buf, &timeout);
}


VOS_RET_E vosMsg_getEventHandle(const void *msgHandle, void *eventHandle)
{
   return (oalVosMsg_getEventHandle((VosMsgHandle *) msgHandle, eventHandle));
}


VosEntityId vosMsg_getHandleEid(const void *msgHandle)
{
   const VosMsgHandle *handle = (const VosMsgHandle *) msgHandle;

   return (handle == NULL ? 0 : handle->eid);
}


VosMsgHeader *vosMsg_duplicate(const VosMsgHeader *msg)
{
   UINT32 totalLen;
   void *newMsg;

   totalLen = sizeof(VosMsgHeader) + msg->dataLength;
   newMsg = VOS_MALLOC_FLAGS(totalLen, 0);
   if (newMsg != NULL)
   {
      memcpy(newMsg, msg, totalLen);
   }

   return newMsg;
}


void vosMsg_putBack(void *msgHandle, VosMsgHeader **buf)
{
   VosMsgHandle *handle = (VosMsgHandle *) msgHandle;
   VosMsgHeader *prevMsg;

   if (VOS_MSG_REQUEUE == (*buf)->type)
   {
      VOS_MEM_FREE_BUF_AND_NULL_PTR(*buf);
      return;
   }

   (*buf)->next = NULL;

   /* put the new message at the end of the putBackQueue */
   if (handle->putBackQueue == NULL)
   {
      handle->putBackQueue = (*buf);
   }
   else
   {
      prevMsg = handle->putBackQueue;
      while (prevMsg->next != NULL)
      {
         prevMsg = prevMsg->next;
      }

      prevMsg->next = (*buf);
   }

   /* we've taken ownership of this msg, so null out caller's pointer */
   *buf = NULL;

   return;
}


void vosMsg_requeuePutBacks(void *msgHandle)
{
   VosMsgHandle *handle = (VosMsgHandle *) msgHandle;
   VosMsgHeader requeueMsg = EMPTY_MSG_HEADER;

   if (handle->putBackQueue)
   {
       requeueMsg.type = VOS_MSG_REQUEUE;
       requeueMsg.flags_requeue = 1;
       requeueMsg.src = handle->putBackQueue->src;
       requeueMsg.dst = handle->putBackQueue->dst;
       oalVosMsg_send(handle->commFd, &requeueMsg);
   }

   return;
}


void vosMsg_receivePutBack(void *msgHandle, VosMsgHeader **buf)
{
    VosMsgHandle *handle = (VosMsgHandle *)msgHandle;
    VosMsgHeader *msg;

    msg = handle->putBackQueue;
    if (msg)
    {
        handle->putBackQueue = msg->next;
        msg->next = NULL;

        (*buf) = msg;
    }
    else
    {
        (*buf) = NULL;
    }
}

