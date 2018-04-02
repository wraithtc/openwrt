#include "dbbasic.h"

//在这个文件主要添加几个debug 相关的函数


void PrintSimpleDeviceEntry(struct simpleDeviceEntry *input)
{
    DEBUG_PRINTF("====[%s]:====\n",__func__);
    if(input == NULL)
    {
        DEBUG_PRINTF("====%s end =====\n",__func__);
        return;
    }
    else
    {
        DEBUG_PRINTF("====simpleDeviceEntry id: %s=====\n",input->deviceid);
        DEBUG_PRINTF("====simpleDeviceEntry type: %d ===\n", input->type);
        DEBUG_PRINTF("====simpleDeviceEntry name: %s ===\n", input->name);
        DEBUG_PRINTF("====simpleDeviceEntry status: %d===\n", input->status);
    }
    printf("=====\n");
    PrintSimpleDeviceEntry((struct simpleDeviceEntry *)input->root.next);
}  



void PrintUserEntry(struct UserEntry *input)
{
    
    if(input == NULL)
    {
        DEBUG_PRINTF("====%s end ======\n",__func__);
        return;
    }
    else
    {
        DEBUG_PRINTF("====userEntry id: %s ===\n",input->userid);
        DEBUG_PRINTF("====userEntry name: %s===\n", input->username);
        DEBUG_PRINTF("====userEntry grade: %d====\n", input->grade);
    }
    printf("============\n");
    if(input->root.next != NULL)
    {
        PrintUserEntry((struct UserEntry *)input->root.next);
    }
}
