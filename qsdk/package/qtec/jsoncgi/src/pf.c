#include "basic.h"

static int check_inter_ip(char *ip)
{
    DEBUG_PRINTF("====[%s]======ip:%s=======\n",__func__,ip);
    char cmd[64]={0};
    char lanip[64]={0};
    char lannetmask[64]={0};
    ulong l1,l2,l3;
    
    memset(cmd,0,64);
    snprintf(cmd,64,"network.lan.ipaddr");
    rtcfgUciGet(cmd,lanip);

    memset(cmd,0,64);
    snprintf(cmd,64,"network.lan.netmask");
    rtcfgUciGet(cmd,lannetmask);

    DEBUG_PRINTF("===[%s]====lanip: %s=== lannetmask: %s== devip:%s == \n",__func__,lanip,lannetmask,ip);
	l1=inet_addr(lanip);
	l2=inet_addr(ip);
	l3=inet_addr(lannetmask);

	if((l1&l3) != (l2&l3) )
	{
		DEBUG_PRINTF("[%s]====not in the same subnet with lan ip====\n",__func__);
		return -1;
	}

    return 0;    
}

void proc_pf_add(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("===%s===\n",__func__);
    char *input_name;
    char *input_proto;
    char *input_src_dport;
    char *input_dest_ip;
    char *input_dest_port;
    bool input_enabled;
    int ret=0;
    input_enabled = cJSON_GetObjectItem(json_value, "enabled")?cJSON_GetObjectItem(json_value, "enabled")->valueint:0;
	input_name= (cJSON_GetObjectItem(json_value, "name")?cJSON_GetObjectItem(json_value, "name")->valuestring:"");
	input_proto=(cJSON_GetObjectItem(json_value, "proto")?cJSON_GetObjectItem(json_value, "proto")->valuestring:"");
	input_src_dport=(cJSON_GetObjectItem(json_value, "srcdport")?cJSON_GetObjectItem(json_value, "srcdport")->valuestring:"");
    input_dest_ip=(cJSON_GetObjectItem(json_value, "destip")?cJSON_GetObjectItem(json_value, "destip")->valuestring:"");
    input_dest_port=(cJSON_GetObjectItem(json_value, "destport")?cJSON_GetObjectItem(json_value, "destport")->valuestring:"");
    if( (strlen(input_name)==0) || (strlen(input_proto)==0) || (strlen(input_src_dport)==0) || (strlen(input_dest_ip)==0) || (strlen(input_dest_port)==0) )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }

    ret=check_inter_ip(input_dest_ip);
    if(ret !=0 )
    {
        global_weberrorcode=ERR_IP_NOT_INTER;
        return;
    }
    
    ret=add_pf(input_name,input_proto,input_src_dport,input_dest_ip,input_dest_port,input_enabled);
    
    if(ret !=0)
    {
        global_weberrorcode=ERR_RULE_CONFLICT;
        return;
        
    }

    return;
}

void proc_pf_del(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("===%s===\n",__func__);
    char *input_name;
    char *input_proto;
    char *input_src_dport;
    char *input_dest_ip;
    char *input_dest_port;
    bool input_enabled;
    int ret=0;
    input_enabled = cJSON_GetObjectItem(json_value, "enabled")?cJSON_GetObjectItem(json_value, "enabled")->valueint:0;
	input_name= (cJSON_GetObjectItem(json_value, "name")?cJSON_GetObjectItem(json_value, "name")->valuestring:"");
	input_proto=(cJSON_GetObjectItem(json_value, "proto")?cJSON_GetObjectItem(json_value, "proto")->valuestring:"");
	input_src_dport=(cJSON_GetObjectItem(json_value, "srcdport")?cJSON_GetObjectItem(json_value, "srcdport")->valuestring:"");
    input_dest_ip=(cJSON_GetObjectItem(json_value, "destip")?cJSON_GetObjectItem(json_value, "destip")->valuestring:"");
    input_dest_port=(cJSON_GetObjectItem(json_value, "destport")?cJSON_GetObjectItem(json_value, "destport")->valuestring:"");
    if( (strlen(input_name)==0) || (strlen(input_proto)==0) || (strlen(input_src_dport)==0) || (strlen(input_dest_ip)==0) || (strlen(input_dest_port)==0) )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }

#if 0
    ret=check_inter_ip(input_dest_ip);
    if(ret !=0 )
    {
        global_weberrorcode=ERR_IP_NOT_INTER;
        return;
    }
#endif
    ret=del_pf(input_name,input_proto,input_src_dport,input_dest_ip,input_dest_port,input_enabled);

    if(ret==-1)
    {
        global_weberrorcode=ERR_NO_MATCH;
        return;
    }
    else if(ret !=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
        
    }

    return;
}

