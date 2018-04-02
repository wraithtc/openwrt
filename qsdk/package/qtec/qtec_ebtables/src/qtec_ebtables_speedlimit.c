#include "qtec_ebtables_basic.h"

//this c file code just for cgi api
#define DHCP_FILE "/tmp/dhcp.leases"
#define max_device_num 128
#define hostname_len  64

struct DeviceEntry
{
    char mac[64];
    char hostname[hostname_len];
};

//è¾“å‡ºï¼šmac-host çš„entryæ•°ç»„ï¼Œ æ•°ç»„å…ƒç´ ä¸ªæ•°
void pase_dhcp_file(struct DeviceEntry *array, int *num)
{
    FILE *fp=NULL;
    if( (fp=fopen(DHCP_FILE,"r"))==NULL )
    {
        DEBUG_PRINTF("====ERROR!!! == %s=== can't open FILE: "DHCP_FILE"====\n",__func__);
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
#if 0
    for(i=0;i<*num;i++)
    {
        DEBUG_PRINTF("[%s]===array[%d] mac:%s   hostname:%s  ===\n",__func__,i,array[i].mac,array[i].hostname);
    }
#endif
    return 0;
}

int ebtables_get_downlimit(char *input_mac, char* hostname, int *output_enabled)
{
    DEBUG_PRINTF("===[%s]===input_mac:%s =====\n",__func__,input_mac);
    fw_load_ebtables();

    struct list_head *tmp=global_ebtables_speedlimit_rule.next;
    struct ebtables_speedlimit_rule *rule=NULL;
    
    int downlimit=0;
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    char tmp_char[64]={0};
    snprintf(name,256,"downlimit_%s",input_mac);
    while(tmp!=&global_ebtables_speedlimit_rule)
    {
        rule=(struct ebtables_speedlimit_rule* )tmp;

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

    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].limit",index);
    rtcfgUciGet(cmd,tmp_char);
    //sscanf(tmp_char,"%d/sec",&downlimit);
    downlimit = atoi(tmp_char);
    
    memset(tmp_char,0,64);
    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].enabled",index);
    rtcfgUciGet(cmd,tmp_char);
    *output_enabled=atoi(tmp_char);

    memset(tmp_char,0,64);
    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].hostname",index);
    rtcfgUciGet(cmd,tmp_char);
    strncpy(hostname,tmp_char,hostname_len);
    DEBUG_PRINTF("[%s]==get=hostname:%s\n",__func__,hostname);
    if((strncmp(hostname,"unknow",sizeof("unknow"))==0)||(strlen(hostname)==0))
    {
         
        struct DeviceEntry array[max_device_num];
        memset(array,0,sizeof(array));
        int device_num=0;
           
        pase_dhcp_file(array, &device_num);
        int i =0;
           
        for(i=0;i<device_num;i++)
        {
            //DEBUG_PRINTF("[%s]==  array[%d] mac:%s  hostname:%s===\n",__func__,i,array[i].mac,array[i].hostname);
            //DEBUG_PRINTF("[%s]===input_mac:%s===\n",__func__,input_mac);
            if(strncasecmp(input_mac,array[i].mac,sizeof(array[i].mac))==0)
            {
                memset(hostname,0,hostname_len);
                strncpy(hostname,array[i].hostname,hostname_len);
                DEBUG_PRINTF("[%s] reget hostname:%s===\n",__func__,hostname);

                memset(cmd,0,256);
                snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].hostname=%s",index,hostname);
                rtcfgUciSet(cmd);

                rtcfgUciCommit("qtec_ebtables");
                break;
            }
        }
   }

    qtec_ebtables_free_list(&global_ebtables_speedlimit_rule);
    
    return downlimit;
}

int ebtables_add_downlimit_rule(char *input_mac, int input_limit, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]=== input_mac:%s input_limit:%d  input_enabled:%d ==\n",__func__,input_mac,input_limit,input_enabled);

    if(input_limit==0)
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //²Ù×÷uci
    rtcfgUciAdd("qtec_ebtables","speedlimitrule");
    
    char name[256]={0};
    char cmd[256]={0};

    snprintf(name,256,"downlimit_%s",input_mac);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[-1].name=%s",name);
    rtcfgUciSet(cmd);

    rtcfgUciSet("qtec_ebtables.@speedlimitrule[-1].dest=0"); //0:input 1:output


    if(input_enabled == true)
    {
        rtcfgUciSet("qtec_ebtables.@speedlimitrule[-1].enabled=1");
    }
    else 
    {
        rtcfgUciSet("qtec_ebtables.@speedlimitrule[-1].enabled=0");
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[-1].limit=%d",input_limit);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[-1].mac=%s",input_mac);
    rtcfgUciSet(cmd);

    //æ–°åŠ ä¸€ä¸ªå­—æ®µæ˜¾ç¤ºhostname
    struct DeviceEntry array[max_device_num];
    int device_num;
    char hostname[hostname_len]="unknow";
    pase_dhcp_file(array, &device_num);

    int i =0;
    for(i=0;i<device_num;i++)
    {
        if(strncasecmp(input_mac,array[i].mac,sizeof(array[i].mac))==0)
        {
            memset(hostname,0,hostname_len);
            strncpy(hostname,array[i].hostname,hostname_len);
            break;
        }
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[-1].hostname=%s",hostname);
    rtcfgUciSet(cmd);
    
    return 0;
}


