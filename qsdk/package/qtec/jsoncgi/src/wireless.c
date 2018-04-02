#include "basic.h"

static int ProcWifiSecurityCheck(char *key)
{
    int keyLen = 0;
    int findNumberFlag = 0;
    int findLetterFlag = 0;
    int i = 0;
    keyLen = strlen(key);
    for(i = 0; i < keyLen; i++)
    {
        if((key[i] > '0') && (key[i] < '9'))
        {
            findNumberFlag = 1;
			printf("find number.\n");
            break;
        }
    }

    for(i = 0; i < keyLen; i++)
    {
        if((key[i] > 'A') && (key[i] < 'Z'))
        {
            findLetterFlag = 1;
			printf("find letter.\n");
            break;
        }

        if((key[i] > 'a') && (key[i] < 'z'))
        {
            findLetterFlag = 1;
			printf("find letter.\n");
            break;
        }
    }

    if(findNumberFlag && findLetterFlag)
    {
        return 0;
    }
    
    return 1;
}

void proc_wifi_set(cJSON *jsonValue,cJSON *jsonOut)
{
	int ret=0;
	WifiConfig  stConfig = {0};
	int index=0;
	unsigned char bindata[2048]={0};
	char *result=NULL;
	char *password;
	char wifiKeyFirewall[64] = {0};
    int wifiFirewallFlag = 0;
    int len_debug;
    rtcfgUciGet("system.@system[0].wifikeyfirewall",wifiKeyFirewall);
    wifiFirewallFlag=atoi(wifiKeyFirewall);
	
	//DEBUG_PRINTF("proc_wifi_set:%s\n", cJSON_Print(jsonValue));
	
	//2.4g
	char *disable = cJSON_GetObjectItem(jsonValue, "disabled1")?cJSON_GetObjectItem(jsonValue, "disabled1")->valuestring:"";
	
	strcpy(stConfig.Disabled1, disable);
	strcpy(stConfig.Ssid1, (cJSON_GetObjectItem(jsonValue, "ssid1")?cJSON_GetObjectItem(jsonValue, "ssid1")->valuestring:""));
	password =cJSON_GetObjectItem(jsonValue, "key1")?cJSON_GetObjectItem(jsonValue, "key1")->valuestring:"";
    if (strlen(password) != 0)
    {
    	len_debug=base64_decode(password, bindata);
    	result=(char *)my_decrypt(len_debug,bindata, RSA_PRIVATE_KEY_FILE); 
    	if(result == NULL)
    	{
    		global_weberrorcode=ERR_DECRY_FAIL;
    		return;
    	}
    	strcpy(stConfig.Key1, result);
        if(result !=NULL)
        {
            free(result);
            result=NULL;
        }
    }
	char *hidden = cJSON_GetObjectItem(jsonValue, "hidden1")?cJSON_GetObjectItem(jsonValue, "hidden1")->valuestring:"";
	
	strcpy(stConfig.Hidden1, hidden);
	strcpy(stConfig.Encryption1, (cJSON_GetObjectItem(jsonValue, "encryption1")?cJSON_GetObjectItem(jsonValue, "encryption1")->valuestring:""));
	strcpy(stConfig.Bandwith1, (cJSON_GetObjectItem(jsonValue, "bandwith1")?cJSON_GetObjectItem(jsonValue, "bandwith1")->valuestring:""));
	strcpy(stConfig.Wifimode1, (cJSON_GetObjectItem(jsonValue, "wifimode1")?cJSON_GetObjectItem(jsonValue, "wifimode1")->valuestring:""));
	strcpy(stConfig.Channel1, (cJSON_GetObjectItem(jsonValue, "channel1")?cJSON_GetObjectItem(jsonValue, "channel1")->valuestring:""));
	
	//5g
	disable = cJSON_GetObjectItem(jsonValue, "disabled2")?cJSON_GetObjectItem(jsonValue, "disabled2")->valuestring:"";
	
	strcpy(stConfig.Disabled2, disable);
	strcpy(stConfig.Ssid2, (cJSON_GetObjectItem(jsonValue, "ssid2")?cJSON_GetObjectItem(jsonValue, "ssid2")->valuestring:""));
	password =cJSON_GetObjectItem(jsonValue, "key2")?cJSON_GetObjectItem(jsonValue, "key2")->valuestring:"";
    if (strlen(password) != 0)
    {
    	memset(bindata, 0, 2048);
    	len_debug=base64_decode(password, bindata);    
    	result=(char *)my_decrypt(len_debug,bindata, RSA_PRIVATE_KEY_FILE); 
    	if(result == NULL)
    	{
    		global_weberrorcode=ERR_DECRY_FAIL;
    		return;
    	}
    	strcpy(stConfig.Key2, result);
        if(result!=NULL)
        {
            free(result);
        }
    }
    
	hidden = cJSON_GetObjectItem(jsonValue, "hidden2")?cJSON_GetObjectItem(jsonValue, "hidden2")->valuestring:"";

	strcpy(stConfig.Hidden2, hidden);
	strcpy(stConfig.Encryption2, (cJSON_GetObjectItem(jsonValue, "encryption2")?cJSON_GetObjectItem(jsonValue, "encryption2")->valuestring:""));
	strcpy(stConfig.Bandwith2, (cJSON_GetObjectItem(jsonValue, "bandwith2")?cJSON_GetObjectItem(jsonValue, "bandwith2")->valuestring:""));
	strcpy(stConfig.Wifimode2, (cJSON_GetObjectItem(jsonValue, "wifimode2")?cJSON_GetObjectItem(jsonValue, "wifimode2")->valuestring:""));
	strcpy(stConfig.Channel2, (cJSON_GetObjectItem(jsonValue, "channel2")?cJSON_GetObjectItem(jsonValue, "channel2")->valuestring:""));
		
	if(wifiFirewallFlag)
    {
        ret +=  ProcWifiSecurityCheck(stConfig.Key1);
        ret +=  ProcWifiSecurityCheck(stConfig.Key2);
    }
    if(ret)
    {
        printf("wifi key is too simple!");
        global_weberrorcode=ERR_PASSWORD_TOO_SIMPLE;
        return;
    }

	ret = setWifiConfig(&stConfig);
	if(ret !=0)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;      
	}
	else
	{
		ProcWifiMsgReq();
	}
}

