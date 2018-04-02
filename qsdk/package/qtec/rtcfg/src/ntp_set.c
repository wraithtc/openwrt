#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "ntp_set.h"
#include "rtcfg_uci.h"

/**
 *func_name: ntpConfigSet: set ntp config into /etc/config/system
 * input: struct ntpConfig *input
 * output: 0: SUCCESS; other: FAIL
 *
 */
int ntpConfigSet(struct ntpConfig *input)
{
    printf("==== ntpConfigSet=====\n");
    
    char cmd[256]={0};
    snprintf(cmd,256,"system.@system[0].timezone=%s",input->timezone);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"system.ntp.enabled=%d",input->enable);
    rtcfgUciSet(cmd);

    rtcfgUciDel("system.ntp.server");

    char tmp[1024]={0};
    strncpy(tmp,input->ntpServers,1024);
    char *head=tmp;
    char *index=tmp;
    for(index=tmp;(*index)!='\0';index++)
    {
        if(' ' == *index)
        {
            *index = '\0';
            memset(cmd,0,256);
            snprintf(cmd,256,"system.ntp.server=%s",head);
            rtcfgUciAddList(cmd);
            index++;
            head=index;
        }
    }
    
    memset(cmd,0,256);
    snprintf(cmd,256,"system.ntp.server=%s",head);
    rtcfgUciAddList(cmd);
    
    rtcfgUciCommit("system");
    system("/etc/init.d/system reload");
    return 0;
}

/**
 * func_name: ntpConfigGet: get ntp config from /etc/config/system
 * output: struct ntpConfig *output to store info
 *          0: SUCCESS; other: FAIL
 */
int ntpConfigGet(struct ntpConfig *output)
{
    printf("==== ntpConfigGet ======\n");
    
    rtcfgUciGet("system.@system[0].timezone",output->timezone);
    
    char tmp_enable[16]={0};
    rtcfgUciGet("system.ntp.enabled",tmp_enable);
    output->enable=atoi(tmp_enable);

    rtcfgUciGet("system.ntp.server",output->ntpServers);

    return 0;
}
