#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "librtcfg.h"
#include "qos.h"
#include "rtcfg_uci.h"

/*qos 配置: 保存在qos.wan.qosmode
配置选项:
1.自动模式   ----0
2.游戏优先   ----1   quake1,liveforspeed, doom3, battlefield1942,worldofwarcraft, guildwars,teamfortress2, mohaa, runesofmagic
3.网页优先   ----2   qq,yahoo,jabber,skypetoskype,chikka,aim,aimwebcontent,http
4.视频优先   ----3   ares,http-rtsp,pplive,rtp,rtsp
*/

//get qos configuration
int Get_qosmode()
{
    DEBUG_PRINTF("===[%s]====\n",__func__);

    int ret=0;
    char qos_mode[64]={0};
    rtcfgUciGet("qos.wan.qosmode", qos_mode);

    if(strlen(qos_mode)==0)
    {
        //若为空值,则默认为自动模式
        return ret;
    }
    else
    {
        ret=atoi(qos_mode);
        return ret;
    }
        
}

int qosmode_init()
{
    DEBUG_PRINTF("===[%s]=====\n",__func__);

    //game
    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[3].layer7=quake1");
    rtcfgUciSet("qos.@classify[3].target=Normal");
    rtcfgUciSet("qos.@classify[3].comment=game");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[4].layer7=liveforspeed");
    rtcfgUciSet("qos.@classify[4].target=Normal");
    rtcfgUciSet("qos.@classify[4].comment=game");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[5].layer7=doom3");
    rtcfgUciSet("qos.@classify[5].target=Normal");
    rtcfgUciSet("qos.@classify[5].comment=game");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[6].layer7=battlefield1942");
    rtcfgUciSet("qos.@classify[6].target=Normal");
    rtcfgUciSet("qos.@classify[6].comment=game");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[7].layer7=worldofwarcraft");
    rtcfgUciSet("qos.@classify[7].target=Normal");
    rtcfgUciSet("qos.@classify[7].comment=game");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[8].layer7=guildwars");
    rtcfgUciSet("qos.@classify[8].target=Normal");
    rtcfgUciSet("qos.@classify[8].comment=game");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[9].layer7=mohaa");
    rtcfgUciSet("qos.@classify[9].target=Normal");
    rtcfgUciSet("qos.@classify[9].comment=game");


    //web
    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[10].layer7=http");
    rtcfgUciSet("qos.@classify[10].target=Normal");
    rtcfgUciSet("qos.@classify[10].comment=web");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[11].layer7=qq");
    rtcfgUciSet("qos.@classify[11].target=Normal");
    rtcfgUciSet("qos.@classify[11].comment=web");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[12].layer7=yahoo");
    rtcfgUciSet("qos.@classify[12].target=Normal");
    rtcfgUciSet("qos.@classify[12].comment=web");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[13].layer7=jabber");
    rtcfgUciSet("qos.@classify[13].target=Normal");
    rtcfgUciSet("qos.@classify[13].comment=web");

    rtcfgUciAdd("qos", "classify");
    rtcfgUciSet("qos.@classify[14].layer7=skypetoskype");
    rtcfgUciSet("qos.@classify[14].target=Normal");
    rtcfgUciSet("qos.@classify[14].comment=web");

    //video
    rtcfgUciAdd("qos","classify");
    rtcfgUciSet("qos.@classify[15].layer7=ares");
    rtcfgUciSet("qos.@classify[15].target=Normal");
    rtcfgUciSet("qos.@classify[15].comment=video");

    rtcfgUciAdd("qos","classify");
    rtcfgUciSet("qos.@classify[16].layer7=http-rtsp");
    rtcfgUciSet("qos.@classify[16].target=Normal");
    rtcfgUciSet("qos.@classify[16].comment=video");

    rtcfgUciAdd("qos","classify");
    rtcfgUciSet("qos.@classify[17].layer7=pplive");
    rtcfgUciSet("qos.@classify[17].target=Normal");
    rtcfgUciSet("qos.@classify[17].comment=video");

    
    rtcfgUciAdd("qos","classify");
    rtcfgUciSet("qos.@classify[18].layer7=rtp");
    rtcfgUciSet("qos.@classify[18].target=Normal");
    rtcfgUciSet("qos.@classify[18].comment=video");
    
    rtcfgUciAdd("qos","classify");
    rtcfgUciSet("qos.@classify[19].layer7=rtsp");
    rtcfgUciSet("qos.@classify[19].target=Normal");
    rtcfgUciSet("qos.@classify[19].comment=video");

    return 0;
}

