#ifndef BASIC_H
#define BASIC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include "cJSON.h"
#include <librtcfg.h>
#include <time.h>
#include <openssl/md5.h>
#include <qtec_firewall_basic.h>
#include <qtec_ebtables_basic.h>

/*
  ● GET（SELECT）：从服务器取出资源（一项或多项）。
  ● POST（CREATE）：在服务器新建一个资源。
  ● PUT（UPDATE）：在服务器更新资源（客户端提供改变后的完整资源）。
  ● PATCH（UPDATE）：在服务器更新资源（客户端提供改变的属性）。
  ● DELETE（DELETE）：从服务器删除资源。
*/

//调试用的函数
#undef DEBUG
#define DEBUG 1
#ifdef DEBUG 

#undef DEBUG_PRINTF

#define DEBUG_PRINTF(format,...)   printf(format, ##__VA_ARGS__); fflush(stdout);
#else
#define DEBUG_PRINTF(format,...)
#endif

//在这里定义错误类型
enum web_error_code {
     ERR_NO_LOGIN = -100,             //用户未登入
     ERR_URL_NOT_SUPPORT= -99,        //URL 不支持
     ERR_METHOD_NOT_SUPPORT=-98,      //method 不支持
     ERR_PARAMETER_MISS=-97,          //关键参数缺失
     ERR_VALUE_WRONG=-96,             //值错误
     ERR_INTERNALLOGIC_WRONG=-95,     //内部逻辑错误
     //登入页面使用的
     ERR_DECRY_FAIL=-94,              //密码解密失败
     ERR_PASSWORD_NOTMATCH=-93,       //密码不匹配

     //wan_detect 使用的
     ERR_WAN_IPWRONG= -92,
     ERR_WAN_GWWRONG= -91,
     ERR_WAN_DNSWRONG= -90,
     ERR_WAN_INTERNETWRONG= -89,
     ERR_WAN_NOTUP= -88,

     ERR_NO_PERMIT=-87,                //在某情形下用户无权限进入处理某些cgi
     ERR_NO_MATCH=-86,                 //没发现对应的表项

	 ERR_NO_SPEEDTEST=-85,             //测速没有开启
	 ERR_PASSWORD_TOO_SIMPLE = -84,    //wifi密码太简单
     ERR_WIFI_PASSWORD_TOO_SHORT = -83,
	 ERR_SERIAL_NUMBER_MISMARCH = -82,
     ERR_RAWKEY_NOT_ENOUGH = -81,
	 ERR_GET_SMB_PSW_FAIL = -80,
	 ERR_GET_UPGRADE_RATE_FAIL = -79,

     ERR_IP_NOT_INTER=-78,   //ip not in lan subnet
     ERR_RULE_CONFLICT=-77,  //rule conflict
     ERR_VPN_TOO_MANY_ENABLE=-76,  //vpn only allow one rule enabled at the same time
	 ERR_GET_UPDATE_INFO_FAIL=-75,
	 ERR_GET_UCI_LOCK_FAIL = -74,
	 ERR_OFFLINE_OR_IN_MACBLOCK = -73,
     ERR_OTHER=0,
     ERR_WIFTIMER_TIME_CONFLICT=1,
     ERR_WIFITIMER_ARG_MISS=2,
     ERR_TOO_MANY_RULES=3,
};


#define CGI_GET_METHOD  1<<0
#define CGI_POST_METHOD 1<<1
#define CGI_PUT_METHOD  1<<2
#define CGI_PATCH_METHOD 1<<3
#define CGI_DELETE_METHOD 1<<4

#define MAX_SSID_LEN 33

//定义函数指针
typedef void (*CGIFunType)(cJSON *, cJSON *); 

struct cgi_module{
    char module_name[64];
    int supportMethod; 
    CGIFunType handle;
    int need_token_auth; //是否需要验证tokenid 0代表不需要，1代表需要
    int permit_limit;  //使用权限， 0 代表只有当系统未配置过才能使用， 1 代表只有当系统配置过才能使用，2代表任何情形都可使用
};


