#include "basic.h"

void *g_msgHandle =NULL; //定义全局消息指针

//建立与smd 之间的联系
int init_connection()
{
    int ret=0;
    DEBUG_PRINTF("====[%s]========\n",__func__);

    ret=vosMsg_init(EID_QTECDEVICEMANAGER, &g_msgHandle);

    if(ret != VOS_RET_SUCCESS)
    {
        vosLog_error("msg initialization failed, ret= %d", ret);
        return ret;
    }

    
    return ret;
}


