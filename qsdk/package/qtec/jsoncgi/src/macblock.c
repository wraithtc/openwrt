#include "basic.h"

//check childrule mac conflict
int check_macblock_add(char *mac)
{
    DEBUG_PRINTF("[%s]=======mac:%s====\n",__func__,mac);
    int ret=0;
    struct list_head *tmp=NULL;
    struct fw3_macblock_rule *rule;
    
    ret=get_macblocktable();
    tmp=global_macblock_rules.next;
    if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return -1;
    }
    
    while(tmp!=&global_macblock_rules)
    {
         
        rule = (struct fw3_macblock_rule *)tmp;
  
        if(strcmp(mac,rule->src_mac) == 0 )
        {
            qtec_fw_free_list(&global_macblock_rules);
            return -1;
        }

        tmp=tmp->next;
    }
    
    qtec_fw_free_list(&global_macblock_rules);
    return 0;
    
}


int proc_macblock_add(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
	char *mac;
    char *enabled;
	char *name;
    bool input_enabled;
    int ret;


	mac = cJSON_GetObjectItem(jsonValue, "macaddr")?cJSON_GetObjectItem(jsonValue, "macaddr")->valuestring:"";
	name = cJSON_GetObjectItem(jsonValue, "name")?cJSON_GetObjectItem(jsonValue, "name")->valuestring:"";
	
    input_enabled= cJSON_GetObjectItem(jsonValue, "enabled")?cJSON_GetObjectItem(jsonValue, "enabled")->valueint:0;

    if( (strlen(mac)==0) )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return ERR_PARAMETER_MISS;
    }

    ret=check_macblock_add(mac);

    if(ret!=0)
    {
        global_weberrorcode=ERR_RULE_CONFLICT;
        return ERR_RULE_CONFLICT;
    }
    
	ret=add_macblock(name,mac, input_enabled);

    if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
	return 0;
}

int proc_macblock_del(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]======\n",__func__);
    char *mac;
	char *name;

    bool input_enabled;
    int ret=0;

    mac = cJSON_GetObjectItem(jsonValue, "macaddr")?cJSON_GetObjectItem(jsonValue, "macaddr")->valuestring:"";
	name = cJSON_GetObjectItem(jsonValue, "name")?cJSON_GetObjectItem(jsonValue, "name")->valuestring:"";
    input_enabled= cJSON_GetObjectItem(jsonValue, "enabled")?cJSON_GetObjectItem(jsonValue, "enabled")->valueint:0;

    if( (strlen(mac)==0)  )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return ERR_PARAMETER_MISS;
    }

    ret=del_macblock(name,mac,input_enabled);

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

int proc_macblock_mod(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);
	char *name;
    char *old_mac;
    bool old_input_enabled;

    char *new_mac;
    bool new_input_enabled;
    int ret=0;
	
	name = cJSON_GetObjectItem(jsonValue, "name")?cJSON_GetObjectItem(jsonValue, "name")->valuestring:"";
    old_mac = cJSON_GetObjectItem(jsonValue, "oldmacaddr")?cJSON_GetObjectItem(jsonValue, "oldmacaddr")->valuestring:"";
    old_input_enabled= cJSON_GetObjectItem(jsonValue, "oldenabled")?cJSON_GetObjectItem(jsonValue, "oldenabled")->valueint:0;
     
    new_mac = cJSON_GetObjectItem(jsonValue, "newmacaddr")?cJSON_GetObjectItem(jsonValue, "newmacaddr")->valuestring:"";
    new_input_enabled= cJSON_GetObjectItem(jsonValue, "newenabled")?cJSON_GetObjectItem(jsonValue, "newenabled")->valueint:0;

    
    if( (strlen(old_mac)==0) || (strlen(new_mac)==0)   )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return ERR_PARAMETER_MISS;
    }

    ret=mod_macblock(name, old_mac,old_input_enabled,new_mac,new_input_enabled);

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

int proc_macblock_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    int ret=0;
    struct list_head *tmp=NULL;
    struct fw3_macblock_rule *rule;
    cJSON *obj = NULL;
    cJSON *obj_entry = NULL;
   
    
    ret=get_macblocktable();
    tmp=global_macblock_rules.next;
    if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    
	obj = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonOut, "data", obj);

    while(tmp!=&global_macblock_rules)
    {
         
        rule = (struct fw3_macblock_rule *)tmp;
        DEBUG_PRINTF("[%s] ===tmp addr:%x====\n",__func__,tmp);
        
        obj_entry=cJSON_CreateObject();
        cJSON_AddItemToArray(obj, obj_entry);
		cJSON_AddItemToObject(obj_entry, "name", cJSON_CreateString(rule->name));
	    cJSON_AddItemToObject(obj_entry, "macaddr", cJSON_CreateString(rule->src_mac));
        cJSON_AddItemToObject(obj_entry, "enabled", cJSON_CreateNumber(rule->enabled));
        tmp=tmp->next;
    }
    return 0;
}

int proc_macblock(cJSON *jsonValue, cJSON *jsonOut)
{
	int ret = 0;
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        ret = proc_macblock_get(jsonValue, jsonOut);
    }
    else if ( (request_method & CGI_POST_METHOD ) != 0 )
    {
        ret = proc_macblock_add(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	ret = proc_macblock_mod(jsonValue, jsonOut);
    }
	else if  ( (request_method & CGI_DELETE_METHOD ) != 0 )
    {
    	ret = proc_macblock_del(jsonValue, jsonOut);
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