void proc_wifi_get(cJSON *jsonValue,cJSON *jsonOut)
{
	cJSON *array = NULL;
	cJSON *obj = NULL;
	int ret=0;
	int index=0;
	int tablenum=0;
	WifiConfig  config;
	unsigned char bindata[2048]={0};    
	
	memset(&config, 0, sizeof(config));
	getWifiConfig(&config);
	
	cJSON_AddItemToObject(jsonOut,"data", obj=cJSON_CreateObject());
    if((0 == strcmp(config.Device1,"wifi0")) && (0 == strcmp(config.Network1,"lan")))
    {
    	cJSON_AddStringToObject(obj,"disabled1",config.Disabled1[0] != 0?config.Disabled1:"0");
    	cJSON_AddStringToObject(obj,"ssid1",config.Ssid1);
    	//base64_encode(config.Key1, bindata, strlen(config.Key1));
    	//cJSON_AddStringToObject(obj,"key1",bindata);
    	cJSON_AddStringToObject(obj,"hidden1",config.Hidden1[0]!= 0?config.Hidden1:"0");
    	cJSON_AddStringToObject(obj,"encryption1",config.Encryption1);
    	cJSON_AddStringToObject(obj,"bandwith1",config.Bandwith1[0]!= 0?config.Bandwith1:"1");
    	cJSON_AddStringToObject(obj,"wifimode1",config.Wifimode1[0]!= 0?config.Wifimode1:"9");
    	cJSON_AddStringToObject(obj,"channel1",config.Channel1[0]!= 0?config.Channel1:"0");
    }

    if((0 == strcmp(config.Device2,"wifi1")) && (0 == strcmp(config.Network2,"lan")))
    {
    	cJSON_AddStringToObject(obj,"disabled2",config.Disabled2[0] != 0?config.Disabled2:"0");
    	cJSON_AddStringToObject(obj,"ssid2",config.Ssid2);
    	//memset(bindata, 0 ,2048);
    	//base64_encode(config.Key2, bindata, strlen(config.Key2));
    	//cJSON_AddStringToObject(obj,"key2",bindata);
    	cJSON_AddStringToObject(obj,"hidden2",config.Hidden2[0]!= 0?config.Hidden2:"0");
    	cJSON_AddStringToObject(obj,"encryption2",config.Encryption2);
    	cJSON_AddStringToObject(obj,"bandwith2",config.Bandwith2[0]!= 0?config.Bandwith2:"2");
    	cJSON_AddStringToObject(obj,"wifimode2",config.Wifimode2[0]!= 0?config.Wifimode2:"14");
    	cJSON_AddStringToObject(obj,"channel2",config.Channel2[0]!= 0?config.Channel2:"0");
    }
}

