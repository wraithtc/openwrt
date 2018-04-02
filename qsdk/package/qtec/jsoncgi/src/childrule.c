#include "basic.h"

//check childrule mac conflict
static int check_childrule_add(char *mac)
{
    DEBUG_PRINTF("[%s]=======mac:%s====\n",__func__,mac);
    int ret=0;
    struct list_head *tmp=NULL;
    struct fw3_childrule *rule;
    
    ret=get_childruletable();
    tmp=global_childrules.next;
    if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return -1;
    }
    
    while(tmp!=&global_childrules)
    {
         
        rule = (struct fw3_childrule *)tmp;
  
        if(strcmp(mac,rule->src_mac) == 0 )
        {
            qtec_fw_free_list(&global_childrules);
            return -1;
        }

        tmp=tmp->next;
    }
    
    qtec_fw_free_list(&global_childrules);
    return 0;
    
}

int proc_childrule_add(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    char *name;
    char *mac;
	char *input_start_time;
    char *input_stop_time;
    char *input_weekdays;
    bool input_enabled;
    int ret;
    
    name = cJSON_GetObjectItem(jsonValue, "name")?cJSON_GetObjectItem(jsonValue, "name")->valuestring:"";
	mac = cJSON_GetObjectItem(jsonValue, "macaddr")?cJSON_GetObjectItem(jsonValue, "macaddr")->valuestring:"";
	input_start_time = cJSON_GetObjectItem(jsonValue, "start_time")?cJSON_GetObjectItem(jsonValue, "start_time")->valuestring:"";
    input_stop_time = cJSON_GetObjectItem(jsonValue, "stop_time")?cJSON_GetObjectItem(jsonValue, "stop_time")->valuestring:"";
    input_weekdays = cJSON_GetObjectItem(jsonValue, "weekdays")?cJSON_GetObjectItem(jsonValue, "weekdays")->valuestring:"";
    input_enabled= cJSON_GetObjectItem(jsonValue, "enabled")?cJSON_GetObjectItem(jsonValue, "enabled")->valueint:0;

    if(input_enabled==1)
    {
        if(  (strlen(mac)==0) || (strlen(input_start_time)==0) || (strlen(input_stop_time)==0) || (strlen(input_weekdays)==0) )
        {
        
            global_weberrorcode=ERR_PARAMETER_MISS;
            return ERR_PARAMETER_MISS;
        }
    }

    ret=check_childrule_add(mac);
    if(ret !=0 )
    {
        global_weberrorcode=ERR_RULE_CONFLICT;
        return ERR_RULE_CONFLICT;
    }
    
	ret=add_childrule(name,mac, input_start_time, input_stop_time,input_weekdays,input_enabled);

    if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
	return 0;
}

int proc_childrule_del(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]======\n",__func__);
	char *mac;
    char *name;
	char *input_start_time;
    char *input_stop_time;
    char *input_weekdays;
    bool input_enabled;
    int ret;

    name = cJSON_GetObjectItem(jsonValue, "name")?cJSON_GetObjectItem(jsonValue, "name")->valuestring:""; //????¡¤?¡À?D???
	mac = cJSON_GetObjectItem(jsonValue, "macaddr")?cJSON_GetObjectItem(jsonValue, "macaddr")->valuestring:"";
	input_start_time = cJSON_GetObjectItem(jsonValue, "start_time")?cJSON_GetObjectItem(jsonValue, "start_time")->valuestring:"";
    input_stop_time = cJSON_GetObjectItem(jsonValue, "stop_time")?cJSON_GetObjectItem(jsonValue, "stop_time")->valuestring:"";
    input_weekdays = cJSON_GetObjectItem(jsonValue, "weekdays")?cJSON_GetObjectItem(jsonValue, "weekdays")->valuestring:"";
    input_enabled= cJSON_GetObjectItem(jsonValue, "enabled")?cJSON_GetObjectItem(jsonValue, "enabled")->valueint:0;

    if(input_enabled == 1)
    {
        if(  (strlen(mac)==0) || (strlen(input_start_time)==0) || (strlen(input_stop_time)==0) || (strlen(input_weekdays)==0) )
        {
            global_weberrorcode=ERR_PARAMETER_MISS;
            return ERR_PARAMETER_MISS;
        }
    }

    ret=del_childrule(name, mac,input_start_time,input_stop_time,input_weekdays,input_enabled);

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

