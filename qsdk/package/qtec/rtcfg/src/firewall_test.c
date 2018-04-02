#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "firewall_set.h"
#include "rtcfg_uci.h"


void main()
{

    printf("==========firewall  test========\n");
#if 0
    struct portForwardRule *outputarray=NULL;
    int num=0;
    int i =0;
    outputarray=getPortForwardRuleTable(&num);
    
    if(num!=0)
    {
        for(i=0;i<num;i++)
        {
            printf("====array[%d].name: %s ===\n",i,outputarray[i].name);
            printf("====array[%d].proto: %s ========\n",i,outputarray[i].proto);
            printf("====array[%d].src_dport: %d =====\n",i,outputarray[i].src_dport);
            printf("====array[%d].dest_ip: %s =======\n",i,outputarray[i].dest_ip);
            printf("====array[%d].dest_port: %d =====\n",i,outputarray[i].dest_port);
        }
    }
    else
    {
        printf("====there is no portForwardRule now=======\n");
    }
    
    if(outputarray!=NULL)
    {
        free(outputarray);
    }

    struct portForwardRule entry;
    memset(&entry,0,sizeof(struct portForwardRule));
    strcpy(entry.name, "djj1");
    strcpy(entry.proto,"tcp udp");
    entry.src_dport=2001;
    strcpy(entry.dest_ip,"192.168.1.101");
    entry.dest_port=3001;

    addPortForwardRule(&entry);

    memset(&entry,0,sizeof(struct portForwardRule));
    strcpy(entry.name, "djj2");
    strcpy(entry.proto,"udp");
    entry.src_dport=2002;
    strcpy(entry.dest_ip,"192.168.1.102");
    entry.dest_port=3002;

    addPortForwardRule(&entry);

    memset(&entry,0,sizeof(struct portForwardRule));
    strcpy(entry.name, "djj3");
    strcpy(entry.proto,"udp tcp");
    entry.src_dport=2003;
    strcpy(entry.dest_ip,"192.168.1.103");
    entry.dest_port=3003;

    addPortForwardRule(&entry);


    outputarray=NULL;

    outputarray=getPortForwardRuleTable(&num);
    
    if(num!=0)
    {
        for(i=0;i<num;i++)
        {
            printf("====array[%d].name: %s ===\n",i,outputarray[i].name);
            printf("====array[%d].proto: %s ========\n",i,outputarray[i].proto);
            printf("====array[%d].src_dport: %d =====\n",i,outputarray[i].src_dport);
            printf("====array[%d].dest_ip: %s =======\n",i,outputarray[i].dest_ip);
            printf("====array[%d].dest_port: %d =====\n",i,outputarray[i].dest_port);
        }
    }
    else
    {
        printf("====there is no portForwardRule now=======\n");
    }
    
    if(outputarray!=NULL)
    {
        free(outputarray);
    }
    
    int index=1;
    delPortForwardRule(index);

    outputarray=NULL;
    outputarray=getPortForwardRuleTable(&num);
    
    if(num!=0)
    {
        for(i=0;i<num;i++)
        {
            printf("====array[%d].name: %s ===\n",i,outputarray[i].name);
            printf("====array[%d].proto: %s ========\n",i,outputarray[i].proto);
            printf("====array[%d].src_dport: %d =====\n",i,outputarray[i].src_dport);
            printf("====array[%d].dest_ip: %s =======\n",i,outputarray[i].dest_ip);
            printf("====array[%d].dest_port: %d =====\n",i,outputarray[i].dest_port);
        }
    }
    else
    {
        printf("====there is no portForwardRule now=======\n");
    }
    
    if(outputarray!=NULL)
    {
        free(outputarray);
    }

    memset(&entry,0,sizeof(struct portForwardRule));
    strcpy(entry.name, "wxl");
    strcpy(entry.proto,"tcp udp");
    entry.src_dport=2005;
    strcpy(entry.dest_ip,"192.168.1.105");
    entry.dest_port=3005;
    index=0;
    editPortForwardRule(&entry,index);
#endif 
#if 0
    struct limitLanSpeedEntry input1;
    memset(&input1,0,sizeof(struct limitLanSpeedEntry));
    strcpy(input1.ipaddr, "192.168.1.100");
    input1.rx=700;
    input1.tx=800;

    struct limitLanSpeedEntry input2;
    memset(&input2,0,sizeof(struct limitLanSpeedEntry));
    strcpy(input2.ipaddr, "192.168.1.101");
    input2.rx=700;
    input2.tx=800;

    setLimitLanSpeedRule(&input1);
    setLimitLanSpeedRule(&input2);
    
    char inputmac[256]={0};
    strcpy(inputmac,"00:0e:c6:d2:32:74");
    setBlockLan(inputmac);
    

    sleep(30);
    sleep(20);
    declineLimitLanSpeedRule(&input1);
    declineLimitLanSpeedRule(&input2);

    declineBlockLan("00:0e:c6:d2:32:74");
    
    DEBUG_PRINTF("====done===\n");
#endif 

#if 0
    struct childRuleEntry test1;
    memset(&test1, 0, sizeof(struct childRuleEntry));
    strcpy(test1.mac,"00:0e:c6:d2:32:74");
    strcpy(test1.timestart,"07:00");
    strcpy(test1.timestop,"08:00");
    strcpy(test1.weekdays,"Mon,Tue,Wed");
    strcpy(test1.monthdays,"3,4,5");
    addChildRule(&test1);

    sleep(25);
    delChildRule(&test1);
#endif

    /**
     * URL BLOCK
     */
    addUrlBlockRule("www.sogou.com");
    sleep(50);
    delUrlBlockRule("www.sogou.com");
}
