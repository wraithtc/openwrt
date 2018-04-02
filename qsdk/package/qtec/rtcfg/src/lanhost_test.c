#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "lanhost.h"
#include "rtcfg_uci.h"


void main()
{

    printf("==========lanhost  test========\n");
#if 0 
    int arpnum=0;
    getArpEntryTableNum(&arpnum);
    printf("=====arpnum: %d====\n", arpnum);

    struct arpEntry *arpEntryArray=malloc(arpnum*sizeof(struct arpEntry));
    memset(arpEntryArray,0,(arpnum*sizeof(struct arpEntry)));

    getArpEntryTable(arpEntryArray,&arpnum);

    int i=0;
    
    for(i=0;i<arpnum;i++)
    {
        printf("arpEntryArray[%d].ipaddr : %s ====\n",i, arpEntryArray[i].ipaddr);
        printf("arpEntryArray[%d].HWType : %d ====\n",i, arpEntryArray[i].HWType);
        printf("arpEntryArray[%d].flags:   %d ====\n",i, arpEntryArray[i].flags);
        printf("arpEntryArray[%d].macaddr: %s ====\n",i, arpEntryArray[i].macaddr);
        printf("arpEntryArray[%d].device:  %s ====\n",i, arpEntryArray[i].device);
    }
#endif 
    lanHostMainLogic();
    int i=0; 
    struct lanHostEntry *alllanhost=NULL;
    int alllanhostnum=0;
    alllanhost = outputAllLanHostInfo(&alllanhostnum);

    for(i=0;i<alllanhostnum;i++)
    {
        DEBUG_PRINTF("alllanhost[%d].online: %d ===\n",i,alllanhost[i].online);
        DEBUG_PRINTF("alllanhost[%d].ipaddr: %s ===\n",i,alllanhost[i].ipaddr);
        DEBUG_PRINTF("alllanhost[%d].macaddr: %s ===\n",i,alllanhost[i].macaddr);
        DEBUG_PRINTF("alllanhost[%d].hostname: %s ===\n",i,alllanhost[i].hostname);
        DEBUG_PRINTF("alllanhost[%d].connect_type: %d ===\n",i,alllanhost[i].connection_type);
    }

    if(alllanhost !=NULL)
    {
        free(alllanhost);
    }


    struct lanHostEntry *onlinelanhost=NULL;
    int onlinelanhostnum=0;
    onlinelanhost = outputOnlineLanHostInfo(&onlinelanhostnum);

    for(i=0;i<onlinelanhostnum;i++)
    {
        DEBUG_PRINTF("onlinelanhost[%d].online: %d ===\n",i,onlinelanhost[i].online);
        DEBUG_PRINTF("onlinelanhost[%d].ipaddr: %s ===\n",i,onlinelanhost[i].ipaddr);
        DEBUG_PRINTF("onlinelanhost[%d].macaddr: %s ===\n",i,onlinelanhost[i].macaddr);
        DEBUG_PRINTF("onlinelanhost[%d].hostname: %s ===\n",i,onlinelanhost[i].hostname);
        DEBUG_PRINTF("onlinelanhost[%d].connect_type: %d ===\n",i,onlinelanhost[i].connection_type);
    }

    if(onlinelanhost !=NULL)
    {
        free(onlinelanhost);
    }
}
