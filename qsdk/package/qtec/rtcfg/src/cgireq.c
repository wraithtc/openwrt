#include <fwk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "librtcfg.h"
#include "rtcfg_uci.h"


//this file api  only just send message to smd, don't need receive 
//may memory leak in myclient 
#define ERR_INTERNALLOGIC_WRONG -95

struct Rtcfg_VosMsgBody
{
	VosMsgHeader stHead;
	char buf[2048];
};




int ProcFirewallRestartReq()
{
    void *g_msgHandle;
    DEBUG_PRINTF("====[%s]====%d====\n",__func__,__LINE__);
    int ret;
    struct Rtcfg_VosMsgBody stMsg={0};
    struct Rtcfg_VosMsgBody *pstReplyMsg;
    vosLog_init(EID_CGIREQ);
    vosLog_setDestination(VOS_LOG_DEST_STDERR);
    vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
    if(ret != VOS_RET_SUCCESS)
    {
        vosLog_error("[%s] dm msg initialization failed, ret=%d ",__func__, ret);
        vosMsg_cleanup(&g_msgHandle);
        return ERR_INTERNALLOGIC_WRONG;
    }

    stMsg.stHead.dataLength=0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_FIREWALL_RESTART_REQ;
    stMsg.stHead.flags_request = 1;

    vosMsg_send(g_msgHandle,&stMsg);
    vosMsg_cleanup(&g_msgHandle);
    return 0;
    
}


int ProcQosRestartReq()
{
    void *g_msgHandle;
    DEBUG_PRINTF("====[%s]====%d====\n",__func__,__LINE__);
    int ret;
    struct Rtcfg_VosMsgBody stMsg={0};
    struct Rtcfg_VosMsgBody *pstReplyMsg;
    vosLog_init(EID_CGIREQ);
    vosLog_setDestination(VOS_LOG_DEST_STDERR);
    vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
    if(ret != VOS_RET_SUCCESS)
    {
        
        vosLog_error("[%s] dm msg initialization failed, ret=%d ",__func__, ret);
        vosMsg_cleanup(&g_msgHandle);
        return ERR_INTERNALLOGIC_WRONG;
    }

    stMsg.stHead.dataLength=0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_QOS_RESTART_REQ;
    stMsg.stHead.flags_request = 1;

    vosMsg_send(g_msgHandle,&stMsg);
    vosMsg_cleanup(&g_msgHandle);
    return 0;
    
}


