#include "basic.h"


// 在这个文件，提供接口api 给 websock 和 zigbee 消息处理的线程 进行调用


 /*
 根据地址搜索设备
 入参数: list_head 指定哪个列表， 目前可输入 global_searchedDeviceEntryList_head 或 global_ManagedDeviceEntryList_head
         input_ieee_addr, nw_addr ,只需要匹配一个就可返回，
 返回 结果 为 
 若找到，则返回 一个指向堆内存的指针，所以函数使用者除非非常自信，否则不建议去修改它的数据，另外不要去free it
 根据地址搜索设备
 若没找到，则返回NULL, 所以调用者需要做判断
*/
struct simpleDeviceEntry * GetDevInfoByAddr( struct simpleDeviceEntry *input_list_head, char * input_ieee_addr, int nw_addr)
{
    DEBUG_PRINTF("===[%s]====input_list_head: 0x %x===== input_ieee_addr: %s === nw_addr: %d ====\n", __func__, input_list_head,input_ieee_addr,nw_addr);
    
    if( input_list_head == NULL)
    {
        return NULL;
    }
    if( input_list_head == global_ManagedDeviceEntryList_head )
    {
        return findManagedDeviceEntryByAddr(input_ieee_addr, nw_addr);
    }
    else if (input_list_head == global_searchedDeviceEntryList_head)
    {
        return findSearchedDeviceEntryByAddr(input_ieee_addr, nw_addr);
    }
    else
    {
        printf("====[%s] the input is wrong ====\n", __func__);
        return NULL;
    }
}

/*
根据设备id 来搜索设备
*/
struct simpleDeviceEntry * GetDevInfoById(struct simpleDeviceEntry *input_list_head, char *input_deviceid)
{
    DEBUG_PRINTF("===[%s]====input_list_head: 0x %x === input_deviceid: %s =====\n",__func__,input_list_head, input_deviceid);

    if(input_list_head == NULL)
    {
        return NULL;
    }

    if(input_list_head == global_ManagedDeviceEntryList_head )
    {
        return findManagedDeviceEntryByDeviceId(input_deviceid);
    }
    else if(input_list_head == global_searchedDeviceEntryList_head )
    {
        return findSearchedDeviceEntryByDeviceId(input_deviceid);
    }
    else
    {
        printf("===[%s] the input is wrong ====\n", __func__);
        return NULL;
    }
}

//添加一个设备到搜索列表
//入参数:
// input_entry 是栈上的内存，
// 返回值0 为成功
int AddDeviceInfo2SearchedDeviceList(struct simpleDeviceEntry *input_entry)
{
    DEBUG_PRINTF("===[%s] ====\n", __func__);
    PrintSimpleDeviceEntry(input_entry);
    struct simpleDeviceEntry *newentry = malloc_simpleDeviceEntry();
    strcpy(newentry->deviceid,input_entry->deviceid);
    newentry->type = input_entry->type;
    strcpy(newentry->name, input_entry->name);
    newentry->status =input_entry->status;
    strcpy(newentry->ieee_addr,input_entry->ieee_addr);
    newentry->nw_addr = input_entry->nw_addr;
    strcpy(newentry->version, input_entry->version);
    strcpy(newentry->model, input_entry->model);
    strcpy(newentry->seq, input_entry->seq);

    return AddToSearchedDeviceList(newentry);    
}

//添加一个设备到管理设备列表
//入参数:
// input_entry 是栈上的内存，
// 返回值0 为成功
int AddDeviceInfo2ManagedDeviceList(struct simpleDeviceEntry *input_entry)
{
    DEBUG_PRINTF("===[%s] ====\n", __func__);
    struct simpleDeviceEntry *newentry = malloc_simpleDeviceEntry();
    strcpy(newentry->deviceid,input_entry->deviceid);
    newentry->type = input_entry->type;
    strcpy(newentry->name, input_entry->name);
    newentry->status =input_entry->status;
    strcpy(newentry->ieee_addr,input_entry->ieee_addr);
    newentry->nw_addr = input_entry->nw_addr;
    strcpy(newentry->version, input_entry->version);
    strcpy(newentry->model, input_entry->model);
    strcpy(newentry->seq, input_entry->seq);

    return AddToManagedDeviceList(newentry);    
}

//从管理设备列表里删除设备
//入参: char * input_deviceid
int DelDeviceInfo2ManagedDeviceList(char *input_deviceid)
{
    DEBUG_PRINTF("===[%s]====input_deviceid:%s ===\n",__func__, input_deviceid);

    return RemoveFromManagedDeviceList(input_deviceid);
}

//清零 搜索设备列表
int ClearDevSearchList()
{
    DEBUG_PRINTF("====[%s]=====\n",__func__);
    return RefreshSearchedDeviceList();
}

