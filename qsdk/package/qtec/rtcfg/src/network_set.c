#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "network_set.h"
#include "rtcfg_uci.h"

int wanStaticConfigSet(struct wanStaticConfig *input)
{
    printf("wan mode is changed to static mode\n");
    
    rtcfgUciSet("network.wan.proto=static");
    char cmd[256]={0};
    snprintf(cmd,256,"network.wan.ipaddr=%s",input->ipaddr);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"network.wan.netmask=%s",input->netmask);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"network.wan.gateway=%s",input->gateway);
    rtcfgUciSet(cmd);

    if(strlen(input->dns) != 0 )
    {
        memset(cmd, 0, 256);
        snprintf(cmd,256,"network.wan.dns=%s",input->dns);
        rtcfgUciSet(cmd);

        memset(cmd,0,256);
        snprintf(cmd,256,"system.@system[0].dns=%s",input->dns);
        rtcfgUciSet(cmd);

        rtcfgUciCommit("system");
    }
    
    #if 0
    //about dns, we should del it first, and then add list one by one 
    rtcfgUciDel("network.wan.dns");
    
    char tmp[256]={0};
    strncpy(tmp,input->dns,256);
    char *head=tmp;
    char *index=tmp;
    for(index=tmp;(*index)!='\0';index++)
    {
        if(' ' == *index)
        {
            *index='\0';
            memset(cmd,0,256);
            snprintf(cmd,256,"network.wan.dns=%s",head);
            rtcfgUciAddList(cmd);
            index++;
            head=index;
        }
    }
    #endif
    
    rtcfgUciCommit("network");
   // system("/etc/init.d/network restart");
    system("ubus call network.interface.wan prepare");
    system("ifup wan");

  
    return 0;
}


int wanDhcpConfigSet()
{
    printf("wan mode is changed to dhcp mode\n");
    
    rtcfgUciSet("network.wan.proto=dhcp");
    rtcfgUciDel("network.wan.dns");
    rtcfgUciCommit("network");
    //system("/etc/init.d/network restart");
    system("ubus call network.interface.wan prepare");
    system("ifup wan");

    return 0;
}

// 0: success    -1 :fail
int wanPppoeConfigSet(struct wanPppoeConfig *input)
{
    printf("wan mode is changed to pppoe mode\n");

    rtcfgUciSet("network.wan.proto=pppoe");
    rtcfgUciDel("network.wan.dns");
    char cmd[256]={0};
    snprintf(cmd,256,"network.wan.username=%s",input->username);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"network.wan.password=%s",input->password);
    rtcfgUciSet(cmd);
    rtcfgUciCommit("network");

    system("ubus call network.interface.wan prepare");
    system("ifup wan");
    
    return 0;
}

//返回0即成功，非0 即失败
int getWanConnectionType(int *result_type)
{
    DEBUG_PRINTF("===[%s]====\n",__func__);
    int ret=0;
    char tmp_value[16]={0};
    ret=rtcfgUciGet(
"network.wan.proto", tmp_value);
    DEBUG_PRINTF("===[%s]===tmp_value: %s===\n",__func__,tmp_value);

    if(ret!=0)
    {
        printf("====get network.wan.proto fail====\n");
        return -1;
    }

    if(strncmp(tmp_value,"static",strlen(tmp_value))==0)
    {
        *result_type=WANSTATIC;
    }
    else if(strncmp(tmp_value,"dhcp",strlen(tmp_value))==0)
    {
        *result_type=WANDHCP;
    }
    else if(strncmp(tmp_value,"pppoe",strlen(tmp_value))==0)
    {
        *result_type=WANPPPOE;
    }
    else
    {
        printf("wrong wan connection type: %s===\n",tmp_value);
        return -1;
    }

    return 0;
    
}


int wanPppoeConfigGet(struct wanPppoeConfig *input)
{

    int ret=0;  
    ret=rtcfgUciGet("network.wan.username",input->username);
    if(ret !=0)
    {
        printf("[%s]==== can't get username===\n", __func__);
        return ret;
    }
    
    ret=rtcfgUciGet("network.wan.password",input->password);
    if(ret!=0)
    {
        printf("[%s]====can't get password===\n",__func__);
        return ret;
    }
    return 0;
}

int wanStaticConfigGet(struct wanStaticConfig *input)
{
    DEBUG_PRINTF("====[%s]===\n",__func__);

    int ret=0;
    
    ret += rtcfgUciGet("network.wan.ipaddr",input->ipaddr);

    ret += rtcfgUciGet("network.wan.netmask",input->netmask);

    ret += rtcfgUciGet("network.wan.gateway",input->gateway);

	ret += rtcfgUciGet("system.@system[0].dns",input->dns);
    if(ret !=0)
    {
        printf("=[%s]====can't wan static config get fail====\n",__func__);
        return -1;
    }
    
    return 0;
}

