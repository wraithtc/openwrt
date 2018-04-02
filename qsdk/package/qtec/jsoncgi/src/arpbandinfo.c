#include "basic.h"

#if 0
void proc_arpband_set(cJSON *jsonValue,cJSON *jsonOut)
{
	char *mac;
	char *ip;

	mac = cJSON_GetObjectItem(jsonValue, "macaddr")?cJSON_GetObjectItem(jsonValue, "macaddr")->valuestring:"";
	ip = cJSON_GetObjectItem(jsonValue, "ipaddr")?cJSON_GetObjectItem(jsonValue, "ipaddr")->valuestring:"";
	setarpband(mac, ip);
	return;
}

void proc_arpband_del(cJSON *jsonValue,cJSON *jsonOut)
{
	char *mac;
	char *ip;

	mac = cJSON_GetObjectItem(jsonValue, "macaddr")?cJSON_GetObjectItem(jsonValue, "macaddr")->valuestring:"";
	ip = cJSON_GetObjectItem(jsonValue, "ipaddr")?cJSON_GetObjectItem(jsonValue, "ipaddr")->valuestring:"";
	delarpband(mac, ip);
	return;

}

void proc_arpband_clear(cJSON *jsonValue,cJSON *jsonOut)
{
	cleararpband();
	return;
}

void proc_arpband_get(cJSON *jsonValue,cJSON *jsonOut)
{
	char buf[1024];
	cJSON *list;
	getarpband(buf, 1024);
	list = cJSON_Parse(buf);
	cJSON_AddItemToObject(jsonOut, "data", list);
	return;
}

void proc_arpband(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_arpband_get(jsonValue, jsonOut);
    }
    else if ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
        proc_arpband_set(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PATCH_METHOD ) != 0 )
    {
    	proc_arpband_del(jsonValue, jsonOut);
    }
	else if  ( (request_method & CGI_DELETE_METHOD ) != 0 )
    {
    	proc_arpband_clear(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
    }
}
#endif 

void proc_arpband_add(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
	char *mac;
	char *ip;
    char *enabled;
    bool input_enabled;
    int ret;

	mac = cJSON_GetObjectItem(jsonValue, "macaddr")?cJSON_GetObjectItem(jsonValue, "macaddr")->valuestring:"";
	ip = cJSON_GetObjectItem(jsonValue, "ipaddr")?cJSON_GetObjectItem(jsonValue, "ipaddr")->valuestring:"";
    input_enabled= cJSON_GetObjectItem(jsonValue, "enabled")?cJSON_GetObjectItem(jsonValue, "enabled")->valueint:0;

    if( (strlen(mac)==0) || (strlen(ip)==0) )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }

    
	ret=add_arpbound(mac, ip, input_enabled);

    if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }
	return;
}

void proc_arpband_del(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]======\n",__func__);
    char *mac;
    char *ip;

    bool input_enabled;
    int ret=0;

    mac = cJSON_GetObjectItem(jsonValue, "macaddr")?cJSON_GetObjectItem(jsonValue, "macaddr")->valuestring:"";
	ip = cJSON_GetObjectItem(jsonValue, "ipaddr")?cJSON_GetObjectItem(jsonValue, "ipaddr")->valuestring:"";
   
    input_enabled= cJSON_GetObjectItem(jsonValue, "enabled")?cJSON_GetObjectItem(jsonValue, "enabled")->valueint:0;

    if( (strlen(mac)==0) || (strlen(ip)==0)  )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }

    ret=del_arpbound(mac,ip,input_enabled);

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

void proc_arpband_mod(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);
    char *old_mac;
    char *old_ip;

    bool old_input_enabled;

    char *new_mac;
    char *new_ip;
  
    bool new_input_enabled;
    int ret=0;

    old_mac = cJSON_GetObjectItem(jsonValue, "oldmacaddr")?cJSON_GetObjectItem(jsonValue, "oldmacaddr")->valuestring:"";
	old_ip = cJSON_GetObjectItem(jsonValue, "oldipaddr")?cJSON_GetObjectItem(jsonValue, "oldipaddr")->valuestring:"";
    old_input_enabled= cJSON_GetObjectItem(jsonValue, "oldenabled")?cJSON_GetObjectItem(jsonValue, "oldenabled")->valueint:0;
     
    new_mac = cJSON_GetObjectItem(jsonValue, "newmacaddr")?cJSON_GetObjectItem(jsonValue, "newmacaddr")->valuestring:"";
	new_ip = cJSON_GetObjectItem(jsonValue, "newipaddr")?cJSON_GetObjectItem(jsonValue, "newipaddr")->valuestring:"";
    new_input_enabled= cJSON_GetObjectItem(jsonValue, "newenabled")?cJSON_GetObjectItem(jsonValue, "newenabled")->valueint:0;

    
    if( (strlen(old_mac)==0) || (strlen(old_ip)==0)  || (strlen(new_mac)==0) || (strlen(new_ip)==0)  )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }

    ret=mod_arpbound(old_mac,old_ip,old_input_enabled,new_mac,new_ip,new_input_enabled);

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

void proc_arpband_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    int ret=0;
    struct list_head *tmp=NULL;
    struct fw3_arpbound_rule *rule;
    cJSON *obj = NULL;
    cJSON *obj_entry = NULL;
   
    
    ret=get_arpboundtable();
    tmp=global_arpbound_rules.next;
    if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }
    
	obj = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonOut, "data", obj);

    while(tmp!=&global_arpbound_rules)
    {
         
        rule = (struct fw3_arpbound_rule *)tmp;
        DEBUG_PRINTF("[%s] ===tmp addr:%x====\n",__func__,tmp);
        printf_arpbound_rule(rule);
        obj_entry=cJSON_CreateObject();
        cJSON_AddItemToArray(obj, obj_entry);
        cJSON_AddItemToObject(obj_entry, "ipaddr", cJSON_CreateString(rule->src_ip));
	    cJSON_AddItemToObject(obj_entry, "macaddr", cJSON_CreateString(rule->src_mac));
        cJSON_AddItemToObject(obj_entry, "enabled", cJSON_CreateNumber(rule->enabled));
        tmp=tmp->next;
    }
    
}

void proc_arpband(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_arpband_get(jsonValue, jsonOut);
    }
    else if ( (request_method & CGI_POST_METHOD ) != 0 )
    {
        proc_arpband_add(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	proc_arpband_mod(jsonValue, jsonOut);
    }
	else if  ( (request_method & CGI_DELETE_METHOD ) != 0 )
    {
    	proc_arpband_del(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return;
    }

    if( ((request_method & CGI_GET_METHOD) == 0) && (global_weberrorcode==0) )
    {
        restart_fw(0);
    }
    
}

