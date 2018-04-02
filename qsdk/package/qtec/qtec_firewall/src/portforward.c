#include "qtec_firewall_basic.h"

static int check_pf_port_conflict(char *port_A, char *port_B)
{
    DEBUG_PRINTF("===[%s]====port_A:%s   port_B:%s ====\n",__func__,port_A,port_B);
    int src_min_port_A=0;
    int src_max_port_A=0;
    int src_min_port_B=0;
    int src_max_port_B=0;
    int n=0;
    int m=0;
    char *p;

    //check A port
    n=strtoul(port_A,&p,10);
    if(errno == ERANGE || errno == EINVAL)
    {
        DEBUG_PRINTF("[%s]==== A port value wrong====\n",__func__);
        return -1;
    }

    if(*p && *p !='-' && *p !=':')
    {
        DEBUG_PRINTF("[%s]====A port value wrong===\n",__func__);
        return -1;
    }
    if(*p)
    {
        m=strtoul(++p,NULL,10);
        if(errno == ERANGE || errno == EINVAL || m<n)
        {
            DEBUG_PRINTF("[%s]====A port value wrong====\n",__func__);
            return -1;
        }
        src_min_port_A=n;
        src_max_port_A=m;
    }
    else
    {
        src_min_port_A=n;
        src_max_port_A=n;
    }

    
    //check B port
    n=strtoul(port_B,&p,10);
    if(errno == ERANGE || errno == EINVAL)
    {
        DEBUG_PRINTF("[%s]====B port value wrong====\n",__func__);
        return -1;
    }

    if(*p && *p !='-' && *p !=':')
    {
        DEBUG_PRINTF("[%s]====B port value wrong===\n",__func__);
        return -1;
    }
    if(*p)
    {
        m=strtoul(++p,NULL,10);
        if(errno == ERANGE || errno == EINVAL || m<n)
        {
            DEBUG_PRINTF("[%s]====B port value wrong====\n",__func__);
            return -1;
        }
        src_min_port_B=n;
        src_max_port_B=m;
    }
    else
    {
        src_min_port_B=n;
        src_max_port_B=n;
    }

    DEBUG_PRINTF("[%s]====src_min_port_A:%d  src_max_port_A:%d   src_min_port_B:%d  src_max_port_B:%d ====\n",__func__,src_min_port_A,src_max_port_A,src_min_port_B,src_max_port_B);

    if( (src_min_port_B > src_max_port_A) || (src_max_port_B < src_min_port_A) )
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

//0: no conflict; -1: have conflict
static int check_pf_conflict(struct fw3_pf *entry, char *input_proto, char* input_ip, char *input_src_dport, char *input_dest_port)
{

    int proto_entry=0; //0:none 1:TCP 2:UDP 3:TCP UDP
    int proto_input=0;
    int ret=0;
  
    
    DEBUG_PRINTF("===[%s]====\n",__func__);


    if( (!strncasecmp(entry->proto,"tcp udp",strlen("tcp udp"))) || (!strncasecmp(entry->proto,"udp tcp",strlen("udp tcp"))) )
    {
        proto_entry=3;
    }
    else if( !strncasecmp(entry->proto,"tcp",strlen(entry->proto)) )
    {
        proto_entry =1;
    }
    else if( !strncasecmp(entry->proto,"udp",strlen(entry->proto)) )
    {
        proto_entry =2;
    }


    if( (!strncasecmp(input_proto,"tcp udp",strlen("tcp udp"))) || (!strncasecmp(input_proto,"udp tcp",strlen("udp tcp"))) )
    {
        proto_input=3;
    }
    else if( !strncasecmp(input_proto,"tcp",strlen(input_proto)) )
    {
        proto_input=1;
    }
    else if( !strncasecmp(input_proto,"udp",strlen(entry->proto)) )
    {
        proto_input =2;
    }

    DEBUG_PRINTF("[%s]======proto_entry:%d ==== proto_input:%d =====\n",__func__,proto_entry,proto_input);
    
    if( (proto_entry & proto_input) == 0)
    {
        DEBUG_PRINTF("[%s]===proto no contain====\n",__func__);
        return 0;
    }

    ret=check_pf_port_conflict(entry->src_dport,input_src_dport);
    if(ret !=0)
    {
        DEBUG_PRINTF("[%s]====src_dport conflict===\n",__func__);
        return -1;
    }
    
    if(strncmp(entry->dest_ip,input_ip,sizeof(entry->dest_ip)))
    {
        DEBUG_PRINTF("[%s]===not the same ip====\n",__func__);
        return 0;
    }

    ret =check_pf_port_conflict(entry->dest_port,input_dest_port);
    if(ret!=0)
    {
        DEBUG_PRINTF("[%s]====dest port conflict====\n",__func__);
        return -1;
    }

    return 0;
}

static int check_pf_value_add(char *input_proto, char *input_ip, char *input_src_dport,char *input_dest_port)
{
    DEBUG_PRINTF("[%s]====input_proto:%s   input_ip:%s  input_src_dport:%s input_dest_port: %s =====\n",__func__,input_proto,input_ip,input_src_dport,input_dest_port);
    int ret=0;
    struct list_head *tmp=NULL;
    struct fw3_pf *pf;
    get_pftable();
    tmp=global_pf_rules.next;

    {
        while(tmp!=&global_pf_rules)
        {
            pf=(struct fw3_pf *)tmp;
            
            ret=check_pf_conflict(pf,input_proto,input_ip,input_src_dport,input_dest_port);
            if(ret !=0)
            {
                return -1;
            }
            
            tmp=tmp->next;
        }
    }


    qtec_fw_free_list(&global_pf_rules);
    return 0;
}

static int check_pf_value_mod(char *input_proto,char *input_src_dport, char *input_dest_ip,char *input_dest_port,char *new_input_proto, char *new_input_src_dport, char *new_input_dest_ip,char *new_input_dest_port)
{
    int ret=0;
    struct list_head *tmp=NULL;
    struct fw3_pf *pf;
    get_pftable();
    tmp=global_pf_rules.next;

    while(tmp!=&global_pf_rules)
    {
        pf=(struct fw3_pf *)tmp;

        if ((!strcmp(pf->proto,input_proto)) && (!strcmp(pf->src_dport,input_src_dport)) && (!strcmp(pf->dest_ip,input_dest_ip)) && (!strcmp(pf->dest_port,input_dest_port)) )
        {
            tmp=tmp->next;
            continue;
        }

        ret=check_pf_conflict(pf,new_input_proto,new_input_dest_ip,new_input_src_dport,new_input_dest_port);
        if(ret !=0)
        {
            return -1;
        }
            
        tmp=tmp->next;
            
    }

    qtec_fw_free_list(&global_pf_rules);
    return 0;
}

int get_pftable()
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    INIT_LIST_HEAD(&global_pf_rules);
    //加载系统内uci的数据
    fw_load_redirect();

    struct fw3_pf *pf=NULL;
    struct fw3_redirects *redirect=NULL;
    DEBUG_PRINTF("[%s]====遍历global_redirect===\n",__func__);
    struct list_head *tmp=global_redirects.next;
    
    while(tmp!=&global_redirects)
    {
        
        //printf_rule(tmp);
        redirect=(struct fw3_redirects*)tmp;
        if( (strncmp(redirect->name, "DMZ",strlen("DMZ"))) && (!strncmp(redirect->target,"DNAT",strlen("DNAT"))) )
        {
            pf=(struct fw3_pf*) alloc_pf();
            pf->enabled=redirect->enabled;
         
            strcpy(pf->name,redirect->name);
            strcpy(pf->proto,redirect->proto);
            strcpy(pf->src_dport,redirect->src_dport);
            strcpy(pf->dest_ip,redirect->dest_ip);
            strcpy(pf->dest_port,redirect->dest_port);

        }
        
        tmp=tmp->next;
      
    }
    DEBUG_PRINTF("leave [%s]====\n",__func__);
	qtec_fw_free_list(&global_redirects);
    return 0;
}



