#include "dbbasic.h"

void *g_msgHandle1 =NULL; //定义全局消息指针

//建立与smd 之间的联系
int init_connection()
{
    int ret=0;
    DEBUG_PRINTF("====[%s]========\n",__func__);
    vosLog_init(EID_QTECDEVICEMANAGER);
    vosLog_setDestination(VOS_LOG_DEST_STDERR);
    vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);

    ret=vosMsg_init(EID_QTECDEVICEMANAGER, &g_msgHandle1);

    if(ret != VOS_RET_SUCCESS)
    {
        vosLog_error("msg initialization failed, ret= %d", ret);
        return ret;
    }

    
    return ret;
}


