#include "basic.h"

/*
  ● GET（SELECT）：从服务器取出资源（一项或多项）。
  ● POST（CREATE）：在服务器新建一个资源。
  ● PUT（UPDATE）：在服务器更新资源（客户端提供改变后的完整资源）。
  ● PATCH（UPDATE）：在服务器更新资源（客户端提供改变的属性）。
  ● DELETE（DELETE）：从服务器删除资源。
*/

/*
struct cgi_module{
    char module_name[64];
    int supportMethod; 
    void* func(cJSON *,cJSON *);
};

*/


struct cgi_module global_cgimoduleArray[]={
    { "getkeycfg", CGI_GET_METHOD, proc_rsacfg_getKey , 0,2},    //获取公钥
    { "logincfg",  CGI_PUT_METHOD, check_gui_password, 0,2},    //用户登入

    //快速配置向导
    { "get_systemconfigure_cfg", CGI_GET_METHOD, GetRouterConfigured,0,2},
    { "get_wantype_cfg",CGI_GET_METHOD, get_wan_type,0,2},  //获取当前上网方式
    { "set_basicwan_cfg",CGI_PUT_METHOD, proc_wan_set,0,0}, //配置wan配置
    { "set_firstconfigure_cfg",CGI_PUT_METHOD, proc_firstconfigure_set,0,0},  //第一次设置

    { "get_stalist_cfg", CGI_GET_METHOD, proc_stalist_get, 1,1},  //get stalist
	{ "get_routerstatus_cfg", CGI_GET_METHOD, proc_routerstatus_get, 1,1}, //proc_routerstatus_get

    //wan detect
    { "get_wandect_cfg", CGI_GET_METHOD, proc_wan_detect,0,2},


    { "get_timertask_cfg", CGI_GET_METHOD, proc_timertask_detect,1,1},
    { "set_timertask_cfg", CGI_PUT_METHOD, proc_timertask_set,1,1},
    
    { "reboot", CGI_PUT_METHOD, proc_reboot,1,1},

    { "restore", CGI_PUT_METHOD, ProcFirstbootReq,1,1},
        
    { "reset_guipassword_cfg", CGI_PUT_METHOD,reset_gui_password,1,1}, //重新设置gui 管理员密码
    { "handle_routerinfo_cfg", CGI_GET_METHOD | CGI_PUT_METHOD, HandleRouterBasicInfo, 1,1}, //获取和设置路由器基本信息


    { "get_update_version", CGI_GET_METHOD, ProcUpdateMsgGet, 1,1},
    { "upgrade", CGI_PUT_METHOD, ProcUpdateReq, 1,1},
    { "get_update_rate", CGI_GET_METHOD, proc_queryupgrade, 1,1},
	
	//wireless
    { "set_wireless_cfg", CGI_PUT_METHOD, proc_wifi_set, 1,1},
    { "get_wireless_cfg", CGI_GET_METHOD, proc_wifi_get, 1,1},

    { "get_offline_cfg", CGI_GET_METHOD, proc_offline_get, 1,1},
    { "proc_onlinetag_cfg", CGI_GET_METHOD|CGI_PUT_METHOD, proc_onlinetag, 1,1},


    { "handle_wan_cfg", CGI_GET_METHOD | CGI_PUT_METHOD, HandleWanCfg, 1,1},
	{ "get_wan_detial_cfg", CGI_GET_METHOD, ProcWanDetialGet, 1, 1},

    //firewall
    { "proc_arpband_cfg", CGI_GET_METHOD|CGI_POST_METHOD|CGI_PUT_METHOD|CGI_DELETE_METHOD, proc_arpband, 1,1},
    { "proc_macblock_cfg", CGI_GET_METHOD|CGI_POST_METHOD|CGI_PUT_METHOD|CGI_DELETE_METHOD, proc_macblock, 1,1},
    { "proc_ddos_cfg", CGI_GET_METHOD|CGI_PUT_METHOD, proc_ddos, 1,1},
    { "proc_childrule_cfg", CGI_GET_METHOD|CGI_POST_METHOD|CGI_PUT_METHOD|CGI_DELETE_METHOD, proc_childrule, 1,1},
    { "proc_speedlimit_cfg", CGI_GET_METHOD|CGI_PUT_METHOD, proc_speedlimit, 1,1},           //this cgi should not be using 
    { "ebtables_proc_speedlimit_cfg", CGI_GET_METHOD|CGI_PUT_METHOD, ebtables_proc_speedlimit, 1,1},
    { "proc_dmz_cfg", CGI_GET_METHOD|CGI_PUT_METHOD, proc_dmz,1,1},
    { "proc_pf_cfg", CGI_GET_METHOD|CGI_POST_METHOD|CGI_PUT_METHOD|CGI_DELETE_METHOD, proc_pf, 1,1},

