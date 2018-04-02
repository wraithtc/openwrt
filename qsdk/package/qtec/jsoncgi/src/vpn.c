#include "basic.h"

void vpn_set_edit_flag()
{
    system("touch /tmp/vpn_edit_flag");
}

int vpn_get_edit_flag()
{
    return (access("/tmp/vpn_edit_flag", F_OK) == 0)?1:0;
}

int proc_add_vpn(cJSON *jsonValue,cJSON *jsonOut)
{
    cJSON *dataObj;
    int ret ;
    char *tmpstr;
    QT_VPN_CFG vpncfg = {0};
    QT_VPN_CFG vpncfgArray[8] = {0};
    int len = sizeof(vpncfgArray)/sizeof(QT_VPN_CFG);
    
    tmpstr = cJSON_GetObjectItem(jsonValue, "description")?cJSON_GetObjectItem(jsonValue, "description")->valuestring:"";
    UTIL_STRNCPY(vpncfg.description, tmpstr, sizeof(vpncfg.description));
    tmpstr = cJSON_GetObjectItem(jsonValue, "mode")?cJSON_GetObjectItem(jsonValue, "mode")->valuestring:"";
    util_strToLower(tmpstr);
    UTIL_STRNCPY(vpncfg.proto, tmpstr, sizeof(vpncfg.proto));
    tmpstr = cJSON_GetObjectItem(jsonValue, "server_ip")?cJSON_GetObjectItem(jsonValue, "server_ip")->valuestring:"";
    UTIL_STRNCPY(vpncfg.serverip, tmpstr, sizeof(vpncfg.serverip));
    tmpstr = cJSON_GetObjectItem(jsonValue, "username")?cJSON_GetObjectItem(jsonValue, "username")->valuestring:"";
    UTIL_STRNCPY(vpncfg.username, tmpstr, sizeof(vpncfg.username));
    tmpstr = cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"";
    UTIL_STRNCPY(vpncfg.password, tmpstr, sizeof(vpncfg.password));
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    ret = QtGetVpn(vpncfgArray, &len);

    if (len >= 8)
    {
        printf("Too many vpn rules\n");
        global_weberrorcode = ERR_TOO_MANY_RULES;
        QtReleaseUciLock();
        return ERR_TOO_MANY_RULES;
    }
    
    ret = QtAddVpn(&vpncfg);
    if (ret != 0)
    {
        printf("Fail to add vpn, ret = %d\n", ret);
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        QtReleaseUciLock();
        return ERR_INTERNALLOGIC_WRONG;
    }
    QtReleaseUciLock();
    return ret;
}

int proc_add_vpn_by_web(cJSON *jsonValue,cJSON *jsonOut)
{
    cJSON *dataObj;
    int ret ;
    char *tmpstr;
    QT_VPN_CFG vpncfg = {0};
    QT_VPN_CFG vpncfgArray[8] = {0};
    int len = sizeof(vpncfgArray)/sizeof(QT_VPN_CFG);
    unsigned char bindata[2048]={0};
    int len_debug;
    char *result=NULL;

    tmpstr = cJSON_GetObjectItem(jsonValue, "description")?cJSON_GetObjectItem(jsonValue, "description")->valuestring:"";
    UTIL_STRNCPY(vpncfg.description, tmpstr, sizeof(vpncfg.description));
    tmpstr = cJSON_GetObjectItem(jsonValue, "mode")?cJSON_GetObjectItem(jsonValue, "mode")->valuestring:"";
    util_strToLower(tmpstr);
    UTIL_STRNCPY(vpncfg.proto, tmpstr, sizeof(vpncfg.proto));
    tmpstr = cJSON_GetObjectItem(jsonValue, "server_ip")?cJSON_GetObjectItem(jsonValue, "server_ip")->valuestring:"";
    UTIL_STRNCPY(vpncfg.serverip, tmpstr, sizeof(vpncfg.serverip));
    tmpstr = cJSON_GetObjectItem(jsonValue, "username")?cJSON_GetObjectItem(jsonValue, "username")->valuestring:"";
    UTIL_STRNCPY(vpncfg.username, tmpstr, sizeof(vpncfg.username));
    tmpstr = cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"";
    memset(bindata, 0, 2048);
	len_debug=base64_decode(tmpstr, bindata);    
	result=(char *)my_decrypt(len_debug,bindata, RSA_PRIVATE_KEY_FILE); 
    if(result == NULL)
	{
		global_weberrorcode=ERR_DECRY_FAIL;
		return ERR_DECRY_FAIL;
	}
    UTIL_STRNCPY(vpncfg.password, result, sizeof(vpncfg.password));
    if(result!=NULL)
    {
        free(result);
    }
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    ret = QtGetVpn(vpncfgArray, &len);
    
    if (len >= 8)
    {
        printf("Too many vpn rules\n");
        global_weberrorcode = ERR_TOO_MANY_RULES;
        QtReleaseUciLock();
        return ERR_TOO_MANY_RULES;
    }
    
    ret = QtAddVpn(&vpncfg);
    if (ret != 0)
    {
        printf("Fail to add vpn, ret = %d\n", ret);
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        QtReleaseUciLock();
        return ERR_INTERNALLOGIC_WRONG;
    }
    QtReleaseUciLock();
    return ret;
}

