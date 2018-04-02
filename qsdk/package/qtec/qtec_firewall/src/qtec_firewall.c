#include "qtec_firewall_basic.h"

void fw_load_rule()
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    INIT_LIST_HEAD(&global_rules);
    
    //´Óuci »ñÈ¡firewall ÐÅÏ¢
    struct uci_package *p = NULL;
    struct uci_section *s;
    struct uci_element *e;
    struct fw3_rule *rule;
	struct uci_context *global_uci;
	
    global_uci=uci_alloc_context();
    
    if(!global_uci)
    {
        DEBUG_PRINTF("[%s]===out of memory==\n",__func__);
        exit(1);
    }

    if (uci_load(global_uci, "firewall", &p))
	{
		uci_perror(global_uci, NULL);
		DEBUG_PRINTF("[%s]===Failed to load /etc/config/firewall",__func__);
        exit(1);
	}

    uci_foreach_element(&p->sections, e)
	{
		s = uci_to_section(e);
       // DEBUG_PRINTF("s->type:%s====\n",s->type);

        //ÔÚÕâ¸öÄ£¿éÀïÎÒÃÇÖ»´¦Àírule
        if(strcmp(s->type,"rule"))
        {
            continue;
        }

        if(!(rule=alloc_rule()))
        {
            DEBUG_PRINTF("[%s]=====alloc_rule fail ====\n",__func__);
            continue;
        }
        
        struct uci_element *e1;
        struct uci_option *o1;
        uci_foreach_element(&s->options, e1)
        {
            o1=uci_to_option(e1);

            //DEBUG_PRINTF("[%s]...e1->name: %s \n",__func__,e1->name);
            //DEBUG_PRINTF("[%s]...o->v.string:%s \n", __func__, o1->v.string);

            if(!strcmp(e1->name,"name"))
            {
                strncpy(rule->name,o1->v.string,sizeof(rule->name));
            }
            else if(!strcmp(e1->name,"target"))
            {
                strncpy(rule->target,o1->v.string,sizeof(rule->target));
            }
            else if(!strcmp(e1->name,"src_mac"))
            {
                strncpy(rule->src_mac,o1->v.string,sizeof(rule->src_mac));
            }
            else if(!strcmp(e1->name,"src"))
            {
                strncpy(rule->src,o1->v.string,sizeof(rule->src));
            }
            else if(!strcmp(e1->name,"dest"))
            {
                strncpy(rule->dest,o1->v.string,sizeof(rule->dest));
            }
            else if(!strcmp(e1->name,"src_ip"))
            {
                strncpy(rule->src_ip,o1->v.string,sizeof(rule->src_ip));
            }
            else if(!strcmp(e1->name,"dest_ip"))
            {
                strncpy(rule->dest_ip,o1->v.string,sizeof(rule->dest_ip));
            }
            else if(!strcmp(e1->name,"enabled"))
            {
                if(!strcmp(o1->v.string,"true") || !strcmp(o1->v.string,"yes") || !strcmp(o1->v.string,"1"))
                    rule->enabled=true;
                else
                    rule->enabled=false;
            } 

            //child rule
            else if(!strcmp(e1->name,"start_time"))
            {
                strncpy(rule->start_time,o1->v.string,sizeof(rule->start_time));
            }
            else if(!strcmp(e1->name,"stop_time"))
            {
                strncpy(rule->stop_time,o1->v.string,sizeof(rule->stop_time));
            }
            else if(!strcmp(e1->name,"weekdays"))
            {
                strncpy(rule->weekdays,o1->v.string,sizeof(rule->weekdays));
            }

            //limitspeed
            else if(!strcmp(e1->name,"limit"))
            {
                char tmp_str[64]={0};
                strncpy(tmp_str,o1->v.string,sizeof(tmp_str));
                sscanf(tmp_str,"%d/sec", &(rule->limit));
            }
        }

        //printf_rule(rule);

    }

	uci_free_context(global_uci);
    global_uci=NULL;

    DEBUG_PRINTF("==leave [%s]====\n",__func__);
}

