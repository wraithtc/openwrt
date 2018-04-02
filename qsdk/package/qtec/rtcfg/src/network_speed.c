#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "librtcfg.h"
#include "rtcfg_uci.h"
#include "pthread.h"

int get_cur_wan_speed(char *output_upspeed, char* output_downspeed)
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    char wan_ifname[64]={0};
    char wwan_ifname[64]={0};
    char cmd[256]={0};
    char tmp[256]={0};
    char tmp_ifname[64]={0};
    FILE *sta_fp;
    FILE *fp;
    int sta_enabled=0;

    //å…ˆåˆ¤æ–­æ— çº¿ä¸­ç»§æ˜¯å¦å¼€å¯
    memset(cmd,0,256);
    snprintf(cmd,256,"ubus call network.interface.wwan status | grep '\"up\"' | sed -e 's/^.*: \\(.*\\),/\\1/g'");
    sta_fp=popen(cmd,"r");
    if(sta_fp)
    {
        memset(tmp,0,256);
        fgets(tmp,sizeof(tmp),sta_fp);
        DEBUG_PRINTF("[%s]===tmpstr:%s===\n",__func__,tmp);
        if(strstr(tmp,"true")!=NULL)
        {
            sta_enabled=1;
        }
        pclose(sta_fp);
    }
    DEBUG_PRINTF("[%s]==sta_enabled:%d===\n",__func__,sta_enabled);

    if(0==sta_enabled)
    {
        rtcfgUciGet("network.wan.ifname", wan_ifname);

        DEBUG_PRINTF("[%s]===wan_ifname:%s===\n",__func__,wan_ifname);

        if((strlen(wan_ifname))==0)
        {
            DEBUG_PRINTF("[%s]===can not get wan ifname===\n",__func__);
            return -1;
        }
        memset(cmd,0,256);
        snprintf(cmd,256,"wan_speedtest.sh %s",wan_ifname);
        fp=popen(cmd,"r");

        if(!fp)
        {
            DEBUG_PRINTF("[%s]===popen fail====\n",__func__);
            return -1;
        }
        memset(tmp,0,256);
        fgets(tmp,sizeof(tmp),fp);
        DEBUG_PRINTF("[%s]====tmp:%s=====\n",__func__,tmp);
    
        sscanf(tmp,"%s %s %s ",tmp_ifname,output_downspeed,output_upspeed);
        DEBUG_PRINTF("[%s]===output_downspeed:%s output_outspeed:%s ===\n",__func__,output_downspeed,output_upspeed);
        pclose(fp);
    
        return 0;
    }
    else 
    {
        rtcfgUciGet("wireless.wds.device", wwan_ifname);
        DEBUG_PRINTF("[%s]===wwan_ifname:%s===\n",__func__,wwan_ifname);

        if((strlen(wwan_ifname))==0)
        {
            DEBUG_PRINTF("[%s]===can not get wwan ifname===\n",__func__);
            return -1;
        }
        memset(cmd,0,256);
        snprintf(cmd,256,"wan_speedtest.sh %s",wwan_ifname);
        fp=popen(cmd,"r");

        if(!fp)
        {
            DEBUG_PRINTF("[%s]===popen fail====\n",__func__);
            return -1;
        }
        memset(tmp,0,256);
        fgets(tmp,sizeof(tmp),fp);
        DEBUG_PRINTF("[%s]====tmp:%s=====\n",__func__,tmp);
    
        sscanf(tmp,"%s %s %s ",tmp_ifname,output_downspeed,output_upspeed);
        DEBUG_PRINTF("[%s]===output_downspeed:%s output_outspeed:%s ===\n",__func__,output_downspeed,output_upspeed);
        pclose(fp);
    
        return 0;
    }
}

//»ñÈ¡wan¿Ú´ø¿íÏÞÖÆÅäÖÃ
int get_wanbandwidth_config(bool *output_enabled, int *output_upload, int *output_download)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    char tmpchar_enabled[6]={0};
    char tmpchar_upload[64]={0};
    char tmpchar_download[64]={0};

    rtcfgUciGet("qos.wan.enabled", tmpchar_enabled);
    rtcfgUciGet("qos.wan.upload", tmpchar_upload);
    rtcfgUciGet("qos.wan.download", tmpchar_download);

    *output_enabled = atoi(tmpchar_enabled);
    *output_upload = atoi(tmpchar_upload);
    *output_download = atoi(tmpchar_download);

    DEBUG_PRINTF("[%s] out === output_enabled:%d  output_upload:%d  output_download:%d ===\n",__func__,*output_enabled,*output_upload,*output_download);

    return 0;
}

