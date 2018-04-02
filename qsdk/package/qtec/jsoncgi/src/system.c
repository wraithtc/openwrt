#include "basic.h"
#include "keyapi.h"
#include <libwebsockets.h>

void GetRouterInfo(cJSON *jsonValue, cJSON *jsonOut)
{
	struct systemInfo output = {0};
	cJSON *obj = NULL;

	obj = cJSON_CreateObject();
	if(NULL == obj)
	{
		return;
	}

	(void)getSystemInfo(&output);
	cJSON_AddItemToObject(jsonOut, "data", obj);
	cJSON_AddItemToObject(obj, "hostname", cJSON_CreateString(output.product));
	cJSON_AddItemToObject(obj, "version", cJSON_CreateString(output.productVersion));
	cJSON_AddItemToObject(obj, "serialnum", cJSON_CreateString(output.serialnum));
	cJSON_AddItemToObject(obj, "devmodel", cJSON_CreateString("QTEC"));
	cJSON_AddItemToObject(obj, "configured", cJSON_CreateNumber(output.configured));
	

}

//获取是否是第一次配置
void GetRouterConfigured(cJSON *jsonValue, cJSON *jsonOut)
{
    cJSON *obj = NULL;
	obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    int configured=0;
    GetSystemConfigured(&configured);
    cJSON_AddItemToObject(obj, "configured", cJSON_CreateNumber(configured));
    
}


void proc_reboot(cJSON *jsonValue, cJSON *jsonOut)
{
    QtReboot();
}

void proc_restore(cJSON *jsonValue, cJSON *jsonOut)
{
    QtRestore();
}

//获取路由器基本信息
void GetRouterBasicInfo(cJSON *jsonValue, cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    cJSON *obj = NULL;
	FILE *fp = NULL;
	obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);

    char router_name[64]={0};
    char ssid1[64]={0};
    char ssid2[64]={0};
    char lanip[64]={0};
    char wanip[64]={0};

    rtcfgUciGet("system.@system[0].routername",router_name);

    //设置默认名
    if(strlen(router_name) == 0 )
    {
        UTIL_STRNCPY(router_name,"3care-gateway",sizeof(router_name));
    }

    rtcfgUciGet("wireless.@wifi-iface[0].ssid",ssid1);
    rtcfgUciGet("wireless.@wifi-iface[1].ssid",ssid2);
    rtcfgUciGet("network.lan.ipaddr",lanip);

	system("ubus call network.interface.wan status | grep \"address\" | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' > /tmp/.wanip");
	fp = fopen("/tmp/.wanip","r");
	fgets(wanip,64,fp);
	fclose(fp);

    cJSON_AddItemToObject(obj, "routername", cJSON_CreateString(router_name));
    cJSON_AddItemToObject(obj, "ssid1", cJSON_CreateString(ssid1));
    cJSON_AddItemToObject(obj, "ssid2", cJSON_CreateString(ssid2));
    cJSON_AddItemToObject(obj, "lanip", cJSON_CreateString(lanip));
    cJSON_AddItemToObject(obj, "wanip", cJSON_CreateString(wanip));    
}

