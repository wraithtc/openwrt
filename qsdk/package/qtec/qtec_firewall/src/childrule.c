#include "qtec_firewall_basic.h"


int get_childruletable()
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    INIT_LIST_HEAD(&global_childrules);
    //加载系统内uci的数据
    fw_load_rule();

    struct fw3_childrule *childrule=NULL;
    struct fw3_rule *rule=NULL;
    DEBUG_PRINTF("[%s]====遍历global_rules===\n",__func__);
    struct list_head *tmp=global_rules.next;
    
    while(tmp!=&global_rules)
    {
        
        //printf_rule(tmp);
        rule=(struct fw3_rule *)tmp;
        if(!strncmp(rule->name, "childrule",strlen("childrule")))
        {
            childrule=(struct fw3_childrule*) alloc_childrule();
            childrule->enabled=rule->enabled;
            sscanf(rule->name,"childrule_%s",childrule->name);
            strcpy(childrule->src_mac,rule->src_mac);
            strcpy(childrule->start_time,rule->start_time);
            strcpy(childrule->stop_time,rule->stop_time);
            strcpy(childrule->weekdays,rule->weekdays);

        }
        
        tmp=tmp->next;
      
    }
    DEBUG_PRINTF("leave [%s]====\n",__func__);
	qtec_fw_free_list(&global_rules);
    return 0;
}


//寮冪敤input_name
int add_childrule(char *input_name, char *input_mac,char *input_start_time, char *input_stop_time, char *input_weekdays, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]===input_name:%s input_mac:%s input_start_time:%s input_stop_time:%s input_weekdays:%s input_enabled:%d ==\n",__func__,input_name,input_mac,input_start_time,input_stop_time,input_weekdays,input_enabled);

    //操作uci
    rtcfgUciAdd("firewall","rule");
    char name[64]={0};
    char cmd[256]={0};
    char hostname[qtec_firewall_hostname_len]="unknow";
    struct Qtec_firewall_DeviceEntry array[qtec_firewall_max_device_num];
    int device_num;

    qtec_firewall_pase_dhcp_file(array,&device_num);
    int i=0;
    for(i=0;i<device_num;i++)
    {
        if(strncasecmp(input_mac,array[i].mac,sizeof(array[i].mac))==0)
        {
            memset(hostname,0,qtec_firewall_hostname_len);
            strncpy(hostname,array[i].hostname,qtec_firewall_hostname_len);
            break;
        }
    }
    snprintf(cmd,256,"firewall.@rule[-1].name=childrule_%s",hostname);
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@rule[-1].src=lan");

    rtcfgUciSet("firewall.@rule[-1].dest=wan");

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].src_mac=%s",input_mac);
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@rule[-1].target=DROP");

    rtcfgUciSet("firewall.@rule[-1].family=ipv4");
    rtcfgUciSet("firewall.@rule[-1].utc_time=0");

    if(input_enabled == true)
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=1");
    }
    else 
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=0");
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].start_time=%s",input_start_time);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].stop_time=%s",input_stop_time);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].weekdays=%s",input_weekdays);
    rtcfgUciSet(cmd);
  
    return 0;
}


int del_childrule(char *input_name,char *input_mac,char *input_start_time, char *input_stop_time, char *input_weekdays, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]===input_name:%s input_mac:%s input_start_time:%s input_stop_time:%s input_weekdays:%s input_enabled:%d ==\n",__func__,input_name,input_mac,input_start_time,input_stop_time,input_weekdays,input_enabled);

    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    
    snprintf(name,256,"childrule_%s",input_name);
    
    
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strncmp(rule->name,"childrule",strlen("childrule"))) && (!strcmp(rule->src_mac,input_mac))  )
        {
            found=index;
            break;
        }

        
        index++;
        tmp=tmp->next;
    }

    if(found==-1)
    {
        DEBUG_PRINTF("[%s] ERROR!!!====CANNOT found match entry====\n",__func__);
        return -1;
    }

    snprintf(cmd,256,"firewall.@rule[%d]",index);
    rtcfgUciDel(cmd);
	qtec_fw_free_list(&global_rules);
    return 0;
     

}

//修改 macblock(主要是使能)
//传入参数: input_mac, input_ip, input_enable, input_index
int mod_childrule(char *old_input_name, char *old_input_mac, char *old_input_start_time, char *old_input_stop_time, char *old_input_weekdays, bool old_input_enabled,char *new_input_name, char *new_input_mac,char *new_input_start_time, char* new_input_stop_time, char *new_input_weekdays,  bool new_input_enabled)
{
    DEBUG_PRINTF("====[%s]==old=input_name,input_mac:%s input_start_time:%s input_stop_time:%s input_weekdays:%s input_enabled:%d ==\n",__func__,old_input_name,old_input_mac,old_input_start_time,old_input_stop_time,old_input_weekdays,old_input_enabled);
    DEBUG_PRINTF("====[%s]==new=input_name,input_mac:%s input_start_time:%s input_stop_time:%s input_weekdays:%s input_enabled:%d ==\n",__func__,new_input_name,new_input_mac,new_input_start_time,new_input_stop_time,new_input_weekdays,new_input_enabled);
    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
   
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char old_name[256]={0};
    char new_name[256]={0};

    snprintf(old_name,256,"childrule_%s",old_input_name);
    snprintf(new_name,256,"childrule_%s",new_input_name);
    
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strncmp(rule->name,"childrule",strlen("childrule"))) && (!strcmp(rule->src_mac,old_input_mac)) )
        {
            found=index;
            break;
        }

        
        index++;
        tmp=tmp->next;
    }

    if(found==-1)
    {
        DEBUG_PRINTF("[%s] ERROR!!!====CANNOT found match entry====\n",__func__);
        return -1;
    }

    DEBUG_PRINTF("[%s]===found:%d==== index:%d===\n",__func__,found,index);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[%d].name=%s",index,new_name);
    rtcfgUciSet(cmd);
    
    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[%d].src_mac=%s",index,new_input_mac);
    rtcfgUciSet(cmd);
        
    if(new_input_enabled == true)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@rule[%d].enabled=1",index);
        rtcfgUciSet(cmd);
    }
    else
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@rule[%d].enabled=0",index);
        rtcfgUciSet(cmd);
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[%d].start_time=%s",index,new_input_start_time);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[%d].stop_time=%s",index,new_input_stop_time);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[%d].weekdays=%s",index,new_input_weekdays);
    rtcfgUciSet(cmd);
    qtec_fw_free_list(&global_rules);
    return 0;
}