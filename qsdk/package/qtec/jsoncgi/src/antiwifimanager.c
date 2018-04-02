#include "basic.h"
#include <fwk.h>

static int GetDevInfoFromLanHost(cJSON **jsonOut);
static int CheckWaitAuth(char *macaddr);
static void GetAuthWaitList(cJSON *jsonOut);
static void GetAuthList(cJSON *jsonOut);
static int ProcAuthPass(char *macin);
static void ProcAuthCancel(char *macin);

struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[2048];
};

void *g_msgHandle;

static int GetDevInfoFromLanHost(cJSON **jsonOut)
{
	int ret = 0;
    VosMsgHeader  stHead={0};
    void *replyMsg = 0;
    VosMsgHeader stReplyHead = {0};
    char *bodyMsg; 
    
	vosLog_init(EID_CGIREQ);
	vosLog_setDestination(VOS_LOG_DEST_STDERR);
	vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		printf("dm msg initialization failed, ret= %d", ret);
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		return -1;
	}

    stHead.dataLength = 0;
    stHead.dst = EID_LANHOST;
    stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stHead.type = VOS_MSG_ANTIWIFI_DEVINFO_GET;
	stHead.flags_request = 1;

	ret = vosMsg_sendAndGetReplyBufWithTimeout(g_msgHandle, (const VosMsgHeader *)&stHead, (VosMsgHeader **)&replyMsg, MSECS_IN_SEC);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		printf("get reply msg failed, ret= %d", ret);
		vosLog_error("get reply msg failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return -1;
	}

	if(replyMsg)
	{
        memcpy(&stReplyHead, replyMsg, sizeof(VosMsgHeader));
		if(stReplyHead.type == VOS_MSG_ANTIWIFI_DEVINFO_GET)
		{
            bodyMsg = malloc(stReplyHead.dataLength);
            memset(bodyMsg, 0, sizeof(bodyMsg));
            memcpy(bodyMsg, replyMsg + sizeof(VosMsgHeader), stReplyHead.dataLength);
			*jsonOut = cJSON_Parse(bodyMsg);
            printf("pstReplyMsg->buf is %s.\n", bodyMsg);
            free(bodyMsg);
		}
        VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);
	}

	vosMsg_cleanup(&g_msgHandle);
	return 0;
}

static int CheckWaitAuth(char *macaddr)
{
    /* 检查mac是否在黑名单或者白名单中 */
    int ret = 0;
	int i = 0;
    int enable = 0;
    int size = 0;
    char mac[64] = {0};
    char ip[64] = {0};
    char name[64] = {0};
    int type = 0;
    char cmd[256] = {0};
    cJSON *jsonin, *jsonout, *obj, *array;
    i = -1;
	ret = 0;
    while(ret == 0)
    {
        i++;
        memset(cmd,0,256);
        snprintf(cmd,256,"antiwifi.@whitelist[%d].macaddr",i);
        memset(mac,0,64);
        ret = rtcfgUciGet(cmd,mac);
        if(0 ==ret)
        {
            if(0 == strncmp(macaddr, mac, 64))
            {
                printf("this mac is in whitelsit.\n");
                return 1;
            }
        }
    }

    jsonout = cJSON_CreateObject();
    proc_macblock_get(jsonin, jsonout);

    if(jsonout)
    {
        array = cJSON_GetObjectItem(jsonout, "data");
        size=cJSON_GetArraySize(array);
        for(i = 0; i < size; i++)
        {
            obj = cJSON_GetArrayItem(array, i);
            memset(mac, 0, 64);
            strcpy(mac, (cJSON_GetObjectItem(obj, "macaddr")?cJSON_GetObjectItem(obj, "macaddr")->valuestring:""));
            enable =  cJSON_GetObjectItem(obj, "enabled")?cJSON_GetObjectItem(obj, "enabled")->valueint:0;
            if(0 == strncmp(macaddr, mac, 64))
            {
                if(enable)
                {
                    printf("this mac is in blacklsit.\n");
                    cJSON_Delete(jsonout);
                    return 1;
                }
            }
        }        
    }

    cJSON_Delete(jsonout);
    return 0;    
}