void proc_wireless_set(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("===[%s]====\n",__func__);
    int ret=0;
    WifiDevice stWifiDevice = {0};
	WifiIface  stWifiIface = {0};
    int index=0;
    index = cJSON_GetObjectItem(jsonValue, "index")?cJSON_GetObjectItem(jsonValue, "index")->valueint:0;
	strcpy(stWifiDevice.WifiDevice, (cJSON_GetObjectItem(jsonValue, "wifi-device")?cJSON_GetObjectItem(jsonValue, "wifi-device")->valuestring:""));
	strcpy(stWifiDevice.Type, (cJSON_GetObjectItem(jsonValue, "type")?cJSON_GetObjectItem(jsonValue, "type")->valuestring:""));
	strcpy(stWifiDevice.Country, (cJSON_GetObjectItem(jsonValue, "country")?cJSON_GetObjectItem(jsonValue, "country")->valuestring:""));
	strcpy(stWifiDevice.Channel, (cJSON_GetObjectItem(jsonValue, "channel")?cJSON_GetObjectItem(jsonValue, "channel")->valuestring:""));
	strcpy(stWifiDevice.Disabled, (cJSON_GetObjectItem(jsonValue, "disabled")?cJSON_GetObjectItem(jsonValue, "disabled")->valuestring:""));
	strcpy(stWifiIface.Device, (cJSON_GetObjectItem(jsonValue, "device")?cJSON_GetObjectItem(jsonValue, "device")->valuestring:""));
	strcpy(stWifiIface.Network, (cJSON_GetObjectItem(jsonValue, "network")?cJSON_GetObjectItem(jsonValue, "network")->valuestring:""));
	strcpy(stWifiIface.Mode, (cJSON_GetObjectItem(jsonValue, "mode")?cJSON_GetObjectItem(jsonValue, "mode")->valuestring:""));
	strcpy(stWifiIface.Ssid, (cJSON_GetObjectItem(jsonValue, "ssid")?cJSON_GetObjectItem(jsonValue, "ssid")->valuestring:""));
	strcpy(stWifiIface.Encryption, (cJSON_GetObjectItem(jsonValue, "encryption")?cJSON_GetObjectItem(jsonValue, "encryption")->valuestring:""));
    char *password=NULL;
    password =cJSON_GetObjectItem(jsonValue, "key")?cJSON_GetObjectItem(jsonValue, "key")->valuestring:"";

    DEBUG_PRINTF("===[%s]===str: %s ===\n",__func__,password);
    unsigned char bindata[2048]={0};
    int len_debug=base64_decode(password, bindata);
    
    char *result=NULL;
    result=(char *)my_decrypt(len_debug,bindata, RSA_PRIVATE_KEY_FILE); 
    if(result == NULL)
    {
        global_weberrorcode=ERR_DECRY_FAIL;
        return;
    }
    strcpy(stWifiIface.Key, result);
    if(result !=NULL)
    {
        free(result);
        result = NULL;
    }
    //strcpy(stWifiIface.Key, (cJSON_GetObjectItem(jsonValue, "key")?cJSON_GetObjectItem(jsonValue, "key")->valuestring:""));
	strcpy(stWifiIface.Wds, (cJSON_GetObjectItem(jsonValue, "wds")?cJSON_GetObjectItem(jsonValue, "wds")->valuestring:""));
	strcpy(stWifiIface.Ifname, (cJSON_GetObjectItem(jsonValue, "ifname")?cJSON_GetObjectItem(jsonValue, "ifname")->valuestring:""));
	strcpy(stWifiIface.Hidden, (cJSON_GetObjectItem(jsonValue, "hidden")?cJSON_GetObjectItem(jsonValue, "hidden")->valuestring:""));
	ret=WifiUciEdit(&stWifiDevice, &stWifiIface, index);

    if(ret !=0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;      
    }
   
}


void proc_wireless_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("===%s==%d=\n",__func__,__LINE__);
	cJSON *array = NULL;
	cJSON *obj = NULL;
    int ret=0;
    int index=0;
    int tablenum=0;
    WifiIface  *pstWifiIface = NULL;
    pstWifiIface = WifiInfoGet(&tablenum);

	cJSON_AddItemToObject(jsonOut,"list",array=cJSON_CreateArray());
    printf("===%s==%d=\n",__func__,__LINE__);fflush(stdout);
	for(index =0; index < tablenum; index++)
	{
        printf("===%s==%d=\n",__func__,__LINE__);fflush(stdout);
	    cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
		cJSON_AddItemToObject(obj,"mode",cJSON_CreateString(pstWifiIface[index].Mode));
		cJSON_AddItemToObject(obj,"ssid",cJSON_CreateString(pstWifiIface[index].Ssid));
		cJSON_AddItemToObject(obj,"encryption",cJSON_CreateString(pstWifiIface[index].Encryption));
		cJSON_AddItemToObject(obj,"key",cJSON_CreateString(pstWifiIface[index].Key));
        printf("===%s==%d=\n",__func__,__LINE__);fflush(stdout);
		
	}
	if(pstWifiIface)
        free(pstWifiIface);
    
    cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
}