//全局变量
extern int global_weberrorcode;
extern char global_requesturl[64];
extern struct cgi_module global_cgimoduleArray[];
extern int request_method;

#define MAX_CONTENT_LEN 1024

//令牌长度
#define MAX_TOKENID_LEN 129

//密码加密解密 rsa 公钥私钥文件
#define RSA_PRIVATE_KEY_FILE "/tmp/rsa_private.key"
#define RSA_PUBLIC_KEY_FILE "/tmp/rsa_public.key"



//各个文件的函数名
//basic.c
void GetNextSynKeyId(unsigned char *keyid);
void GetFirstEncodeKey(unsigned char *key);
int err2msg(int errcode,char *input_msg, int msg_len);

//getKey.c
void proc_rsacfg_getKey(cJSON *jsonValue,cJSON *jsonOut);

//guiPassword.c
void check_gui_password(cJSON *jsonValue,cJSON *jsonOut);
void proc_firstconfigure_set(cJSON *jsonValue,cJSON *jsonOut);
void reset_gui_password(cJSON *jsonValue,cJSON *jsonOut);
int ProcPasswordSetByApp(cJSON *jsonValue,cJSON *jsonOut);
int ProcPasswordCheckByApp(cJSON *jsonValue,cJSON *jsonOut);
int ProcFirstconfigureSetByApp(cJSON *jsonValue,cJSON *jsonOut);

//wan.c
void get_wan_type(cJSON *json_value,cJSON *jsonOut);
void proc_wan_set(cJSON *json_value, cJSON *jsonOut);
void proc_wan_get(cJSON *json_value,cJSON *jsonOut);
void proc_wan_detect(cJSON *json_value, cJSON *jsonOut);
void HandleWanCfg(cJSON *jsonValue, cJSON *jsonOut);
int ProcWanSetByApp(cJSON *json_value, cJSON *jsonOut);
int ProcWanGetByApp(cJSON *json_value,cJSON *jsonOut);
void ProcWanDetialGet(cJSON *jsonValue, cJSON *jsonOut);

//system.c
void GetRouterConfigured(cJSON *jsonValue, cJSON *jsonOut);
void proc_reboot(cJSON *jsonValue, cJSON *jsonOut);
void proc_restore(cJSON *jsonValue, cJSON *jsonOut);
void proc_upgrade(cJSON *jsonValue, cJSON *jsonOut);
int ProcKeyReq(cJSON *jsonValue, cJSON *jsonOut, char *userid, char *deviceid, unsigned char *keyid); 
VOS_RET_E proc_queryupgrade(cJSON *jsonValue, cJSON *jsonOut);
void ProcQuicklyCheck(cJSON *jsonValue, cJSON *jsonOut);


//cgimsgreq.c
int proc_stalist_get(cJSON *json_value,cJSON *jsonOut);
void proc_routerstatus_get(cJSON *json_value,cJSON *jsonOut);
void ProcWifiMsgReq();
int CheckUpdateVersion(char *updateversion);
int ProcUpdateMsgGet(cJSON *json_value,cJSON *jsonOut);
int ProcUpdateReq(cJSON *json_value,cJSON *jsonOut);
void ProcFirstbootReq(cJSON *json_value,cJSON *jsonOut);

//timertask.c
void proc_timertask_set(cJSON *jsonValue,cJSON *jsonOut);
void proc_timertask_detect(cJSON *jsonValue,cJSON *jsonOut);



void HandleRouterBasicInfo(cJSON *jsonValue, cJSON *jsonOut);


