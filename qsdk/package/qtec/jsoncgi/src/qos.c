#include "basic.h"

static int max_int_qos= 9999999;
static int invaild_value_qos=-99;

void proc_qos_set(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("===%s===\n",__func__);
    int ret=0;
    int qosmode=0;
    qosmode = cJSON_GetObjectItem(json_value, "qosmode")?cJSON_GetObjectItem(json_value, "qosmode")->valueint:0;

    bool enabled;
    int upload;
    int download;
    int tmp_enabled;
    char tmp_char[16];
  
    download= cJSON_GetObjectItem(json_value, "download")?cJSON_GetObjectItem(json_value, "download")->valueint:0;
    upload= cJSON_GetObjectItem(json_value, "upload")?cJSON_GetObjectItem(json_value, "upload")->valueint:0;
    tmp_enabled= cJSON_GetObjectItem(json_value, "enabled")?cJSON_GetObjectItem(json_value, "enabled")->valueint:0;

    DEBUG_PRINTF("[%s]===enabled:%d upload:%d download:%d ====\n",__func__,enabled,upload,download);

    if( (download < 0) || (upload <0) )
    {
        global_weberrorcode= ERR_VALUE_WRONG;
        return;
    }

    if(qosmode != invaild_value_qos)
    {
	    ret=Set_qosmode(&qosmode,0);
    
        if(ret!=0)
        {
            global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
            return;
        }
    }

    if(tmp_enabled == invaild_value_qos)
    {
        rtcfgUciGet("qos.wan.enabled",tmp_char);
        enabled = atoi(tmp_char);

    }
    else
        enabled = tmp_enabled;
    
    //max_value is 1000M and 0 stand for no limit
    if( (download==0) || (download > (1000*1024)) )
    {
        download= max_int_qos;
    }

    if( (upload==0) || (upload>(1000*1024)) )
    {
        upload=max_int_qos;
    }
    
    
    ret= set_wanbandwidth_config(enabled, upload, download,1);
    return;
}


void proc_qos_get(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("===%s===\n",__func__);
    int ret=0;
    cJSON *obj = NULL;
    obj = cJSON_CreateObject();
    
    ret = Get_qosmode();

    bool enabled=0;
    int upload=0;
    int download=0;
    
    get_wanbandwidth_config(&enabled, &upload, &download);
   
    if(upload == max_int_qos)
    {
        upload=0;
    }

    if(download == max_int_qos)
    {
        download=0;
    }
    
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "enabled", cJSON_CreateNumber(enabled));
    cJSON_AddItemToObject(obj, "download", cJSON_CreateNumber(download));
    cJSON_AddItemToObject(obj, "upload", cJSON_CreateNumber(upload));
    
	cJSON_AddItemToObject(obj,"qosmode",cJSON_CreateNumber(ret));
}

int proc_qos(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_qos_get(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	proc_qos_set(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return ERR_METHOD_NOT_SUPPORT;
    }
	return 0;  
}