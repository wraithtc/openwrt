#include "basic.h"



/*
*  备注: 大部分数据 使用双向链表保存，
*       链表表项的增加，只能在链表的tail处添加
*       链表表项的删除，要维护好双链表的指针
*/
void qtec_list_init(struct qtec_list *input)
{
    input->next = NULL;
    input->prev = NULL;
}

void qtec_insert_list(struct qtec_list *input, struct qtec_list ** list_head, struct qtec_list ** list_tail)
{
    DEBUG_PRINTF("====%s=========\n",__func__);
    if(input == NULL)
    {
        DEBUG_PRINTF("=%s:===input is null ====\n", __func__);
        return;
    }
    if( *list_head == NULL)
    {
       
        
        if( *list_tail != NULL)
        {
            global_error= ERR_DATA_WRONG;
            program_quit();
        }
        *list_head=input;
        *list_tail=input;
 
    }
    else
    {

        if(*list_tail==NULL)
        {
            global_error= ERR_DATA_WRONG;
            program_quit();
        }

        (*list_tail)->next=input;
        input->prev=(*list_tail);
        (*list_tail)=input;

    }
}

void qtec_remove_list(struct qtec_list *input, struct qtec_list ** list_head, struct qtec_list ** list_tail)
{
    DEBUG_PRINTF("====%s======\n",__func__);
    if(input == NULL)
    {
        DEBUG_PRINTF("=%s:===input is null ====\n", __func__);
        return;
    }

    if(input->next == NULL)
    {
        if((*list_tail) != input)
        {
            global_error = ERR_DATA_WRONG;
            program_quit();
        }
        if(input->prev != NULL)
        {
            input->prev->next=NULL;
            (*list_tail)=input->prev;
        }
        else
        {
            //这个list 只有input 这一个entry
            if((*list_head) != input)
            {
                global_error = ERR_DATA_WRONG;
                program_quit();
            }
            
            (*list_head) =NULL;
            (*list_tail) =NULL;
        }
    }
    else
    {
        if(input->prev == NULL)
        {
            if((*list_head) != input)
            {
                global_error = ERR_DATA_WRONG;
                program_quit();
            }
            input->next->prev = NULL;
            (*list_head) = input->next;
        }
        else
        {
            input->next->prev=input->prev;
            input->prev->next=input->next;
        }
    }
}

struct simpleDeviceEntry * malloc_simpleDeviceEntry()
{
    struct simpleDeviceEntry *output;
    output = malloc(sizeof(struct simpleDeviceEntry));
    memset(output,0,sizeof(struct simpleDeviceEntry));
    qtec_list_init(&(output->root));
    DEBUG_PRINTF("==[%s]====outputaddress: 0x %x==\n",__func__,output);
    return output;
}

void free_simpleDeviceEntry(struct simpleDeviceEntry *input)
{
    DEBUG_PRINTF("=====%s======input address: 0x %x =\n",__func__,input);
    if(input !=NULL)
    {
        free(input);
    }
}

struct UserEntry * malloc_userEntry()
{
    struct UserEntry *output=NULL;
    output= malloc(sizeof(struct UserEntry));

    memset(output,0,sizeof(struct UserEntry));
    qtec_list_init(&(output->root));
    DEBUG_PRINTF("==[%s]====output address: 0x %x ===\n", __func__,output);
    return output;
}

void free_userEntry(struct UserEntry *input)
{
    DEBUG_PRINTF("===[%s]=====input address: 0x %x ===\n", __func__,input);
    if(input !=NULL)
    {
        free(input);
    }
}

struct FingerPrintEntry* malloc_fingerPrintEntry()
{
    struct FingerPrintEntry *output=NULL;
    output= malloc(sizeof(struct FingerPrintEntry));

    memset(output,0,sizeof(struct FingerPrintEntry));
    qtec_list_init(&(output->root));
    DEBUG_PRINTF("==[%s]====output address: 0x %x ===\n", __func__,output);
    return output;
}

void free_fingerPrintEntry(struct FingerPrintEntry* input)
{
    DEBUG_PRINTF("===[%s]=====input address: 0x %x ===\n", __func__,input);
    if(input !=NULL)
    {
        free(input);
    }
}

struct LogEntry* malloc_logEntry()
{
    struct LogEntry *output = NULL;
    output = malloc(sizeof(struct LogEntry));

    memset(output,0,sizeof(struct LogEntry));
    qtec_list_init(&( output->root));
    DEBUG_PRINTF("===[%s]===output address: 0x %x===\n",__func__,output);
    return output;
}

