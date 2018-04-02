#include "basic.h"
#include <fwk.h>

struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[4096];
};

void *g_msgHandle;

int CheckUpdateVersion(char *updateversion)
{
    FILE *fp = NULL;
    char str[64];
    char localversion[64] = {0};
    fp=fopen("/etc/device_info","r");
    
    while((fgets(str,64,fp))!=NULL)
    {
        DEBUG_PRINTF("=====str : %s=====\n",str);
        if(strncmp(str,"SOFTWARE_VERSION",strlen("SOFTWARE_VERSION"))==0)
        {
            sscanf(str,"SOFTWARE_VERSION=%s", localversion);
            DEBUG_PRINTF("SOFTWARE_VERSION: %s==\n", localversion);
            break;
        }
    }

    fclose(fp);
    if(!strcmp(localversion, updateversion))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void ProcWifiMsgReq()
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_WIFI_REQ;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);
	return;
}

void ProcFirstbootReq(cJSON *json_value,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_FIRSTBOOT_REQ;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);
	return;
}

void ProcRawkeyAddReq()
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_RAWKEY_REQ;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);
	return;
}

int ProcSpeedTestReq()
{
    DEBUG_PRINTF("====[%s]====%d====\n",__func__,__LINE__);
    int ret;
    struct VosMsgBody stMsg={0};
    struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
    if(ret != VOS_RET_SUCCESS)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        vosLog_error("[%s] dm msg initialization failed, ret=%d ",__func__, ret);
        vosMsg_cleanup(&g_msgHandle);
        return ERR_INTERNALLOGIC_WRONG;
    }

    stMsg.stHead.dataLength=0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_SPEEDTEST_REQ;
    stMsg.stHead.flags_request = 1;

    vosMsg_send(g_msgHandle,&stMsg);
    vosMsg_cleanup(&g_msgHandle);
    return 0;
    
}

//å¼ºåˆ¶ç£ç›˜æ ¼å¼åŒ–
int ProcDiskReformatReq()
{
    DEBUG_PRINTF("====[%s]====%d====\n",__func__,__LINE__);
    int ret;
    struct VosMsgBody stMsg={0};
    struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
    if(ret != VOS_RET_SUCCESS)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        vosLog_error("[%s] dm msg initialization failed, ret=%d ",__func__, ret);
        vosMsg_cleanup(&g_msgHandle);
        return ERR_INTERNALLOGIC_WRONG;
    }

    stMsg.stHead.dataLength=0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_DISKREFORMAT_REQ;
    stMsg.stHead.flags_request = 1;

    vosMsg_send(g_msgHandle,&stMsg);
    vosMsg_cleanup(&g_msgHandle);
    return 0;
    
}

//å°è¯•æ‰“å¼€ç£ç›˜
int ProcDiskCheckReq()
{
    DEBUG_PRINTF("====[%s]====%d====\n",__func__,__LINE__);
    int ret;
    struct VosMsgBody stMsg={0};
    struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
    if(ret != VOS_RET_SUCCESS)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        vosLog_error("[%s] dm msg initialization failed, ret=%d ",__func__, ret);
        vosMsg_cleanup(&g_msgHandle);
        return ERR_INTERNALLOGIC_WRONG;
    }

    stMsg.stHead.dataLength=0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_DISKCHECK_REQ;
    stMsg.stHead.flags_request = 1;

    vosMsg_send(g_msgHandle,&stMsg);
    vosMsg_cleanup(&g_msgHandle);
    return 0;
    
}


int ProcUpdateReq(cJSON *json_value,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
	cJSON *obj;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;
	int isKeepConfig = cJSON_GetObjectItem(json_value, "keepconfig")?cJSON_GetObjectItem(json_value, "keepconfig")->valueint:1;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return ERR_INTERNALLOGIC_WRONG;
	}
	
	obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    if(isKeepConfig)
    {
        stMsg.stHead.type = VOS_MSG_UPDATE_KEEPCFG_REQ;
    }
    else
    {
        stMsg.stHead.type = VOS_MSG_UPDATE_REQ;
    }
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);
	return 0;
}

