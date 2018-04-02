#include "basic.h"

void proc_ddos_set(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    bool input_enabled;
    int ret=0;    
    input_enabled= cJSON_GetObjectItem(jsonValue, "enabled")?cJSON_GetObjectItem(jsonValue, "enabled")->valueint:0;

    ret=set_ddos(input_enabled);

    if(ret != 0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }
    return;
}

void proc_ddos_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    cJSON *obj = NULL;
	obj = cJSON_CreateObject();
    bool enabled=0;
    get_ddos(&enabled);
    cJSON_AddItemToObject(jsonOut, "data", obj);
 
    cJSON_AddItemToObject(obj, "enabled", cJSON_CreateNumber(enabled));
    return;
    	
}

void proc_ddos(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_ddos_get(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	proc_ddos_set(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return;
    }

 
}
