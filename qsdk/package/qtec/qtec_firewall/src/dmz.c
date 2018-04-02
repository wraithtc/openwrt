#include "qtec_firewall_basic.h"

int get_dmz_rule(struct fw3_dmz* result_dmz)
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    INIT_LIST_HEAD(&global_redirects);
    fw_load_redirect();

    struct fw3_redirects* redirect=NULL;
    int found =-1;
    int index=0;
    DEBUG_PRINTF("[%s]====±éÀúglobal_redirects===\n",__func__);
    struct list_head *tmp=global_redirects.next;
    
    while(tmp!=&global_redirects)
    {
        
        //printf_rule(tmp);
        redirect=(struct fw3_redirects*)tmp;
        if(!strncmp(redirect->name, "DMZ",strlen("DMZ")))
        {
            result_dmz->enabled = redirect->enabled;
            strcpy(result_dmz->dest_ip,redirect->dest_ip);
            found=index;
            break;
        }
        index++;
        tmp=tmp->next;
    }
    if(found==-1)
    {
        DEBUG_PRINTF("[%s]====cannot found DMZ rule in cur config====\n",__func__);
        return -1;
    }
    DEBUG_PRINTF("[%s]===return found: %d===\n",__func__,found);
	qtec_fw_free_list(&global_redirects);
    return found;
}
    

int set_dmz_rule(char *input_dest_ip, bool enabled)
{
    DEBUG_PRINTF("[%s]===input_dest_ip:%s enabled:%d===\n",__func__,input_dest_ip,enabled);
    int ret=0;
    struct fw3_dmz tmp_dmz={0};
    ret=get_dmz_rule(&tmp_dmz);
    char name[64]={0};
    char cmd[256]={0};
    if(ret==-1)
    {
        DEBUG_PRINTF("[%s]====create dmz rule====\n",__func__);

        //²Ù×÷uci
        rtcfgUciAdd("firewall","redirect");

    
        snprintf(cmd,256,"firewall.@redirect[-1].name=DMZ");
        rtcfgUciSet(cmd);

        rtcfgUciSet("firewall.@redirect[-1].src=wan");

        

        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@redirect[-1].dest_ip=%s",input_dest_ip);
        rtcfgUciSet(cmd);

        rtcfgUciSet("firewall.@redirect[-1].target=DNAT");

        rtcfgUciSet("firewall.@redirect[-1].family=ipv4");

        if(enabled == true)
        {
            rtcfgUciSet("firewall.@redirect[-1].enabled=1");
        }
        else 
        {
            rtcfgUciSet("firewall.@redirect[-1].enabled=0");
        }
    }
    else
    {
        DEBUG_PRINTF("[%s]====modify dmz rule====\n",__func__);
        snprintf(cmd,256,"firewall.@redirect[%d].dest_ip=%s",ret,input_dest_ip);
        rtcfgUciSet(cmd);

        if(enabled == true)
        {
            memset(cmd,0,256);
            snprintf(cmd,256,"firewall.@redirect[%d].enabled=1",ret);
            rtcfgUciSet(cmd);
        }
        else 
        {
            memset(cmd,0,256);
            snprintf(cmd,256,"firewall.@redirect[%d].enabled=0",ret);
            rtcfgUciSet(cmd);
        }
        
    }   

    restart_fw(0);
    return 0;
    
}