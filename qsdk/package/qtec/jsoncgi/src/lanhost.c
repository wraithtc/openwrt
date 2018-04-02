#include "basic.h"
#include <fwk.h>

struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[4096];
};

void *g_msgHandle;


void proc_stalist_get(cJSON *json_value,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
	cJSON *subJson;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;
	vosLog_init(EID_WEBCGI);
	vosLog_setDestination(VOS_LOG_DEST_STDERR);
	vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);

    ret = vosMsg_init(EID_WEBCGI, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_LANHOST;
    stMsg.stHead.src = EID_WEBCGI;
    stMsg.stHead.type = VOS_MSG_HOSTLAN_GETINFO;
	stMsg.stHead.flags_request = 1;

	ret = vosMsg_sendAndGetReplyBufWithTimeout(g_msgHandle, &stMsg, &pstReplyMsg, MSECS_IN_SEC);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("get reply msg failed, ret= %d", ret);
		return;
	}

	if(pstReplyMsg)
	{
		if(stMsg.stHead.type == VOS_MSG_HOSTLAN_GETINFO)
		{
			subJson = cJSON_Parse(stMsg.buf);
			cJSON_AddItemToObject(jsonOut, "data", subJson);
		}
	}

	return;
}