int ebtables_del_downlimit_rule(char *input_mac, int input_limit, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]=== input_mac:%s input_limit:%d  input_enabled:%d ==\n",__func__,input_mac,input_limit,input_enabled);

    if(input_limit==0)
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //¼ÓÔØÏµÍ³ÄÚuciµÄÊý¾Ý
    fw_load_ebtables();

    struct list_head *tmp=global_ebtables_speedlimit_rule.next;
    struct ebtables_speedlimit_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    snprintf(name,256,"downlimit_%s",input_mac);
    while(tmp!=&global_ebtables_speedlimit_rule)
    {
        rule=(struct ebtables_speedlimit_rule*)tmp;

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

    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d]",index);
    rtcfgUciDel(cmd);
	qtec_ebtables_free_list(&global_ebtables_speedlimit_rule);
    return 0;
}


int ebtables_mod_downlimit_rule(char *input_mac, int old_limit,  bool old_enabled, int new_limit,  bool new_enabled)
{
    DEBUG_PRINTF("====[%s]==input_mac:%s=== old:limit:%d enabled:%d==\n",__func__,input_mac,old_limit,old_enabled);
    DEBUG_PRINTF("====[%s]==new=limit:%d enabled:%d ==\n",__func__,new_limit,new_enabled);

    if( (old_limit==0) || (new_limit==0) )
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //¼ÓÔØÏµÍ³ÄÚuciµÄÊý¾Ý
    fw_load_ebtables();

    struct list_head *tmp=global_ebtables_speedlimit_rule.next;
    struct ebtables_speedlimit_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    snprintf(name,256,"downlimit_%s",input_mac);
    
    while(tmp!=&global_ebtables_speedlimit_rule)
    {
        rule=(struct ebtables_speedlimit_rule*)tmp;

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
        snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].enabled=1",index);
        rtcfgUciSet(cmd);
    }
    else
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].enabled=0",index);
        rtcfgUciSet(cmd);
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].limit=%d",index,new_limit);
    rtcfgUciSet(cmd);
	qtec_ebtables_free_list(&global_ebtables_speedlimit_rule);
    
    return 0;
}


//uplimit 
int ebtables_get_uplimit(char *input_mac, char* hostname, int *output_enabled)
{
    DEBUG_PRINTF("===[%s]===input_mac:%s =====\n",__func__,input_mac);
    fw_load_ebtables();

    struct list_head *tmp=global_ebtables_speedlimit_rule.next;
    struct ebtables_speedlimit_rule *rule=NULL;
    
    int uplimit=0;
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    char tmp_char[64]={0};
    snprintf(name,256,"uplimit_%s",input_mac);
    while(tmp!=&global_ebtables_speedlimit_rule)
    {
        rule=(struct ebtables_speedlimit_rule* )tmp;

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

    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].limit",index);
    rtcfgUciGet(cmd,tmp_char);
    //sscanf(tmp_char,"%d/sec",&downlimit);
    uplimit = atoi(tmp_char);
    
    memset(tmp_char,0,64);
    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].enabled",index);
    rtcfgUciGet(cmd,tmp_char);
    *output_enabled=atoi(tmp_char);
    
    memset(tmp_char,0,64);
    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].hostname",index);
    rtcfgUciGet(cmd,tmp_char);
    strncpy(hostname,tmp_char,hostname_len);
    DEBUG_PRINTF("[%s]==get=hostname:%s\n",__func__,hostname);
    if((strncmp(hostname,"unknow",sizeof("unknow"))==0)||(strlen(hostname)==0))
    {
      
        struct DeviceEntry array[max_device_num];
        memset(array,0,sizeof(array));
        int device_num=0;
        
        pase_dhcp_file(array, &device_num);
        int i =0;
        
        for(i=0;i<device_num;i++)
        {
            //DEBUG_PRINTF("[%s]==  array[%d] mac:%s  hostname:%s===\n",__func__,i,array[i].mac,array[i].hostname);
            //DEBUG_PRINTF("[%s]===input_mac:%s===\n",__func__,input_mac);
            if(strncasecmp(input_mac,array[i].mac,sizeof(array[i].mac))==0)
            {
                memset(hostname,0,hostname_len);
                
                strncpy(hostname,array[i].hostname,hostname_len);
                DEBUG_PRINTF("[%s] reget hostname:%s===\n",__func__,hostname);
                
                memset(cmd,0,256);
                snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].hostname=%s",index,hostname);
                rtcfgUciSet(cmd);

                rtcfgUciCommit("qtec_ebtables");
                break;
            }
        }
    }
    
    qtec_ebtables_free_list(&global_ebtables_speedlimit_rule);
    
    return uplimit;
}

