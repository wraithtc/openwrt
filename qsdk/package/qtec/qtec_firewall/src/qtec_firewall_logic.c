#include "qtec_firewall_basic.h"

struct uci_context *global_uci;
struct list_head global_rules;
struct list_head global_redirects;
int global_rules_num=0;
struct list_head global_arpbound_rules;
int global_arpbound_num=0;  //这个数值表示当前 所有arpbound 项目的最大值 
struct list_head global_macblock_rules;
struct list_head global_childrules;
struct list_head global_downlimit_rules;
struct list_head global_uplimit_rules;
struct list_head global_dmz_rules;
struct list_head global_pf_rules;

void qtec_firewall_main()
{

#if 0
    INIT_LIST_HEAD(&global_rules);
    INIT_LIST_HEAD(&global_arpbound_rules);
    //从uci firewall 中load rules
    fw_load_rule();

    //遍历global_rules
    struct fw3_arpbound_rule *arpbound_rule=NULL;
    struct fw3_rule *rule=NULL;
    DEBUG_PRINTF("[%s]====遍历global_rules===\n",__func__);
    struct list_head *tmp=global_rules.next;
    
    while(tmp!=&global_rules)
    {
        DEBUG_PRINTF("[%s]======\n",__func__);
        printf_rule(tmp);
        rule=(struct fw3_rule *)tmp;
        if(!strncmp(rule->name, "arpbound",strlen("arpbound")))
        {
            arpbound_rule=alloc_arpbound_rule();
            arpbound_rule->enabled=rule->enabled;
           
            strcpy(arpbound_rule->name,rule->name);
            strcpy(arpbound_rule->src_ip,rule->src_ip);
            //strcpy(arpbound_rule->src_mac,( ++((char *)rule->src_mac) ));
            strcpy(arpbound_rule->src_mac,getArpBoundMac(rule->src_mac));
            arpbound_rule->arpbound_rule_index=getArpBoundIndex(arpbound_rule->name);
            if(global_arpbound_num < arpbound_rule->arpbound_rule_index)
            {
                global_arpbound_num = arpbound_rule->arpbound_rule_index;
            }
            
            printf_arpbound_rule(arpbound_rule);
        }
        
        tmp=tmp->next;
      
    }

    //add_arpbound("a8:1e:84:5c:fa:72", "10.0.0.1", true);
    printf_all();
    del_arpbound(9);
    printf_all();
#endif 
  // add_arpbound("a8:1e:84:5c:fa:72","10.0.0.1",true);
   //add_arpbound("a8:1e:84:5c:fa:73","10.0.0.2",true);
   get_arpboundtable();

   printf_all();
}