void GetRouterBasicInfoByApp(cJSON *jsonValue, cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    cJSON *obj = NULL;
	FILE *fp = NULL;
	obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);

    char router_name[64]={0};
    char ssid1[64]={0};
    char ssid2[64]={0};
    char lanip[64]={0};
    char wanip[64]={0};
	char wanmac[64]={0};
	char tmp1[64] = {0};
	char tmp2[64] = {0};
	char tmp3[64] = {0};
	char tmp4[64] = {0};
	char buffer[256] = {0};

    rtcfgUciGet("wireless.@wifi-iface[0].ssid",ssid1);
    rtcfgUciGet("wireless.@wifi-iface[1].ssid",ssid2);
    rtcfgUciGet("network.lan.ipaddr",lanip);
	rtcfgUciGet("network.wan.macaddr",wanmac);

	system("ifconfig eth0 > /tmp/.wanmac");
    fp = fopen("/tmp/.wanmac","r");
	if(fp)
	{
		fgets(buffer,256,fp);
    	sscanf(buffer, "%s %s %s %s %s", tmp1,tmp2,tmp3,tmp4, wanmac);
		fclose(fp);
		fp = NULL;
	}
	
	system("ubus call network.interface.wan status | grep \"address\" | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' > /tmp/.wanip");
	fp = fopen("/tmp/.wanip","r");
	fgets(wanip,64,fp);
	fclose(fp);

    cJSON_AddItemToObject(obj, "lfwifissid", cJSON_CreateString(ssid1));
    cJSON_AddItemToObject(obj, "hfwifissid", cJSON_CreateString(ssid2));
    cJSON_AddItemToObject(obj, "lanipaddress", cJSON_CreateString(lanip));
    cJSON_AddItemToObject(obj, "wanipaddress", cJSON_CreateString(wanip));
	cJSON_AddItemToObject(obj, "wanmac", cJSON_CreateString(wanmac));
	cJSON_AddItemToObject(obj, "cputype", cJSON_CreateString("qtec"));
	cJSON_AddItemToObject(obj, "cpubrand", cJSON_CreateString("qtecbrand")); 
	cJSON_AddItemToObject(obj, "cpufactory", cJSON_CreateString("qtecfactory"));        
}
//设置路由器基本信息
void SetRouterBasicInfo(cJSON *jsonValue, cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
    
    char routername[64]={0};
    UTIL_STRNCPY(routername,cJSON_GetObjectItem(jsonValue, "routername")?cJSON_GetObjectItem(jsonValue, "routername")->valuestring:"",sizeof(routername));
    if(strlen(routername) == 0)
    {
        global_weberrorcode=ERR_PARAMETER_MISS;
        return;
    }
    char cmd[256]={0};
    int ret=0;
    snprintf(cmd,256,"system.@system[0].routername=%s",routername);
    ret=rtcfgUciSet(cmd);
    if(ret !=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
    }
    
}

void HandleRouterBasicInfo(cJSON *jsonValue, cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);

    if( (request_method & CGI_GET_METHOD) != 0)
    {
        GetRouterBasicInfo(jsonValue, jsonOut);
    }
    else if ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
        SetRouterBasicInfo(jsonValue, jsonOut);
    }
    else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
    }
}

void proc_upgrade(cJSON *jsonValue, cJSON *jsonOut)
{
    int isKeepConfig = cJSON_GetObjectItem(jsonValue, "keepconfig")?cJSON_GetObjectItem(jsonValue, "keepconfig")->valueint:1;

    QtUpgradeSoftware(1);
}

VOS_RET_E proc_queryupgrade(cJSON *jsonValue, cJSON *jsonOut)
{
    cJSON *dataObj;
    char statusStr[32] = {0};
    VOS_RET_E ret;
    
    ret = QtQueryUpgrade(statusStr, (int)sizeof(statusStr));

    if (ret == VOS_RET_SUCCESS)
    {
        dataObj = cJSON_CreateObject();
        if (dataObj == NULL)
        {
            printf("Fail to create dataObj!\n");
			global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
            return ERR_INTERNALLOGIC_WRONG;
        }

        cJSON_AddItemToObject(dataObj, "status", cJSON_CreateString(statusStr));
        cJSON_AddItemToObject(jsonOut, "data", dataObj);
		return VOS_RET_SUCCESS;
    }
	else
	{
		global_weberrorcode=ERR_GET_UPGRADE_RATE_FAIL;
		return ERR_GET_UPGRADE_RATE_FAIL;
	}
}