static void GetAuthWaitList(cJSON *jsonOut)
{
    int ret, waitAuthFlag, i, size;
    char macaddr[64] = {0};
    char ip[64] = {0};
    char name[64] = {0};
    int type = 0;
    cJSON *array, *objin, *objout;
    cJSON *subJson, *devlist;

    ret = GetDevInfoFromLanHost(&subJson);
    if(0 == ret)
    {
        devlist = cJSON_GetObjectItem(subJson, "data");
    }
    else
    {
        printf("GetAuthWaitList fail.\n");
        return;
    }

    cJSON_AddItemToObject(jsonOut,"data",array=cJSON_CreateArray());
    if(devlist)
    {
        size=cJSON_GetArraySize(devlist);
        for(i = 0; i < size; i++)
        {
            objin = cJSON_GetArrayItem(devlist, i);
            memset(macaddr, 0, 64);
            memset(ip, 0, 64);
            memset(name, 0, 64);
            strcpy(macaddr, (cJSON_GetObjectItem(objin, "macaddr")?cJSON_GetObjectItem(objin, "macaddr")->valuestring:""));
            strcpy(ip, (cJSON_GetObjectItem(objin, "ipaddr")?cJSON_GetObjectItem(objin, "ipaddr")->valuestring:""));
            strcpy(name, (cJSON_GetObjectItem(objin, "staname")?cJSON_GetObjectItem(objin, "staname")->valuestring:""));
            type = cJSON_GetObjectItem(objin, "devicetype")?cJSON_GetObjectItem(objin, "devicetype")->valueint:0;
            
            waitAuthFlag = CheckWaitAuth(macaddr);
            if(0 == waitAuthFlag)
            {
                cJSON_AddItemToArray(array,objout=cJSON_CreateObject());
    			cJSON_AddItemToObject(objout,"dev_name",cJSON_CreateString(name));
                cJSON_AddItemToObject(objout,"dev_type",cJSON_CreateNumber(type));
                cJSON_AddItemToObject(objout,"dev_ip",cJSON_CreateString(ip));
                cJSON_AddItemToObject(objout,"dev_mac",cJSON_CreateString(macaddr));
            }
        }        
    }

    cJSON_Delete(subJson);
}


static void GetAuthList(cJSON *jsonOut)
{
    int i, type = 0;
    int ret = 0;
    char ip[64] = {0};
    char mac[64] = {0};
    char name[64] = {0};
    char devtype[64] = {0};
    char cmd[256] = {0};
    cJSON *array, *obj;

    cJSON_AddItemToObject(jsonOut,"data",array=cJSON_CreateArray());
    
    i = -1;
	ret = 0;
    while(ret == 0)
    {
        i++;
        memset(cmd,0,256);
        snprintf(cmd,256,"antiwifi.@whitelist[%d].macaddr",i);
        memset(mac,0,64);
        ret = rtcfgUciGet(cmd,mac);
        if(0 ==ret)
        {
            memset(name, 0, 64);
            cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
            cJSON_AddItemToObject(obj,"dev_mac",cJSON_CreateString(mac));

            memset(ip, 0, 64);
            snprintf(cmd,256,"antiwifi.@whitelist[%d].ipaddr",i);
			rtcfgUciGet(cmd,ip);
            cJSON_AddItemToObject(obj,"dev_ip",cJSON_CreateString(ip));

            memset(name, 0, 64);
            snprintf(cmd,256,"antiwifi.@whitelist[%d].name",i);
			rtcfgUciGet(cmd,name);
            cJSON_AddItemToObject(obj,"dev_name",cJSON_CreateString(name));

            memset(devtype, 0, 64);
            snprintf(cmd,256,"antiwifi.@whitelist[%d].devtype",i);
			rtcfgUciGet(cmd,devtype);
            cJSON_AddItemToObject(obj,"dev_type",cJSON_CreateNumber(atoi(devtype)));
        }
    }
    
    return;
}

