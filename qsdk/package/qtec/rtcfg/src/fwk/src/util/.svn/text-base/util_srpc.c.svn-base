#include "fwk.h"


typedef struct
{
    VosMsgHeader msg;
    SRPC_MSG_HEADER_T srpcMsg;
} SRPC_MSG_REQ_HEADER_T;


static const SRPC_SERVER_TASK_PARAMS_T *sg_taskParams;


VOS_RET_E SRPC_msgProcess(void *msgHandle, const VosMsgHeader *msg)
{
    VOS_RET_E ret;
    SRPC_MSG_REQ_HEADER_T *msgReq = (SRPC_MSG_REQ_HEADER_T *)msg;
    SRPC_SERVER_HANDLE_T srpcHandle = NULL;
    UINT32 msgId;
    
    if ((NULL == msgHandle) || (NULL == msg) || (NULL == sg_taskParams))
    {
        vosLog_error("msgHandle = %p, msg = %p, sg_taskParams = %p\n", msgHandle, msg, sg_taskParams);
        return VOS_RET_INVALID_ARGUMENTS;
    }

    if (msg->dataLength < sizeof(SRPC_MSG_REQ_HEADER_T) - sizeof(VosMsgHeader))
    {
        vosLog_error("Receive uncompleted srpc request msg, length(%u).\\n", msg->dataLength);
        return VOS_RET_INVALID_ARGUMENTS;
    }

    msgId = msgReq->msg.wordData;
    if (msgId >= sg_taskParams->tblSize)
    {
        vosLog_error("Unknown msg id(%u)\n", msgId);
        return VOS_RET_INTERNAL_ERROR;
    }
    
    srpcHandle = sg_taskParams->table[msgId];
    if (NULL == srpcHandle)
    {
        vosLog_error("Unsupported msg id(%u)\n", msgId);
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = srpcHandle(msgHandle, (VosMsgHeader *)msg);
    if (VOS_RET_SUCCESS != ret)
    {
        vosLog_error("Call srpc fail(%u)\n", ret);
        return ret;
    }
    
    return VOS_RET_SUCCESS;
}


VOS_RET_E SRPC_setTaskParams(VosEntityId eid, const SRPC_SERVER_TASK_PARAMS_T *taskParams)
{
    sg_taskParams = taskParams;
    return VOS_RET_SUCCESS;
}
