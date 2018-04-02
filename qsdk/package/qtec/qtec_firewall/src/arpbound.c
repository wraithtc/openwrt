#include "qtec_firewall_basic.h"

#if 0
int getArpBoundIndex(char *name)
{
    DEBUG_PRINTF("[%s]====name:%s====\n",__func__,name);
    int i=0;
    sscanf(name,"arpbound%d",&i);
    DEBUG_PRINTF("[%s]===return i:%d====\n",__func__,i);
    return i;
}
#endif 

char * getArpBoundMac(char *input)
{
    //DEBUG_PRINTF("[%s] === input: %s====\n",__func__,input);

    if(*input == '!')
    {
        while (isspace(*++input));
    }
    //DEBUG_PRINTF("[%s]=== output: %s====\n",__func__,input);
    return input;
}


int get_arpboundtable()
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    INIT_LIST_HEAD(&global_arpbound_rules);
    //加载系统内uci的数据
    fw_load_rule();

    struct fw3_arpbound_rule *arpbound_rule=NULL;
    struct fw3_rule *rule=NULL;
    DEBUG_PRINTF("[%s]====遍历global_rules===\n",__func__);
    struct list_head *tmp=global_rules.next;
    
    while(tmp!=&global_rules)
    {
        
        //printf_rule(tmp);
        rule=(struct fw3_rule *)tmp;
        if(!strncmp(rule->name, "arpbound",strlen("arpbound")))
        {
            arpbound_rule=alloc_arpbound_rule();
            arpbound_rule->enabled=rule->enabled;
            DEBUG_PRINTF("[%s]====arpbound_rule address: %x===\n", __func__, arpbound_rule);
            //strcpy(arpbound_rule->name,rule->name);
            strcpy(arpbound_rule->src_ip,rule->src_ip);
            //strcpy(arpbound_rule->src_mac,( ++((char *)rule->src_mac) ));
            strcpy(arpbound_rule->src_mac,getArpBoundMac(rule->src_mac));

            //不考虑索引
            #if 0
            arpbound_rule->arpbound_rule_index=getArpBoundIndex(arpbound_rule->name);
            if(global_arpbound_num < arpbound_rule->arpbound_rule_index)
            {
                global_arpbound_num = arpbound_rule->arpbound_rule_index;
            }
            #endif
            
            printf_arpbound_rule(arpbound_rule);
        }
        
        tmp=tmp->next;
      
    }
	qtec_fw_free_list(&global_rules);
    DEBUG_PRINTF("leave [%s]====\n",__func__);
    return 0;
}

//添加arpbound (只在队尾加)
//传入参数 mac ip enable
int add_arpbound(char *input_mac, char *input_ip, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]===input_mac:%s input_ip:%s input_enable:%d ==\n",__func__,input_mac,input_ip,input_enabled);

    //操作uci
    rtcfgUciAdd("firewall","rule");
    char name[64]={0};
    char cmd[256]={0};
//因为取消表项索引
//  snprintf(name,64,"arpbound",++global_arpbound_num);
    
    snprintf(cmd,256,"firewall.@rule[-1].name=arpbound");
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@rule[-1].src=lan");

    rtcfgUciSet("firewall.@rule[-1].dest=wan");

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].src_mac=!%s",input_mac);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].src_ip=%s",input_ip);
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@rule[-1].target=DROP");

    rtcfgUciSet("firewall.@rule[-1].family=ipv4");

    if(input_enabled == true)
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=1");
    }
    else 
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=0");
    }

#if 0
    //处理数据逻辑
    struct fw3_arpbound_rule *arpbound_rule=alloc_arpbound_rule();
    arpbound_rule->arpbound_rule_index=global_arpbound_num;
    arpbound_rule->enabled=input_enable;
    strcpy(arpbound_rule->name,name);
    strcpy(arpbound_rule->src_ip,input_ip);
    strcpy(arpbound_rule->src_mac,input_mac);

    struct fw3_rule *rule=alloc_rule();
    rule->enabled=input_enable;
    strcpy(rule->name,name);
    strcpy(rule->src_ip,input_ip);
    strcpy(rule->src_mac,input_mac);
#endif
	
    return 0;
}


int del_arpbound(char *input_mac, char *input_ip, bool input_enabled)
{
    DEBUG_PRINTF("[%s]===input_mac:%s  input_ip:%s  input_enable:%d ===\n",__func__,input_mac, input_ip,input_enabled);

    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
    char mac_string[64]={0};
    int index=0;
    char cmd[256]={0};
    int found=-1;
    snprintf(mac_string,64,"!%s",input_mac);
    
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strcmp(rule->name,"arpbound")) && (!strcmp(rule->src_mac,mac_string)) && (!strcmp(rule->src_ip,input_ip)) && (rule->enabled==input_enabled) )
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
     

