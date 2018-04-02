#include "qtec_firewall_basic.h"

#if 0
//该函数弃用
int get_downlimittable()
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    INIT_LIST_HEAD(&global_downlimit_rules);
    //加载系统内uci的数据
    fw_load_rule();

    struct fw3_downlimit_rule* downlimit_rule=NULL;
    struct fw3_rule *rule=NULL;
    DEBUG_PRINTF("[%s]====遍历global_rules===\n",__func__);
    struct list_head *tmp=global_rules.next;
    
    while(tmp!=&global_rules)
    {
        
        //printf_rule(tmp);
        rule=(struct fw3_rule *)tmp;
        if(!strncmp(rule->name, "downlimit",strlen("downlimit")))
        {
            downlimit_rule= (struct fw3_downlimit_rule*) alloc_downlimit_rule();
            downlimit_rule->enabled=rule->enabled;
         
            strcpy(downlimit_rule->dest_ip,rule->dest_ip);
            downlimit_rule->limit=rule->limit;

        }
        
        tmp=tmp->next;
      
    }
    DEBUG_PRINTF("leave [%s]====\n",__func__);
    return 0;
}
#endif

int get_downlimit(char *input_mac, int *output_enabled)
{
    DEBUG_PRINTF("===[%s]===input_mac:%s =====\n",__func__,input_mac);
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
    
    int downlimit=0;
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    char tmp_char[64]={0};
    snprintf(name,256,"downlimit_%s",input_mac);
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strcmp(rule->name,name)) )
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
        return downlimit;
    }

    snprintf(cmd,256,"firewall.@rule[%d].limit",index);
    rtcfgUciGet(cmd,tmp_char);
    sscanf(tmp_char,"%d/sec",&downlimit);

    memset(tmp_char,0,64);
    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[%d].enabled",index);
    rtcfgUciGet(cmd,tmp_char);
    *output_enabled=atoi(tmp_char);
    qtec_fw_free_list(&global_rules);
    return downlimit;
     

}


int add_downlimit_rule(char *input_mac,char *input_destip, int input_limit, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]===input_destip:%s input_mac:%s input_limit:%d  input_enabled:%d ==\n",__func__,input_destip,input_mac,input_limit,input_enabled);

    if(input_limit==0)
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //操作uci
    rtcfgUciAdd("firewall","rule");
    char name[256]={0};
    char cmd[256]={0};

    snprintf(name,256,"downlimit_%s",input_mac);
    snprintf(cmd,256,"firewall.@rule[-1].name=%s",name);
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@rule[-1].src=wan");

    rtcfgUciSet("firewall.@rule[-1].dest=lan");

    rtcfgUciSet("firewall.@rule[-1].target=ACCEPT");

    rtcfgUciSet("firewall.@rule[-1].family=ipv4");

    if(input_enabled == true)
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=1");
    }
    else 
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=0");
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].limit=%d/sec",input_limit);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].dest_ip=%s",input_destip);
    rtcfgUciSet(cmd);
    
    return 0;
}


int del_downlimit_rule(char *input_mac,char *input_destip, int input_limit, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]===input_destip:%s input_mac:%s input_limit:%d  input_enabled:%d ==\n",__func__,input_destip,input_mac,input_limit,input_enabled);

    if(input_limit==0)
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    snprintf(name,256,"downlimit_%s",input_mac);
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strcmp(rule->name,name)) && (rule->enabled==input_enabled) && (rule->limit==input_limit)  )
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


int mod_downlimit_rule(char *input_mac, char *input_destip, int old_limit,  bool old_enabled, int new_limit,  bool new_enabled)
{
    DEBUG_PRINTF("====[%s]==input_mac:%s=== destip:%s old:limit:%d enabled:%d==\n",__func__,input_mac,input_destip,old_limit,old_enabled);
    DEBUG_PRINTF("====[%s]==new=limit:%d enabled:%d ==\n",__func__,new_limit,new_enabled);

    if( (old_limit==0) || (new_limit==0) )
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    snprintf(name,256,"downlimit_%s",input_mac);
    
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strcmp(rule->name,name)) && (rule->enabled==old_enabled) && (rule->limit==old_limit) )
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
   
        
    if(new_enabled == true)
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
    snprintf(cmd,256,"firewall.@rule[%d].limit=%d/sec",index,new_limit);
    rtcfgUciSet(cmd);
	qtec_fw_free_list(&global_rules);
    
    return 0;
}

#if 0
//uplimit
int get_uplimittable()
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    INIT_LIST_HEAD(&global_uplimit_rules);
    //加载系统内uci的数据
    fw_load_rule();

    struct fw3_uplimit_rule* uplimit_rule=NULL;
    struct fw3_rule *rule=NULL;
    DEBUG_PRINTF("[%s]====遍历global_rules===\n",__func__);
    struct list_head *tmp=global_rules.next;
    
    while(tmp!=&global_rules)
    {
        
        //printf_rule(tmp);
        rule=(struct fw3_rule *)tmp;
        if(!strncmp(rule->name, "uplimit1",strlen("uplimit1")))
        {
            uplimit_rule= (struct fw3_uplimit_rule *)alloc_uplimit_rule();
            uplimit_rule->enabled=rule->enabled;
         
            strcpy(uplimit_rule->src_ip,rule->src_ip);
            uplimit_rule->limit=rule->limit;

        }
        
        tmp=tmp->next;
      
    }
    DEBUG_PRINTF("leave [%s]====\n",__func__);
    return 0;
}
#endif