    //qtec_disk
    { "proc_qtec_disk_cfg", CGI_GET_METHOD|CGI_POST_METHOD|CGI_PUT_METHOD|CGI_DELETE_METHOD,proc_qtec_disk,1,1},

    //samba
    { "get_smb_pwd", CGI_GET_METHOD, proc_get_smb_pwd, 1,1},

    //vpn
    { "add_vpn_cfg", CGI_POST_METHOD, proc_add_vpn_by_web, 1,1},
    { "set_vpn_cfg", CGI_POST_METHOD, proc_edit_vpn_by_web, 1,1},
    { "del_vpn_cfg", CGI_POST_METHOD, proc_del_vpn, 1,1},
    { "get_vpn_cfg", CGI_GET_METHOD, proc_get_vpn, 1,1},
    { "set_vpn_sw", CGI_POST_METHOD, proc_set_vpn_sw, 1,1},
	
	{ "proc_wanbandwidth_cfg", CGI_GET_METHOD | CGI_POST_METHOD | CGI_PUT_METHOD, proc_wanbandwidth,1,1},
    { "proc_wan_speedtest_cfg", CGI_GET_METHOD | CGI_POST_METHOD, proc_wanspeed,1,1},
    //wifi txpower
    { "set_wifi_txpower", CGI_POST_METHOD, procWifiSetTxpower, 1,1},
    { "get_wifi_txpower", CGI_GET_METHOD, procWifiGetTxpower, 1,1},
    //guest wifi
    { "set_guest_wifi", CGI_POST_METHOD, procSetGuestWifi, 1,1},
    { "get_guest_wifi", CGI_GET_METHOD, procGetGuestWifi, 1,1},

    //qos
    { "proc_qos_cfg", CGI_GET_METHOD | CGI_PUT_METHOD, proc_qos, 1,1},
	//anti wifi
	{ "set_antiwifi", CGI_POST_METHOD, ProcAntiwifiSet, 1,1},
	{ "get_antiwifi_status", CGI_GET_METHOD, ProcAntiwifiStatusGet, 1,1},
	{ "set_antiwifi_question", CGI_POST_METHOD, ProcQuestionSet, 1,1},
	{ "set_antiwifi_admin_forbidden", CGI_POST_METHOD, ProcAntiWifiAdminSet, 1,1},
	{ "get_antiwifi_dev_list", CGI_GET_METHOD, ProcAntiWifiDevGet, 1,1},
	{ "set_authed_antiwifi_dev", CGI_POST_METHOD, ProcAntiWifiAuthSet, 1,1},
	{ "get_antiwifi_authinfo", CGI_GET_METHOD, ProcAntiWifiQueandAswGet, 1,1},
	{ "get_antiwifi_question", CGI_GET_METHOD, ProcAntiWifiQuestionGet, 0,2},
	{ "check_antiwifi_password", CGI_POST_METHOD, ProcAntiWifiPasswordCheck, 0,2},
    //wifitimer
    { "set_wifi_timer", CGI_POST_METHOD, proc_wifitimer_sw_set, 1,1},
	{ "get_wifi_timer", CGI_GET_METHOD, proc_wifitimer_get, 1,1},
	{ "set_wifi_timer_rule", CGI_POST_METHOD, proc_wifitimer_set, 1,1},
	{ "del_wifi_timer_rule", CGI_POST_METHOD, proc_wifitimer_del, 1,1},

    //specialcare
    { "proc_specialcare_cfg", CGI_GET_METHOD | CGI_PUT_METHOD, proc_specialcare, 1,1},

