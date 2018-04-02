#include "basic.h"

void proc_ntp_set(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("===%s===\n",__func__);
    int ret=0;
    struct ntpConfig ntpNode={0};
    ntpNode.enable = cJSON_GetObjectItem(json_value, "enable")?cJSON_GetObjectItem(json_value, "enable")->valueint:0;
	strcpy(ntpNode.timezone, (cJSON_GetObjectItem(json_value, "timezone")?cJSON_GetObjectItem(json_value, "timezone")->valuestring:""));
	strcpy(ntpNode.ntpServers, (cJSON_GetObjectItem(json_value, "ntpservers")?cJSON_GetObjectItem(json_value, "ntpservers")->valuestring:""));
	ret=ntpConfigSet(&ntpNode);
    
    if(ret !=0)
    {
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));  
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("ntp set fail"));
        
    }
    else
    {
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
       
    }
}


void proc_ntp_get(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("===%s===\n",__func__);
    int ret=0;
    cJSON *obj = NULL;
    obj = cJSON_CreateObject();
    struct ntpConfig ntpNode={0};
    ret = ntpConfigGet(&ntpNode);
    
    cJSON_AddItemToObject(jsonOut, "data", obj);
	cJSON_AddItemToObject(obj,"enable",cJSON_CreateNumber(ntpNode.enable));
	cJSON_AddItemToObject(obj,"timezone",cJSON_CreateString(ntpNode.timezone));
	cJSON_AddItemToObject(obj,"ntpservers",cJSON_CreateString(ntpNode.ntpServers));
    cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
}