int ProcUpdateMsgGet(cJSON *json_value,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
	char *downloadUrl, *versionNo;
    char buffer[256] = {0};
    FILE *fp = NULL;
	cJSON *subJson = NULL;
    cJSON *obj = NULL;
    char str[64];
    char localversion[64] = {0};
    struct VosMsgBody stMsg={0};
	VosMsgHeader *pstReplyMsg;

    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    
    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		printf("dm msg initialization failed, ret= %d", ret);
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		return ERR_INTERNALLOGIC_WRONG;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_MYWEBSOCKET;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_UPDATE_GETINFO;
	stMsg.stHead.flags_request = 1;

	ret = vosMsg_sendAndGetReplyBufWithTimeout(g_msgHandle, (const VosMsgHeader *)&stMsg, &pstReplyMsg, MSECS_IN_SEC);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_GET_UPDATE_INFO_FAIL;
		printf("get reply msg failed, ret= %d", ret);
		vosLog_error("get reply msg failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return ERR_GET_UPDATE_INFO_FAIL;
	}

	if(pstReplyMsg)
	{
		if(pstReplyMsg->type == VOS_MSG_UPDATE_GETINFO)
		{
            //»ñÈ¡·µ»ØµÄÐÅÏ¢
			printf("ProcUpdateMsgGet: pstReplyMsg->buf is %s.\n", ((struct VosMsgBody *)pstReplyMsg)->buf);
			subJson = cJSON_Parse(((struct VosMsgBody *)pstReplyMsg)->buf); 
            if(subJson == NULL)
            {
                return ERR_INTERNALLOGIC_WRONG;
            }
            
            downloadUrl = cJSON_GetObjectItem(subJson, "downloadUrl")?cJSON_GetObjectItem(subJson, "downloadUrl")->valuestring:"";
            versionNo = cJSON_GetObjectItem(subJson, "versionNo")?cJSON_GetObjectItem(subJson, "versionNo")->valuestring:"";

            //±£´ædownloadurlÐÅÏ¢
            fp=fopen("/tmp/downloadUrl","w+");
            snprintf(buffer, 256, "downloadUrl:%s", downloadUrl);
	        fprintf(fp, "%s", buffer);
            fclose(fp);

            fp=fopen("/etc/device_info","r");
            if(fp)
            {
                while((fgets(str,64,fp))!=NULL)
                {
                    DEBUG_PRINTF("=====str : %s=====\n",str);
                    if(strncmp(str,"SOFTWARE_VERSION",strlen("SOFTWARE_VERSION"))==0)
                    {
                        sscanf(str,"SOFTWARE_VERSION=%s", localversion);
                        DEBUG_PRINTF("SOFTWARE_VERSION: %s==\n", localversion);
                        break;
                    }
                }

                fclose(fp);
            }
            
            cJSON_AddItemToObject(obj, "updateversionNo", cJSON_CreateString(versionNo));
            cJSON_AddItemToObject(obj, "localversionNo", cJSON_CreateString(localversion));
            if(!strcmp(localversion, versionNo))
            {
                cJSON_AddItemToObject(obj, "effectivity", cJSON_CreateNumber(0));
            }
            else
            {
                cJSON_AddItemToObject(obj, "effectivity", cJSON_CreateNumber(1));
            }
            cJSON_Delete(subJson);
		}
        VOS_MEM_FREE_BUF_AND_NULL_PTR(pstReplyMsg);
	}

	vosMsg_cleanup(&g_msgHandle);
	return 0;
}