void free_logEntry(struct LogEntry *input)
{
    DEBUG_PRINTF("===[%s] === input address: 0x %x===\n",__func__,input);
    if(input != NULL)
    {
        free(input);
    }
}

#if 0
struct UserEntry * qtec_alloc_userEntry()
{
    DEBUG_PRINTF("=================%s============\n",__func__);
    struct UserEntry *output;
    output=malloc(sizeof(struct UserEntry));
    memset(output,0,sizeof(struct UserEntry));

    qtec_list_init(&output->root);
    if(global_usertable_head == NULL)
    {
        if(global_usertable_tail != NULL)
        {
            global_error = ERR_DATA_WRONG;
            program_quit();
        }
        global_usertable_head=output;
        global_usertable_tail=output;
    }
    else
    {
        if(global_usertable_tail ==NULL)
        {
            global_error = ERR_DATA_WRONG;
            program_quit();
        }
        global_usertable_tail->root.next=output;
        output->root.prev=global_usertable_tail;
        global_usertable_tail = output;
    }
    
    return output;    
}

void qtec_free_userEntry(struct UserEntry *input)
{
    DEBUG_PRINTF("==============%s=============\n",__func__);
    if(input == NULL)
    {
        global_error = ERROR_MEM;
        DEBUG_PRINTF("=====%s====input is null ====\n",__func__);
        return;
    }

    if(input->root.prev == NULL)
    {
        DEBUG_PRINTF("====%s====this entry is first one ====\n",__func__);
        if(input->root.next !=NULL)
        {
            input->root.next->prev=NULL;
            global_usertable_head=input->root.next;
        }
        else
        {
            if(input->root.next == NULL)
            {
                DEBUG_PRINTF("====%s === this entry is last one =====\n", __func__);
                input->root.prev->next=NULL;
                global_usertable_tail=input->root.prev;
            }
            else
            {
                input->root.prev->next = input->root.next;
                input->root.next->prev = input->root.prev;
            }
        }
        free(input);
    }
    
}



struct DeviceEntry * qtec_alloc_deviceEntry()
{
    struct DeviceEntry *output;
    output=malloc(sizeof(struct DeviceEntry));
    memset(output,0,sizeof(struct DeviceEntry));
    qtec_list_init(&output->root);
    return output;
};


void qtec_free_deviceEntry(struct DeviceEntry * input)
{
    if(input !=NULL)
    {
        if(input->root.prev == NULL)
        {
            DEBUG_PRINTF("=====%s====this deviceEntry is first one====\n",__func__);
            if(input->root.next !=NULL)
            {
                input->root.next->prev=NULL;
                global_devicetable_head=input->root.next;
            }
            else
            {
                global_devicetable_head=NULL;
                global_devicetable_tail=NULL;
            }
        }
        else
        {
            if(input->root.next == NULL)
            {
                DEBUG_PRINTF("====%s====this entry is last one ==== \n", __func__);
                input->root.prev->next=NULL;
                global_devicetable_tail=input->root.prev;
            }
            else
            {
                input->root.prev->next=input->root.next;
                input->root.next->prev=input->root.prev;
            }
        }
        free(input);
    }
    else
    {
        global_error = ERROR_MEM;
        program_quit(); 
    }
}



struct DeviceManagerEntry * qtec_alloc_deviceManagerEntry()
{
    struct DeviceManagerEntry *output;
    output=malloc(sizeof(struct DeviceManagerEntry));
    memset(output,0,sizeof(struct DeviceManagerEntry));
    qtec_list_init(&output->root);
    qtec_list_init(&output->deviceEntry);
}

void qtec_free_deviceManagerEntry(struct DeviceManagerEntry *input)
{
    if(input !=NULL)
    {
        if(input->root.prev == NULL)
        {
            DEBUG_PRINTF("=====%s====this DeviceManagerEntry is first one====\n",__func__);
            if(input->root.next !=NULL)
                input->root.next->prev=NULL;
        }
        else
        {
            if(input->root.next == NULL)
            {
                DEBUG_PRINTF("====%s====this DeviceManagerEntry is last one ==== \n", __func__);
                input->root.prev->next=NULL;
            }
            else
            {
                input->root.prev->next=input->root.next;
                input->root.next->prev=input->root.prev;
            }
        }

        //first free deviceManagerEntry's deviceEntry
        while(input->deviceEntry.next !=NULL)
        {
            qtec_free_deviceEntry((struct DeviceEntry *)(input->deviceEntry.next));
        }
        
        free(input);
    }
    else
    {
        global_error = ERROR_MEM;
        program_quit(); 
    }
}
#endif