int add_pf(char *input_name,char *input_proto,char *input_src_dport, char *input_dest_ip,char *input_dest_port, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]===input_name:%s input_proto:%s input_src_dport:%s input_dest_ip:%s input_dest_port:%s input_enabled:%d ==\n",__func__,input_name,input_proto,input_src_dport,input_dest_ip,input_dest_port,input_enabled);

    int ret=0;
    ret=check_pf_value_add(input_proto, input_dest_ip,input_src_dport,input_dest_port);
    if(ret != 0)
    {
        DEBUG_PRINTF("==[%s]====value wrong ====\n",__func__);
        return -1;
    }
    //操作uci
    rtcfgUciAdd("firewall","redirect");
    char name[64]={0};
    char cmd[256]={0};

    snprintf(cmd,256,"firewall.@redirect[-1].name=%s",input_name);
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@redirect[-1].src=wan");

    rtcfgUciSet("firewall.@redirect[-1].dest=lan");

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[-1].proto=%s",input_proto);
    rtcfgUciSet(cmd);

    rtcfgUciSet("firewall.@redirect[-1].target=DNAT");


    if(input_enabled == true)
    {
        rtcfgUciSet("firewall.@redirect[-1].enabled=1");
    }
    else 
    {
        rtcfgUciSet("firewall.@redirect[-1].enabled=0");
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[-1].src_dport=%s",input_src_dport);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[-1].dest_ip=%s",input_dest_ip);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[-1].dest_port=%s",input_dest_port);
    rtcfgUciSet(cmd);
 
    return 0;
}


