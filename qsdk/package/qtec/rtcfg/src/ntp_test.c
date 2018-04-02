#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "ntp_set.h"
#include "rtcfg_uci.h"


void main()
{
    printf("==========ntp set test========\n");
    
    struct ntpConfig output;
    memset(&output,0,sizeof(struct ntpConfig));
    ntpConfigGet(&output);
    printf("===ntpConfig output:==========\n");
    printf("===output.enable: %d==========\n",output.enable);
    printf("===output.timezone: %s========\n", output.timezone);
    printf("===output.ntpServers: %s======\n", output.ntpServers);

    struct ntpConfig test;
    test.enable=1;
    strcpy(test.timezone,"ART3");
    strcpy(test.ntpServers,"1.2.2.3 3.2.2.1 1.3.3.2");
    ntpConfigSet(&test);


    memset(&output,0,sizeof(struct ntpConfig));
    ntpConfigGet(&output);
    printf("===ntpConfig output:==========\n");
    printf("===output.enable: %d==========\n",output.enable);
    printf("===output.timezone: %s========\n", output.timezone);
    printf("===output.ntpServers: %s======\n", output.ntpServers);
}
