#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "fwk.h"
#include "librtcfg.h"

#define DEBUG_FILE    "/tmp/wdsctrl"

void *g_mcHandle =NULL;


struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[4096];
};


int main(int argc, char *argv[])
{
	 DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;
	vosLog_init(EID_WDS_CTRL);
	vosLog_setDestination(VOS_LOG_DEST_STDERR);
	vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);

    ret = vosMsg_init(EID_WDS_CTRL, &g_mcHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		return -1;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_WDS_CTRL);
    stMsg.stHead.type = VOS_MSG_WDS_DOWN;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_mcHandle, &stMsg);
	vosMsg_cleanup(&g_mcHandle);
	return 0;
}