int ProcKeyReq(cJSON *jsonValue, cJSON *jsonOut, char *userid, char *deviceid, unsigned char *keyid) 
{
	char devicename[32] = {0};
	unsigned char keyId[KEYIDLEN+1] = {0};
	unsigned char key[KEYLEN+1] = {0};
    unsigned char keybased[32] = {0};
	SrcMessage stSrcMessage = {0};
	DestMessage stDestMessage = {0};
	int usedcount = 0;
	int unusedcount = 0;
	int i = 0;
	int index, ret;
	unsigned char *keytemp, *keyidtemp;

	cJSON *obj = NULL;
	cJSON *array = NULL;
	cJSON *subJson =NULL;

	subJson = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", subJson);

	struct tagCQtQkMangent *pstcqtqkmangent; 

	pstcqtqkmangent = GetCQtQkMangent(); 

	//get info
	stSrcMessage.keyNumber = cJSON_GetObjectItem(jsonValue, "keynumber")?cJSON_GetObjectItem(jsonValue, "keynumber")->valueint:4; 
	stSrcMessage.keyLength = KEYLEN;
	strcpy(stSrcMessage.localUserId , userid);
	strcpy(stSrcMessage.localDeviceId, deviceid);
	strcpy(devicename, (cJSON_GetObjectItem(jsonValue, "devicename")?cJSON_GetObjectItem(jsonValue, "devicename")->valuestring:""));

	stDestMessage.keyId = malloc(stSrcMessage.keyNumber * 16 + 16);
	stDestMessage.key = malloc(stSrcMessage.keyNumber * 16 + 16);
	memset(stDestMessage.keyId, 0, stSrcMessage.keyNumber * 16 + 16);
	memset(stDestMessage.key, 0, stSrcMessage.keyNumber * 16 + 16);

	//get synkey
	ret = C_DealClientSynReq(&stDestMessage, &stSrcMessage);
	if(ret)
	{
		ReleaseCQtQkMangent(&pstcqtqkmangent);
		free(stDestMessage.keyId);
		free(stDestMessage.key);
        ProcRawkeyAddReq();
		return ERR_INTERNALLOGIC_WRONG;
	}

	ret = C_DeleteSynKeyByNodeId(pstcqtqkmangent, keyid, userid, deviceid);
    if(ret)
	{
		ReleaseCQtQkMangent(&pstcqtqkmangent);
		free(stDestMessage.keyId);
		free(stDestMessage.key);
        ProcRawkeyAddReq();
		return ERR_INTERNALLOGIC_WRONG;
	}
	//jsonout
	cJSON_AddItemToObject(subJson, "keynumber", cJSON_CreateNumber(stDestMessage.keyNumber));
	cJSON_AddItemToObject(subJson, "deviceid", cJSON_CreateString(stDestMessage.pushDeviceId));
	cJSON_AddItemToObject(subJson, "devicename", cJSON_CreateString(devicename));
	cJSON_AddItemToObject(subJson,"keylist",array=cJSON_CreateArray());

	keytemp = stDestMessage.key;
	keyidtemp = stDestMessage.keyId;

	for(index = 0; index < stSrcMessage.keyNumber; index++)
	{
		memset(keyId, 0 ,KEYIDLEN+1);
		memset(key, 0 ,KEYLEN+1);
		memcpy(keyId, keyidtemp, KEYIDLEN);
		memcpy(key, keytemp, KEYLEN);
        lws_b64_encode_string(key, 16, keybased, 32);
		cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
		cJSON_AddItemToObject(obj,"keyid",cJSON_CreateString(keyId));
		cJSON_AddItemToObject(obj,"key",cJSON_CreateString(keybased));
		printf("keyid:%s, synkey:",keyId);
		for(i = 0; i < 16;i++)
		{
			printf("0x%2x ",key[i]);
		}
		printf("\n");
		keytemp += KEYLEN;
		keyidtemp += KEYIDLEN;
	} 		

	ReleaseCQtQkMangent(&pstcqtqkmangent);
	free(stDestMessage.keyId);
	free(stDestMessage.key);

	ProcRawkeyAddReq();
	return 0 ;	
}