#if 0
    int found=-1;
    if(input_index == global_arpbound_num)
    {
        global_arpbound_num--;
    }
    //处理uci 数据
    char name[64]={0};
    snprintf(name,64,"arpbound%d",input_index);

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
    struct fw3_arpbound_rule *arpbound_rule=NULL;
    
    int index=0;
    char cmd[256]={0};
    
    while(tmp != &global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        if(!strcmp(rule->name,name))
        {
            found =index;
            break;
        }
        tmp=tmp->next;
        index++;
    }

    if(found == -1)
    {
        DEBUG_PRINTF("[%s]==cannot find correspond index====\n",__func__);
        return -1;
    }
    else
    {
        DEBUG_PRINTF("[%s]===found:%d==== index:%d====\n",__func__,found,index);
        snprintf(cmd,256,"firewall.@rule[%d]",found);
        rtcfgUciDel(cmd);
    }

    //处理内部数据逻辑
    //删除global_rules 对应的表项
    list_del(tmp);
    free(rule);
   
    //删除global_arpbound_rules 对应的表项
    tmp=global_arpbound_rules.next;
    while(tmp != &global_arpbound_rules)
    {
        arpbound_rule=(struct fw3_arpbound_rule *)tmp;
        if( arpbound_rule->arpbound_rule_index == input_index)
        {
            list_del(tmp);
            free(arpbound_rule);
            break;
        }
        tmp=tmp->next;
    }
    
    return 0;
#endif 
}


void proc_family_firewall_cancel()
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    int ret=0;
    struct list_head *tmp=NULL;
    struct fw3_arpbound_rule *rule;

    
    ret=get_arpboundtable();
    tmp=global_arpbound_rules.next;
    if(ret!=0)
    {
        return;
    }
    
    while(tmp!=&global_arpbound_rules)
    {
        rule = (struct fw3_arpbound_rule *)tmp;
        DEBUG_PRINTF("[%s] ===tmp addr:%x====\n",__func__,tmp);
        printf_arpbound_rule(rule);
        del_arpbound(rule->src_mac, rule->src_ip, rule->enabled);
        tmp=tmp->next;
    }

    qtec_fw_free_list(&global_arpbound_rules);
	return;
}


//修改 arpbound (主要是使能)
//传入参数: input_mac, input_ip, input_enable, input_index
int mod_arpbound(char *old_input_mac, char *old_input_ip, bool old_input_enabled,char *new_input_mac, char *new_input_ip, bool new_input_enabled)
{
    DEBUG_PRINTF("[%s]===old_input_mac:%s  old_input_ip:%s  old_input_enable:%d  ====\n",__func__,old_input_mac,old_input_ip,old_input_enabled);
    DEBUG_PRINTF("[%s]===new_input_mac:%s  new_input_ip:%s  new_input_enable:%d  ====\n",__func__,new_input_mac,new_input_ip,new_input_enabled);

    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
    char mac_string[64]={0};
    int index=0;
    char cmd[256]={0};
    int found=-1;
    snprintf(mac_string,64,"!%s",old_input_mac);
    
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strcmp(rule->name,"arpbound")) && (!strcmp(rule->src_mac,mac_string)) && (!strcmp(rule->src_ip,old_input_ip)) && (rule->enabled==old_input_enabled) )
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
    snprintf(cmd,256,"firewall.@rule[%d].src_mac=!%s",index,new_input_mac);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[%d].src_ip=%s",index,new_input_ip);
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
	qtec_fw_free_list(&global_rules);
    return 0;
#if 0
    //处理uci 数据
    char name[64]={0};
    snprintf(name,64,"arpbound%d",input_index);
    
    int found =-1;
    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
    struct fw3_arpbound_rule *arpbound_rule=NULL;
    
    int index=0;
    char cmd[256]={0};
    
    while(tmp != &global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        if(!strcmp(rule->name,name))
        {
            found =index;
            break;
        }
        tmp=tmp->next;
        index++;
    }

    if(found==-1)
    {
        DEBUG_PRINTF("[%s]====cannot find the corresponding input_index===\n",__func__);
        return -1;
    }
    else
    {
        DEBUG_PRINTF("[%s]===found:%d==== index:%d===\n",__func__,found,index);
        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@rule[%d].src_mac=!%s",index,input_mac);
        rtcfgUciSet(cmd);

        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@rule[%d].src_ip=%s",index,input_ip);
        rtcfgUciSet(cmd);
        
        if(input_enable == true)
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
    }

    //维护数据
    rule->enabled=input_enable;
    strcpy(rule->src_ip,input_ip);
    strcpy(rule->src_mac,input_mac);

    //删除global_arpbound_rules 对应的表项
    tmp=global_arpbound_rules.next;
    while(tmp != &global_arpbound_rules)
    {
        arpbound_rule=(struct fw3_arpbound_rule *)tmp;
        if( arpbound_rule->arpbound_rule_index == input_index)
        {
            break;
        }
        tmp=tmp->next;
    }

    arpbound_rule->enabled=input_enable;
    strcpy(arpbound_rule->src_ip,input_ip);
    strcpy(arpbound_rule->src_mac,input_mac);
#endif
}