void proc_wireless(cJSON *json_value)
{
	WifiDevice stWifiDevice = {0};
	WifiIface  stWifiIface = {0};
	cJSON *array = NULL;
	cJSON *obj = NULL;
	WifiIface  *pstWifiIface = NULL;
	char device[64] = {0};
	int index = 0;
	int tablenum = 0; 	
	int method;
	switch (method)
	{
		case WIFIADD:
			strcpy(stWifiDevice.WifiDevice, (cJSON_GetObjectItem(json_value, "wifi-device")?cJSON_GetObjectItem(json_value, "wifi-device")->valuestring:""));
			strcpy(stWifiDevice.Type, (cJSON_GetObjectItem(json_value, "type")?cJSON_GetObjectItem(json_value, "type")->valuestring:""));
			strcpy(stWifiDevice.Country, (cJSON_GetObjectItem(json_value, "country")?cJSON_GetObjectItem(json_value, "country")->valuestring:""));
			strcpy(stWifiDevice.Channel, (cJSON_GetObjectItem(json_value, "channel")?cJSON_GetObjectItem(json_value, "channel")->valuestring:""));
			strcpy(stWifiDevice.Disabled, (cJSON_GetObjectItem(json_value, "disabled")?cJSON_GetObjectItem(json_value, "disabled")->valuestring:""));
			strcpy(stWifiIface.Device, (cJSON_GetObjectItem(json_value, "device")?cJSON_GetObjectItem(json_value, "device")->valuestring:""));
			strcpy(stWifiIface.Network, (cJSON_GetObjectItem(json_value, "network")?cJSON_GetObjectItem(json_value, "network")->valuestring:""));
			strcpy(stWifiIface.Mode, (cJSON_GetObjectItem(json_value, "mode")?cJSON_GetObjectItem(json_value, "mode")->valuestring:""));
			strcpy(stWifiIface.Ssid, (cJSON_GetObjectItem(json_value, "ssid")?cJSON_GetObjectItem(json_value, "ssid")->valuestring:""));
			strcpy(stWifiIface.Encryption, (cJSON_GetObjectItem(json_value, "encryption")?cJSON_GetObjectItem(json_value, "encryption")->valuestring:""));
			strcpy(stWifiIface.Key, (cJSON_GetObjectItem(json_value, "key")?cJSON_GetObjectItem(json_value, "key")->valuestring:""));
			strcpy(stWifiIface.Wds, (cJSON_GetObjectItem(json_value, "wds")?cJSON_GetObjectItem(json_value, "wds")->valuestring:""));
			strcpy(stWifiIface.Ifname, (cJSON_GetObjectItem(json_value, "ifname")?cJSON_GetObjectItem(json_value, "ifname")->valuestring:""));
			strcpy(stWifiIface.Hidden, (cJSON_GetObjectItem(json_value, "hidden")?cJSON_GetObjectItem(json_value, "hidden")->valuestring:""));
			(void)WifiUciAdd(&stWifiDevice, &stWifiIface);
			break;

		case WIFIDEL:
			strcpy(device, (cJSON_GetObjectItem(json_value, "wifi-device")?cJSON_GetObjectItem(json_value, "wifi-device")->valuestring:""));
			index = cJSON_GetObjectItem(json_value, "delindex")?cJSON_GetObjectItem(json_value, "delindex")->valueint:0xffff;
			(void)WifiUciDel(device, index);
			break;

		case WIFIEDIT:
			index = cJSON_GetObjectItem(json_value, "index")?cJSON_GetObjectItem(json_value, "index")->valueint:0;
			strcpy(stWifiDevice.WifiDevice, (cJSON_GetObjectItem(json_value, "wifi-device")?cJSON_GetObjectItem(json_value, "wifi-device")->valuestring:""));
			strcpy(stWifiDevice.Type, (cJSON_GetObjectItem(json_value, "type")?cJSON_GetObjectItem(json_value, "type")->valuestring:""));
			strcpy(stWifiDevice.Country, (cJSON_GetObjectItem(json_value, "country")?cJSON_GetObjectItem(json_value, "country")->valuestring:""));
			strcpy(stWifiDevice.Channel, (cJSON_GetObjectItem(json_value, "channel")?cJSON_GetObjectItem(json_value, "channel")->valuestring:""));
			strcpy(stWifiDevice.Disabled, (cJSON_GetObjectItem(json_value, "disabled")?cJSON_GetObjectItem(json_value, "disabled")->valuestring:""));
			strcpy(stWifiIface.Device, (cJSON_GetObjectItem(json_value, "device")?cJSON_GetObjectItem(json_value, "device")->valuestring:""));
			strcpy(stWifiIface.Network, (cJSON_GetObjectItem(json_value, "network")?cJSON_GetObjectItem(json_value, "network")->valuestring:""));
			strcpy(stWifiIface.Mode, (cJSON_GetObjectItem(json_value, "mode")?cJSON_GetObjectItem(json_value, "mode")->valuestring:""));
			strcpy(stWifiIface.Ssid, (cJSON_GetObjectItem(json_value, "ssid")?cJSON_GetObjectItem(json_value, "ssid")->valuestring:""));
			strcpy(stWifiIface.Encryption, (cJSON_GetObjectItem(json_value, "encryption")?cJSON_GetObjectItem(json_value, "encryption")->valuestring:""));
			strcpy(stWifiIface.Key, (cJSON_GetObjectItem(json_value, "key")?cJSON_GetObjectItem(json_value, "key")->valuestring:""));
			strcpy(stWifiIface.Wds, (cJSON_GetObjectItem(json_value, "wds")?cJSON_GetObjectItem(json_value, "wds")->valuestring:""));
			strcpy(stWifiIface.Ifname, (cJSON_GetObjectItem(json_value, "ifname")?cJSON_GetObjectItem(json_value, "ifname")->valuestring:""));
			strcpy(stWifiIface.Hidden, (cJSON_GetObjectItem(json_value, "hidden")?cJSON_GetObjectItem(json_value, "hidden")->valuestring:""));
			(void)WifiUciEdit(&stWifiDevice, &stWifiIface, index);
			break;

		case WIFIGET:
			pstWifiIface = WifiInfoGet(&tablenum);
			if(NULL != pstWifiIface)
			{
				cJSON_AddItemToObject(json_value,"list",array=cJSON_CreateArray());
				for(index =0; index < tablenum; index++)
				{
					cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
					cJSON_AddItemToObject(obj,"mode",cJSON_CreateString(pstWifiIface->Mode));
					cJSON_AddItemToObject(obj,"ssid",cJSON_CreateString(pstWifiIface->Ssid));
					cJSON_AddItemToObject(obj,"encryption",cJSON_CreateString(pstWifiIface->Encryption));
					cJSON_AddItemToObject(obj,"key",cJSON_CreateString(pstWifiIface->Key));
					pstWifiIface++;
				}
			}
			free(pstWifiIface);
			pstWifiIface = NULL;
			break;

		default:
			break;
	
	}
	
	return ;	
}

