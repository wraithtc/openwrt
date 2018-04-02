#include "basic.h"

int proc_speedlimit_mod(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);
    char *mac;
    char *ip;
    int old_downlimit;
    int old_uplimit;
    bool old_enabled;
    int new_downlimit;
    int new_uplimit;
    bool new_enabled;
    int tmp_enabled;//?a??2?那y?迆?a??o‘那y角??Y那㊣??車D辰a辰?㏒?∩?那?足?2?
    int ret=0;
    int cur_downlimit=0;
    int cur_uplimit=0;
    mac = cJSON_GetObjectItem(jsonValue, "mac")?cJSON_GetObjectItem(jsonValue, "mac")->valuestring:"";
    ip = cJSON_GetObjectItem(jsonValue, "ip")?cJSON_GetObjectItem(jsonValue, "ip")->valuestring:"";
    old_downlimit= cJSON_GetObjectItem(jsonValue, "olddownlimit")?cJSON_GetObjectItem(jsonValue, "olddownlimit")->valueint:0;
    old_uplimit= cJSON_GetObjectItem(jsonValue, "olduplimit")?cJSON_GetObjectItem(jsonValue, "olduplimit")->valueint:0;
    old_enabled= cJSON_GetObjectItem(jsonValue, "oldenabled")?cJSON_GetObjectItem(jsonValue, "oldenabled")->valueint:0;
     
    new_downlimit= cJSON_GetObjectItem(jsonValue, "newdownlimit")?cJSON_GetObjectItem(jsonValue, "newdownlimit")->valueint:0;
    new_uplimit= cJSON_GetObjectItem(jsonValue, "newuplimit")?cJSON_GetObjectItem(jsonValue, "newuplimit")->valueint:0;
    new_enabled= cJSON_GetObjectItem(jsonValue, "newenabled")?cJSON_GetObjectItem(jsonValue, "newenabled")->valueint:0;

    DEBUG_PRINTF("[%s]===mac:%s ip:%s ====\n",__func__,mac,ip);
    DEBUG_PRINTF("[%s]===old_downlimit:%d old_uplimit:%d old_enabled:%d====\n",__func__,old_downlimit,old_uplimit,old_enabled);
    DEBUG_PRINTF("[%s]===new_downlimit:%d new_uplimit:%d new_enabled:%d====\n",__func__,new_downlimit,new_uplimit,new_enabled);
    if( (strlen(mac)==0) || (strlen(ip)==0)   )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return ERR_PARAMETER_MISS;
    }
    cur_downlimit=get_downlimit(mac,&tmp_enabled);
    cur_uplimit=get_uplimit(mac,&tmp_enabled);

    if( (cur_downlimit!=old_downlimit) || (cur_uplimit !=old_uplimit) )
    {
        global_weberrorcode=ERR_VALUE_WRONG;
        return ERR_VALUE_WRONG;
    }

    if(old_downlimit==0)
    {
        if(new_downlimit==0)
        {
            DEBUG_PRINTF("[%s]===downlimit do nothing====\n",__func__);
        }
        else
        {
            add_downlimit_rule(mac,ip,new_downlimit,new_enabled);
        }
    }
    else
    {
        if(new_downlimit==0)
        {
            ret=del_downlimit_rule(mac,ip,old_downlimit,old_enabled);
        }
        else
        {
            ret=mod_downlimit_rule(mac,ip,old_downlimit,old_enabled,new_downlimit,new_enabled);
        }
    }

    if(ret==-1)
    {
        global_weberrorcode=ERR_NO_MATCH;
        return ERR_NO_MATCH;
    }
    else if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }

    if(old_uplimit==0)
    {
        if(new_uplimit==0)
        {
            DEBUG_PRINTF("[%s]===uplimit do nothing====\n",__func__);
        }
        else
        {
            add_uplimit_rule(mac,new_uplimit,new_enabled);
        }
    }
    else
    {
        if(new_uplimit==0)
        {
            ret=del_uplimit_rule(mac,old_uplimit,old_enabled);
        }
        else
        {
            ret=mod_uplimit_rule(mac,old_uplimit,old_enabled,new_uplimit,new_enabled);
        }
    }

    if(ret==-1)
    {
        global_weberrorcode=ERR_NO_MATCH;
        return ERR_NO_MATCH;
    }
    else if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    
    return 0;
    
}

int proc_speedlimit_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]2====\n",__func__);
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
    ip = cJSON_GetObjectItem(jsonValue, "ip")?cJSON_GetObjectItem(jsonValue, "ip")->valuestring:"";
    DEBUG_PRINTF("[%s]===mac:%s ip:%s ====\n",__func__,mac,ip);
    if( (strlen(mac)==0) || (strlen(ip)==0)   )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return ERR_PARAMETER_MISS;
    }
    downlimit=get_downlimit(mac,&down_enabled);
    uplimit=get_uplimit(mac,&up_enabled);

    enabled= (down_enabled | up_enabled);

    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "downlimit", cJSON_CreateNumber(downlimit));
    cJSON_AddItemToObject(obj, "uplimit", cJSON_CreateNumber(uplimit));
    cJSON_AddItemToObject(obj, "enabled", cJSON_CreateNumber(enabled));
    
    return 0;
}

int proc_speedlimit(cJSON *jsonValue, cJSON *jsonOut)
{
	int ret = 0;
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        ret = proc_speedlimit_get(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	ret = proc_speedlimit_mod(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return ERR_METHOD_NOT_SUPPORT;
    }

    if( ((request_method & CGI_GET_METHOD) == 0) && (global_weberrorcode==0) )
    {
        restart_fw(0);
    }
	return ret;
    
}