int proc_stalist_get(cJSON *json_value,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
	char *ipaddress;
	cJSON *subJson;
    struct VosMsgBody stMsg={0};
	void *replyMsg = 0;

    VosMsgHeader stReplyHead = {0};
    char *bodyMsg;    

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		printf("dm msg initialization failed, ret= %d", ret);
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		return ERR_INTERNALLOGIC_WRONG;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_LANHOST;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_HOSTLAN_GETINFO;
	stMsg.stHead.flags_request = 1;

	ret = vosMsg_sendAndGetReplyBufWithTimeout(g_msgHandle, (const VosMsgHeader *)&stMsg, (VosMsgHeader **)&replyMsg, MSECS_IN_SEC);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		printf("get reply msg failed, ret= %d", ret);
		vosLog_error("get reply msg failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return ERR_INTERNALLOGIC_WRONG;
	}

	if(replyMsg)
	{
        memcpy(&stReplyHead, replyMsg, sizeof(VosMsgHeader));
		if(stReplyHead.type == VOS_MSG_HOSTLAN_GETINFO)
		{
            bodyMsg = malloc(stReplyHead.dataLength);
            memset(bodyMsg, 0, sizeof(bodyMsg));
            memcpy(bodyMsg, replyMsg + sizeof(VosMsgHeader), stReplyHead.dataLength);
			subJson = cJSON_Parse(bodyMsg);
			ipaddress = getenv("REMOTE_ADDR");
			if(ipaddress)
			{
    			cJSON_AddItemToObject(subJson,"ownip",cJSON_CreateString(ipaddress));
			}
			printf("proc_stalist_get: bodyMsg is %s, dataLength:%d.\n", bodyMsg, stReplyHead.dataLength);
			cJSON_AddItemToObject(jsonOut, "data", subJson);
            free(bodyMsg);
		}
        VOS_MEM_FREE_BUF_AND_NULL_PTR(replyMsg);
	}

	vosMsg_cleanup(&g_msgHandle);
	return 0;
}