int proc_childrule_mod(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);
    char *old_name;
    char *old_mac;
    char *old_start_time;
    char *old_stop_time;
    char *old_weekdays;
    bool old_input_enabled;

    char *new_name;
    char *new_mac;
    char *new_start_time;
    char *new_stop_time;
    char *new_weekdays;
    bool new_input_enabled;
    int ret=0;

    old_name=cJSON_GetObjectItem(jsonValue, "oldname")?cJSON_GetObjectItem(jsonValue, "oldname")->valuestring:"";
    old_mac = cJSON_GetObjectItem(jsonValue, "oldmacaddr")?cJSON_GetObjectItem(jsonValue, "oldmacaddr")->valuestring:"";
	old_start_time= cJSON_GetObjectItem(jsonValue, "oldstarttime")?cJSON_GetObjectItem(jsonValue, "oldstarttime")->valuestring:"";
    old_stop_time= cJSON_GetObjectItem(jsonValue, "oldstoptime")?cJSON_GetObjectItem(jsonValue, "oldstoptime")->valuestring:"";
    old_weekdays= cJSON_GetObjectItem(jsonValue, "oldweekdays")?cJSON_GetObjectItem(jsonValue, "oldweekdays")->valuestring:"";
    old_input_enabled= cJSON_GetObjectItem(jsonValue, "oldenabled")?cJSON_GetObjectItem(jsonValue, "oldenabled")->valueint:0;

    new_name=cJSON_GetObjectItem(jsonValue, "newname")?cJSON_GetObjectItem(jsonValue, "newname")->valuestring:"";
    new_mac = cJSON_GetObjectItem(jsonValue, "newmacaddr")?cJSON_GetObjectItem(jsonValue, "newmacaddr")->valuestring:"";
    new_start_time= cJSON_GetObjectItem(jsonValue, "newstarttime")?cJSON_GetObjectItem(jsonValue, "newstarttime")->valuestring:"";
    new_stop_time= cJSON_GetObjectItem(jsonValue, "newstoptime")?cJSON_GetObjectItem(jsonValue, "newstoptime")->valuestring:"";
    new_weekdays= cJSON_GetObjectItem(jsonValue, "newweekdays")?cJSON_GetObjectItem(jsonValue, "newweekdays")->valuestring:"";
    new_input_enabled= cJSON_GetObjectItem(jsonValue, "newenabled")?cJSON_GetObjectItem(jsonValue, "newenabled")->valueint:0;

    if(new_input_enabled == 1)
    {
        if((strlen(new_mac)==0) || (strlen(new_start_time)==0) || (strlen(new_stop_time)==0) || (strlen(new_weekdays)==0))
        {
            global_weberrorcode=ERR_PARAMETER_MISS;
            return ERR_PARAMETER_MISS;
        }
    }
    ret=mod_childrule(old_name,old_mac,old_start_time,old_stop_time,old_weekdays,old_input_enabled,new_name,new_mac,new_start_time,new_stop_time,new_weekdays,new_input_enabled);

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

int proc_childrule_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    int ret=0;
    struct list_head *tmp=NULL;
    struct fw3_childrule *rule;
    cJSON *obj = NULL;
    cJSON *obj_entry = NULL;
   
    
    ret=get_childruletable();
    tmp=global_childrules.next;
    if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    
	obj = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonOut, "data", obj);

    while(tmp!=&global_childrules)
    {
         
        rule = (struct fw3_childrule *)tmp;
        DEBUG_PRINTF("[%s] ===tmp addr:%x====\n",__func__,tmp);
      
        obj_entry=cJSON_CreateObject();
        cJSON_AddItemToArray(obj, obj_entry);
        cJSON_AddItemToObject(obj_entry, "hostname", cJSON_CreateString(rule->name));
	    cJSON_AddItemToObject(obj_entry, "macaddr", cJSON_CreateString(rule->src_mac));
        cJSON_AddItemToObject(obj_entry, "starttime", cJSON_CreateString(rule->start_time));
        cJSON_AddItemToObject(obj_entry, "stoptime", cJSON_CreateString(rule->stop_time));
        cJSON_AddItemToObject(obj_entry, "weekdays", cJSON_CreateString(rule->weekdays));
        cJSON_AddItemToObject(obj_entry, "enabled", cJSON_CreateNumber(rule->enabled));
        tmp=tmp->next;
    }
    
    qtec_fw_free_list(&global_childrules);
    return 0;
}

int proc_childrule(cJSON *jsonValue, cJSON *jsonOut)
{
	int ret = 0;
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        ret = proc_childrule_get(jsonValue, jsonOut);
    }
    else if ( (request_method & CGI_POST_METHOD ) != 0 )
    {
        ret = proc_childrule_add(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	ret = proc_childrule_mod(jsonValue, jsonOut);
    }
	else if  ( (request_method & CGI_DELETE_METHOD ) != 0 )
    {
    	ret = proc_childrule_del(jsonValue, jsonOut);
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