static int ProcAuthPass(char *macin)
{
    int size = 0;
	int i = 0;
    int searchflag = 0;
    int ret = 0;
    char name[64] = {0};
    int type = 0;
    char ip[64] = {0};
    char macaddr[64] = {0};
    char buffer[256] = {0};
    char nametmp[64] = {0};
    int typetmp = 0;
    char iptmp[64] = {0};
    char macaddrtmp[64] = {0};
    char buffertmp[256] = {0};
    char cmd[256] = {0};
	char cmd1[256] = {0};
    char cmd2[256] = {0};
	char cmd3[256] = {0};
    cJSON *jsonOut, *devlist, *obj;

    ret = GetDevInfoFromLanHost(&jsonOut);
    if(0 == ret)
    {
        devlist = cJSON_GetObjectItem(jsonOut, "data");
    }
    else
    {
        printf("GetAuthWaitList fail.\n");
        return ERR_INTERNALLOGIC_WRONG;
    }

    if(devlist)
    {
        size=cJSON_GetArraySize(devlist);
        for(i = 0; i < size; i++)
        {
            obj = cJSON_GetArrayItem(devlist, i);
			memset(macaddr, 0, 64);
            strcpy(macaddr, (cJSON_GetObjectItem(obj, "macaddr")?cJSON_GetObjectItem(obj, "macaddr")->valuestring:""));
            if(0 == strncmp(macin, macaddr, 64))
            {
                searchflag = 1;
                printf(" find the dev.\n");
                break;
            }
        }        
    }
    
    if(searchflag)
    {
        /* add to whitelist */
        i = -1;
		ret = 0;
        while(ret == 0)
        {
            i++;
            memset(cmd,0,256);
            snprintf(cmd,256,"antiwifi.@whitelist[%d].macaddr",i);
            memset(macaddrtmp,0,64);
            ret = rtcfgUciGet(cmd,macaddrtmp);
            if(0 ==ret)
            {
                if(0 == strncmp(macaddr, macaddrtmp, 64))
                {
                    printf("this mac is in whitelsit.\n");
                    cJSON_Delete(jsonOut);
                    return 0;
                }
            }
        }
        
        strcpy(macaddr, (cJSON_GetObjectItem(obj, "macaddr")?cJSON_GetObjectItem(obj, "macaddr")->valuestring:""));
        strcpy(ip, (cJSON_GetObjectItem(obj, "ipaddr")?cJSON_GetObjectItem(obj, "ipaddr")->valuestring:""));
        strcpy(name, (cJSON_GetObjectItem(obj, "staname")?cJSON_GetObjectItem(obj, "staname")->valuestring:""));
		if(0 == name[0])
		{
			strcpy(name, "unknow");
		}
        type = cJSON_GetObjectItem(obj, "devicetype")?cJSON_GetObjectItem(obj, "devicetype")->valueint:0;
        
		rtcfgUciAdd("antiwifi","whitelist");
        memset(cmd, 0, 256);
        snprintf(cmd,256,"antiwifi.@whitelist[-1].ipaddr=%s", ip);
        rtcfgUciSet(cmd);

        memset(cmd, 0, 256);
        snprintf(cmd,256,"antiwifi.@whitelist[-1].macaddr=%s", macaddr);
        rtcfgUciSet(cmd);

        memset(cmd, 0, 256);
        snprintf(cmd,256,"antiwifi.@whitelist[-1].name=%s", name);
        rtcfgUciSet(cmd);

        memset(cmd, 0, 256);
        snprintf(cmd,256,"antiwifi.@whitelist[-1].devtype=%d", type);
        rtcfgUciSet(cmd);

        rtcfgUciCommit("antiwifi");
        
        /* 设置规则 */
        snprintf(cmd, 256, "ebtables -D devaccess -s %s -j RETURN", macin);
        snprintf(cmd1, 256, "ebtables -D devaccess -d %s -j RETURN", macin);
    	snprintf(cmd2, 256, "iptables -t nat -D prerouting_lan_rule -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macin);
		snprintf(cmd3, 256, "iptables -t mangle -D PREROUTING -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macin);
        system(cmd);
        system(cmd1);
    	system(cmd2);
		system(cmd3);
        memset(cmd, 0, 256);
        memset(cmd1, 0, 256);
        memset(cmd2, 0, 256);
		memset(cmd3, 0, 256);
        snprintf(cmd, 256, "ebtables -I devaccess -s %s -j RETURN", macin);
        snprintf(cmd1, 256, "ebtables -I devaccess -d %s -j RETURN", macin);
    	snprintf(cmd2, 256, "iptables -t nat -I prerouting_lan_rule -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macin);
		snprintf(cmd3, 256, "iptables -t mangle -I PREROUTING -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macin);
        system(cmd);
        system(cmd1);
    	system(cmd2);
		system(cmd3);
    }
    else
    {
        printf("can not find the device.\n");
        cJSON_Delete(jsonOut);
        return ERR_OFFLINE_OR_IN_MACBLOCK;
    }
    
    cJSON_Delete(jsonOut);
    return 0;
}


static void ProcAuthCancel(char *macin)
{
    char mac[64] = {0};
    char buffer[256] = {0};
	char cmd[256] = {0};
	char cmd1[256] = {0};
    char cmd2[256] = {0};
	char cmd3[256] = {0};
    int searchflag = 0;
    int i, ret;
	
	printf("====[%s]====%d===\n",__func__,__LINE__);
    i = -1;
	ret = 0;
    while(ret == 0)
    {
        i++;
        memset(cmd,0,256);
        snprintf(cmd,256,"antiwifi.@whitelist[%d].macaddr",i);
        memset(mac,0,64);
        ret = rtcfgUciGet(cmd,mac);
		printf("====[%s]====%d===\n",__func__,__LINE__);
		printf("mac:%s, macin:%s.\n",mac, macin);
        if(0 == ret)
        {
            if(0 == strncmp(macin, mac, 64))
            {
                printf("ProcAuthCancel:this mac is in whitelsit.\n");
                searchflag = 1;
                break;
            }
        }
    }

    if(searchflag)
    {
		DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
        memset(cmd,0,256);
        snprintf(cmd,256,"antiwifi.@whitelist[%d]",i);
        rtcfgUciDel(cmd);
        rtcfgUciCommit("antiwifi");
        
        /* 设置规则 */
        snprintf(cmd, 256, "ebtables -D devaccess -s %s -j RETURN", macin);
        snprintf(cmd1, 256, "ebtables -D devaccess -d %s -j RETURN", macin);
    	snprintf(cmd2, 256, "iptables -t nat -D prerouting_lan_rule -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macin);
		snprintf(cmd3, 256, "iptables -t mangle -D PREROUTING -m mac --mac-source %s -m mark --mark 0x50 -j ACCEPT", macin);
        system(cmd);
        system(cmd1);
    	system(cmd2);
		system(cmd3);
    }
	
	return;
}


void ProcAntiwifiSet(cJSON *json_value,cJSON *jsonOut)
{
    int enableflag = 0;
    char cmd[256]={0};
    char question[256] = {0};
    char answer[256] = {0};
	int findipflag = 0;
	char cmd1[256] = {0};
	char buffer[256] = {0};
    char lanip[64] = {0};
    char netmask[64] = {0};
	char ip[64] = {0};
	char macaddr[64] = {0};
	char tmpchar1[64]={0};
    char tmpchar2[64]={0};
    char tmpchar3[64]={0};
    char tmpchar4[64]={0};
	char *ipaddress;
    FILE *fp;
    
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	rtcfgUciGet("network.lan.ipaddr",lanip);
    rtcfgUciGet("network.lan.netmask",netmask);
    enableflag = cJSON_GetObjectItem(json_value, "enable")?cJSON_GetObjectItem(json_value, "enable")->valueint:0;
    /* set enable flag */
    snprintf(cmd,256,"antiwifi.@systeminfo[0].enable=%d",enableflag);
    rtcfgUciSet(cmd);
    
    if(enableflag)
    {
        /* save question and answer */
        strcpy(question, (cJSON_GetObjectItem(json_value, "question")?cJSON_GetObjectItem(json_value, "question")->valuestring:""));
        strcpy(answer, (cJSON_GetObjectItem(json_value, "answer")?cJSON_GetObjectItem(json_value, "answer")->valuestring:""));
        memset(cmd, 0, 256);
        snprintf(cmd,256,"antiwifi.@systeminfo[0].question=%s",question);
        rtcfgUciSet(cmd);
        memset(cmd, 0, 256);
        snprintf(cmd,256,"antiwifi.@systeminfo[0].answer=%s",answer);
        rtcfgUciSet(cmd);

        /* ebtables set */
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -t nat -D PREROUTING -i ath0 -j mark --mark-set 0x50");
		printf("cmd:%s.\n", cmd);
		system(cmd);

        memset(cmd1, 0, 256);
        snprintf(cmd1, 256, "ebtables -t nat -D PREROUTING -i ath1 -j mark --mark-set 0x50");
		printf("cmd1:%s.\n", cmd1);
		system(cmd1);
        
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -t nat -A PREROUTING -i ath0 -j mark --mark-set 0x50");
		printf("cmd:%s.\n", cmd);
		system(cmd);

        memset(cmd1, 0, 256);
        snprintf(cmd1, 256, "ebtables -t nat -A PREROUTING -i ath1 -j mark --mark-set 0x50");
		printf("cmd1:%s.\n", cmd1);
		system(cmd1);
        
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "iptables -t nat -D prerouting_lan_rule -p tcp -m multiport --dport 80,8080,443 ! -d %s -m mark --mark 0x50 -j DNAT --to %s:81", lanip, lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);
        
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "iptables -t nat -A prerouting_lan_rule -p tcp -m multiport --dport 80,8080,443 ! -d %s -m mark --mark 0x50 -j DNAT --to %s:81", lanip, lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);

        /* set deviceaccess add routeraccess enable */
		rtcfgUciSet("antiwifi.@systeminfo[0].router_access=1");
        rtcfgUciSet("antiwifi.@systeminfo[0].lan_dev_access=1");
        memset(cmd, 0, 256);
		snprintf(cmd, 256, "iptables -t mangle -D PREROUTING -p tcp -d %s --dport 80 -m mark --mark 0x50 -j DROP", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);
        
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "iptables -t mangle -A PREROUTING -p tcp -d %s --dport 80 -m mark --mark 0x50 -j DROP", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);

        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -D devaccess -i ath0 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -D devaccess -i ath1 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
        memset(cmd, 0, 256);
		snprintf(cmd, 256, "ebtables -D devaccess -p ipv4 --ip-dst %s -j RETURN", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);

        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -A devaccess -i ath0 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -A devaccess -i ath1 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ebtables -I devaccess -p ipv4 --ip-dst %s -j RETURN", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);

		/* allow own mac */
		ipaddress = getenv("REMOTE_ADDR");
		if(NULL == ipaddress)
		{
			printf("can not get REMOTE_ADDR.\n");
			return;
		}
		printf("ipaddress is:%s.\n", ipaddress);
		fp = fopen("/proc/net/arp", "r");
		if(NULL == fp)
		{
			printf("can not get macaddr.\n");
            rtcfgUciCommit("antiwifi");
			return;
		}
		else
		{
			memset(buffer, 0, 256);
			fgets(buffer,256,fp);
			while(fgets(buffer,256,fp))
			{	
				memset(ip, 0, 64);			
				sscanf(buffer,"%s %s %s %s %s %s", ip, tmpchar1, tmpchar2, macaddr, tmpchar3, tmpchar4);
				printf("ip is:%s,macaddr is:%s.\n", ip, macaddr);
				if(0 == strncmp(ip, ipaddress, 64))
				{
					findipflag = 1;
					break;
				}
			}
            fclose(fp);
		}
		if(findipflag)
		{
			ProcAuthPass(macaddr);
		}
    }
    else
    {
		/* cancel antiwifi enable, lanDevAccess and routerAccess rule should delete */
		memset(cmd, 0, 256);
		snprintf(cmd,256,"antiwifi.@systeminfo[0].router_access=0");
    	rtcfgUciSet(cmd);

    	memset(cmd, 0, 256);
    	snprintf(cmd,256,"antiwifi.@systeminfo[0].lan_dev_access=0");
    	rtcfgUciSet(cmd);

        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -t nat -D PREROUTING -i ath0 -j mark --mark-set 0x50");
		printf("cmd:%s.\n", cmd);
		system(cmd);

        memset(cmd1, 0, 256);
        snprintf(cmd1, 256, "ebtables -t nat -D PREROUTING -i ath1 -j mark --mark-set 0x50");
		printf("cmd1:%s.\n", cmd1);
		system(cmd1);
        
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "iptables -t nat -D prerouting_lan_rule -p tcp -m multiport --dport 80,8080,443 ! -d %s -m mark --mark 0x50 -j DNAT --to %s:81", lanip, lanip);
		system(cmd);

        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -D devaccess -i ath0 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -D devaccess -i ath1 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ebtables -D devaccess -p ipv4 --ip-dst %s -j RETURN", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);


		memset(cmd, 0, 256);
		snprintf(cmd, 256, "iptables -t mangle -D PREROUTING -p tcp -d %s --dport 80 -m mark --mark 0x50 -j DROP", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);

		ipaddress = getenv("REMOTE_ADDR");
		if(NULL == ipaddress)
		{
			printf("can not get REMOTE_ADDR.\n");
            rtcfgUciCommit("antiwifi");
			return;
		}
		printf("ipaddress is:%s.\n", ipaddress);
		fp = fopen("/proc/net/arp", "r");
		if(NULL == fp)
		{
			printf("can not get macaddr.\n");
			return;
		}
		else
		{
			memset(buffer, 0, 256);
			fgets(buffer,256,fp);
			while(fgets(buffer,256,fp))
			{	
				memset(ip, 0, 64);			
				sscanf(buffer,"%s %s %s %s %s %s", ip, tmpchar1, tmpchar2, macaddr, tmpchar3, tmpchar4);
				printf("ip is:%s,macaddr is:%s.\n", ip, macaddr);
				if(0 == strncmp(ip, ipaddress, 64))
				{
					findipflag = 1;
					break;
				}
			}
            fclose(fp);
		}
		if(findipflag)
		{
			ProcAuthCancel(macaddr);
		}
    }
    
    rtcfgUciCommit("antiwifi");

	return;
}


void ProcAntiwifiStatusGet(cJSON *json_value,cJSON *jsonOut)
{
    char enable[16]={0};
    char routeAccess[16] = {0};
    char lanDevAccess[16] = {0};
    cJSON *data = NULL;
    
    rtcfgUciGet("antiwifi.@systeminfo[0].enable", enable);
    rtcfgUciGet("antiwifi.@systeminfo[0].router_access", routeAccess);
    rtcfgUciGet("antiwifi.@systeminfo[0].lan_dev_access", lanDevAccess);

    data = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", data);

    if(enable[0])
    {
        cJSON_AddItemToObject(data,"enable",cJSON_CreateNumber(atoi(enable)));
    }
    else
    {
         cJSON_AddItemToObject(data,"enable",cJSON_CreateNumber(0));
    }

    if(routeAccess[0])
    {
        cJSON_AddItemToObject(data,"router_access",cJSON_CreateNumber(atoi(routeAccess)));
    }
    else
    {
         cJSON_AddItemToObject(data,"router_access",cJSON_CreateNumber(0));
    }

    if(lanDevAccess[0])
    {
        cJSON_AddItemToObject(data,"lan_dev_access",cJSON_CreateNumber(atoi(lanDevAccess)));
    }
    else
    {
         cJSON_AddItemToObject(data,"lan_dev_access",cJSON_CreateNumber(0));
    }
    
    return;
}

void ProcQuestionSet(cJSON *json_value,cJSON *jsonOut)
{
    char question[256] = {0};
    char answer[256] = {0};
    char cmd[256] = {0};

    strcpy(question, (cJSON_GetObjectItem(json_value, "question")?cJSON_GetObjectItem(json_value, "question")->valuestring:""));
    strcpy(answer, (cJSON_GetObjectItem(json_value, "answer")?cJSON_GetObjectItem(json_value, "answer")->valuestring:""));

    memset(cmd, 0, 256);
    snprintf(cmd,256,"antiwifi.@systeminfo[0].question=%s",question);
    rtcfgUciSet(cmd);
    memset(cmd, 0, 256);
    snprintf(cmd,256,"antiwifi.@systeminfo[0].answer=%s",answer);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("antiwifi");
    return;
}


void ProcAntiWifiAdminSet(cJSON *json_value,cJSON *jsonOut)
{
    int routerAccess = 0;
    int lanDevAccess = 0;
    char cmd[256]={0};
	char lanip[64] = {0};
    char netmask[64] = {0};

	rtcfgUciGet("network.lan.ipaddr",lanip);
    rtcfgUciGet("network.lan.netmask",netmask);
    routerAccess = cJSON_GetObjectItem(json_value, "router_access")?cJSON_GetObjectItem(json_value, "router_access")->valueint:0;
    lanDevAccess = cJSON_GetObjectItem(json_value, "lan_dev_access")?cJSON_GetObjectItem(json_value, "lan_dev_access")->valueint:0;

    snprintf(cmd,256,"antiwifi.@systeminfo[0].router_access=%d",routerAccess);
    rtcfgUciSet(cmd);

    memset(cmd, 0, 256);
    snprintf(cmd,256,"antiwifi.@systeminfo[0].lan_dev_access=%d",lanDevAccess);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("antiwifi");

    if(routerAccess)
	{
        memset(cmd, 0, 256);
		snprintf(cmd, 256, "iptables -t mangle -D PREROUTING -p tcp -d %s --dport 80 -m mark --mark 0x50 -j DROP", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);
        
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "iptables -t mangle -A PREROUTING -p tcp -d %s --dport 80 -m mark --mark 0x50 -j DROP", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);
	}
	else
	{
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "iptables -t mangle -D PREROUTING -p tcp -d %s --dport 80 -m mark --mark 0x50 -j DROP", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);
	}
    
	if(lanDevAccess)
	{
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -D devaccess -i ath0 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -D devaccess -i ath1 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
        memset(cmd, 0, 256);
		snprintf(cmd, 256, "ebtables -D devaccess -p ipv4 --ip-dst %s -j RETURN", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);

        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -A devaccess -i ath0 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -A devaccess -i ath1 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ebtables -I devaccess -p ipv4 --ip-dst %s -j RETURN", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);

	}
	else
	{
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -D devaccess -i ath0 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
        memset(cmd, 0, 256);
        snprintf(cmd, 256, "ebtables -D devaccess -i ath1 -p ipv4 --ip-dst %s/%s -j DROP", lanip, netmask);
        system(cmd);
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ebtables -D devaccess -p ipv4 --ip-dst %s -j RETURN", lanip);
		printf("cmd:%s.\n", cmd);
		system(cmd);
	}
	
	return;
}