int qosmode_funtion(int *input_qosmode)
{
    DEBUG_PRINTF("===[%s]====input_qosmode:%d====\n",__func__,*input_qosmode);
    //自动模式
    if(*input_qosmode==0)
    {
        rtcfgUciSet("qos.@classify[3].target=Normal");
        rtcfgUciSet("qos.@classify[4].target=Normal");
        rtcfgUciSet("qos.@classify[5].target=Normal");
        rtcfgUciSet("qos.@classify[6].target=Normal");
        rtcfgUciSet("qos.@classify[7].target=Normal");
        rtcfgUciSet("qos.@classify[8].target=Normal");
        rtcfgUciSet("qos.@classify[9].target=Normal");
        rtcfgUciSet("qos.@classify[10].target=Normal");
        rtcfgUciSet("qos.@classify[11].target=Normal");
        rtcfgUciSet("qos.@classify[12].target=Normal");
        rtcfgUciSet("qos.@classify[13].target=Normal");
        rtcfgUciSet("qos.@classify[14].target=Normal");
        rtcfgUciSet("qos.@classify[15].target=Normal");
        rtcfgUciSet("qos.@classify[16].target=Normal");
        rtcfgUciSet("qos.@classify[17].target=Normal");
        rtcfgUciSet("qos.@classify[18].target=Normal");
        rtcfgUciSet("qos.@classify[19].target=Normal");
        
    }
    //游戏优先
    else if(*input_qosmode==1)
    {
        rtcfgUciSet("qos.@classify[3].target=Priority");
        rtcfgUciSet("qos.@classify[4].target=Priority");
        rtcfgUciSet("qos.@classify[5].target=Priority");
        rtcfgUciSet("qos.@classify[6].target=Priority");
        rtcfgUciSet("qos.@classify[7].target=Priority");
        rtcfgUciSet("qos.@classify[8].target=Priority");
        rtcfgUciSet("qos.@classify[9].target=Priority");
        rtcfgUciSet("qos.@classify[10].target=Normal");
        rtcfgUciSet("qos.@classify[11].target=Normal");
        rtcfgUciSet("qos.@classify[12].target=Normal");
        rtcfgUciSet("qos.@classify[13].target=Normal");
        rtcfgUciSet("qos.@classify[14].target=Normal");
        rtcfgUciSet("qos.@classify[15].target=Normal");
        rtcfgUciSet("qos.@classify[16].target=Normal");
        rtcfgUciSet("qos.@classify[17].target=Normal");
        rtcfgUciSet("qos.@classify[18].target=Normal");
        rtcfgUciSet("qos.@classify[19].target=Normal");
    }
    else if(*input_qosmode==2)
    {
        rtcfgUciSet("qos.@classify[3].target=Normal");
        rtcfgUciSet("qos.@classify[4].target=Normal");
        rtcfgUciSet("qos.@classify[5].target=Normal");
        rtcfgUciSet("qos.@classify[6].target=Normal");
        rtcfgUciSet("qos.@classify[7].target=Normal");
        rtcfgUciSet("qos.@classify[8].target=Normal");
        rtcfgUciSet("qos.@classify[9].target=Normal");
        rtcfgUciSet("qos.@classify[10].target=Priority");
        rtcfgUciSet("qos.@classify[11].target=Priority");
        rtcfgUciSet("qos.@classify[12].target=Priority");
        rtcfgUciSet("qos.@classify[13].target=Priority");
        rtcfgUciSet("qos.@classify[14].target=Priority");
        rtcfgUciSet("qos.@classify[15].target=Normal");
        rtcfgUciSet("qos.@classify[16].target=Normal");
        rtcfgUciSet("qos.@classify[17].target=Normal");
        rtcfgUciSet("qos.@classify[18].target=Normal");
        rtcfgUciSet("qos.@classify[19].target=Normal");
    }
    else if(*input_qosmode==3)
    {
        rtcfgUciSet("qos.@classify[3].target=Normal");
        rtcfgUciSet("qos.@classify[4].target=Normal");
        rtcfgUciSet("qos.@classify[5].target=Normal");
        rtcfgUciSet("qos.@classify[6].target=Normal");
        rtcfgUciSet("qos.@classify[7].target=Normal");
        rtcfgUciSet("qos.@classify[8].target=Normal");
        rtcfgUciSet("qos.@classify[9].target=Normal");
        rtcfgUciSet("qos.@classify[10].target=Normal");
        rtcfgUciSet("qos.@classify[11].target=Normal");
        rtcfgUciSet("qos.@classify[12].target=Normal");
        rtcfgUciSet("qos.@classify[13].target=Normal");
        rtcfgUciSet("qos.@classify[14].target=Normal");
        rtcfgUciSet("qos.@classify[15].target=Priority");
        rtcfgUciSet("qos.@classify[16].target=Priority");
        rtcfgUciSet("qos.@classify[17].target=Priority");
        rtcfgUciSet("qos.@classify[18].target=Priority");
        rtcfgUciSet("qos.@classify[19].target=Priority");
        
    }
        
    rtcfgUciCommit("qos");
    return 0;
    
}


//创建进程锁，用来避免进程间造成逻辑错误
//set qos configuration
int Set_qosmode(int *input_qosmode,int flag)
{
    DEBUG_PRINTF("===[%s]====input_qosmode:%d====\n",__func__,*input_qosmode);

    int ret=0;
    char cmd[256]={0};
    char tmp_result[256]={0};
    int int_result=0;

    int_result=Get_qosmode();
    if(int_result == *input_qosmode)
    {
        DEBUG_PRINTF("[%s]====qos mode don't changed===\n",__func__);
        return 0;
    }
    
    memset(tmp_result,0,256);
    rtcfgUciGet("qos.@classify[-1].comment", tmp_result);
    if( (strcmp(tmp_result,"game")!=0) && (strcmp(tmp_result,"web")!=0) && (strcmp(tmp_result,"video")!=0) )
    {
        DEBUG_PRINTF("[%s]===init qos rules===\n",__func__);
        qosmode_init();
    }
    snprintf(cmd,256,"qos.wan.qosmode=%d",*input_qosmode);
    rtcfgUciSet(cmd);

    qosmode_funtion(input_qosmode);
    rtcfgUciCommit("qos");

    if(flag == 1)
    {
        //system("/etc/init.d/qos restart");
        return ProcQosRestartReq();
    }
    else
    {
        return 0;
    }
}
