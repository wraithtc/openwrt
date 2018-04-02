/* OS dependent messaging functions go here */

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "vos_msg_oal.h"
#include "fwk.h"


VOS_RET_E oalVosMsg_init(VosEntityId eid, void **msgHandle)
{
   VosMsgHandle *handle;
   const VosEntityInfo *eInfo;
   struct sockaddr_un serverAddr;
   SINT32 rc;

   if ((eInfo = vosEid_getEntityInfo(eid)) == NULL)
   {
      vosLog_error("Unkown eid %d", eid);
      return VOS_RET_INVALID_ARGUMENTS;
   }

   if ((handle = (VosMsgHandle *) VOS_MALLOC_FLAGS(sizeof(VosMsgHandle), ALLOC_ZEROIZE)) == NULL)
   {
      vosLog_error("could not allocate storage for msg handle");
      return VOS_RET_RESOURCE_EXCEEDED;
   }

   /* store caller's eid */
   handle->eid = eid;

#ifdef DESKTOP_LINUX
   /*
    * Applications may be run without smd on desktop linux, so if we
    * don't see a socket for smd, don't bother connecting to it.
    */
   {
      struct stat statbuf;

      if ((rc = stat(SMD_MESSAGE_ADDR, &statbuf)) < 0)
      {
         handle->commFd = UTIL_INVALID_FD;
         handle->standalone = TRUE;
         *msgHandle = (void *) handle;
         vosLog_notice("no smd server socket detected, running in standalone mode.");
         return VOS_RET_SUCCESS;
      }
   }
#endif /* DESKTOP_LINUX */


      /*
       * Create a unix domain socket.
       */
      handle->commFd = socket(AF_LOCAL, SOCK_STREAM, 0);
      if (handle->commFd < 0)
      {
         vosLog_error("Could not create socket");
         VOS_FREE(handle);
         return VOS_RET_INTERNAL_ERROR;
      }


      /*
       * Set close-on-exec, even though all apps should close their
       * fd's before fork and exec.
       */
      if ((rc = fcntl(handle->commFd, F_SETFD, FD_CLOEXEC)) != 0)
      {
         vosLog_error("set close-on-exec failed, rc=%d errno=%d", rc, errno);
         close(handle->commFd);
         VOS_FREE(handle);
         return VOS_RET_INTERNAL_ERROR;
      }


      /*
       * Connect to smd.
       */
      memset(&serverAddr, 0, sizeof(serverAddr));
      serverAddr.sun_family = AF_LOCAL;
      strncpy(serverAddr.sun_path, SMD_MESSAGE_ADDR, sizeof(serverAddr.sun_path));

      rc = connect(handle->commFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
      if (rc != 0)
      {
         vosLog_error("connect to %s failed, rc=%d errno=%d", SMD_MESSAGE_ADDR, rc, errno);
         close(handle->commFd);
         VOS_FREE(handle);
         return VOS_RET_INTERNAL_ERROR;
      }
      else
      {
         vosLog_debug("commFd=%d connected to smd", handle->commFd);
      }

      /* send a launched message to smd */
      {
         VOS_RET_E ret;
         VosMsgHeader launchMsg = EMPTY_MSG_HEADER;

         launchMsg.type = VOS_MSG_APP_LAUNCHED;
         launchMsg.src = (eInfo && eInfo->flags & EIF_MULTIPLE_INSTANCES) ? MAKE_SPECIFIC_EID(getpid(), eid) : eid;
         launchMsg.dst = EID_SMD;
         launchMsg.flags_event = 1;
         launchMsg.wordData = getpid();

         if ((ret = oalVosMsg_send(handle->commFd, &launchMsg)) != VOS_RET_SUCCESS)
         {
            close(handle->commFd);
            VOS_FREE(handle);
            return VOS_RET_INTERNAL_ERROR;
         }
         else
         {
            vosLog_debug("sent LAUNCHED message to smd");
         }
      }

   /* successful, set handle pointer */
   *msgHandle = (void *) handle;

   return VOS_RET_SUCCESS;
}


void oalVosMsg_cleanup(void **msgHandle)
{
   VosMsgHandle *handle = (VosMsgHandle *) *msgHandle;

   if (handle->commFd != UTIL_INVALID_FD)
   {
      close(handle->commFd);
   }

   VOS_MEM_FREE_BUF_AND_NULL_PTR((*msgHandle));

   return;
}


VOS_RET_E oalVosMsg_getEventHandle(const VosMsgHandle *msgHandle, void *eventHandle)
{
   SINT32 *fdPtr = (SINT32 *) eventHandle;

   *fdPtr = msgHandle->commFd;

   return VOS_RET_SUCCESS;
}


VOS_RET_E oalVosMsg_send(SINT32 fd, const VosMsgHeader *buf)
{ 
   UINT32 totalLen;
   SINT32 rc;
   VOS_RET_E ret=VOS_RET_SUCCESS;

   totalLen = sizeof(VosMsgHeader) + buf->dataLength;

   rc = write(fd, buf, totalLen);
   if (rc < 0)
   {
      if (errno == EPIPE)
      {
         /*
          * This could happen when smd tries to write to an app that
          * has exited.  Don't print out a scary error message.
          * Just return an error code and let upper layer app handle it.
          */
         vosLog_debug("got EPIPE, dest app is dead");
         return VOS_RET_DISCONNECTED;
      }
      else
      {
         vosLog_error("write failed, errno=%d", errno);
         ret = VOS_RET_INTERNAL_ERROR;
      }
   }
   else if (rc != (SINT32) totalLen)
   {
      vosLog_error("unexpected rc %d, expected %u", rc, totalLen);
      ret = VOS_RET_INTERNAL_ERROR;
   }

   return ret;
}


static VOS_RET_E waitForDataAvailable(SINT32 fd, UINT32 *timeout)
{
   struct timeval tv;
   fd_set readFds;
   SINT32 rc;

   if (NULL == timeout)
   {
      vosLog_error("timeout is NULL!");
      return VOS_RET_INVALID_ARGUMENTS;
   }

   FD_ZERO(&readFds);
   FD_SET(fd, &readFds);

   tv.tv_sec = *timeout / MSECS_IN_SEC;
   tv.tv_usec = (*timeout % MSECS_IN_SEC) * USECS_IN_MSEC;

   rc = select(fd+1, &readFds, NULL, NULL, &tv);
   *timeout = tv.tv_sec * MSECS_IN_SEC + tv.tv_usec / USECS_IN_MSEC;

   if ((rc == 1) && (FD_ISSET(fd, &readFds)))
   {
      return VOS_RET_SUCCESS;
   }
   else
   {
      return VOS_RET_TIMED_OUT;
   }
}


VOS_RET_E oalVosMsg_receive(SINT32 fd, VosMsgHeader **buf, UINT32 *timeout)
{ 
   VosMsgHeader *msg;
   SINT32 rc;
   VOS_RET_E ret;
   SINT32 totalReadSoFar = 0;
   SINT32 totalRemaining = sizeof(VosMsgHeader);
   char *inBuf;

   if (buf == NULL)
   {
      vosLog_error("buf is NULL!");
      return VOS_RET_INVALID_ARGUMENTS;
   }
   else
   {
      *buf = NULL;
   }

   /*
    * Read just the header in the first read.
    * Do not try to read more because we might get part of 
    * another message in the TCP socket.
    */
   msg = (VosMsgHeader *) VOS_MALLOC_FLAGS(sizeof(VosMsgHeader), ALLOC_ZEROIZE);
   if (msg == NULL)
   {
      vosLog_error("alloc of msg header failed");
      return VOS_RET_RESOURCE_EXCEEDED;
   }

   inBuf = (char *)msg;
   while (totalReadSoFar < sizeof(VosMsgHeader))
   {
      vosLog_debug("reading segment: soFar=%d total=%d", totalReadSoFar, totalRemaining);
      if (timeout)
      {
         if ((ret = waitForDataAvailable(fd, timeout)) != VOS_RET_SUCCESS)
         {
            VOS_FREE(msg);
            return ret;
         }
      }
 
      rc = read(fd, inBuf, totalRemaining);
 	 
      if ((rc == 0) || ((rc == -1) && (errno == 131)))  /* new 2.6.21 kernel seems to give us this before rc==0 */
      {
         /* broken connection */
         VOS_FREE(msg);
         return VOS_RET_DISCONNECTED;
      }
      else if (rc < 0)
      {
         vosLog_error("bad read, rc=%d errno=%d", rc, errno);
         VOS_FREE(msg);
         return VOS_RET_INTERNAL_ERROR;
      }
      else
      {
         inBuf += rc;
         totalReadSoFar += rc;
         totalRemaining -= rc;
      }
   }

   if (msg->dataLength > 0)
   {
      totalReadSoFar = 0;
      totalRemaining = msg->dataLength;

      /* there is additional data in the message */
      msg = (VosMsgHeader *) VOS_REALLOC(msg, sizeof(VosMsgHeader) + msg->dataLength);
      if (msg == NULL)
      {
         vosLog_error("realloc failed");
         VOS_FREE(msg);
         return VOS_RET_RESOURCE_EXCEEDED;
      }

      inBuf = (char *) (msg + 1);
      while (totalReadSoFar < msg->dataLength)
      {
         vosLog_debug("reading segment: soFar=%d total=%d", totalReadSoFar, totalRemaining);
         if (timeout)
         {
            if ((ret = waitForDataAvailable(fd, timeout)) != VOS_RET_SUCCESS)
            {
               VOS_FREE(msg);
               return ret;
            }
         }

         rc = read(fd, inBuf, totalRemaining);
		 
         if (rc <= 0)
         {
            vosLog_error("bad data read, rc=%d errno=%d readSoFar=%d remaining=%d", rc, errno, totalReadSoFar, totalRemaining);
            VOS_FREE(msg);
            return VOS_RET_INTERNAL_ERROR;
         }
         else
         {
            inBuf += rc;
            totalReadSoFar += rc;
            totalRemaining -= rc;
         }
      }
   }

   *buf = msg;

   return VOS_RET_SUCCESS;
}

