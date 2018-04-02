#ifndef __VOS_MSG_OAL_H__
#define __VOS_MSG_OAL_H__


#include "fwk.h"


/** This is the internal structure of the message handle.
 *
 * It is highly OS dependent.  Management applications should not
 * use any of its fields directly.
 *
 */
typedef struct
{
   VosEntityId  eid;        /**< Entity id of the owner of this handle. */
   SINT32       commFd;     /**< communications fd */
   UBOOL8       standalone; /**< are we running without smd, for unittests */
   VosMsgHeader *putBackQueue;  /**< Messages pushed back into the handle. */
} VosMsgHandle;



/** Initialize messaging system.
 *
 * Same semantics as vosMsg_init().
 * 
 * @param eid       (IN)  Entity id of the calling process.
 * @param msgHandle (OUT) msgHandle.
 *
 * @return VOS_RET_E enum.
 */
VOS_RET_E oalVosMsg_init(VosEntityId eid, void **msgHandle);


/** Clean up messaging system.
 *
 * Same semantics as vosMsg_cleanup().
 * @param msgHandle (IN) This was the msg_handle that was
 *                       created by vosMsg_init().
 */
void oalVosMsg_cleanup(void **msgHandle);



/** Send a message (blocking).
 *
 * Same semantics as vosMsg_send().
 *
 * @param fd        (IN) The commFd to send the msg out of.
 * @param buf       (IN) This buf contains a VosMsgHeader and possibly
 *                       more data depending on the message type.
 * @return VOS_RET_E enum.
 */
VOS_RET_E oalVosMsg_send(SINT32 fd, const VosMsgHeader *buf);


/** Receive a new message from fd.
 *
 * @param fd         (IN) commFd to receive input from.
 * @param buf       (OUT) Returns a pointer to message buffer.  Caller is responsible
 *                        for freeing the buffer.
 * @param timeout    (IN) Pointer to UINT32, specifying the timeout in milliseconds.
 *                        If pointer is NULL, receive will block until a message is
 *                        received.
 *
 * @return VOS_RET_E enum.
 */
VOS_RET_E oalVosMsg_receive(SINT32 fd, VosMsgHeader **buf, UINT32 *timeout);


/** Get operating system dependent handle for receive message notification.
 *
 * Same semantics as vosMsg_getEventHandle();
 * @param msgHandle    (IN) This was the msgHandle created by vosMsg_init().
 * @param eventHandle (OUT) This is the OS dependent event handle.  For LINUX,
 *                          eventHandle is the file descriptor number.
 * @return VOS_RET_E enum.
 */
VOS_RET_E oalVosMsg_getEventHandle(const VosMsgHandle *msgHandle, void *eventHandle);



#endif /* __VOS_MSG_OAL_H__ */