void ProcQuicklyCheck(cJSON *jsonValue, cJSON *jsonOut)
{
    bool ddosenabled=0;
    int  telentenabled = 0;
    int  ftpenabled = 0;
    int  sambaenabled = 0;
    int  redirectenabled = 0;
    int  dmzenabled = 0;
    int  wififirewallenable = 0;
    int  ret = 0;
    char redirectname[64] = {0};
    char wifiKeyFirewall[64] = {0};
    char buf[256] = {0};
    FILE *fp = NULL;
    cJSON *obj = NULL;

    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);

    /* wififirewall */
    rtcfgUciGet("system.@system[0].wifikeyfirewall",wifiKeyFirewall);
    wififirewallenable=atoi(wifiKeyFirewall);
    
    //ddos
    get_ddos(&ddosenabled);

    //telentd
    system("ps >/tmp/.quicklycheckinfo");
    fp = fopen("/tmp/.quicklycheckinfo", "r");

    if (fp != NULL)
	{
		while (fgets(buf, 256, fp)!= NULL)
		{
            if(strstr(buf, "telnetd"))
            {
                telentenabled = 1;
            }
            else if(strstr(buf, "ftpd"))
            {
                ftpenabled = 1;
            }
            else if(strstr(buf, "smbd"))
            {
                sambaenabled = 1;
            }
		}
        fclose(fp);
	}

    //virtualservice
    ret=rtcfgUciGet("firewall.@redirect[0].name",redirectname);
    if(redirectname[0])
    {
        redirectenabled = 1;
    }
    
    //dmz
    struct fw3_dmz tmp_dmz={0};
    ret=get_dmz_rule(&tmp_dmz);
    if(-1 != ret)
    {
        dmzenabled = 1;
    }

    cJSON_AddItemToObject(obj, "ddos", cJSON_CreateNumber(ddosenabled));
    cJSON_AddItemToObject(obj, "telent", cJSON_CreateNumber(telentenabled));
    cJSON_AddItemToObject(obj, "ftp", cJSON_CreateNumber(ftpenabled));
    cJSON_AddItemToObject(obj, "samba", cJSON_CreateNumber(sambaenabled));
    cJSON_AddItemToObject(obj, "virtualservice", cJSON_CreateNumber(redirectenabled));
    cJSON_AddItemToObject(obj, "dmz", cJSON_CreateNumber(dmzenabled));
    cJSON_AddItemToObject(obj, "wififirewall", cJSON_CreateNumber(wififirewallenable));
    
    return;
}


int proc_onekeyswitch(cJSON *jsonValue, cJSON *jsonOut)
{
    ProcOneKeySwitchMsgReq();
    return 0;
}

int proc_get_onekeyswitch_status(cJSON *jsonValue, cJSON *jsonOut)
{
    int ret = 0;
    int status;
    cJSON* dataObj = cJSON_CreateObject();

    if (dataObj == NULL)
    {
        printf("Fail to create data object\n");
        return -1;
    }

    ret = QtGetOneKeySwitchStatus(&status);
    cJSON_AddItemToObject(dataObj, "status", cJSON_CreateNumber(status));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    return ret;
}


int proc_get_firewall_cfg(cJSON *jsonValue, cJSON *jsonOut)
{
    QT_FIREWALL_CFG_T firewallCfg = {0};
    cJSON* dataObj = cJSON_CreateObject();

    if (dataObj == NULL)
    {
        printf("Fail to create data object\n");
        return -1;
    }
    QtGetFirewallStatus(&firewallCfg);
    cJSON_AddItemToObject(dataObj, "url_firewall", cJSON_CreateNumber(firewallCfg.url));
    cJSON_AddItemToObject(dataObj, "family_firewall", cJSON_CreateNumber(firewallCfg.family));
    cJSON_AddItemToObject(dataObj, "password_firewall", cJSON_CreateNumber(firewallCfg.pwd));
    cJSON_AddItemToObject(dataObj, "dns_hijack_firewall", cJSON_CreateNumber(firewallCfg.hijack));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    return 0;
}

