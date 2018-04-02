#include "basic.h"

void ebtables_proc_speedlimit_mod(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);
    char *mac;
    
    int old_downlimit;
    int old_uplimit;
    bool old_enabled;
    int new_downlimit;
    int new_uplimit;
    bool new_enabled;
    int tmp_enabled;
    int ret=0;
    int cur_downlimit=0;
    int cur_uplimit=0;
    mac = cJSON_GetObjectItem(jsonValue, "mac")?cJSON_GetObjectItem(jsonValue, "mac")->valuestring:"";
    
    old_downlimit= cJSON_GetObjectItem(jsonValue, "olddownlimit")?cJSON_GetObjectItem(jsonValue, "olddownlimit")->valueint:0;
    old_uplimit= cJSON_GetObjectItem(jsonValue, "olduplimit")?cJSON_GetObjectItem(jsonValue, "olduplimit")->valueint:0;
    old_enabled= cJSON_GetObjectItem(jsonValue, "oldenabled")?cJSON_GetObjectItem(jsonValue, "oldenabled")->valueint:0;
     
    new_downlimit= cJSON_GetObjectItem(jsonValue, "newdownlimit")?cJSON_GetObjectItem(jsonValue, "newdownlimit")->valueint:0;
    new_uplimit= cJSON_GetObjectItem(jsonValue, "newuplimit")?cJSON_GetObjectItem(jsonValue, "newuplimit")->valueint:0;
    new_enabled= cJSON_GetObjectItem(jsonValue, "newenabled")?cJSON_GetObjectItem(jsonValue, "newenabled")->valueint:0;

    DEBUG_PRINTF("[%s]===mac:%s  ====\n",__func__,mac);
    DEBUG_PRINTF("[%s]===old_downlimit:%d old_uplimit:%d old_enabled:%d====\n",__func__,old_downlimit,old_uplimit,old_enabled);
    DEBUG_PRINTF("[%s]===new_downlimit:%d new_uplimit:%d new_enabled:%d====\n",__func__,new_downlimit,new_uplimit,new_enabled);
    if( strlen(mac)==0   )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }
    char hostname_1[64]={0};
    char hostname_2[64]={0};
    cur_downlimit=ebtables_get_downlimit(mac,hostname_1,&tmp_enabled);
    cur_uplimit=ebtables_get_uplimit(mac,hostname_2,&tmp_enabled);

    if( (cur_downlimit!=old_downlimit) || (cur_uplimit !=old_uplimit) )
    {
        global_weberrorcode=ERR_VALUE_WRONG;
        return;
    }

    if(old_downlimit==0)
    {
        if(new_downlimit==0)
        {
            DEBUG_PRINTF("[%s]===downlimit do nothing====\n",__func__);
        }
        else
        {
            ebtables_add_downlimit_rule(mac,new_downlimit,new_enabled);
        }
    }
    else
    {
        if(new_downlimit==0)
        {
            ret=ebtables_del_downlimit_rule(mac,old_downlimit,old_enabled);
        }
        else
        {
            ret=ebtables_mod_downlimit_rule(mac,old_downlimit,old_enabled,new_downlimit,new_enabled);
        }
    }

    if(ret==-1)
    {
        global_weberrorcode=ERR_NO_MATCH;
        return;
    }
    else if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }

    if(old_uplimit==0)
    {
        if(new_uplimit==0)
        {
            DEBUG_PRINTF("[%s]===uplimit do nothing====\n",__func__);
        }
        else
        {
            ebtables_add_uplimit_rule(mac,new_uplimit,new_enabled);
        }
    }
    else
    {
        if(new_uplimit==0)
        {
            ret=ebtables_del_uplimit_rule(mac,old_uplimit,old_enabled);
        }
        else
        {
            ret=ebtables_mod_uplimit_rule(mac,old_uplimit,old_enabled,new_uplimit,new_enabled);
        }
    }

    if(ret==-1)
    {
        global_weberrorcode=ERR_NO_MATCH;
        return;
    }
    else if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }
    
    return;
    
}

void ebtables_proc_speedlimit_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    int ret=0;
    char *mac;
    char *ip;
    int downlimit=0;
    int uplimit=0;
    int enabled=0;
    int down_enabled=0;
    int up_enabled=0;
    cJSON *obj = NULL;
    mac = cJSON_GetObjectItem(jsonValue, "mac")?cJSON_GetObjectItem(jsonValue, "mac")->valuestring:"";
    DEBUG_PRINTF("[%s]===mac:%s ====\n",__func__,mac);
    if( strlen(mac)==0 )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }
	char hostname_1[64]={0};
    char hostname_2[64]={0};
    downlimit=ebtables_get_downlimit(mac,hostname_1,&down_enabled);
    uplimit=ebtables_get_uplimit(mac,hostname_2,&up_enabled);
 

    enabled= (down_enabled | up_enabled);

    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "hostname", cJSON_CreateString(hostname_2));
    cJSON_AddItemToObject(obj, "downlimit", cJSON_CreateNumber(downlimit));
    cJSON_AddItemToObject(obj, "uplimit", cJSON_CreateNumber(uplimit));
    cJSON_AddItemToObject(obj, "enabled", cJSON_CreateNumber(enabled));
   
    
    return;
}

void ebtables_proc_speedlimit(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        ebtables_proc_speedlimit_get(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	ebtables_proc_speedlimit_mod(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return;
    }

    if( ((request_method & CGI_GET_METHOD) == 0) && (global_weberrorcode==0) )
    {
        restart_ebtables();
    }
    
}

