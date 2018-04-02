#include "basic.h"

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
        DEBUG_PRINTF("====simpleDeviceEntry ieee_addr:%s==\n",input->ieee_addr);
        DEBUG_PRINTF("====simpleDeviceEntry nw_addr: %d===\n",input->nw_addr);
        DEBUG_PRINTF("====simpleDeviceEntry version: %s===\n",input->version);
        DEBUG_PRINTF("====simpleDeviceEntry model: %s === \n", input->model);
        DEBUG_PRINTF("====simpleDeviceEntry seq : %s======\n", input->seq);
    }
    printf("=====\n");
    PrintSimpleDeviceEntry(input->root.next);
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
        PrintUserEntry(input->root.next);
    }
}