void proc_routerstatus_get(cJSON *json_value,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret, routertx, routerrx, htx, hrx, usage, sysusage, totalusage;
    char routername[64] = "3care-gateway";
    char romVersion[64] = {0};
    char macaddr[64] = {0};
    char tmp1[64] = {0};
	char tmp2[64] = {0};
	char tmp3[64] = {0};
	char tmp4[64] = {0};
    char sn[64] = "10001/10000001";
    char cpuHz[64] = "717.000MHz";
    char cpuusage[64] = {0};
    char ramsize[64] = {0};
    char ramuse[64] = {0};
    char ramtype[64] = "DDR3";
    char ramHz[64] = "800.000MHz";
    char buffer[256] = {0};
    char buftmp[20] = {0};
	cJSON *subJson, *data;
    struct VosMsgBody stMsg={0};
	VosMsgHeader  *pstReplyMsg;
    FILE *fpcpu, *fpramsize, *fpramuse, *fp;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		printf("dm msg initialization failed, ret= %d", ret);
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_LANHOST;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_ROUTER_GETSTATUS;
	stMsg.stHead.flags_request = 1;

	ret = vosMsg_sendAndGetReplyBufWithTimeout(g_msgHandle, (const VosMsgHeader *)&stMsg, &pstReplyMsg, MSECS_IN_SEC);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		printf("get reply msg failed, ret= %d", ret);
		vosLog_error("get reply msg failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

	if(pstReplyMsg)
	{
		if(pstReplyMsg->type == VOS_MSG_ROUTER_GETSTATUS)
		{
			subJson = cJSON_Parse(((struct VosMsgBody *)pstReplyMsg)->buf);
            if (subJson)
            {
                routertx = cJSON_GetObjectItem(subJson, "routertx")?cJSON_GetObjectItem(subJson, "routertx")->valueint:0;
                routerrx = cJSON_GetObjectItem(subJson, "routerrx")?cJSON_GetObjectItem(subJson, "routerrx")->valueint:0;
                htx = cJSON_GetObjectItem(subJson, "htx")?cJSON_GetObjectItem(subJson, "htx")->valueint:0;
                hrx = cJSON_GetObjectItem(subJson, "hrx")?cJSON_GetObjectItem(subJson, "hrx")->valueint:0;
    			printf("proc_routerstatus_get: pstReplyMsg->buf is %s.\n", ((struct VosMsgBody *)pstReplyMsg)->buf);
                cJSON_Delete(subJson);
            }
		}
        VOS_MEM_FREE_BUF_AND_NULL_PTR(pstReplyMsg);
        
	}

    system("ifconfig eth0 > /tmp/.wanmac");
    fp = fopen("/tmp/.wanmac","r");
	if(fp)
	{
		fgets(buffer,256,fp);
    	sscanf(buffer, "%s %s %s %s %s", tmp1,tmp2,tmp3,tmp4, macaddr);
		fclose(fp);
		fp = NULL;
	}

    system("top -n 1 |grep CPU >/tmp/cpuusage.txt");
    system("free -h |grep Mem | awk -F ' ' '{print $2}' >/tmp/ramsize.txt");
    system("free -h |grep Mem | awk -F ' ' '{print $3}' >/tmp/ramuse.txt");

    fpramsize = fopen("/tmp/ramsize.txt", "r");
    fpramuse = fopen("/tmp/ramuse.txt", "r");
    fpcpu = fopen("/tmp/cpuusage.txt", "r");

    if(fpramsize)
    {
		memset(buffer, 0, 256);
		fgets(buffer,256,fpramsize);
		sscanf(buffer, "%s\n", ramsize);
        fclose(fpramsize);
    }
    
    if(fpramuse)
    {
		memset(buffer, 0, 256);
		fgets(buffer,256,fpramuse);
		sscanf(buffer, "%s\n", ramuse);
        fclose(fpramuse);
    }

    if(fpcpu)
    {

        fgets(buffer, 256, fpcpu);
        sscanf(buffer,  "CPU:   %d%s usr  %d%", &usage, buftmp, &sysusage);
        totalusage = usage + sysusage;
        sprintf(cpuusage, "%d%%", totalusage);
        fclose(fpcpu);
    }

	fp=fopen("/etc/device_info","r");
    if(fp)
    {
        while((fgets(buffer,256,fp))!=NULL)
        {
            if(strncmp(buffer,"SOFTWARE_VERSION",strlen("SOFTWARE_VERSION"))==0)
            {
                sscanf(buffer,"SOFTWARE_VERSION=%s", romVersion);
                DEBUG_PRINTF("SOFTWARE_VERSION: %s==\n", romVersion);
                break;
            }
        }
        fclose(fp);
        fp = NULL;
    }

    data = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", data);

    cJSON_AddItemToObject(data,"routername",cJSON_CreateString(routername));
    cJSON_AddItemToObject(data,"romVersion",cJSON_CreateString(romVersion));
    cJSON_AddItemToObject(data,"macaddr",cJSON_CreateString(macaddr));
    cJSON_AddItemToObject(data,"sn",cJSON_CreateString(sn));
    cJSON_AddItemToObject(data,"routertx",cJSON_CreateNumber(routertx));
    cJSON_AddItemToObject(data,"htx",cJSON_CreateNumber(htx));
    cJSON_AddItemToObject(data,"routerrx",cJSON_CreateNumber(routerrx));
    cJSON_AddItemToObject(data,"hrx",cJSON_CreateNumber(hrx));
    cJSON_AddItemToObject(data,"cpunum",cJSON_CreateNumber(4));
    cJSON_AddItemToObject(data,"cpuHz",cJSON_CreateString(cpuHz));
    cJSON_AddItemToObject(data,"cpuusage",cJSON_CreateString(cpuusage));
    cJSON_AddItemToObject(data,"ramsize",cJSON_CreateString(ramsize));
    cJSON_AddItemToObject(data,"ramuse",cJSON_CreateString(ramuse));
    cJSON_AddItemToObject(data,"ramtype",cJSON_CreateString(ramtype));
    cJSON_AddItemToObject(data,"ramHz",cJSON_CreateString(ramHz));
    
	vosMsg_cleanup(&g_msgHandle);
	return;
}

void ProcSetGuestWifiMsgReq()
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;


    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_SET_GUEST_WIFI_REQ;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);
	return;
}

void ProcWdsSetReq()
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_SET_WDS_REQ;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);

	return;
}

void ProcOneKeySwitchMsgReq()
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_ONE_KEY_SWITCH;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);

	return;
}

void ProcFirewallSetMsgReq()
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_FIREWALL_SET;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);

	return;
}

void ProcLanCfgSetMsgReq()
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_LAN_CFG_SET;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);

	return;
}

void ProcSetVpnMsgReq()
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = 0;
    stMsg.stHead.dst = EID_CGIMSGPROC;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_VPN_SET;
	stMsg.stHead.flags_request = 1;

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);

	return;
}