void proc_pf_mod(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("===%s===\n",__func__);
    char *input_name;
    char *input_proto;
    char *input_src_dport;
    char *input_dest_ip;
    char *input_dest_port;
    bool input_enabled;
    char *new_input_name;
    char *new_input_proto;
    char *new_input_src_dport;
    char *new_input_dest_ip;
    char *new_input_dest_port;
    bool new_input_enabled;
    int ret=0;
    input_enabled = cJSON_GetObjectItem(json_value, "enabled")?cJSON_GetObjectItem(json_value, "enabled")->valueint:0;
	input_name= (cJSON_GetObjectItem(json_value, "name")?cJSON_GetObjectItem(json_value, "name")->valuestring:"");
	input_proto=(cJSON_GetObjectItem(json_value, "proto")?cJSON_GetObjectItem(json_value, "proto")->valuestring:"");
	input_src_dport=(cJSON_GetObjectItem(json_value, "srcdport")?cJSON_GetObjectItem(json_value, "srcdport")->valuestring:"");
    input_dest_ip=(cJSON_GetObjectItem(json_value, "destip")?cJSON_GetObjectItem(json_value, "destip")->valuestring:"");
    input_dest_port=(cJSON_GetObjectItem(json_value, "destport")?cJSON_GetObjectItem(json_value, "destport")->valuestring:"");

    new_input_enabled = cJSON_GetObjectItem(json_value, "newenabled")?cJSON_GetObjectItem(json_value, "newenabled")->valueint:0;
	new_input_name= (cJSON_GetObjectItem(json_value, "newname")?cJSON_GetObjectItem(json_value, "newname")->valuestring:"");
	new_input_proto=(cJSON_GetObjectItem(json_value, "newproto")?cJSON_GetObjectItem(json_value, "newproto")->valuestring:"");
	new_input_src_dport=(cJSON_GetObjectItem(json_value, "newsrcdport")?cJSON_GetObjectItem(json_value, "newsrcdport")->valuestring:"");
    new_input_dest_ip=(cJSON_GetObjectItem(json_value, "newdestip")?cJSON_GetObjectItem(json_value, "newdestip")->valuestring:"");
    new_input_dest_port=(cJSON_GetObjectItem(json_value, "newdestport")?cJSON_GetObjectItem(json_value, "newdestport")->valuestring:"");

    if( (strlen(input_name)==0) || (strlen(input_proto)==0) || (strlen(input_src_dport)==0) || (strlen(input_dest_ip)==0) || (strlen(input_dest_port)==0) )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }
    
    if( (strlen(new_input_name)==0) || (strlen(new_input_proto)==0) || (strlen(new_input_src_dport)==0) || (strlen(new_input_dest_ip)==0) || (strlen(new_input_dest_port)==0) )
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }
    
    ret=check_inter_ip(new_input_dest_ip);
    if(ret !=0 )
    {
        global_weberrorcode=ERR_IP_NOT_INTER;
        return;
    }
    
    ret=mod_pf(input_name,input_proto,input_src_dport,input_dest_ip,input_dest_port,input_enabled,new_input_name,new_input_proto,new_input_src_dport,new_input_dest_ip,new_input_dest_port,new_input_enabled);

    if(ret==-1)
    {
        global_weberrorcode=ERR_NO_MATCH;
        return;
    }
    else if(ret !=0)
    {
        global_weberrorcode=ERR_RULE_CONFLICT;
        return;
        
    }

    return;
}

void proc_pf_get(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    int ret=0;
    struct list_head *tmp=NULL;
    struct fw3_pf *pf;
    cJSON *obj = NULL;
    cJSON *obj_entry = NULL;
   
    
    ret=get_pftable();
    tmp=global_pf_rules.next;
    if(ret!=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }
    
	obj = cJSON_CreateArray();
    cJSON_AddItemToObject(jsonOut, "data", obj);

    while(tmp!=&global_pf_rules)
    {
         
        pf = (struct fw3_pf *)tmp;
        obj_entry=cJSON_CreateObject();
        cJSON_AddItemToArray(obj, obj_entry);
        cJSON_AddItemToObject(obj_entry, "name", cJSON_CreateString(pf->name));
	    cJSON_AddItemToObject(obj_entry, "proto", cJSON_CreateString(pf->proto));
        cJSON_AddItemToObject(obj_entry, "srcdport", cJSON_CreateString(pf->src_dport));
        cJSON_AddItemToObject(obj_entry, "destip", cJSON_CreateString(pf->dest_ip));
        cJSON_AddItemToObject(obj_entry, "destport", cJSON_CreateString(pf->dest_port));
        cJSON_AddItemToObject(obj_entry, "enabled", cJSON_CreateNumber(pf->enabled));
        tmp=tmp->next;
    }
    
    qtec_fw_free_list(&global_pf_rules);
}

void proc_pf(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_pf_get(jsonValue, jsonOut);
    }
    else if ( (request_method & CGI_POST_METHOD ) != 0 )
    {
        proc_pf_add(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	proc_pf_mod(jsonValue, jsonOut);
    }
	else if  ( (request_method & CGI_DELETE_METHOD ) != 0 )
    {
    	proc_pf_del(jsonValue, jsonOut);
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