//wireless.c
void proc_wifi_set(cJSON *jsonValue,cJSON *jsonOut);
void proc_wifi_get(cJSON *jsonValue,cJSON *jsonOut);
int ProcWifiSetByApp(cJSON *jsonValue,cJSON *jsonOut);
void ProcWifiGetByApp(cJSON *jsonValue,cJSON *jsonOut);
void ProcWifiFirewallSet(cJSON *jsonValue,cJSON *jsonOut);
void ProcWifiFirewallGet(cJSON *jsonValue,cJSON *jsonOut);

//clientinfo.c
void proc_offline_get(cJSON *jsonValue,cJSON *jsonOut);
void proc_onlinetag(cJSON *jsonValue,cJSON *jsonOut);

//arpbandinfo.c
void proc_arpband(cJSON *jsonValue,cJSON *jsonOut);
//macblock.c
int check_macblock_add(char *mac);
int proc_macblock(cJSON *jsonValue, cJSON *jsonOut);
int proc_macblock_get(cJSON *jsonValue,cJSON *jsonOut);
//ddos.c
void proc_ddos(cJSON *jsonValue, cJSON *jsonOut);
//childrule.c
int proc_childrule(cJSON *jsonValue, cJSON *jsonOut);
//speedlimit.c
int proc_speedlimit(cJSON * jsonValue, cJSON * jsonOut);
//dmz.c
void proc_dmz(cJSON * jsonValue, cJSON * jsonOut);
//pf.c
void proc_pf_del(cJSON *json_value, cJSON *jsonOut);
void proc_pf_add(cJSON *json_value, cJSON *jsonOut);
void proc_pf_mod(cJSON *json_value, cJSON *jsonOut);
void proc_pf_get(cJSON *json_value, cJSON *jsonOut);
void proc_pf(cJSON *jsonValue, cJSON *jsonOut);
//qtec_disk.c
int proc_qtec_disk_post(cJSON *jsonValue,cJSON *jsonOut);
int proc_qtec_disk_get(cJSON *jsonValue,cJSON *jsonOut);
int proc_qtec_disk_check(cJSON *jsonValue,cJSON *jsonOut);
int proc_qtec_disk_unmount(cJSON *jsonValue,cJSON *jsonOut);



int proc_qtec_disk(cJSON *jsonValue, cJSON *jsonOut);


//wan_bandwidth.c
void proc_wanbandwidthconfig_set(cJSON *jsonValue,cJSON *jsonOut);
void proc_wanbandwidthconfig_get(cJSON *jsonValue,cJSON *jsonOut);
void proc_wanbandwidth_test(cJSON *jsonValue,cJSON *jsonOut);
void proc_wanbandwidth(cJSON *jsonValue, cJSON *jsonOut);
int proc_wanspeed(cJSON *jsonValue, cJSON *jsonOut);

int proc_get_smb_pwd(cJSON *jsonValue,cJSON *jsonOut);

//vpn
void vpn_set_edit_flag();
int vpn_get_edit_flag();
int proc_add_vpn(cJSON * jsonValue, cJSON * jsonOut);
int proc_add_vpn_by_web(cJSON * jsonValue, cJSON * jsonOut);
int proc_edit_vpn(cJSON * jsonValue, cJSON * jsonOut);
int proc_edit_vpn_by_web(cJSON *jsonValue,cJSON *jsonOut);
int proc_del_vpn(cJSON * jsonValue, cJSON * jsonOut);
int proc_get_vpn(cJSON * jsonValue, cJSON * jsonOut);
int proc_set_vpn_sw(cJSON *jsonValue,cJSON *jsonOut);
void ProcSetVpnMsgReq();


int procWifiSetTxpower(cJSON *jsonValue,cJSON *jsonOut);
int procWifiGetTxpower(cJSON *jsonValue,cJSON *jsonOut);

int procSetGuestWifi(cJSON *jsonValue,cJSON *jsonOut);
int procGetGuestWifi(cJSON *jsonValue,cJSON *jsonOut);
void ProcSetGuestWifiMsgReq();