int del_pf(char *input_name,char *input_proto,char *input_src_dport, char *input_dest_ip,char *input_dest_port, bool input_enabled)
{
     DEBUG_PRINTF("====[%s]===input_name:%s input_proto:%s input_src_dport:%s input_dest_ip:%s input_dest_port:%s input_enabled:%d ==\n",__func__,input_name,input_proto,input_src_dport,input_dest_ip,input_dest_port,input_enabled);

    //加载系统内uci的数据
    fw_load_redirect();

    struct list_head *tmp=global_redirects.next;
    struct fw3_pf *pf=NULL;
    struct fw3_redirects *redirect;
    int index=0;
    char cmd[256]={0};
    int found=-1;

    
    while(tmp!=&global_redirects)
    {
        redirect=(struct fw3_redirects*)tmp;
        

        if( (!strcmp(redirect->name,input_name)) && (!strcmp(redirect->proto,input_proto)) && (redirect->enabled==input_enabled) && (!strcmp(redirect->src_dport,input_src_dport)) && (!strcmp(redirect->dest_ip,input_dest_ip)) && (!strcmp(redirect->dest_port,input_dest_port)) )
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

    snprintf(cmd,256,"firewall.@redirect[%d]",index);
    rtcfgUciDel(cmd);
	qtec_fw_free_list(&global_redirects);
    return 0;
     

}

int mod_pf(char *input_name,char *input_proto,char *input_src_dport, char *input_dest_ip,char *input_dest_port, bool input_enabled,char *new_input_name,char *new_input_proto, char *new_input_src_dport, char *new_input_dest_ip,char *new_input_dest_port,bool new_input_enabled)
{
    DEBUG_PRINTF("====[%s]==old=input_name:%s input_proto:%s input_src_dport:%s input_dest_ip:%s input_dest_port:%s input_enabled:%d ==\n",__func__,input_name,input_proto,input_src_dport,input_dest_ip,input_dest_port,input_enabled);
    DEBUG_PRINTF("====[%s]==new=input_name:%s input_proto:%s input_src_dport:%s input_dest_ip:%s input_dest_port:%s input_enabled:%d ==\n",__func__,new_input_name,new_input_proto,new_input_src_dport,new_input_dest_ip,new_input_dest_port,input_enabled);
    int ret=0;
    ret=check_pf_value_mod(input_proto,input_src_dport,input_dest_ip,input_dest_port, new_input_proto,new_input_src_dport,new_input_dest_ip,new_input_dest_port);
    if(ret !=0)
    {
        DEBUG_PRINTF("===[%s]====value wrong====\n",__func__);
        return -2;
    }
    
    fw_load_redirect();

    struct list_head *tmp=global_redirects.next;
    struct fw3_pf *pf=NULL;
    struct fw3_redirects *redirect;
    int index=0;
    char cmd[256]={0};
    int found=-1;

    
    while(tmp!=&global_redirects)
    {
        redirect=(struct fw3_redirects*)tmp;
        

        if( (!strcmp(redirect->name,input_name)) && (!strcmp(redirect->proto,input_proto)) && (redirect->enabled==input_enabled) && (!strcmp(redirect->src_dport,input_src_dport)) && (!strcmp(redirect->dest_ip,input_dest_ip)) && (!strcmp(redirect->dest_port,input_dest_port)) )
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
    snprintf(cmd,256,"firewall.@redirect[%d].name=%s",index,new_input_name);
    rtcfgUciSet(cmd);
        
    if(new_input_enabled == true)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@redirect[%d].enabled=1",index);
        rtcfgUciSet(cmd);
    }
    else
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"firewall.@redirect[%d].enabled=0",index);
        rtcfgUciSet(cmd);
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[%d].proto=%s",index,new_input_proto);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[%d].src_dport=%s",index,new_input_src_dport);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[%d].dest_ip=%s",index,new_input_dest_ip);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"firewall.@redirect[%d].dest_port=%s",index,new_input_dest_port);
    rtcfgUciSet(cmd);
	qtec_fw_free_list(&global_redirects);
    return 0;
}