void ProcAntiWifiDevGet(cJSON *json_value,cJSON *jsonOut)
{
    int mode = 0;
    mode = cJSON_GetObjectItem(json_value, "mode")?cJSON_GetObjectItem(json_value, "mode")->valueint:0;

    if(mode)
    {
        /* 获取待认证列表 */
        GetAuthList(jsonOut);
    }
    else
    {
        /* 获取已认证列表 */
        GetAuthWaitList(jsonOut);
    }
}

int ProcAntiWifiAuthSet(cJSON *json_value,cJSON *jsonOut)
{
    char macaddr[64] = {0};
    int  auth = 0;
    int  block = 0;
	char name[64] = {0};
	int	 ret = 0;

	printf("====[%s]====%d===\n",__func__,__LINE__);
    auth = cJSON_GetObjectItem(json_value, "auth")?cJSON_GetObjectItem(json_value, "auth")->valueint:0;
    block = cJSON_GetObjectItem(json_value, "block")?cJSON_GetObjectItem(json_value, "block")->valueint:0;
    strcpy(macaddr, (cJSON_GetObjectItem(json_value, "dev_mac")?cJSON_GetObjectItem(json_value, "dev_mac")->valuestring:""));
	strcpy(name, (cJSON_GetObjectItem(json_value, "dev_name")?cJSON_GetObjectItem(json_value, "dev_name")->valuestring:""));

	printf("auth value:%d.\n", auth);
    if(1 ==auth)
    {
        /* 设置用户白名单，取消重定向 */
        ret = ProcAuthPass(macaddr);
        if(ret)
        {
            global_weberrorcode=ret;
            return ret;
        }
    }
    else if(2 == auth)
    {
        /* 把用户从白名单中删除，设置重定向 */
        ProcAuthCancel(macaddr);
    }

    if(1 == block)
    {
        /* 把用户添加到黑名单,从白名单删除 */
		ProcAuthCancel(macaddr);
		ret = check_macblock_add(macaddr);
		if(ret!=0)
		{
		    global_weberrorcode=ERR_RULE_CONFLICT;
		    return ERR_RULE_CONFLICT;
		}
		
		ret = add_macblock(name,macaddr, 1);
		if(ret!=0)
		{
		    global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		    return ERR_INTERNALLOGIC_WRONG;
		}
		restart_fw(0);
    }
    else if(2 == block)
    {
        /* 把用户从黑名单删除，添加到白名单 */
        ProcAuthPass(macaddr);
		ret = del_macblock(name,macaddr, 1);
		if(ret==-1)
		{
		    global_weberrorcode=ERR_NO_MATCH;
		    return ERR_NO_MATCH;
		}
		else if(ret!=0)
		{
		    global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		    return ERR_INTERNALLOGIC_WRONG;
		}
		restart_fw(0);
    }
	
	return 0;        
}