int ProcWifiSetByApp(cJSON *jsonValue,cJSON *jsonOut)
{
	int ret=0;
	WifiConfig  stConfig = {0};
	char wifiKeyFirewall[64] = {0};
    int wifiFirewallFlag = 0;

    rtcfgUciGet("system.@system[0].wifikeyfirewall",wifiKeyFirewall);
    wifiFirewallFlag=atoi(wifiKeyFirewall);
	
	strcpy(stConfig.Encryption1, "psk-mixed");
    strcpy(stConfig.Encryption2, "psk-mixed");
	//2.4g
	strcpy(stConfig.Disabled1, (cJSON_GetObjectItem(jsonValue, "lfdisabled")?cJSON_GetObjectItem(jsonValue, "lfdisabled")->valuestring:""));
	strcpy(stConfig.Ssid1, (cJSON_GetObjectItem(jsonValue, "lfssid")?cJSON_GetObjectItem(jsonValue, "lfssid")->valuestring:""));
	strcpy(stConfig.Key1, (cJSON_GetObjectItem(jsonValue, "lfkey")?cJSON_GetObjectItem(jsonValue, "lfkey")->valuestring:""));
	strcpy(stConfig.Hidden1, (cJSON_GetObjectItem(jsonValue, "lfhiden")?cJSON_GetObjectItem(jsonValue, "lfhiden")->valuestring:""));
	
	//5g
	strcpy(stConfig.Disabled2, (cJSON_GetObjectItem(jsonValue, "hfdisabled")?cJSON_GetObjectItem(jsonValue, "hfdisabled")->valuestring:""));
	strcpy(stConfig.Ssid2, (cJSON_GetObjectItem(jsonValue, "hfssid")?cJSON_GetObjectItem(jsonValue, "hfssid")->valuestring:""));
	strcpy(stConfig.Key2, (cJSON_GetObjectItem(jsonValue, "hfkey")?cJSON_GetObjectItem(jsonValue, "hfkey")->valuestring:""));
	strcpy(stConfig.Hidden2, (cJSON_GetObjectItem(jsonValue, "hfhiden")?cJSON_GetObjectItem(jsonValue, "hfhiden")->valuestring:""));

	if(wifiFirewallFlag)
    {
        ret +=  ProcWifiSecurityCheck(stConfig.Key1);
        ret +=  ProcWifiSecurityCheck(stConfig.Key2);
    }
    if(ret)
    {
        printf("wifi key is too simple!");
        return ERR_PASSWORD_TOO_SIMPLE;
    }

	ret = setWifiConfig(&stConfig);
	ProcWifiMsgReq();
	return ret;
}

