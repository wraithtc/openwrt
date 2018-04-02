#include "basic.h"

void proc_dmz_set(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    bool input_enabled;
    int ret=0;
    char *input_destip;
    input_destip = cJSON_GetObjectItem(jsonValue, "destip")?cJSON_GetObjectItem(jsonValue, "destip")->valuestring:"";
    input_enabled= cJSON_GetObjectItem(jsonValue, "enabled")?cJSON_GetObjectItem(jsonValue, "enabled")->valueint:0;

    if(strlen(input_destip)==0)
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }
    
    ret=set_dmz_rule(input_destip,input_enabled);

    if(ret != 0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }
    return;
}

void proc_dmz_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    cJSON *obj = NULL;
	obj = cJSON_CreateObject();
    struct fw3_dmz tmp_dmz={0};
    int ret=get_dmz_rule(&tmp_dmz);
    
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "enabled", cJSON_CreateNumber(tmp_dmz.enabled));
    cJSON_AddItemToObject(obj, "destip", cJSON_CreateString(tmp_dmz.dest_ip));
    
    return;
    	
}

void proc_dmz(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_dmz_get(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	proc_dmz_set(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return;
    }

 
}

