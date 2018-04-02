#include "qtec_firewall_basic.h"

struct fw3_rule* alloc_rule()
{
    struct fw3_rule *rule = calloc(1,sizeof(*rule));

    if(rule){

        memset(rule,0,sizeof(*rule));

        list_add_tail(&rule->list,&global_rules);

        rule->enabled=true;  //默认值
    }

        return rule;
}

struct fw3_arpbound_rule* alloc_arpbound_rule()
{
    struct fw3_arpbound_rule *arpbound_rule = calloc(1,sizeof(*arpbound_rule));
    if(arpbound_rule){
        memset(arpbound_rule,0,sizeof(*arpbound_rule));
        list_add_tail(&arpbound_rule->list,&global_arpbound_rules);
        arpbound_rule->enabled=true;
    }

    return arpbound_rule;
}

struct fw3_macblock_rule* alloc_macblock_rule()
{
    struct fw3_macblock_rule *macblock_rule = calloc(1,sizeof(*macblock_rule));
    if(macblock_rule){
        memset(macblock_rule,0,sizeof(*macblock_rule));
        list_add_tail(&macblock_rule->list,&global_macblock_rules);
        macblock_rule->enabled=true;
    }

    return macblock_rule;        
}

struct fw3_childrule* alloc_childrule()
{
    struct fw3_childrule *child_rule = calloc(1,sizeof(*child_rule));
    if(child_rule){
        memset(child_rule,0,sizeof(*child_rule));
        list_add_tail(&child_rule->list,&global_childrules);
        child_rule->enabled=true;
    }

    return child_rule;        
}

struct fw3_downlimit_rule* alloc_downlimit_rule()
{
    struct fw3_downlimit_rule *downlimit_rule =calloc(1,sizeof(*downlimit_rule));
    if(downlimit_rule){
        memset(downlimit_rule,0,sizeof(*downlimit_rule));
        list_add_tail(&downlimit_rule->list,&global_downlimit_rules);
        downlimit_rule->enabled=true;
    }

    return downlimit_rule;
}

struct fw3_uplimit_rule* alloc_uplimit_rule()
{
    struct fw3_uplimit_rule *uplimit_rule =calloc(1,sizeof(*uplimit_rule));
    if(uplimit_rule){
        memset(uplimit_rule,0,sizeof(*uplimit_rule));
        list_add_tail(&uplimit_rule->list,&global_uplimit_rules);
        uplimit_rule->enabled=true;
    }

    return uplimit_rule;
}

struct fw3_redirects* alloc_redirect()
{
    struct fw3_redirects* redirect= calloc(1,sizeof(*redirect));

    if(redirect){

        memset(redirect,0,sizeof(*redirect));

        list_add_tail(&redirect->list,&global_redirects);

        redirect->enabled=true;  //默认值
    }

        return redirect;
}

struct fw3_dmz* alloc_dmz()
{
    struct fw3_dmz* dmz= calloc(1,sizeof(*dmz));

    if(dmz){

        memset(dmz,0,sizeof(*dmz));

        list_add_tail(&dmz->list,&global_dmz_rules);

        dmz->enabled=true;  //默认值
    }

        return dmz;
}

struct fw3_pf* alloc_pf()
{
    struct fw3_pf* pf= calloc(1,sizeof(*pf));

    if(pf){

        memset(pf,0,sizeof(*pf));

        list_add_tail(&pf->list,&global_pf_rules);

        pf->enabled=true;  //默认值
    }

        return pf;
}

void printf_rule(struct fw3_rule *input)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    DEBUG_PRINTF("rule enabled:%s \n",(input->enabled == true ? "true":"false"));
    DEBUG_PRINTF("rule name: %s \n",input->name);
    DEBUG_PRINTF("rule target: %s \n", input->target);
    DEBUG_PRINTF("rule src_mac: %s \n", input->src_mac);
    DEBUG_PRINTF("rule src:%s \n", input->src);
    DEBUG_PRINTF("rule dest:%s \n", input->dest);
    DEBUG_PRINTF("rule src_ip:%s \n", input->src_ip);
    DEBUG_PRINTF("rule dest_ip:%s \n", input->dest_ip);
    
}

void printf_arpbound_rule(struct fw3_arpbound_rule *input)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    DEBUG_PRINTF("arpbound rule enabled:%s \n",(input->enabled == true ? "true":"false"));
   // DEBUG_PRINTF("arpbound rule name: %s \n",input->name);

    DEBUG_PRINTF("arpbound rule src_mac:%s \n", input->src_mac);

    DEBUG_PRINTF("arpbound rule src_ip:%s \n", input->src_ip);

   // DEBUG_PRINTF("arpbound rule arpbound_rule_index:%d \n", input->arpbound_rule_index);
   
    
}

//遍历当前程序所有的结构列表
void printf_all()
{
    DEBUG_PRINTF("******************************\n",__func__);
    DEBUG_PRINTF("[%s]===show global rules======\n",__func__);
    struct list_head *tmp=global_rules.next;
    while(tmp != &global_rules)
    {
        printf_rule((struct fw3_rule*)tmp);
        tmp=tmp->next;
    }

    DEBUG_PRINTF("[%s]====show global arpbound rules=====\n",__func__);
    tmp = global_arpbound_rules.next;
    while(tmp!=&global_arpbound_rules)
    {
        printf_arpbound_rule((struct fw3_arpbound_rule*) tmp);
        tmp=tmp->next;
    }
}

//flag:0 ===> cgi; 1===> system
void restart_fw(int flag)
{
    rtcfgUciCommit("firewall");
    if(flag ==1)
    {
        system("/etc/init.d/firewall restart &");
        
    }
    else
    {
        ProcFirewallRestartReq();
    }
}

void
qtec_fw_free_list(struct list_head *head)
{
	struct list_head *entry, *tmp;

	if (!head)
		return;

	list_for_each_safe(entry, tmp, head)
	{
		list_del(entry);
		free(entry);
	}
	 INIT_LIST_HEAD(head);
	//free(head);
	//head = NULL;
}