void ProcWifiGetByApp(cJSON *jsonValue,cJSON *jsonOut)
{
	cJSON *array = NULL;
	cJSON *obj = NULL;
	int ret=0;
	WifiConfig  config= {0}; 
	
	getWifiConfig(&config);
	
	cJSON_AddItemToObject(jsonOut,"data", obj=cJSON_CreateObject());
    if((0 == strcmp(config.Device1,"wifi0")) && (0 == strcmp(config.Network1,"lan")))
    {   
    	cJSON_AddStringToObject(obj,"lfssid",config.Ssid1);
    	//cJSON_AddStringToObject(obj,"lfkey",config.Key1);
    	cJSON_AddStringToObject(obj,"lfdisabled",(config.Disabled1[0] != 0)?config.Disabled1:"0");
    	cJSON_AddStringToObject(obj,"lfhiden",(config.Hidden1[0] != 0)?config.Hidden1:"0");
    }
    
    if((0 == strcmp(config.Device2,"wifi1")) && (0 == strcmp(config.Network2,"lan")))
    {
    	cJSON_AddStringToObject(obj,"hfssid",config.Ssid2);
    	//cJSON_AddStringToObject(obj,"hfkey",config.Key2);
    	cJSON_AddStringToObject(obj,"hfdisabled",(config.Disabled2[0] != 0)?config.Disabled2:"0");
    	cJSON_AddStringToObject(obj,"hfhiden",(config.Hidden2[0] != 0)?config.Hidden2:"0");
    }
}

int procWifiSetTxpower(cJSON *jsonValue,cJSON *jsonOut)
{
    int ret=0;
    int mode;

    mode = cJSON_GetObjectItem(jsonValue, "mode")?cJSON_GetObjectItem(jsonValue, "mode")->valueint:0;

    ret = QtSetWifTxpower(mode);

    if(ret)
	{
		return ERR_INTERNALLOGIC_WRONG;
	}
    else
	{
		return 0;
	}
}

int procWifiGetTxpower(cJSON *jsonValue,cJSON *jsonOut)
{
    int ret=0;
    int mode;
    cJSON *dataObj = NULL;

    dataObj = cJSON_CreateObject();
    if (dataObj == NULL)
    {
        printf("Fail to create data json obj!\n");
        return;	
    }

    ret = QtGetWifiMode(&mode);
    cJSON_AddItemToObject(dataObj, "mode", cJSON_CreateNumber(mode));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    if(ret)
	{
		return ERR_INTERNALLOGIC_WRONG;
	}
    else
	{
		return 0;
	}
}

int procSetGuestWifi(cJSON *jsonValue,cJSON *jsonOut)
{
    int ret=0;
    QT_GUEST_WIFI_CFG guestCfg = {0};
    char * tmp;
    
    guestCfg.enable = cJSON_GetObjectItem(jsonValue, "enable")?cJSON_GetObjectItem(jsonValue, "enable")->valueint:0;
    guestCfg.isHide = cJSON_GetObjectItem(jsonValue, "isHide")?cJSON_GetObjectItem(jsonValue, "isHide")->valueint:0;
    tmp = cJSON_GetObjectItem(jsonValue, "name")?cJSON_GetObjectItem(jsonValue, "name")->valuestring:"";
    UTIL_STRNCPY(guestCfg.name, tmp, MAX_SSID_LEN);
    
    ret = QtSetGuestWifi(&guestCfg);
    if(ret !=0)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;      
	}
	else
	{
		ProcSetGuestWifiMsgReq();
	}

    return ret;
}

