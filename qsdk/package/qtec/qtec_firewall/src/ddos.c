#include "qtec_firewall_basic.h"

int get_ddos(bool *enable)
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    int ret=0;
    char tmp_char[256]={0};
    ret=rtcfgUciGet("system.@system[0].ddos",tmp_char);

    if(ret!=0)
    {
        *enable=1;
    }
    else
    {
        *enable = atoi(tmp_char);
    }
    return 0;
}
    

int set_ddos(bool enable)
{
    DEBUG_PRINTF("[%s]===enable:%d===\n",__func__,enable);

    bool cur_enable;
    char tmp_char[256]={0};
    struct list_head *tmp=NULL;
    struct fw3_rule *rule=NULL;
    int index =0;
    char cmd[256]={0};
    get_ddos(&cur_enable);

    DEBUG_PRINTF("[%s]===cur_enable:%d===\n",__func__,cur_enable);

    if(cur_enable == enable)
    {
        DEBUG_PRINTF("[%s]===enable is the same as cur ddos status ,do nothing===\n",__func__);
        return 0;
    }

    
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"system.@system[0].ddos=%d",enable);
        rtcfgUciSet(cmd);

        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@defaults[0].syn_flood=%d",enable);
        rtcfgUciSet(cmd);

        fw_load_rule();

        tmp=global_rules.next;
        rule=NULL;
        index=0;
        while(tmp!=&global_rules)
        {
            rule=(struct fw3_rule *)tmp;
            if( !(strcmp(rule->name,"Allow-DHCP-Renew")) || !(strcmp(rule->name,"Allow-Ping")) || !(strcmp(rule->name,"Allow-DHCPv6")) || !(strcmp(rule->name,"Allow-ICMPv6-Input")) || !(strcmp(rule->name,"Allow-ICMPv6-Forward")) )
            {
                memset(cmd,0,256);
                snprintf(cmd,256,"firewall.@rule[%d].enabled=%d",index,enable);
                rtcfgUciSet(cmd);
            }
           
            index++;
            tmp=tmp->next;

        }

    }

    restart_fw(0);
    qtec_fw_free_list(&global_rules);
    return 0;
    
}

