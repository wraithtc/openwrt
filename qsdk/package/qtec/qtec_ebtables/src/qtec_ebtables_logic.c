#include "qtec_ebtables_basic.h"

//2017/09/30, wjj: this module current only maintain the lan speedlimit rule

//全局变量
struct list_head global_ebtables_speedlimit_rule;

void fw_load_ebtables()
{
	DEBUG_PRINTF("[%s]=======\n",__func__);
	INIT_LIST_HEAD(&global_ebtables_speedlimit_rule);

	//get ebtables rule information from uci 
	struct uci_package *p = NULL;
	struct uci_section *s;
	struct uci_element *e;

	struct ebtables_speedlimit_rule *rule;
	struct uci_context *global_uci;

	global_uci=uci_alloc_context();

	if(!global_uci)
	{
		DEBUG_PRINTF("[%s]====out of memory====\n",__func__);
		return;
	}

	if(uci_load(global_uci,"qtec_ebtables",&p))
	{
		DEBUG_PRINTF("[%s]====failed to load /etc/config/qtec_ebtables",__func__);
		return;
	}

	uci_foreach_element(&p->sections,e)
	{
		s=uci_to_section(e);

		DEBUG_PRINTF("s->type:%s====\n",s->type);

		if(!(rule = alloc_speedlimit_rule()) )
		{
			DEBUG_PRINTF("[%s]==== alloc_rule fail====\n",__func__);
			continue;
		}

		struct uci_element *e1;
		struct uci_option *o1;
		uci_foreach_element(&s->options,e1)
		{
			o1=uci_to_option(e1);
			DEBUG_PRINTF("[%s]...e1->name: %s \n",__func__,e1->name);
            DEBUG_PRINTF("[%s]...o->v.string:%s \n", __func__, o1->v.string);

			if(!strcmp(e1->name,"name"))
			{
				strncpy(rule->name,o1->v.string,sizeof(rule->name));
			}
            else if(!strcmp(e1->name,"mac"))
            {
                strncpy(rule->mac,o1->v.string,sizeof(rule->mac));
            }
			else if(!strcmp(e1->name,"dest"))
			{
                rule->dest = atoi(o1->v.string);
			}
            else if(!strcmp(e1->name,"limit"))
            {
                rule->limit= atoi(o1->v.string);
            }
            else if(!strcmp(e1->name,"enabled"))
            {
                rule->enabled=atoi(o1->v.string);
            }
            else if(!strcmp(e1->name,"src_if"))
            {
                strncpy(rule->src_if,o1->v.string,sizeof(rule->src_if));
            }
            else if(!strcmp(e1->name,"dest_ip"))
            {
                strncpy(rule->dest_ip,o1->v.string,sizeof(rule->dest_ip));
            }
            else if(!strcmp(e1->name,"dst_if"))
            {
                strncpy(rule->dst_if,o1->v.string,sizeof(rule->dst_if));
            }
            else if(!strcmp(e1->name,"target"))
            {
                strncpy(rule->target,o1->v.string,sizeof(rule->target));
            }
            
		}
	}
}

void restart_ebtables()
{
    rtcfgUciCommit("qtec_ebtables");
    system("/etc/init.d/qtec_ebtables restart");
}