    //wds
    { "get_wds_cfg", CGI_GET_METHOD, proc_wdscfg_get, 1,1},
	{ "get_wds_wifi_scan", CGI_GET_METHOD, proc_wds_scan, 1,1},
	{ "set_wds_cfg", CGI_POST_METHOD, proc_wdscfg_set, 1,1},
	{ "set_up_wds", CGI_POST_METHOD, proc_wds_setup, 1,1},
	{ "get_wds_status", CGI_GET_METHOD, proc_wds_status_get, 1,1},

    //onekeyswitch
    {"onekeyswitch", CGI_POST_METHOD, proc_onekeyswitch, 1, 1},
    {"get_onekeyswitch", CGI_GET_METHOD, proc_get_onekeyswitch_status, 1, 1},
    
    //firewall
    {"set_firewall_status", CGI_POST_METHOD, proc_set_firewall_cfg, 1, 1},
    {"get_firewall_status", CGI_GET_METHOD, proc_get_firewall_cfg, 1, 1},

    //hosts
    {"set_hosts", CGI_POST_METHOD, proc_set_hosts, 1, 1},
    {"get_hosts", CGI_GET_METHOD, proc_get_hosts, 1, 1},

    //lan
    {"set_lan_cfg", CGI_POST_METHOD, proc_landhcp_set, 1, 1},
    {"get_lan_cfg", CGI_GET_METHOD, proc_landhcp_get, 1, 1},

	//wifi_firewall
	{"set_wifi_firewall_cfg", CGI_POST_METHOD, ProcWifiFirewallSet, 1, 1},
    {"get_wifi_firewall_cfg", CGI_GET_METHOD, ProcWifiFirewallGet, 1, 1},
};

#define NUM_CGI_MODULE (sizeof(global_cgimoduleArray)/sizeof(struct cgi_module))

int methodstr2int(char *input_method)
{
    int ret=0;
    if(strncmp(input_method,"get",strlen(input_method))==0)
    {
        ret=CGI_GET_METHOD;
    }
    else if(strncmp(input_method,"post",strlen(input_method))==0)
    {
        ret=CGI_POST_METHOD;
    }
    else if(strncmp(input_method,"put",strlen(input_method))==0)
    {
        ret=CGI_PUT_METHOD;
    }
    else if(strncmp(input_method,"patch",strlen(input_method))==0)
    {
        ret=CGI_PATCH_METHOD;
    }
    else if(strncmp(input_method,"delete",strlen(input_method))==0)
    {
        ret=CGI_DELETE_METHOD;
    }

    return ret;
}


void decode_message(char *msg)
{
	cJSON *json, *jsonValue, *jsonOut;
	char *target, *buffer, *method;
	int  f0, f1, f2;
    char *dup2file = (access("/tmp/webdebug", F_OK) == 0)?"/tmp/webdebug":"/dev/null";
	
	json = cJSON_Parse(msg);
	if (!json)
	{
		printf("error!");
		return;		
	}

    //将打印输出到串口
	f0 = dup(STDOUT_FILENO);
	f1 = open(dup2file, O_RDWR | O_APPEND);
	f2 = dup2(f1, STDOUT_FILENO);
	close(f1);
	

	target = cJSON_GetObjectItem(json, "requestUrl")?cJSON_GetObjectItem(json, "requestUrl")->valuestring:"";
    method = cJSON_GetObjectItem(json, "method")?cJSON_GetObjectItem(json, "method")->valuestring:"";
    DEBUG_PRINTF("===target: %s===\n", target);
    DEBUG_PRINTF("===method: %s=== \n",method);
   
    request_method = methodstr2int(method);
    UTIL_STRNCPY(global_requesturl,target,sizeof(global_requesturl));
    
	jsonValue = cJSON_GetObjectItem(json, "data");
	
	jsonOut = cJSON_CreateObject();

    int num = NUM_CGI_MODULE;
  
    
    int index=0;
    for(index=0;index<num;index++)
    {
        if(strncmp(target,global_cgimoduleArray[index].module_name,strlen(target))==0)
        {
            int configured=0;
            GetSystemConfigured(&configured);

            if((global_cgimoduleArray[index].permit_limit==0) && (configured!=0))
            {
                DEBUG_PRINTF("[%s]===this url only called in system unconfigured state, but now system is configured===\n",__func__);
                global_weberrorcode=ERR_NO_PERMIT;
                goto OUT;
            }
            else if((global_cgimoduleArray[index].permit_limit==1) &&(configured!=1))
            {
                DEBUG_PRINTF("[%s]===this url only called in system configured state, but now system is unconfigured ===\n",__func__);
                global_weberrorcode=ERR_NO_PERMIT;
                goto OUT;
            }
            
            if((request_method & global_cgimoduleArray[index].supportMethod) == 0)
            {
                global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
                goto OUT;
            }
            else
            {
                //检测tokenid
                if( global_cgimoduleArray[index].need_token_auth == 1)
                {

                    int ret=webCheckTokenId();
                    if(ret<0)
                    {
                        DEBUG_PRINTF("[%s]====tokenid auth fail===\n",__func__);
                        global_weberrorcode=ERR_NO_LOGIN;
                        goto OUT;
                    }
                }
    
                global_cgimoduleArray[index].handle(jsonValue, jsonOut);

                goto OUT;
            }
        }
    }
    
    if(index == num)
    {
        global_weberrorcode = ERR_URL_NOT_SUPPORT;
    }
   

OUT:
    err2replymsg(global_weberrorcode, global_requesturl, jsonOut);
    
	fflush(stdout);
	dup2(f0, f2);
	buffer = cJSON_Print(jsonOut);
	printf("%s\n",buffer);
	free(buffer);

	cJSON_Delete(jsonOut);
	cJSON_Delete(json);
	
	return;
}