void fw_load_redirect()
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    INIT_LIST_HEAD(&global_redirects);
    
    //´Óuci »ñÈ¡firewall ÐÅÏ¢
    struct uci_package *p = NULL;
    struct uci_section *s;
    struct uci_element *e;
    struct fw3_redirects *redirect;
	struct uci_context *global_uci;
    global_uci=uci_alloc_context();
    
    if(!global_uci)
    {
        DEBUG_PRINTF("[%s]===out of memory==\n",__func__);
        exit(1);
    }

    if (uci_load(global_uci, "firewall", &p))
	{
		uci_perror(global_uci, NULL);
		DEBUG_PRINTF("[%s]===Failed to load /etc/config/firewall",__func__);
        exit(1);
	}

    uci_foreach_element(&p->sections, e)
	{
		s = uci_to_section(e);
        //DEBUG_PRINTF("s->type:%s====\n",s->type);

        //ÔÚÕâ¸öÄ£¿éÀïÎÒÃÇÖ»´¦Àíredirect
        if(strcmp(s->type,"redirect"))
        {
            continue;
        }

        if(!(redirect=alloc_redirect()))
        {
            DEBUG_PRINTF("[%s]=====alloc_redirect fail ====\n",__func__);
            continue;
        }
        
        struct uci_element *e1;
        struct uci_option *o1;
        uci_foreach_element(&s->options, e1)
        {
            o1=uci_to_option(e1);

            //DEBUG_PRINTF("[%s]...e1->name: %s \n",__func__,e1->name);
            //DEBUG_PRINTF("[%s]...o->v.string:%s \n", __func__, o1->v.string);

            if(!strcmp(e1->name,"name"))
            {
                strncpy(redirect->name,o1->v.string,sizeof(redirect->name));
            }
            else if(!strcmp(e1->name,"target"))
            {
                strncpy(redirect->target,o1->v.string,sizeof(redirect->target));
            }
            else if(!strcmp(e1->name,"src"))
            {
                strncpy(redirect->src,o1->v.string,sizeof(redirect->src));
            }
            else if(!strcmp(e1->name,"dest"))
            {
                strncpy(redirect->dest,o1->v.string,sizeof(redirect->dest));
            }
            else if(!strcmp(e1->name,"src_ip"))
            {
                strncpy(redirect->src_ip,o1->v.string,sizeof(redirect->src_ip));
            }
            else if(!strcmp(e1->name,"dest_ip"))
            {
                strncpy(redirect->dest_ip,o1->v.string,sizeof(redirect->dest_ip));
            }
            else if(!strcmp(e1->name,"enabled"))
            {
                if(!strcmp(o1->v.string,"true") || !strcmp(o1->v.string,"yes") || !strcmp(o1->v.string,"1"))
                    redirect->enabled=true;
                else
                    redirect->enabled=false;
            }
            else if(!strcmp(e1->name,"proto"))
            {
                strncpy(redirect->proto,o1->v.string,sizeof(redirect->proto));
            }
            else if(!strcmp(e1->name,"src_dport"))
            {
                strncpy(redirect->src_dport,o1->v.string,sizeof(redirect->src_dport));
            }
            else if(!strcmp(e1->name,"dest_port"))
            {
                strncpy(redirect->dest_port,o1->v.string,sizeof(redirect->dest_port));
            }

        }


    }
	
	uci_free_context(global_uci);
    global_uci=NULL;
    DEBUG_PRINTF("==leave [%s]====\n",__func__);
}



//è¾“å‡ºï¼šmac-host çš„entryæ•°ç»„ï¼Œ æ•°ç»„å…ƒç´ ä¸ªæ•°
void qtec_firewall_pase_dhcp_file(struct Qtec_firewall_DeviceEntry *array, int *num)
{
    FILE *fp=NULL;
    if( (fp=fopen(qtec_firewall_DHCP_FILE,"r"))==NULL )
    {
        DEBUG_PRINTF("====ERROR!!! == %s=== can't open FILE: "qtec_firewall_DHCP_FILE"====\n",__func__);
        return ;
    }
    int i=0; 
    char str[1024]={0};
    char tmpchar1[64]={0};
    char tmpchar2[64]={0};
    char tmpchar3[64]={0};
    char tmpchar4[64]={0};
    char tmpchar5[64]={0};
    while( (fgets(str,1024,fp)) !=NULL )
    {
        DEBUG_PRINTF("===func:%s ==== str==== %s===\n",__func__,str);
        //for example:
        //1497447462 f4:31:c3:10:87:ec 192.168.1.118 iphone 01:f4:31:31:c3:10:87:ec
        sscanf(str,"%s %s %s %s %s",tmpchar1,tmpchar2,tmpchar3,tmpchar4,tmpchar5);
        //DEBUG_PRINTF("==tmpchar1: %s====\n",tmpchar1);
        //DEBUG_PRINTF("==tmpchar2: %s====\n",tmpchar2);
        //DEBUG_PRINTF("==tmpchar3: %s====\n",tmpchar3);
        //DEBUG_PRINTF("==tmpchar4: %s====\n",tmpchar4);
        //DEBUG_PRINTF("==tmpchar5: %s====\n",tmpchar5);
        #if 0   
        for(i=0;i<lanHostEntryTableNum;i++)
        {
            if ( strncmp(lanHostEntryTable[i].macaddr,tmpchar2,strlen(tmpchar2))==0 )
            {
                memset(lanHostEntryTable[i].hostname,0,sizeof(lanHostEntryTable[i].hostname));
                strncpy(lanHostEntryTable[i].hostname,tmpchar4, sizeof(lanHostEntryTable[i].hostname));
                lanHostEntryTable[i].connection_type=1;
                break;
            }
        }
        #endif
        strncpy(array[i].hostname,tmpchar4,sizeof(array[i].hostname));
        strncpy(array[i].mac,tmpchar2,sizeof(array[i].mac));
        i++;
        memset(str,0,1024);
        memset(tmpchar1,0,64);
        memset(tmpchar2,0,64);
        memset(tmpchar3,0,64);
        memset(tmpchar4,0,64);
        memset(tmpchar5,0,64);
    }
    *num=i;
    fclose(fp);
#if 1
    for(i=0;i<*num;i++)
    {
        DEBUG_PRINTF("[%s]===array[%d] mac:%s   hostname:%s  ===\n",__func__,i,array[i].mac,array[i].hostname);
    }
#endif
    return 0;
}