void ProcDisplayFirstKeyMsgReq(char *key)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	int ret;
    struct VosMsgBody stMsg={0};
	struct VosMsgBody *pstReplyMsg;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    stMsg.stHead.dataLength = strlen(key)+1;
    stMsg.stHead.dst = EID_SCRCTRL;
    stMsg.stHead.src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
    stMsg.stHead.type = VOS_MSG_FIRST_KEY_DISPLAY_REQ;
	stMsg.stHead.flags_request = 1;
    UTIL_STRNCPY(stMsg.buf, key, sizeof(stMsg.buf));

	vosMsg_send(g_msgHandle, &stMsg);
	vosMsg_cleanup(&g_msgHandle);
	return;

}


void ProcAddWifiTimer(TASK_INFO_STRU *pstTaskInfo)
{
    VosMsgHeader *msg = NULL;
    UINT8 ret;
    
    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    msg = (VosMsgHeader *)malloc(sizeof(VosMsgHeader) + sizeof(TASK_INFO_STRU));
    memset(msg, 0, sizeof(VosMsgHeader) + sizeof(TASK_INFO_STRU));
    if (msg)
    {
        memcpy(msg+1, pstTaskInfo, sizeof(TASK_INFO_STRU));
        msg->dataLength = sizeof(TASK_INFO_STRU);
        msg->dst = EID_TIMER_TASK;
        msg->src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
        msg->type = VOS_MSG_ADD_WIFI_TIMER_TASK;
        msg->flags_request = 1;
        vosMsg_send(g_msgHandle, msg);
        free(msg);
    }
    vosMsg_cleanup(&g_msgHandle);
}

void ProcDelWifiTimer(UINT16 taskId)
{
    UINT8 ret;
    VosMsgHeader *msg = NULL;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return 1;
	}

    msg = (VosMsgHeader *)malloc(sizeof(VosMsgHeader));
    memset(msg, 0, sizeof(VosMsgHeader));
    if (msg)
    {
        msg->dataLength = 0;
        msg->dst = EID_TIMER_TASK;
        msg->src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
        msg->type = VOS_MSG_DEL_WIFI_TIMER_TASK;
        msg->wordData = taskId;
        vosMsg_send(g_msgHandle, msg);
        free(msg);
    }
    vosMsg_cleanup(&g_msgHandle);
}

void ProcEditWifiTimer(TASK_INFO_STRU *pstTaskInfo)
{
    VosMsgHeader *msg = NULL;
    UINT8 ret;
    
    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return;
	}

    msg = (VosMsgHeader *)malloc(sizeof(VosMsgHeader) + sizeof(TASK_INFO_STRU));
    memset(msg, 0, sizeof(VosMsgHeader) + sizeof(TASK_INFO_STRU));
    if (msg)
    {
        memcpy(msg+1, pstTaskInfo, sizeof(TASK_INFO_STRU));
        msg->dataLength = sizeof(TASK_INFO_STRU);
        msg->dst = EID_TIMER_TASK;
        msg->src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
        msg->type = VOS_MSG_EDIT_WIFI_TIMER_TASK;
        msg->flags_request = 1;
        vosMsg_send(g_msgHandle, msg);
        free(msg);
    }
    vosMsg_cleanup(&g_msgHandle);
}

void ProcSetWifiTimerSw(TASK_SW_INFO *pstTaskSwInfo)
{
    UINT8 ret;
    VosMsgHeader *msg = NULL;

    ret = vosMsg_init(EID_CGIREQ, &g_msgHandle);
	if(ret != VOS_RET_SUCCESS)
	{
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		vosMsg_cleanup(&g_msgHandle);
		return 1;
	}

    msg = (VosMsgHeader *)malloc(sizeof(VosMsgHeader) + sizeof(TASK_SW_INFO));
    memset(msg, 0, sizeof(VosMsgHeader) + sizeof(TASK_SW_INFO));
    if (msg)
    {
        msg->dataLength = sizeof(TASK_SW_INFO);
        msg->dst = EID_TIMER_TASK;
        msg->src = MAKE_SPECIFIC_EID(getpid(), EID_CGIREQ);
        msg->type = VOS_MSG_SET_WIFI_TIMER_TASK_SW;
        memcpy(msg+1, pstTaskSwInfo, sizeof(TASK_SW_INFO));

        vosMsg_send(g_msgHandle, msg);
        free(msg);
    }
    vosMsg_cleanup(&g_msgHandle);
}


