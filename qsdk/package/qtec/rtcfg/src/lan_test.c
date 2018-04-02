#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "lan_set.h"
#include "rtcfg_uci.h"


void main()
{

    printf("==========lan set test========\n");
    struct lanConfig output;
    memset(&output,0,sizeof(struct lanConfig));
    
    lanConfigGet(&output);

    printf("=====output.ipaddress : %s ======\n",output.ipaddress);
    printf("=====output.netmask   : %s ======\n", output.netmask);
    printf("=====output.dhcpPoolStart: %s =====\n", output.dhcpPoolStart);
    printf("=====output.dhcpPoolLimit: %s =====\n", output.dhcpPoolLimit);

    struct lanConfig input;
    memset(&input,0,sizeof(struct lanConfig));
    strcpy(input.ipaddress,"10.0.0.1");
    strcpy(input.netmask,"255.255.255.0");
    strncpy(input.dhcpPoolStart, "10.0.0.10", sizeof(input.dhcpPoolStart));
    strncpy(input.dhcpPoolLimit, "10.0.0.2", sizeof(input.dhcpPoolLimit));
    
    lanConfigSet(&input);

    memset(&output,0,sizeof(struct lanConfig));
    lanConfigGet(&output);
    printf("=====output.ipaddress : %s ======\n",output.ipaddress);
    printf("=====output.netmask   : %s ======\n", output.netmask);
    printf("=====output.dhcpPoolStart: %s =====\n", output.dhcpPoolStart);
    printf("=====output.dhcpPoolLimit: %s =====\n", output.dhcpPoolLimit);

    struct staticLeaseConfig *outputarray=NULL;
    int num=0;
    int i =0;
    outputarray=getStaticLeaseArray(&num);
    
    if(num!=0)
    {
        for(i=0;i<num;i++)
        {
            printf("====array[%d].hostname: %s ===\n",i,outputarray[i].hostname);
            printf("====array[%d].mac: %s ========\n",i,outputarray[i].mac);
            printf("====array[%d].ipaddress: %s =====\n",i,outputarray[i].ipaddress);
        }
    }
    else
    {
        printf("====there is no static lease entry now=======\n");
    }
    
    if(outputarray!=NULL)
    {
        free(outputarray);
    }

    struct staticLeaseConfig entry;
    memset(&entry,0,sizeof(struct staticLeaseConfig));
    strcpy(entry.hostname, "djj1");
    strcpy(entry.mac,"00:0e:c6:d2:32:70");
    strcpy(entry.ipaddress,"192.168.1.100");

    addStaticLeaseEntry(&entry);


    memset(&entry,0,sizeof(struct staticLeaseConfig));
    strcpy(entry.hostname, "djj2");
    strcpy(entry.mac,"00:0e:c6:d2:32:71");
    strcpy(entry.ipaddress,"192.168.1.101");

    addStaticLeaseEntry(&entry);

    memset(&entry,0,sizeof(struct staticLeaseConfig));
    strcpy(entry.hostname, "djj3");
    strcpy(entry.mac,"00:0e:c6:d2:32:72");
    strcpy(entry.ipaddress,"192.168.1.102");

    addStaticLeaseEntry(&entry);

    outputarray=NULL;
    outputarray=getStaticLeaseArray(&num);
    if(num!=0)
    {
        for(i=0;i<num;i++)
        {
            printf("====array[%d].hostname: %s ===\n",outputarray[i].hostname);
            printf("====array[%d].mac: %s ========\n",outputarray[i].mac);
            printf("====array[%d].ipaddress: %s =====\n",outputarray[i].ipaddress);
        }
    }
    else
    {
        printf("====there is no static lease entry now=======\n");
    }

    if(outputarray!=NULL)
    {
        free(outputarray);
    }
    
    int index=1;
    delStaticLeaseEntry(index);

    outputarray=NULL;
    outputarray=getStaticLeaseArray(&num);
    if(num!=0)
    {
        for(i=0;i<num;i++)
        {
            printf("====array[%d].hostname: %s ===\n",i,outputarray[i].hostname);
            printf("====array[%d].mac: %s ========\n",i,outputarray[i].mac);
            printf("====array[%d].ipaddress: %s =====\n",i,outputarray[i].ipaddress);
        }
    }
    else
    {
        printf("====there is no static lease entry now=======\n");
    }

    if(outputarray!=NULL)
    {
        free(outputarray);
    }
}
