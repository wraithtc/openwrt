#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "network_set.h"
#include "rtcfg_uci.h"


void main()
{
    printf("==========network set test========\n");
    
    //case1: test rtcfgUciGet 
    printf("****test rtcfgUciGet******\n");
    char value[256];
    rtcfgUciGet("network.wan.proto",value);
    printf("result: %s\n",value);
    
    memset(value,0,256);
    rtcfgUciGet("dsaf.sadfaf.sdfaf",value);
    printf("result: %s\n",value);
    
    memset(value,0,256);
    rtcfgUciGet("network.wan.ipaddr",value);
    printf("result: %s\n",value);

    memset(value,0,256);
    rtcfgUciGet("network.wan.dns",value);
    printf("result: %s\n", value);

    memset(value,0,256);
    rtcfgUciGet("network.wan",value);
    printf("result:%s\n",value);

    //case2: test rtcfgUciSet
    printf("******test rtcfgUciSet*****\n");
    rtcfgUciSet("network.wan.gateway=10.0.0.1");
    rtcfgUciSet("network.wan.ipaddr=12.2.2.1");

    //case3: test rtcfgUciAddList and rtcfgUciDelList
    printf("*****test rtcfgUciAddList*****\n");
    rtcfgUciAddList("network.wan.dns=1.1.1.1");
    rtcfgUciAddList("network.wan.dns=2.2.2.2");
    rtcfgUciAddList("network.wan.dns=3.3.3.3");
    memset(value,0,256);
    rtcfgUciGet("network.wan.dns",value);
    printf("result:%s \n", value);

    rtcfgUciDelList("network.wan.dns=2.2.2.2");
    memset(value,0,256);
    rtcfgUciGet("network.wan.dns",value);
    printf("result:%s \n", value);

    //case4: test rtcfgUciDel
    printf("*****test rtcfgUciDel****\n");
    memset(value,0,256);
    rtcfgUciGet("network.wan.ipaddr",value);
    printf("result:%s \n", value);
    rtcfgUciDel("network.wan.ipaddr");
    memset(value,0,256);
    rtcfgUciGet("network.wan.ipaddr",value);
    printf("result:%s\n", value);

    //case5: test rtcfgUciCommit
    printf("****test rtcfgUciCommit****\n");
    rtcfgUciCommit("network");
}