int get_uplimit(char *input_mac, int *output_enabled)
{
    DEBUG_PRINTF("===[%s]===input_mac:%s====\n",__func__,input_mac);
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;

    int uplimit=0;
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    char tmp_char[64]={0};
    snprintf(name,256,"uplimit1_%s",input_mac);
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strcmp(rule->name,name)) )
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
        return uplimit;
    }

    snprintf(cmd,256,"firewall.@rule[%d].limit",index);
    rtcfgUciGet(cmd,tmp_char);
    sscanf(tmp_char,"%d/sec",&uplimit);

    memset(cmd,0,256);
    memset(tmp_char,0,64);
    snprintf(cmd,256,"firewall.@rule[%d].enabled",index);
    rtcfgUciGet(cmd,tmp_char);
    *output_enabled=atoi(tmp_char);
	qtec_fw_free_list(&global_rules);
    return uplimit;
}

int add_uplimit_rule(char *input_srcmac,int input_limit, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]===input_srcmac:%s input_limit:%d  input_enabled:%d ==\n",__func__,input_srcmac,input_limit,input_enabled);

    if(input_limit==0)
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //操作uci
    rtcfgUciAdd("firewall","rule");
    char name[256]={0};
    char cmd[256]={0};

    snprintf(name,256,"uplimit1_%s",input_srcmac);
    snprintf(cmd,256,"firewall.@rule[-1].name=%s",name);
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@rule[-1].src=lan");

    rtcfgUciSet("firewall.@rule[-1].dest=wan");

    rtcfgUciSet("firewall.@rule[-1].target=ACCEPT");

    rtcfgUciSet("firewall.@rule[-1].family=ipv4");

    if(input_enabled == true)
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=1");
    }
    else 
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=0");
    }
    
    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].limit=%d/sec",input_limit);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].src_mac=%s",input_srcmac);
    rtcfgUciSet(cmd);

    rtcfgUciAdd("firewall","rule");
    memset(name,0,256);
    snprintf(name,256,"uplimit2_%s",input_srcmac);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].name=%s",name);
    rtcfgUciSet(cmd);
    
    rtcfgUciSet("firewall.@rule[-1].src=lan");

    rtcfgUciSet("firewall.@rule[-1].dest=wan");

    rtcfgUciSet("firewall.@rule[-1].target=DROP");

    rtcfgUciSet("firewall.@rule[-1].family=ipv4");

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].src_mac=%s",input_srcmac);
    rtcfgUciSet(cmd);

    if(input_enabled == true)
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=1");
    }
    else 
    {
        rtcfgUciSet("firewall.@rule[-1].enabled=0");
    }
    
    return 0;
}


int del_uplimit_rule(char *input_srcmac,int input_limit, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]===input_srcmac:%s input_limit:%d  input_enabled:%d ==\n",__func__,input_srcmac,input_limit,input_enabled);

    if(input_limit==0)
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    snprintf(name,256,"uplimit1_%s",input_srcmac);
    
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strcmp(rule->name,name)) && (rule->enabled==input_enabled) && (rule->limit==input_limit) )
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
    snprintf(cmd,256,"firewall.@rule[%d]",index);
    rtcfgUciDel(cmd);
	qtec_fw_free_list(&global_rules);
    //这一刻会造成内存泄漏，但考虑到暂时是cgi调用，所以暂时不考虑这问题
    //因为每删除一条规则，uci的索引都会重新排列
    fw_load_rule();
    tmp=global_rules.next;
    index=0;
    found=-1;
    memset(name,0,256);
    snprintf(name,256,"uplimit2_%s",input_srcmac);
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strcmp(rule->name,name)) && (rule->enabled==input_enabled)  )
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
    snprintf(cmd,256,"firewall.@rule[%d]",index);
    rtcfgUciDel(cmd);
	qtec_fw_free_list(&global_rules);
    return 0;
     

}


int mod_uplimit_rule(char *input_mac, int old_limit,  bool old_enabled, int new_limit,  bool new_enabled)
{
    DEBUG_PRINTF("====[%s]=input_mac:%s=old=input_limit:%d  input_enabled:%d ==\n",__func__,input_mac,old_limit,old_enabled);
    DEBUG_PRINTF("====[%s]==new=input_limit:%d  input_enabled:%d ==\n",__func__,new_limit,new_enabled);

    if( (old_limit==0) || (new_limit==0) )
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    snprintf(name,256,"uplimit1_%s",input_mac);
    
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( (!strcmp(rule->name,name)) && (rule->enabled==old_enabled) && (rule->limit==old_limit) )
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
        
    if(new_enabled == true)
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
    snprintf(cmd,256,"firewall.@rule[%d].limit=%d/sec",index,new_limit);
    rtcfgUciSet(cmd);
	qtec_fw_free_list(&global_rules);
    //这一刻会造成内存泄漏，但考虑到暂时是cgi调用，所以暂时不考虑这问题
    //因为每删除一条规则，uci的索引都会重新排列
    fw_load_rule();
    tmp=global_rules.next;
    index=0;
    found=-1;
    memset(name,0,256);
    snprintf(name,256,"uplimit2_%s",input_mac);
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if( !strcmp(rule->name,name) )
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
    if(new_enabled == true)
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
    snprintf(cmd,256,"firewall.@rule[%d]",index);
    qtec_fw_free_list(&global_rules);
    return 0;
}