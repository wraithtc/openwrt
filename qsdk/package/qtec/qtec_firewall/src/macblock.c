#include "qtec_firewall_basic.h"

int get_macblocktable()
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    INIT_LIST_HEAD(&global_macblock_rules);
    //加载系统内uci的数据
    fw_load_rule();

    struct fw3_macblock_rule *macblock_rule=NULL;
    struct fw3_rule *rule=NULL;
    DEBUG_PRINTF("[%s]====遍历global_rules===\n",__func__);
    struct list_head *tmp=global_rules.next;
    
    while(tmp!=&global_rules)
    {
        
        //printf_rule(tmp);
        rule=(struct fw3_rule *)tmp;
        if(!strncmp(rule->name, "macblock1",strlen("macblock1")))
        {
            macblock_rule=(struct fw3_macblock_rule*) alloc_macblock_rule();
			sscanf(rule->name,"macblock1_%s",macblock_rule->name);
            macblock_rule->enabled=rule->enabled;
         
            strcpy(macblock_rule->src_mac,rule->src_mac);

        }
        
        tmp=tmp->next;
      
    }
    

	qtec_fw_free_list(&global_rules);
	
    return 0;
}

//添加macblock (只在队尾加)

int add_macblock(char *input_name, char *input_mac, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]===input_name:%s input_mac:%s input_enable:%d ==\n",__func__,input_name, input_mac,input_enabled);

    //操作uci
    rtcfgUciAdd("firewall","rule");
    char name[64]={0};
    char cmd[256]={0};

    //forward chain 
    snprintf(cmd,256,"firewall.@rule[-1].name=macblock1_%s",input_name);
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@rule[-1].src=lan");

    rtcfgUciSet("firewall.@rule[-1].dest=wan");

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].src_mac=%s",input_mac);
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

    //input chain
    rtcfgUciAdd("firewall","rule");
    snprintf(cmd,256,"firewall.@rule[-1].name=macblock2_%s",input_name);
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@rule[-1].src=lan");

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@rule[-1].src_mac=%s",input_mac);
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
	

    return 0;
}


int del_macblock( char *input_name,char *input_mac, bool input_enabled)
{
    DEBUG_PRINTF("[%s]===input_name:%s input_mac:%s  input_enable:%d ===\n",__func__, input_name,input_mac,input_enabled);

    //forward chain
    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
	char tmp_name[256]={0};

	memset(tmp_name,0,256);
	snprintf(tmp_name,256,"macblock1_%s",input_name);
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);


        if( (!strncmp(rule->name,"macblock1",strlen("macblock1"))) && (!strcmp(rule->src_mac,input_mac)) && (rule->enabled==input_enabled) )
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
	
    //input chain 
    fw_load_rule();
    index=0;
    tmp=global_rules.next;
    rule=NULL;
	
  	memset(tmp_name,0,256);
	snprintf(tmp_name,256,"macblock2_%s",input_name);
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if(  (!strncmp(rule->name,"macblock2",strlen("macblock2"))) && (!strcmp(rule->src_mac,input_mac)) && (rule->enabled==input_enabled) )
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
int mod_macblock(char *input_name, char *old_input_mac,  bool old_input_enabled,char *new_input_mac,  bool new_input_enabled)
{
    DEBUG_PRINTF("[%s]===old_input_mac:%s old_input_enable:%d  ====\n",__func__,old_input_mac,old_input_enabled);
    DEBUG_PRINTF("[%s]===new_input_mac:%s new_input_enable:%d  ====\n",__func__,new_input_mac,new_input_enabled);

    //加载系统内uci的数据
    fw_load_rule();

    struct list_head *tmp=global_rules.next;
    struct fw3_rule *rule=NULL;
   
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char tmp_name[256]={0};

	memset(tmp_name,0,256);
	snprintf(tmp_name,256,"macblock1_%s",input_name);
	
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);
		
        if( (!strncmp(rule->name,"macblock1",strlen("macblock1"))) && (!strcmp(rule->src_mac,old_input_mac)) && (rule->enabled==old_input_enabled) )
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
    index=0;
    tmp=global_rules.next;
	memset(tmp_name,0,256);
	snprintf(tmp_name,256,"macblock2_%s",input_name);
    while(tmp!=&global_rules)
    {
        rule=(struct fw3_rule *)tmp;
        printf_rule(rule);

        if(  (!strncmp(rule->name,"macblock2",strlen("macblock2"))) &&(!strcmp(rule->src_mac,old_input_mac)) && (rule->enabled==old_input_enabled) )
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
	
	qtec_fw_free_list(&global_rules);
    return 0;
}