//给设备改名
int RenameDevName(char *input_devid, char *input_name)
{
    DEBUG_PRINTF("===[%s]===input_devid:%s, input_name:%s===\n",__func__,input_devid,input_name);
    struct simpleDeviceEntry* result = findManagedDeviceEntryByDeviceId(input_devid);
    if(result == NULL)
    {
        printf("====[%s]=== can't find the match device===\n",__func__);
        return -1;
    }
    else
    {
        memset(result->name,0,sizeof(result->name));
        strcpy(result->name,input_name);
        UpdateEntryToManagedDeviceTable(result);
    }
    return 0;
}

//添加指纹
int AddFingerPrint(char *input_devid, char *input_userid, char *input_fingerprintid)
{
    DEBUG_PRINTF("===[%s]====input_devid:%s, input_userid: %s, fingerprintid: %s ==== \n", __func__,input_devid, input_userid, input_fingerprintid);

    struct FingerPrintEntry *newentry= malloc_fingerPrintEntry();
    strcpy(newentry->deviceid,input_devid);
    strcpy(newentry->userid, input_userid);
    strcpy(newentry->fingerprintid,input_fingerprintid);
    int rownum= getMaxRowIdFromDatabase("fingerPrint_table")+1;
    DEBUG_PRINTF("==[%s]=== rownum is %d===\n",__func__,rownum);
    snprintf(newentry->name,64,"fingerprint%d",rownum);
    DEBUG_PRINTF("==[%s]===name: %s===\n",__func__,newentry->name);
    return AddToFingerPrintEntryList(newentry);
}

//删除指纹
int DelFingerPrint(char *input_devid, char *input_userid, char *input_fingerprintid)
{
    DEBUG_PRINTF("==[%s]===input_devid:%s, input_userid:%s, input_fingerprintid: %s ===\n", __func__,input_devid,input_userid,input_fingerprintid);

    return RemoveFromFingerPrinterEntryList(input_fingerprintid, input_devid);
}

//rename 指纹
int RenameFingerPrint(char *input_devid, char *input_userid, char *input_fingerprintid, char *input_fingerprintname)
{
    DEBUG_PRINTF("===[%s]===input_devid:%s, input_userid:%s, input_fingerprintid:%s, input_fingerprintname:%s===\n",__func__,input_devid,input_userid,input_fingerprintid,input_fingerprintname);
    struct FingerPrintEntry *result =findFingerPrintEntry(input_fingerprintid,input_devid);
    if(result == NULL)
    {
        printf("====[%s]=== can't find the match fingerprint===\n",__func__);
        return -1;
    }
    else
    {
        if(strcmp(result->userid, input_userid) != 0)
        {
            printf("===[%s]=== the user id is wrong ===\n",__func__);
            return -1;
        }
        memset(result->name,0,sizeof(result->name));
        strcpy(result->name,input_fingerprintname);
        UpdateEntryToFingerPrintTable(result);
    }
    return 0;
}

//获取某一用户在某一设备下的所有指纹的信息
//入参: buf 指向一块足够大的栈上的内存用来存储结果，结构为数组
//返回 数组的个数
//-1 代表错误
int GetFingerPrintsByUserIdDeviceId(char *input_devid, char* input_userid, struct FingerPrintEntry* buf)
{
    DEBUG_PRINTF("==[%s] === input_devid: %s , input_userid: %s ===\n", __func__,input_devid,input_userid);
    int count =0;

    if(global_FingerPrintEntryList_head == NULL)
    {
        DEBUG_PRINTF("==[%s]=== global_FingerPrintEntryList is null ===\n",__func__);
        return 0;
    }

    struct FingerPrintEntry* tmp = global_FingerPrintEntryList_head;

    while(tmp!=NULL)
    {
        if( (strcmp(tmp->deviceid,input_devid)==0) && (strcmp(tmp->userid,input_userid)==0) )
        {
           strcpy(buf[count].fingerprintid, tmp->fingerprintid);
           strcpy(buf[count].name,tmp->name);
           strcpy(buf[count].deviceid,tmp->deviceid);
           strcpy(buf[count].userid,tmp->userid);
           count++;
        }
        tmp=tmp->root.next;
    }

    return count;
}

//保存log 到本地缓存和数据库
int savelog2local(char *input_time, int opratetype, char * input_usrid, char * devid)
{
    DEBUG_PRINTF("===[%s]===input_time:%s, opratetype:%d, input_usrid: %s, devid:%s ===\n",__func__,input_time,opratetype,input_usrid,devid);
    struct LogEntry *newentry = malloc_logEntry();
    strcpy(newentry->time,input_time);
    newentry->opratetype=opratetype;
    strcpy(newentry->userid, input_usrid);
    strcpy(newentry->devid,devid);

    return AddToLogEntryList(newentry);
}

//获取缓存的第一条logEntry
struct LogEntry *getFirstLog()
{
    DEBUG_PRINTF("===[%s]=====\n",__func__);
    return global_LogEntryList_head;
}

//删除内存里的第一条log
int DelFirstLog()
{
    DEBUG_PRINTF("===[%s]=====\n",__func__);
    return      RemoveFirstEntryFromLogEntryList();
}
