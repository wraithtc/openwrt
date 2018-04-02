#include <stdio.h>
#include "fwk.h"

void *g_msgHandle;

int main(int argc, const char *argv[])
{
    int ret;
    printf("-----------------\n");

    VosMsgHeader stMsg={0};
    printf("----------%s---%d--\n",__FUNCTION__, __LINE__);
    ret = vosMsg_init(EID_CTCSTP, &g_msgHandle);
    printf("----------%s---%d--\n",__FUNCTION__, __LINE__);
    stMsg.dataLength = 0;
    stMsg.dst = EID_DMSD;
    stMsg.src = EID_CTCSTP;
    stMsg.flags_event = 1;
    stMsg.type = VOS_MSG_SET_LOG_LEVEL;
    printf("----------%s---%d--\n",__FUNCTION__, __LINE__);
    while(1)
    {
        printf("----------%s---%d--\n",__FUNCTION__, __LINE__);
        ret = vosMsg_send(g_msgHandle, &stMsg);
        sleep(5);
    }

}