#if 1
int main()
{

	int length;
	char *method;
	char *inputstring;

	printf("Content-Type: application/json\n\n");

    fflush(stdout);
	method = getenv("REQUEST_METHOD"); 
 
	if(method == NULL)
	{
		return 1;   
	}
	
	//post method,read from stdin
	if(!strcmp(method, "POST")) 
	{

		length = atoi(getenv("CONTENT_LENGTH")); 
		if(length != 0)
		{
			inputstring = malloc(sizeof(char)*length + 1); 
            if (inputstring)
            {
    			if (0 != fread(inputstring, sizeof(char), length, stdin)) 
    			{
    				decode_message(inputstring);
    			}
                free(inputstring);
            }
	    }
	}

	//get method
	else if(!strcmp(method, "GET"))
	{

		inputstring = getenv("QUERY_STRING");   
		length = strlen(inputstring);

	}

	return 0;
}
#endif


//用来调试获取http头信息
int http_main()
{
    extern char   **environ;
    int nlen = 0;
    int i;
    char szContent[MAX_CONTENT_LEN];
    char **penv;
    char *req = NULL;

    memset(szContent, 0, MAX_CONTENT_LEN);
        
    printf("Content-type: text/html\n\n");
    
    for ( penv = environ; *penv; penv++ )
        printf("%s<br>", *penv);

    if ( strcmp("POST", getenv("REQUEST_METHOD")) == 0 )
    {
        nlen = atoi(getenv("CONTENT_LENGTH"));
        for (i = 0; i < nlen; i++ )    
        {
            if ( i < MAX_CONTENT_LEN )
                szContent[i] = fgetc(stdin);
            else
                break;
        }
        printf("<p>%s</p>", szContent);
    }

    char *TOKEN_ID=NULL;
    TOKEN_ID=getenv("HTTP_TOKEN_ID");
    printf("<p>%s</p>",TOKEN_ID);
    return 0;
}

//该main函数是用来调试
void debug_main()
//void main()
{
   // proc_wan_detect();    
#if 0
   int i=checkPingResult("1.1.342.3242", "eth0.2");
   DEBUG_PRINTF("====i: %d ====\n",i);
   DEBUG_PRINTF("===2===\n");

   i=checkPingResult("192.168.90.1","eth0.2");
    DEBUG_PRINTF("====i: %d ====\n",i);

   DEBUG_PRINTF("===3====\n");
   i=checkPingResult("12.0.0.1","eth0.2");
   DEBUG_PRINTF("====i: %d ====\n",i);
   DEBUG_PRINTF("====4====\n");
   i=checkPingResult("192.168.90.1", "eth0.342");
   DEBUG_PRINTF("====i: %d ====\n",i);
#endif 
}