int ebtables_add_uplimit_rule(char *input_mac, int input_limit, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]=== input_mac:%s input_limit:%d  input_enabled:%d ==\n",__func__,input_mac,input_limit,input_enabled);

    if(input_limit==0)
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //²Ù×÷uci
    rtcfgUciAdd("qtec_ebtables","speedlimitrule");
    
    char name[256]={0};
    char cmd[256]={0};

    snprintf(name,256,"uplimit_%s",input_mac);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[-1].name=%s",name);
    rtcfgUciSet(cmd);

    rtcfgUciSet("qtec_ebtables.@speedlimitrule[-1].dest=1"); //0:input 1:output


    if(input_enabled == true)
    {
        rtcfgUciSet("qtec_ebtables.@speedlimitrule[-1].enabled=1");
    }
    else 
    {
        rtcfgUciSet("qtec_ebtables.@speedlimitrule[-1].enabled=0");
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[-1].limit=%d",input_limit);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[-1].mac=%s",input_mac);
    rtcfgUciSet(cmd);

    //æ–°åŠ ä¸€ä¸ªå­—æ®µæ˜¾ç¤ºhostname
    struct DeviceEntry array[max_device_num];
    int device_num;
    char hostname[hostname_len]="unknow";
    pase_dhcp_file(array, &device_num);

    int i =0;
    for(i=0;i<device_num;i++)
    {
        if(strncasecmp(input_mac,array[i].mac,sizeof(array[i].mac))==0)
        {
            memset(hostname,0,hostname_len);
            strncpy(hostname,array[i].hostname,hostname_len);
            break;
        }
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[-1].hostname=%s",hostname);
    rtcfgUciSet(cmd);
    return 0;
}


int ebtables_del_uplimit_rule(char *input_mac, int input_limit, bool input_enabled)
{
    DEBUG_PRINTF("====[%s]=== input_mac:%s input_limit:%d  input_enabled:%d ==\n",__func__,input_mac,input_limit,input_enabled);

    if(input_limit==0)
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //¼ÓÔØÏµÍ³ÄÚuciµÄÊý¾Ý
    fw_load_ebtables();

    struct list_head *tmp=global_ebtables_speedlimit_rule.next;
    struct ebtables_speedlimit_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    snprintf(name,256,"uplimit_%s",input_mac);
    while(tmp!=&global_ebtables_speedlimit_rule)
    {
        rule=(struct ebtables_speedlimit_rule*)tmp;

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

    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d]",index);
    rtcfgUciDel(cmd);
	qtec_ebtables_free_list(&global_ebtables_speedlimit_rule);
    return 0;
}


int ebtables_mod_uplimit_rule(char *input_mac, int old_limit,  bool old_enabled, int new_limit,  bool new_enabled)
{
    DEBUG_PRINTF("====[%s]==input_mac:%s=== old:limit:%d enabled:%d==\n",__func__,input_mac,old_limit,old_enabled);
    DEBUG_PRINTF("====[%s]==new=limit:%d enabled:%d ==\n",__func__,new_limit,new_enabled);

    if( (old_limit==0) || (new_limit==0) )
    {
        DEBUG_PRINTF("[%s]===input limit is 0, no limit===\n",__func__);
        return -2;
    }
    
    //¼ÓÔØÏµÍ³ÄÚuciµÄÊý¾Ý
    fw_load_ebtables();

    struct list_head *tmp=global_ebtables_speedlimit_rule.next;
    struct ebtables_speedlimit_rule *rule=NULL;
  
    int index=0;
    char cmd[256]={0};
    int found=-1;
    char name[256]={0};
    snprintf(name,256,"uplimit_%s",input_mac);
    
    while(tmp!=&global_ebtables_speedlimit_rule)
    {
        rule=(struct ebtables_speedlimit_rule*)tmp;

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
        snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].enabled=1",index);
        rtcfgUciSet(cmd);
    }
    else
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].enabled=0",index);
        rtcfgUciSet(cmd);
    }

    memset(cmd,0,256);
    snprintf(cmd,256,"qtec_ebtables.@speedlimitrule[%d].limit=%d",index,new_limit);
    rtcfgUciSet(cmd);
	qtec_ebtables_free_list(&global_ebtables_speedlimit_rule);
    
    return 0;
}

