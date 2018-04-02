#include "basic.h"


void proc_wanbandwidthconfig_set(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);
    bool enabled;
    int upload;
    int download;
    int ret;
     
    download= cJSON_GetObjectItem(jsonValue, "download")?cJSON_GetObjectItem(jsonValue, "download")->valueint:0;
    upload= cJSON_GetObjectItem(jsonValue, "upload")?cJSON_GetObjectItem(jsonValue, "upload")->valueint:0;
    enabled= cJSON_GetObjectItem(jsonValue, "enabled")?cJSON_GetObjectItem(jsonValue, "enabled")->valueint:0;

    DEBUG_PRINTF("[%s]===enabled:%d upload:%d download:%d ====\n",__func__,enabled,upload,download);
 
    ret= set_wanbandwidth_config(enabled, upload, download,1);
    
    
    return;
    
}

void proc_wanbandwidthconfig_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    bool enabled=0;
    int upload=0;
    int download=0;
    int ret=0;
    ret=get_wanbandwidth_config(&enabled, &upload, &download);
    cJSON *obj = NULL;

    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "download", cJSON_CreateNumber(download));
    cJSON_AddItemToObject(obj, "upload", cJSON_CreateNumber(upload));
    cJSON_AddItemToObject(obj, "enabled", cJSON_CreateNumber(enabled));
    
    return;
}


void proc_wanbandwidth_test(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=========\n",__func__);
    int ret=0;
    float output_upload=0;
    float output_download=0;
    char upload[64]={0};
    char download[64]={0};
    cJSON *obj = NULL;
    ret=wan_speedtest(&output_upload, &output_download);
    snprintf(upload,sizeof(upload),"%f M",output_upload);
    snprintf(download,sizeof(download),"%f M",output_download);

    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "download", cJSON_CreateString(download));
    cJSON_AddItemToObject(obj, "upload", cJSON_CreateString(upload));

    return;
   
}


void proc_wanbandwidth(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_wanbandwidthconfig_get(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	proc_wanbandwidthconfig_set(jsonValue, jsonOut);
    }
    else if ( (request_method & CGI_POST_METHOD) != 0 )
    {
        proc_wanbandwidth_test(jsonValue,jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return;
    }
}

int proc_wanspeed_get(cJSON *jsonValue, cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    int ret=0;
    int speedtest=0;
    char upspeed[64]={0};
    char downspeed[64]={0};
    char tmp_char[6]={0};
    cJSON *obj = NULL;
    char upload[64]={0};
    char download[64]={0};

    rtcfgUciGet("system.@system[0].speedtest",tmp_char);
    if(strlen(tmp_char) !=0)
    {
        speedtest = atoi(tmp_char);
    }
	else
	{
		global_weberrorcode=ERR_NO_SPEEDTEST;
		return ERR_NO_SPEEDTEST;
	}
	
    DEBUG_PRINTF("===[%s]====speedtest is %d===\n",__func__,speedtest); //speedtest may be 0: testing; 1: tested; 2:canceling;

    if(speedtest!=1)
    {
        ret=get_cur_wan_speed(upspeed, downspeed);

        speedtest=0;
    
        obj = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonOut, "data", obj);
        cJSON_AddItemToObject(obj, "speedtest", cJSON_CreateNumber(speedtest));
        cJSON_AddItemToObject(obj, "downspeed", cJSON_CreateString(downspeed));
        cJSON_AddItemToObject(obj, "upspeed", cJSON_CreateString(upspeed));
    }
    else
    {
        rtcfgUciGet("system.@system[0].output_upload",upload);
        rtcfgUciGet("system.@system[0].output_download",download);
        
        obj = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonOut, "data", obj);
        cJSON_AddItemToObject(obj, "speedtest", cJSON_CreateNumber(speedtest));
        cJSON_AddItemToObject(obj, "download", cJSON_CreateString(download));
        cJSON_AddItemToObject(obj, "upload", cJSON_CreateString(upload));
    }
    
    
    return 0;
}

int proc_wanspeed_set(cJSON *jsonValue, cJSON *jsonOut)
{
	int ret = 0;
    DEBUG_PRINTF("[%s]====\n",__func__);
    int action=0;
     
    action= cJSON_GetObjectItem(jsonValue, "action")?cJSON_GetObjectItem(jsonValue, "action")->valueint:1;
    DEBUG_PRINTF("[%s]====action is %d ===\n",__func__,action);
    if(1==action)
    {
        //rtcfgUciSet("system.@system[0].speedtest=2");
        //ret = ProcSpeedTestReq();
        system("killall -s SIGUSR2 qtec_speedtest");
        rtcfgUciSet("system.@system[0].speedtest=0");
        //system("qtec_speedtest&");
        ret= ProcSpeedTestReq();
    }
    else
    {
        //rtcfgUciSet("system.@system[0].speedtest=2");
        //DEBUG_PRINTF("[%s]====cancel speedtest===\n",__func__);
        system("killall -s SIGUSR1 qtec_speedtest");
     
    }
    return ret;
}

int proc_wanspeed(cJSON *jsonValue, cJSON *jsonOut)
{
	int ret = 0;
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        ret = proc_wanspeed_get(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_POST_METHOD ) != 0 )
    {
    	ret = proc_wanspeed_set(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return ERR_METHOD_NOT_SUPPORT;
    }

	return ret;
}