int proc_edit_vpn(cJSON *jsonValue,cJSON *jsonOut)
{
    cJSON *dataObj;
    int ret ;
    char *tmpstr;
    QT_VPN_CFG vpncfg = {0};
    
    tmpstr = cJSON_GetObjectItem(jsonValue, "description")?cJSON_GetObjectItem(jsonValue, "description")->valuestring:"";
    UTIL_STRNCPY(vpncfg.description, tmpstr, sizeof(vpncfg.description));
    tmpstr = cJSON_GetObjectItem(jsonValue, "mode")?cJSON_GetObjectItem(jsonValue, "mode")->valuestring:"";
    util_strToLower(tmpstr);
    UTIL_STRNCPY(vpncfg.proto, tmpstr, sizeof(vpncfg.proto));
    tmpstr = cJSON_GetObjectItem(jsonValue, "server_ip")?cJSON_GetObjectItem(jsonValue, "server_ip")->valuestring:"";
    UTIL_STRNCPY(vpncfg.serverip, tmpstr, sizeof(vpncfg.serverip));
    tmpstr = cJSON_GetObjectItem(jsonValue, "username")?cJSON_GetObjectItem(jsonValue, "username")->valuestring:"";
    UTIL_STRNCPY(vpncfg.username, tmpstr, sizeof(vpncfg.username));
    tmpstr = cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"";
    UTIL_STRNCPY(vpncfg.password, tmpstr, sizeof(vpncfg.password));
    tmpstr = cJSON_GetObjectItem(jsonValue, "ifname")?cJSON_GetObjectItem(jsonValue, "ifname")->valuestring:"";
    UTIL_STRNCPY(vpncfg.ifname, tmpstr, sizeof(vpncfg.ifname));
    dataObj = cJSON_CreateObject();
    if (dataObj == NULL)
    {
        printf("Fail to create data json obj!\n");
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    ret = QtEditVpn(&vpncfg);
    QtReleaseUciLock();
    if (ret != 0)
    {
        printf("Fail to add vpn, ret = %d\n", ret);
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    vpn_set_edit_flag();
    ProcSetVpnMsgReq();
    return ret;
}

int proc_edit_vpn_by_web(cJSON *jsonValue,cJSON *jsonOut)
{
    cJSON *dataObj;
    int ret ;
    char *tmpstr;
    QT_VPN_CFG vpncfg = {0};
    unsigned char bindata[2048]={0};
    int len_debug;
    char *result=NULL;
    
    tmpstr = cJSON_GetObjectItem(jsonValue, "description")?cJSON_GetObjectItem(jsonValue, "description")->valuestring:"";
    UTIL_STRNCPY(vpncfg.description, tmpstr, sizeof(vpncfg.description));
    tmpstr = cJSON_GetObjectItem(jsonValue, "mode")?cJSON_GetObjectItem(jsonValue, "mode")->valuestring:"";
    util_strToLower(tmpstr);
    UTIL_STRNCPY(vpncfg.proto, tmpstr, sizeof(vpncfg.proto));
    tmpstr = cJSON_GetObjectItem(jsonValue, "server_ip")?cJSON_GetObjectItem(jsonValue, "server_ip")->valuestring:"";
    UTIL_STRNCPY(vpncfg.serverip, tmpstr, sizeof(vpncfg.serverip));
    tmpstr = cJSON_GetObjectItem(jsonValue, "username")?cJSON_GetObjectItem(jsonValue, "username")->valuestring:"";
    UTIL_STRNCPY(vpncfg.username, tmpstr, sizeof(vpncfg.username));
    tmpstr = cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"";
    if (strlen(tmpstr) != 0)
    {
        memset(bindata, 0, 2048);
    	len_debug=base64_decode(tmpstr, bindata);    
    	result=(char *)my_decrypt(len_debug,bindata, RSA_PRIVATE_KEY_FILE); 
        if(result == NULL)
    	{
    		global_weberrorcode=ERR_DECRY_FAIL;
    		return ERR_DECRY_FAIL;
    	}
        UTIL_STRNCPY(vpncfg.password, result, sizeof(vpncfg.password));
        if(result!=NULL)
        {
            free(result);
        }
    }
    tmpstr = cJSON_GetObjectItem(jsonValue, "ifname")?cJSON_GetObjectItem(jsonValue, "ifname")->valuestring:"";
    UTIL_STRNCPY(vpncfg.ifname, tmpstr, sizeof(vpncfg.ifname));
    dataObj = cJSON_CreateObject();
    if (dataObj == NULL)
    {
        printf("Fail to create data json obj!\n");
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    ret = QtEditVpn(&vpncfg);
    QtReleaseUciLock();
    if (ret != 0)
    {
        printf("Fail to add vpn, ret = %d\n", ret);
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    vpn_set_edit_flag();
    ProcSetVpnMsgReq();
    return ret;
}

int proc_del_vpn(cJSON *jsonValue,cJSON *jsonOut)
{
    cJSON *dataObj;
    int ret ;
    char *tmpstr;
    char ifname[32];

    tmpstr = cJSON_GetObjectItem(jsonValue, "ifname")?cJSON_GetObjectItem(jsonValue, "ifname")->valuestring:"";
    UTIL_STRNCPY(ifname, tmpstr, sizeof(ifname));
    dataObj = cJSON_CreateObject();
    if (dataObj == NULL)
    {
        printf("Fail to create data json obj!\n");
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    ret = QtDelVpn(ifname);
    QtReleaseUciLock();
    if (ret != 0)
    {
        printf("Fail to add vpn, ret = %d\n", ret);
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    ProcSetVpnMsgReq();
    return ret;
}

int proc_get_vpn(cJSON *jsonValue,cJSON *jsonOut)
{
    cJSON *dataObj, *vpnObj;
    cJSON *itemObj;
    int ret ;
    char *tmpstr;
    QT_VPN_CFG vpncfg[8] = {0};
    int len = sizeof(vpncfg)/sizeof(QT_VPN_CFG);
    int i = 0;
    int enable;
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    ret = QtGetVpn(vpncfg, &len);
    QtReleaseUciLock();
    if (ret != 0)
    {
        printf("Fail to get vpn, ret = %d\n", ret);
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    
    vpnObj = cJSON_CreateArray();
    if (vpnObj == NULL)
    {
        printf("Fail to create vpnObj json obj!\n");
        return -9;	
    }

    dataObj = cJSON_CreateObject();
    if (dataObj == NULL)
    {
        printf("Fail to create data json obj!\n");
        global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    
    for (i = 0 ; i < len; i++)
    {
        itemObj = cJSON_CreateObject();
        if (itemObj == NULL)
        {
            printf("Fail to create data json obj!\n");
            global_weberrorcode = ERR_INTERNALLOGIC_WRONG;
        	return ERR_INTERNALLOGIC_WRONG;
        }

        cJSON_AddItemToObject(itemObj, "description", cJSON_CreateString(vpncfg[i].description));
        cJSON_AddItemToObject(itemObj, "ifname", cJSON_CreateString(vpncfg[i].ifname));
        cJSON_AddItemToObject(itemObj, "mode", cJSON_CreateString(vpncfg[i].proto));
        cJSON_AddItemToObject(itemObj, "server_ip", cJSON_CreateString(vpncfg[i].serverip));
        cJSON_AddItemToObject(itemObj, "username", cJSON_CreateString(vpncfg[i].username));
        //cJSON_AddItemToObject(itemObj, "password", cJSON_CreateString(vpncfg[i].password));
        cJSON_AddItemToObject(itemObj, "enable", cJSON_CreateNumber(vpncfg[i].enable));
        cJSON_AddItemToObject(itemObj, "status", cJSON_CreateString((vpncfg[i].status && !vpn_get_edit_flag())?"up":"down"));
        cJSON_AddItemToArray(vpnObj, itemObj);
    }
    cJSON_AddItemToObject(dataObj, "vpn_list", vpnObj);
    QtGetVpnSw(&enable);
    cJSON_AddItemToObject(dataObj, "enable", cJSON_CreateNumber(enable));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);

    

    return ret;
}


int proc_set_vpn_sw(cJSON *jsonValue,cJSON *jsonOut)
{
    int vpnenable = 0;
    char ifname[32] = {0};
    int enable = 0;
    int ret, i, j, enableNum = 0;
    cJSON* vpnArray, *itemObj;
    int arraySize;

    if (!cJSON_GetObjectItem(jsonValue, "enable")
        || !cJSON_GetObjectItem(jsonValue, "vpn"))
    {
        printf("miss argument\n");
        global_weberrorcode = ERR_PARAMETER_MISS;
        return ERR_PARAMETER_MISS;
    }

    vpnenable = cJSON_GetObjectItem(jsonValue, "enable")->valueint;
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    QtSetVpnSw(vpnenable);

    vpnArray = cJSON_GetObjectItem(jsonValue, "vpn");
    arraySize = cJSON_GetArraySize(vpnArray);

    for (i = 0; i < arraySize; i++)
    {
        itemObj = cJSON_GetArrayItem(vpnArray, i);
        if (!cJSON_GetObjectItem(itemObj, "ifname")
            || !cJSON_GetObjectItem(itemObj, "enable"))
        {
            printf("miss argument int itemobj\n");
            global_weberrorcode = ERR_PARAMETER_MISS;
            QtReleaseUciLock();
            return ERR_PARAMETER_MISS;
        }
        enable = cJSON_GetObjectItem(itemObj, "enable")->valueint;

        if (enable)
        {
            enableNum++;
            if (enableNum >= 2)
            {
                printf("Only allow one vpn rule enable!\n");
                global_weberrorcode = ERR_VPN_TOO_MANY_ENABLE;
                QtReleaseUciLock();
                return ERR_VPN_TOO_MANY_ENABLE;
            }
        }
    }
    
    for (i = 0; i < arraySize; i++)
    {
        itemObj = cJSON_GetArrayItem(vpnArray, i);
        if (!cJSON_GetObjectItem(itemObj, "ifname")
            || !cJSON_GetObjectItem(itemObj, "enable"))
        {
            printf("miss argument int itemobj\n");
            global_weberrorcode = ERR_PARAMETER_MISS;
            QtReleaseUciLock();
            return ERR_PARAMETER_MISS;
        }

        UTIL_STRNCPY(ifname, cJSON_GetObjectItem(itemObj, "ifname")->valuestring, sizeof(ifname));
        enable = cJSON_GetObjectItem(itemObj, "enable")->valueint;
#if 0        
        if (enable)
        {
            QtGetVpn(vpnCfgArray, &len);
            for (j = 0; j < len; j++)
            {
                printf("ifname1:%s, ifname2:%s, enable1:%d, enable2:%d\n", vpnCfgArray[j].ifname, ifname, vpnCfgArray[j].enable, enable);
                if (!strncmp(vpnCfgArray[j].ifname, ifname, sizeof(vpnCfgArray[j].ifname)))
                {
                    continue;
                }

                if (vpnCfgArray[j].enable)
                {
                    printf("Only allow one vpn rule enable!\n");
                    global_weberrorcode = ERR_VPN_TOO_MANY_ENABLE;
                    return ERR_VPN_TOO_MANY_ENABLE;
                }
            }
        }
#endif
        ret = QtSetVpnIfSw(ifname, enable, vpnenable);
        if (ret != 0)
        {
            printf("Fail to set vpn sw\n");
            global_weberrorcode = ret;
            QtReleaseUciLock();
            return ret;
        }
    }
    QtReleaseUciLock();
    ProcSetVpnMsgReq();
    return 0;
}