int procGetGuestWifi(cJSON *jsonValue,cJSON *jsonOut)
{
    int ret=0;
    QT_GUEST_WIFI_CFG guestCfg = {0};
    cJSON *dataObj = NULL;

    dataObj = cJSON_CreateObject();
    if (dataObj == NULL)
    {
        printf("Fail to create data json obj!\n");
        return;	
    }

    ret = QtGetGuestWifi(&guestCfg);
    cJSON_AddItemToObject(dataObj, "enable", cJSON_CreateNumber(guestCfg.enable));
    cJSON_AddItemToObject(dataObj, "isHide", cJSON_CreateNumber(guestCfg.isHide));
    cJSON_AddItemToObject(dataObj, "name", cJSON_CreateString(guestCfg.name));
    cJSON_AddItemToObject(dataObj, "userNum", cJSON_CreateNumber(guestCfg.guestUserNum));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    return ret;
}

int proc_wdscfg_get(cJSON *jsonValue,cJSON *jsonOut)
{
    int ret;
    QT_WDS_BASIC_CFG wdsBasicCfg = {0};
    cJSON* dataObj;
    if ((dataObj = cJSON_CreateObject()) == NULL)
    {
        printf("Fail to create daba obj\n");
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    ret = QtWdsGetBasicCfg(&wdsBasicCfg);
    if (ret != 0)
    {
        printf("Fail to get wds basic cofnig\n");
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    
    cJSON_AddItemToObject(dataObj, "enable", cJSON_CreateNumber(wdsBasicCfg.enable));
    cJSON_AddItemToObject(dataObj, "status", cJSON_CreateNumber(wdsBasicCfg.status));
    cJSON_AddItemToObject(dataObj, "ssid", cJSON_CreateString(wdsBasicCfg.ssid));
    cJSON_AddItemToObject(dataObj, "mac", cJSON_CreateString(wdsBasicCfg.mac));
    cJSON_AddItemToObject(dataObj, "isSameNet", cJSON_CreateNumber(wdsBasicCfg.isChangeLanIp));
    cJSON_AddItemToObject(dataObj, "suggestLanIp", cJSON_CreateString(wdsBasicCfg.suggestLanIp));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    return ret;
}

int proc_wds_scan(cJSON *jsonValue,cJSON *jsonOut)
{
    QT_WDS_WIFI_INFO wdsArray[128] = {0};
    int ret;
    cJSON* dataObj, *wifiArray, *itemObj;
    int len = sizeof(wdsArray)/sizeof(QT_WDS_WIFI_INFO);
    int i = 0;
    if ((dataObj = cJSON_CreateObject()) == NULL)
    {
        printf("Fail to create daba obj\n");
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }

    if ((wifiArray = cJSON_CreateArray()) == NULL)
    {
        printf("Fail to create wifiArray obj\n");
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    
    ret = QtWdsScanWifi(wdsArray, &len);
    if (ret != 0)
    {
        printf("fail to scan wds wifi\n");
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    printf("@@@@@@@@@@@webcgi@@@@@@@@@@@@\n");
    for (i = 0; i < len; i++)
    {
        printf("ssid:%s\n", wdsArray[i].ssid);
        printf("mac:%s\n", wdsArray[i].mac);
        printf("channel:%d\n", wdsArray[i].channel);
        printf("power:%d\n", wdsArray[i].power);
        printf("encryption:%s\n", wdsArray[i].encryption);
        printf("\n");
    }

    for (i = 0; i < len; i++)
    {
        if ((itemObj = cJSON_CreateObject()) == NULL)
        {
            printf("Fail to create item obj\n");
            global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        	return ERR_INTERNALLOGIC_WRONG;
        }
        cJSON_AddItemToObject(itemObj, "ssid", cJSON_CreateString(wdsArray[i].ssid));
        cJSON_AddItemToObject(itemObj, "mac", cJSON_CreateString(wdsArray[i].mac));
        cJSON_AddItemToObject(itemObj, "channel", cJSON_CreateNumber(wdsArray[i].channel));
        cJSON_AddItemToObject(itemObj, "power", cJSON_CreateNumber(wdsArray[i].power));
        cJSON_AddItemToObject(itemObj, "encrypt", cJSON_CreateString(wdsArray[i].encryption));
        cJSON_AddItemToObject(itemObj, "mode", cJSON_CreateNumber(wdsArray[i].mode));
        cJSON_AddItemToArray(wifiArray, itemObj);
    }

    cJSON_AddItemToObject(dataObj, "wifi", wifiArray);
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    return ret;
    
}

int proc_wdscfg_set(cJSON *jsonValue,cJSON *jsonOut)
{
    int ret;
    QT_WDS_BASIC_CFG wdsBasicCfg = {0};
    QT_WDS_BASIC_CFG oldWdsCfg = {0};

    wdsBasicCfg.enable = cJSON_GetObjectItem(jsonValue, "enable")?cJSON_GetObjectItem(jsonValue, "enable")->valueint:0;
    ret = QtWdsGetBasicCfg(&oldWdsCfg);
    if (ret != 0)
    {
        printf("fail to set wds basic cfg\n");
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    ret = QtWdsSetBasicCfg(&wdsBasicCfg);
    if (ret != 0)
    {
        printf("fail to set wds basic cfg\n");
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    if (!wdsBasicCfg.enable && oldWdsCfg.status)
    {printf("----------restart wifi------\n");
        ProcWifiMsgReq();
    }
    return 0;
}

int proc_wds_setup(cJSON *jsonValue,cJSON *jsonOut)
{
    int ret;
    QT_WDS_WIFI_INFO wdsInfo = {0};
    cJSON* dataObj;

    if (!cJSON_GetObjectItem(jsonValue, "ssid")
        || !cJSON_GetObjectItem(jsonValue, "mac")
        || !cJSON_GetObjectItem(jsonValue, "password")
        || !cJSON_GetObjectItem(jsonValue, "mode")
        || !cJSON_GetObjectItem(jsonValue, "encrypt"))
    {
       printf("miss argument\n"); 
       global_weberrorcode = ERR_PARAMETER_MISS;
       return ERR_PARAMETER_MISS;
    }

    UTIL_STRNCPY(wdsInfo.ssid, cJSON_GetObjectItem(jsonValue, "ssid")->valuestring, sizeof(wdsInfo.ssid));
    UTIL_STRNCPY(wdsInfo.mac, cJSON_GetObjectItem(jsonValue, "mac")->valuestring, sizeof(wdsInfo.mac));
    UTIL_STRNCPY(wdsInfo.password, cJSON_GetObjectItem(jsonValue, "password")->valuestring, sizeof(wdsInfo.password));
    UTIL_STRNCPY(wdsInfo.encryption, cJSON_GetObjectItem(jsonValue, "encrypt")->valuestring, sizeof(wdsInfo.encryption));
    wdsInfo.mode =  cJSON_GetObjectItem(jsonValue, "mode")->valueint;
    ret = QtWdsSetUp(&wdsInfo);

    if (ret != 0)
    {
        printf("Fail to set up wds\n");
        if (ret == -2)
        {
            global_weberrorcode =  ERR_WIFI_PASSWORD_TOO_SHORT;
            ret = ERR_WIFI_PASSWORD_TOO_SHORT;
        }
        return ret;
    }

    ProcWdsSetReq();
    return 0;
}

int proc_wds_status_get(cJSON *jsonValue,cJSON *jsonOut)
{
    int ret;
    QT_WDS_WIFI_NET_INFO wdsNetCfg = {0};
    cJSON* dataObj;
    
    if ((dataObj = cJSON_CreateObject()) == NULL)
    {
        printf("Fail to create daba obj\n");
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }

    ret = QtWdsGetStatus(&wdsNetCfg);
    if (ret != 0)
    {
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        printf("Fail to get wds status\n");
        return ERR_INTERNALLOGIC_WRONG;
    }

    cJSON_AddItemToObject(dataObj, "ipaddr", cJSON_CreateString(wdsNetCfg.ipaddr));
    cJSON_AddItemToObject(dataObj, "netmask", cJSON_CreateString(wdsNetCfg.netmask));
    cJSON_AddItemToObject(dataObj, "dns", cJSON_CreateString(wdsNetCfg.dns));
    cJSON_AddItemToObject(dataObj, "gateway", cJSON_CreateString(wdsNetCfg.gateway));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    return 0;
}

void ProcWifiFirewallSet(cJSON *jsonValue,cJSON *jsonOut)
{
    int enable = 0;
    char cmd[256]={0};
    enable = cJSON_GetObjectItem(jsonValue, "enable")?cJSON_GetObjectItem(jsonValue, "enable")->valueint:0;

    snprintf(cmd,256,"system.@system[0].wifikeyfirewall=%d",enable);
    rtcfgUciSet(cmd);
    return;
}

void ProcWifiFirewallGet(cJSON *jsonValue,cJSON *jsonOut)
{
    int enable = 0;
    char cmd[256]={0};
    char wifiKeyFirewall[64] = {0};
    int wifiFirewallFlag = 0;
    cJSON *obj = NULL;
    
    rtcfgUciGet("system.@system[0].wifikeyfirewall",wifiKeyFirewall);
    wifiFirewallFlag=atoi(wifiKeyFirewall);

    cJSON_AddItemToObject(jsonOut,"data", obj=cJSON_CreateObject());
    cJSON_AddItemToObject(obj,"enbale",cJSON_CreateNumber(wifiFirewallFlag));
    return;
}
