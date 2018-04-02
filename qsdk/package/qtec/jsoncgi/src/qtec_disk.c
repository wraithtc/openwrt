#include "basic.h"

int proc_qtec_disk_post(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);
    return ProcDiskReformatReq();
    
    
}

int proc_qtec_disk_check(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);
    return ProcDiskCheckReq();
}

int proc_qtec_disk_umount(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    cJSON *obj = NULL;
    int ret=0;
    char tmp[256]={0};
    #if 0
    FILE *pp = popen("umount /dev/mapper/qtec_disk;mount | grep qtec_disk > /tmp/qtec_umount.txt","r");
    if(!pp)
    {
        DEBUG_PRINTF("[%s]===popen fail=====\n",__func__);
        
    }
    else
    {
        pclose(pp);
    }
    #endif
    system("umount /dev/mapper/qtec_disk;mount | grep qtec_disk > qtec_umount");
    
    FILE *fp = fopen("qtec_umount","r");
    if(!fp)
    {
         DEBUG_PRINTF("[%s]====fopen fail====\n",__func__);
    }
    else
    {
       
        DEBUG_PRINTF("[%s]=====fopen success====\n",__func__);
        DEBUG_PRINTF("[%s]===fp:%d===\n",__func__,fp);
        fgets(tmp,sizeof(tmp),fp);
        DEBUG_PRINTF("[%s]===tmp is %s===\n",__func__,tmp);
        fclose(fp);
    }

    DEBUG_PRINTF("[%s]===tmp is %s====\n",__func__,tmp);

    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    if(strlen(tmp)==0)
    {
        ret=0;
        cJSON_AddItemToObject(obj, "result_message",cJSON_CreateString("umount success"));
    }
    else
    {
        ret=1;
        cJSON_AddItemToObject(obj, "result_message",cJSON_CreateString("umount fail, please check whether disk is use"));
    }
    

    cJSON_AddItemToObject(obj, "result_code", cJSON_CreateNumber(ret));
      
    return 0;
}

int proc_qtec_disk_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    int ret=0;
    char tmp_char[256]={0};
    int status_errorcode=0;
    char status_message[256]={0};
    int process_statuscode=0;
    char process_message[256]={0};
    
    cJSON *obj = NULL;
    
    rtcfgUciGet("qtec_disk.status.status_errorcode",tmp_char);
    status_errorcode=atoi(tmp_char);

    rtcfgUciGet("qtec_disk.status.status_message",status_message);

    memset(tmp_char,0,256);
    rtcfgUciGet("qtec_disk.process.process_statuscode",tmp_char);
    process_statuscode=atoi(tmp_char);

    rtcfgUciGet("qtec_disk.process.process_message",process_message);
    
    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "status_errorcode", cJSON_CreateNumber(status_errorcode));
    cJSON_AddItemToObject(obj, "status_message", cJSON_CreateString(status_message));
    cJSON_AddItemToObject(obj, "process_statuscode", cJSON_CreateNumber(process_statuscode));
    cJSON_AddItemToObject(obj, "process_message", cJSON_CreateString(process_message));  
    if(status_errorcode==0 && process_statuscode==0)
    {
        char tmp[64]={0};
        char disk_used[64]={0};
        char disk_size[64]={0};
        char disk_capacity[64]={0};
        FILE *pp=popen("df -Ph | grep ^/dev/mapper/qtec_disk | awk '{print $3,$4,$5}'","r");
        if(!pp)
        {
            DEBUG_PRINTF("[%s]====fail popen===\n",__func__);
        }
        else
        {
            fgets(tmp,sizeof(tmp),pp);
            DEBUG_PRINTF("[%s]  tmp is %s",__func__,tmp);
            sscanf(tmp,"%s %s %s",disk_used,disk_size,disk_capacity);
            cJSON_AddItemToObject(obj, "disk_size", cJSON_CreateString(disk_size));
            cJSON_AddItemToObject(obj, "disk_used", cJSON_CreateString(disk_used));
            cJSON_AddItemToObject(obj, "disk_capacity", cJSON_CreateString(disk_capacity));
            pclose(pp);
        }
    }
    return 0;
}

int proc_qtec_disk(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        return proc_qtec_disk_get(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_POST_METHOD ) != 0 )
    {
    	return proc_qtec_disk_post(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	return proc_qtec_disk_check(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_DELETE_METHOD) != 0 )
    {
        return proc_qtec_disk_umount(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return ERR_METHOD_NOT_SUPPORT;
    }

    
}

