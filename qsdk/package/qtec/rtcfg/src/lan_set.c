#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "lan_set.h"
#include "rtcfg_uci.h"
#include "fwk.h"
#include <arpa/inet.h>

/**
 * funcname: lanConfigSet   
 *          set lan config to /etc/config/network, and dhcp config to /var/etc/dnsmasq.conf
 *
 */
int lanConfigSet(struct lanConfig *input)
{
    unsigned long netmask = 0, start = 0, end = 0;
    printf("=====lanConfigSet========\n" );
    printf("start:%s, end:%s\n", input->dhcpPoolStart, input->dhcpPoolLimit);
    char cmd[256]={0};
    if(strlen(input->ipaddress) !=0)
    {
        snprintf(cmd,256,"network.lan.ipaddr=%s",input->ipaddress);
        rtcfgUciSet(cmd);
    }

    if(strlen(input->netmask)!=0)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"network.lan.netmask=%s",input->netmask);
        rtcfgUciSet(cmd);
    }
    netmask = inet_addr(input->netmask);
    start = inet_addr(input->dhcpPoolStart);
    end = inet_addr(input->dhcpPoolLimit);
    printf("netmask:%u, start:%u\n", netmask, start);
    if(strlen(input->dhcpPoolStart) !=0)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"dhcp.lan.start=%d",(~htonl(netmask))&htonl(start));
        rtcfgUciSet(cmd);
    }

    if(strlen(input->dhcpPoolLimit) !=0)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"dhcp.lan.limit=%d",((~htonl(netmask))&htonl(end)) - ((~htonl(netmask))&htonl(start)) +1);
        rtcfgUciSet(cmd);
    }

    input->dhcpEnable?rtcfgUciSet("dhcp.lan.interface=lan"):rtcfgUciSet("dhcp.lan.interface=none");
    rtcfgUciCommit("network");
    rtcfgUciCommit("dhcp");
    
    return 0;

}

/**
 *
 * funcname: lanConfigGet  get lan config from /etc/config/network and /var/etc/dnsmasq.conf
 *
 */
int lanConfigGet(struct lanConfig *output)
{
    int start, end;
    unsigned long netmask, ipaddr;
    struct in_addr addr_start, addr_end;
    printf("============lanConfigGet============\n");
    rtcfgUciGet("network.lan.ipaddr",output->ipaddress);
    ipaddr = inet_addr(output->ipaddress);
    rtcfgUciGet("network.lan.netmask",output->netmask);
    netmask = inet_addr(output->netmask);
    char tmp_start[16]={0};
    rtcfgUciGet("dhcp.lan.start",tmp_start);
    start = ipaddr & netmask | htonl(atoi(tmp_start));
    memcpy(&addr_start, &start, 4);
    strncpy(output->dhcpPoolStart, inet_ntoa(addr_start), sizeof(output->dhcpPoolStart));
    char tmp_limit[16]={0};
    rtcfgUciGet("dhcp.lan.limit",tmp_limit);
    end = ipaddr & netmask | htonl(atoi(tmp_limit) + atoi(tmp_start) - 1);
    memcpy(&addr_end, &end, 4);
    strncpy(output->dhcpPoolLimit, inet_ntoa(addr_end), sizeof(output->dhcpPoolLimit));
    char value[8] = {0};
    rtcfgUciGet("dhcp.lan.interface", value);
    output->dhcpEnable=(strncmp(value, "lan", 3) == 0)?1:0;
    
    return 0;
}

/**
 * funcname: GetStaticLeaseArray
 *          get whole StaticLeaseArray from /etc/config/dhcp
 * output: one pointer point to new malloc memory to store the info of staticleasearray, so this function caller need to free this memory.
 */
struct staticLeaseConfig * getStaticLeaseArray(int* array_num)
{
    printf("=========getStaticLeaseArray===========\n");
    struct staticLeaseConfig *output=NULL;
    int ret=0;
    int index=-1;
    int num=0;
    char cmd[256]={0};
    char tmp_store[16]={0};
    while(ret == 0)
    {
        index++;
        memset(cmd,0,256);
        snprintf(cmd,256,"dhcp.@host[%d]",index);
        memset(tmp_store,0,16);
        ret = rtcfgUciGet(cmd,tmp_store);
    }
    
    num=index;
    if(num == 0)
    {
        output=NULL;
        *array_num=0;
        printf("===current no StaticLeaseEntry exist====\n");
        return output;
    }

    output=malloc(num*sizeof(struct staticLeaseConfig));

    for(index=0;index<num;index++)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"dhcp.@host[%d].name",index);
        rtcfgUciGet(cmd,output[index].hostname);

        memset(cmd,0,256);
        snprintf(cmd,256,"dhcp.@host[%d].mac",index);
        rtcfgUciGet(cmd,output[index].mac);
        
        memset(cmd,0,256);
        snprintf(cmd,256,"dhcp.@host[%d].ip",index);
        rtcfgUciGet(cmd,output[index].ipaddress);
        
    }

    *array_num =num;

    return output;
}

/**
 * func_name: AddStaticLeaseEntry
 *            add StaticLeaseEntry into /etc/config/dhcp
 */
int addStaticLeaseEntry(struct staticLeaseConfig *input)
{
    printf("=========AddStaticLeaseEntry================\n");
    rtcfgUciAdd("dhcp","host"); 
    
    char cmd[256]={0};
    snprintf(cmd,256,"dhcp.@host[-1].name=%s",input->hostname);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"dhcp.@host[-1].mac=%s",input->mac);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"dhcp.@host[-1].ip=%s",input->ipaddress);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("dhcp");
    system("/etc/init.d/dnsmasq reload");
    return 0;
}

/**
 * func_name: DelStaticLeaseEntry
 *            delete one static lease entry of /etc/config/dhcp
 *
 */
int delStaticLeaseEntry(int index)
{
    printf("=========DelStaticLeaseEntry==============\n");
    char cmd[256]={0};
    snprintf(cmd,256,"dhcp.@host[%d]",index);
    int ret=rtcfgUciDel(cmd);

    if(ret!=0)
    {
        printf("====del staticLeaseEntry fail===\n");
        return ret;
    }
    else
    {
        rtcfgUciCommit("dhcp");
        system("/etc/init.d/dnsmasq reload");
        return 0;
    }
}