void ProcAntiWifiQuestionGet(cJSON *json_value,cJSON *jsonOut)
{
	char question[256] = {0};
    cJSON *data = NULL;
    
    rtcfgUciGet("antiwifi.@systeminfo[0].question", question);
    
    data = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", data);

    cJSON_AddItemToObject(data,"question", cJSON_CreateString(question));
    return; 
}


void ProcAntiWifiQueandAswGet(cJSON *json_value,cJSON *jsonOut)
{
    char question[256] = {0};
    char answer[256] = {0};
    cJSON *data = NULL;

    rtcfgUciGet("antiwifi.@systeminfo[0].question", question);
    rtcfgUciGet("antiwifi.@systeminfo[0].answer", answer);
    
    data = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", data);

    cJSON_AddItemToObject(data,"question", cJSON_CreateString(question));
    cJSON_AddItemToObject(data,"answer", cJSON_CreateString(answer));
    
    return; 
}

void ProcAntiWifiPasswordCheck(cJSON *json_value,cJSON *jsonOut)
{
    FILE *fp;
	char *ipaddress;
    char ip[64] = {0};
	char macaddr[64] = {0};
    char buffer[256] = {0};
    char answer[256] = {0};
	char answerstore[256] = {0};
	char tmpchar1[64]={0};
    char tmpchar2[64]={0};
    char tmpchar3[64]={0};
    char tmpchar4[64]={0};
    int auth = 0;
    int findMacFlag = 0;
    int waitAuthFlag = 0;
    cJSON *data = NULL;

	data = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", data);

    strcpy(answer, (cJSON_GetObjectItem(json_value, "answer")?cJSON_GetObjectItem(json_value, "answer")->valuestring:""));
	printf("answer is:%s.\n", answer);
	/* check answer */
    rtcfgUciGet("antiwifi.@systeminfo[0].answer", answerstore);
    auth = strncmp(answer, answerstore, 256);

    if(0 == auth)
    {
        /* get macaddr */
		ipaddress = getenv("REMOTE_ADDR");
		printf("ipaddress is:%s.\n", ipaddress);
		fp = fopen("/proc/net/arp", "r");
		if(NULL == fp)
		{
			printf("can not get macaddr.\n");
			return;
		}
		else
		{
			memset(buffer, 0, 256);
			fgets(buffer,256,fp);
			while(fgets(buffer,256,fp))
			{	
				memset(ip, 0, 64);			
				sscanf(buffer,"%s %s %s %s %s %s", ip, tmpchar1, tmpchar2, macaddr, tmpchar3, tmpchar4);
				printf("ip is:%s,macaddr is:%s.\n", ip, macaddr);
				if(0 == strncmp(ip, ipaddress, 64))
				{
                    findMacFlag = 1;
					break;
				}
			}
            fclose(fp);
		}
		/* 认证通过，添加白名单，取消重定向 */
        if(findMacFlag)
        {
            waitAuthFlag = CheckWaitAuth(macaddr);
            if(0 == waitAuthFlag)
            {
                ProcAuthPass(macaddr);
            }
            else
            {
                printf("this mac is in whitelsit.\n");
            }
        	cJSON_AddItemToObject(data,"checkresult", cJSON_CreateNumber(0));
        }
        else
        {
            global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
            return;
        }
    }
	else
	{
		cJSON_AddItemToObject(data,"checkresult", cJSON_CreateNumber(1));
	}
    return; 
}