//antiwifimanager.c
void ProcAntiwifiSet(cJSON *json_value,cJSON *jsonOut);
void ProcAntiwifiStatusGet(cJSON *json_value,cJSON *jsonOut);
void ProcQuestionSet(cJSON *json_value,cJSON *jsonOut);
void ProcAntiWifiAdminSet(cJSON *json_value,cJSON *jsonOut);
void ProcAntiWifiDevGet(cJSON *json_value,cJSON *jsonOut);
int ProcAntiWifiAuthSet(cJSON *json_value,cJSON *jsonOut);
void ProcAntiWifiQuestionGet(cJSON *json_value,cJSON *jsonOut);
void ProcAntiWifiPasswordCheck(cJSON *json_value,cJSON *jsonOut);
void ProcAntiWifiQueandAswGet(cJSON *json_value,cJSON *jsonOut);

//qos.c
int proc_qos(cJSON *jsonValue, cJSON *jsonOut);
void proc_qos_set(cJSON *jsonValue, cJSON *jsonOut);
void proc_qos_get(cJSON *jsonValue, cJSON *jsonOut);

//wifitimer
int proc_wifitimer_set(cJSON *jsonValue,cJSON *jsonOut);
void proc_wifitimer_del(cJSON *jsonValue,cJSON *jsonOut);
int proc_wifitimer_get(cJSON *jsonValue,cJSON *jsonOut);
int proc_wifitimer_sw_set(cJSON *jsonValue,cJSON *jsonOut);
void ProcAddWifiTimer(TASK_INFO_STRU *pstTaskInfo);
void ProcDelWifiTimer(UINT16 taskId);
void ProcEditWifiTimer(TASK_INFO_STRU *pstTaskInfo);
void ProcSetWifiTimerSw(TASK_SW_INFO *pstTaskSwInfo);

//specialcare
void proc_specialcare_cfg_set(cJSON *jsonValue,cJSON *jsonOut);
void proc_specialcare_cfg_get(cJSON *jsonValue,cJSON *jsonOut);
int proc_specialcare(cJSON *jsonValue, cJSON *jsonOut);

//wds
int proc_wdscfg_get(cJSON *jsonValue,cJSON *jsonOut);
int proc_wds_scan(cJSON *jsonValue,cJSON *jsonOut);
int proc_wdscfg_set(cJSON *jsonValue,cJSON *jsonOut);
int proc_wds_setup(cJSON *jsonValue,cJSON *jsonOut);
int proc_wds_status_get(cJSON *jsonValue,cJSON *jsonOut);
void ProcWdsSetReq();

//onekeyswitch
void ProcOneKeySwitchMsgReq();
int proc_onekeyswitch(cJSON *jsonValue, cJSON *jsonOut);
int proc_get_onekeyswitch_status(cJSON *jsonValue, cJSON *jsonOut);

//firewall
int proc_get_firewall_cfg(cJSON *jsonValue, cJSON *jsonOut);
int proc_set_firewall_cfg(cJSON *jsonValue, cJSON *jsonOut);
void ProcFirewallSetMsgReq();


//hosts
int proc_get_hosts(cJSON *jsonValue, cJSON *jsonOut);
int proc_set_hosts(cJSON *jsonValue, cJSON *jsonOut);


//lan
int proc_landhcp_get(cJSON *json_value, cJSON *jsonOut);
int proc_landhcp_set(cJSON *json_value, cJSON *jsonOut);
void ProcLanCfgSetMsgReq();

//ebtables_speedlimit
void ebtables_proc_speedlimit_mod(cJSON *jsonValue,cJSON *jsonOut);
void ebtables_proc_speedlimit_get(cJSON *jsonValue,cJSON *jsonOut);
void ebtables_proc_speedlimit(cJSON *jsonValue, cJSON *jsonOut);

//uci lock
int QtGetUciLock();
int QtReleaseUciLock();

//first key
void ProcDisplayFirstKeyMsgReq(char * key);

#endif