//ÉèÖÃwan¿Ú´ø¿íÏÞÖÆÅäÖÃ
int set_wanbandwidth_config(bool input_enabled, int input_upload, int input_download,int flag)
{
    DEBUG_PRINTF("[%s]====input_enabled:%d input_upload:%d input_download:%d===\n",__func__,input_enabled, input_upload, input_download);
    char cmd[256];
    snprintf(cmd,256,"qos.wan.enabled=%d",input_enabled);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"qos.wan.upload=%d",input_upload);
    rtcfgUciSet(cmd);

    memset(cmd,0,256);
    snprintf(cmd,256,"qos.wan.download=%d",input_download);
    rtcfgUciSet(cmd);
    rtcfgUciCommit("qos");

    if(flag == 1)
    {
        return ProcQosRestartReq();
    }
    else 
        return 0;
   
}

#define WAN_SPEED_TEST_FILE "/tmp/wan_speed.txt"
void thread_ifstat()
{
    DEBUG_PRINTF("[%s]======\n",__func__);
    char wan_ifname[64]={0};
    rtcfgUciGet("network.wan.ifname", wan_ifname);
    char cmd[256]={0};
    snprintf(cmd,256,"ifstat -i %s -n > "WAN_SPEED_TEST_FILE,wan_ifname);
    system(cmd);

    DEBUG_PRINTF("thread_ifstat exit===\n");
}

//wan²à²âËÙ
int wan_speedtest(float *output_upload, float *output_download)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    //ÏÈÈ·ÈÏqos£¬ Èôqos¿ªÆô£¬ÔòÏÈ¹Ø±Õqos
    int qos_enabled=0;
    char tmp_qos_enabled[6]={0};
    rtcfgUciGet("qos.wan.enabled", tmp_qos_enabled);
    qos_enabled=atoi(tmp_qos_enabled);

    if(qos_enabled == 1)
    {
        rtcfgUciSet("qos.wan.enabled=0");
        system("/etc/init.d/qos restart");
    }

    pthread_t id_1;
    int ret=0;
    char cmd[256]={0};
    float max_upload=0;
    float max_download=0;
    float upload;
    float download;
    FILE *fp;
    char tmpline[256]={0};
    //´´½¨×Ó½ø³ÌÀ´ÊµÊ±¼ÇÂ¼ÍøËÙ£¬

    ret=pthread_create(&id_1,NULL,(void  *) thread_ifstat,NULL);  
    if(ret!=0)  
    {  
        printf("Create pthread error!\n");  
        return -1;  
    }  


    //²úÉúÊý¾Ý°ü
    system("rm -rf test.img");
    system("wget http://qtec-route-headimg.oss-cn-shanghai.aliyuncs.com/test/test.img");

 
    system("killall ifstat");
    
    //·ÖÎöÊý¾Ý
    fp=fopen(WAN_SPEED_TEST_FILE,"r");
    if(!fp)
    {
        DEBUG_PRINTF("[%s]===wan_speed_test_file cannot find===\n",__func__);
        return -1;
    }
    else
    {
        fgets(tmpline,256,fp);
        fgets(tmpline,256,fp);

        memset(tmpline,0,256);
        while( (fgets(tmpline,256,fp)) !=NULL )
        {
            sscanf(tmpline,"%f %f",&download,&upload);
            if(download > max_download)
            {
                max_download = download;
            }

            if(upload > max_upload)
            {
                max_upload=upload;
            }
        }
       
    }
    DEBUG_PRINTF("===[%s]=== max_upload:%f max_download:%f ===\n",__func__,max_upload,max_download);
    *output_upload = (max_upload *8)/1024 ;
    *output_download = (max_download *8)/1024;
    DEBUG_PRINTF("==[%s]===out: output_upload:%f   output_download:%f ====\n", __func__,*output_upload, *output_download);

    //Èç¹ûÔ­ÏÈqos¿ªÆô£¬ÔòÖØÐÂ´ò¿ªqos
    if(qos_enabled == 1)
    {
        rtcfgUciSet("qos.wan.enabled=1");
        system("/etc/init.d/qos restart");
    }
    return 0;
    
}