int proc_set_firewall_cfg(cJSON *jsonValue, cJSON *jsonOut)
{
    int ret = 0;
    QT_FIREWALL_CFG_T firewallCfg = {0};
    
    if (!cJSON_GetObjectItem(jsonValue, "url_firewall")
        || !cJSON_GetObjectItem(jsonValue, "family_firewall")
        || !cJSON_GetObjectItem(jsonValue, "password_firewall")
        || !cJSON_GetObjectItem(jsonValue, "dns_hijack_firewall"))

    {
        printf("Miss argument!\n");
        return -2;
    }

    firewallCfg.url = cJSON_GetObjectItem(jsonValue, "url_firewall")->valueint;
    firewallCfg.family = cJSON_GetObjectItem(jsonValue, "family_firewall")->valueint;
    firewallCfg.pwd = cJSON_GetObjectItem(jsonValue, "password_firewall")->valueint;
    firewallCfg.hijack = cJSON_GetObjectItem(jsonValue, "dns_hijack_firewall")->valueint;
    ret = QtSetFirewallStatus(&firewallCfg);
    ProcFirewallSetMsgReq();
    return ret;
}

int proc_get_hosts(cJSON *jsonValue, cJSON *jsonOut)
{
    QT_HOSTS_CFG hostsCfg[MAX_HOST_NUM] = {0};
    cJSON* dataObj = cJSON_CreateObject();
    cJSON* dataArray;
    int len = MAX_HOST_NUM;
    int i = 0;
    int ret;

    if (dataObj == NULL)
    {
        printf("Fail to create data object\n");
        return -1;
    }

    dataArray = cJSON_CreateArray();

    if (dataArray == NULL)
    {
        cJSON_Delete(dataObj);
        printf("Fail to create data array\n");
        return -1;
    }
    ret = QtGetHosts(hostsCfg, &len);
    for (i = 0; i < len; i++)
    {
        cJSON* itemObj = cJSON_CreateObject();
        cJSON_AddItemToObject(itemObj, "url", cJSON_CreateString(hostsCfg[i].url));
        cJSON_AddItemToObject(itemObj, "ip", cJSON_CreateString(hostsCfg[i].ip));
        cJSON_AddItemToArray(dataArray, itemObj);
    }
    cJSON_AddItemToObject(dataObj, "hosts", dataArray);
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    return ret;
}

int proc_set_hosts(cJSON *jsonValue, cJSON *jsonOut)
{
    int ret = 0;
    char status[128] = {0};
    QT_HOSTS_CFG hostsCfg = {0};
    cJSON* dataArray, *itemObj;
    int arrayLen;
    int i;
    
    if (!cJSON_GetObjectItem(jsonValue, "hosts"))

    {
        printf("Miss argument!\n");
        return -2;
    }
    dataArray = cJSON_GetObjectItem(jsonValue, "hosts");
    arrayLen = cJSON_GetArraySize(dataArray);
    QtEmptyHosts();
    for (i = 0; i < arrayLen; i++)
    {
        memset(&hostsCfg, 0, sizeof(hostsCfg));
        itemObj = cJSON_GetArrayItem(dataArray, i);
        UTIL_STRNCPY(hostsCfg.url, cJSON_GetObjectItem(itemObj, "url")->valuestring, sizeof(hostsCfg.url));
        UTIL_STRNCPY(hostsCfg.ip, cJSON_GetObjectItem(itemObj, "ip")->valuestring, sizeof(hostsCfg.ip));
        printf("ip:%s, url:%s\n", hostsCfg.ip, hostsCfg.url);
        ret = QtSetHosts(&hostsCfg);
    }

    
    ProcFirewallSetMsgReq();
    return 0;
}

int QtGetUciLock()
{
    if (access("/tmp/ucilock",F_OK) == 0)
    {
        printf("uci locked, access fail!\n");
        return -1;
    }
    system("touch /tmp/ucilock");
    return 0;
}

int QtReleaseUciLock()
{
    if (access("/tmp/ucilock",F_OK) != 0)
    {
        printf("uci locked already released\n");
        return 0;
    }
    system("rm /tmp/ucilock");
    return 0;
}