#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <libwebsockets.h>
#include <librtcfg.h>
#include "cJSON.h"
#include <libubus.h>
#include <libubox/uloop.h>
#include <libubox/list.h>  
#include <libubox/blobmsg_json.h>  
#include <json-c/json.h>
#include <systeminfo_get.h>
#include "zigbee.h"
#include "basic.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>  
#include <sys/stat.h>
#include "qtecstruct.h"

struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[4096];
};


int dmwriteback(char *msg)
{
	struct VosMsgBody stMsg = {0};
	if (g_msgHandle == NULL)
		return -1;
		
	DEBUG_PRINTF_GRE("[%s] dmsendmsg2mw:%s\r\n", __func__,msg);
	
	stMsg.stHead.dataLength = strlen(msg)+1;
	stMsg.stHead.dst = EID_MYWEBSOCKET;
	stMsg.stHead.src = EID_QTECDEVICEMANAGER;
	stMsg.stHead.type = 0x10009527;
	stMsg.stHead.flags_response = 1;
	memcpy(stMsg.buf, msg, strlen(msg)+1);
	vosMsg_send(g_msgHandle, &stMsg);
}

int dodevres(char *session, char *data, char *msg, int code)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
	char str[2048];
	char *format = NULL;
	char encryptmsg[2048];
	char *encryptformat;	
	
	encryptformat = "{\\\"data\\\":{%s},\\\"msg\\\":\\\"%s\\\", \\\"code\\\":%d}";
	sprintf(encryptmsg, encryptformat, data, msg, code);

	format = "{\"sessionId\":\"%s\",\"method\":\"response\", \"data\":{\"encryptInfo\":\"%s\",\"encryption\":\"0\", \"keyId\":\"\", \"userid\":\"\", \"deviceid\":\"\"}}";
	sprintf(str, format, session, encryptmsg);

	dmwriteback(str);
    return 0;
}

int handledevicenotfound(char * input_deviceid )
{
    DEBUG_PRINTF("[%s]=======input_deviceid: %s\n",__func__,input_deviceid);
    char str[20480];
    
    char *format = "{\"method\":\"request\", \"data\":{\"url\":\"/route/routerupload/handledevicenotfound\", \"deviceSerialNo\":\"%s\"}}";
	sprintf(str, format, input_deviceid);

    dmwriteback(str);
	return 0;
}
int updatedevlist()
{	
    DEBUG_PRINTF("[%s]=======\n",__func__);
	char str[20480];
	char devinfolist[20480];
	char devinfo[1024];
	int bfirst = 0;
	
	memset(devinfolist,0, sizeof(devinfolist));
	
	char *devformat;
	struct simpleDeviceEntry *pdev = NULL;
	for(pdev = global_ManagedDeviceEntryList_head; pdev!= NULL; pdev=pdev->root.next)
	{
		devformat = "{\"deviceSerialNo\":\"%s\", \"deviceVersion\":\"%s\", \"deviceModel\":\"%s\", \"deviceType\":%d, \"deviceName\":\"%s\"}";
		sprintf(devinfo, devformat, pdev->deviceid, pdev->version, pdev->model, pdev->type, pdev->name);
        DEBUG_PRINTF("===[%s]===devinfo:%s====\n",__func__,devinfo);
		if (bfirst == 0)
		{	
			strcpy(devinfolist, devinfo);
			bfirst = 1;
		}
		else
		{
			strcat(devinfolist, ",");			
			strcat(devinfolist, devinfo);
		}
	}
	
	char *format = "{\"method\":\"request\", \"data\":{\"url\":\"/route/routerupload/updatedeviceinfo\", \"deviceSerialNoList\":[%s]}}";
	sprintf(str, format, devinfolist);

	dmwriteback(str);
		
	return 0;
}

int updatelog(char * time, int opratetype, char* usrid, int code, char *devid)
{	  
	char str[2048];
	int len;	
	
	char *format = "{\"method\":\"request\", \"data\":{\"url\":\"/route/routerupload/informationreport\", \"occurTime\":\"%s\", \"oprateType\":\"%d\", \"userUniqueKey\":\"%s\", \"operateCode\":\"%d\", \"deviceSerialNo\":\"%s\"}}";
	sprintf(str, format, time, opratetype, usrid, code, devid);

	dmwriteback(str);
		
	return 0;
}

int ProcrenameFP(char *session, char *devid ,char *usrid, char *fpid, char *fpname)
{
	int index;
	index = RenameFingerPrint(devid, usrid, fpid, fpname);	
	if (index != -1)
	{	
		dodevres(session, "", "ok", 0);
	}
	else
		dodevres(session, "", "fingerprint not found", 1);
	return 0;
}

int ProcDevRename(char *session, char *devid, char* devname)
{
	int iRet;
	iRet = RenameDevName(devid, devname);
	if (0 == iRet)	
		dodevres(session, "", "ok", 0);	
	else
		dodevres(session, "", "dev not found", DEVMANAGER_DEVICE_NOT_FOUND);
	return 0;
}

int ProcgetdevSearch(char *session)
{	
	char devinfolist[2048];
	char devinfo[128];
	int bfirst = 0;
	char str[2048];
	char *devformat;
	struct simpleDeviceEntry *pdev = NULL;

	memset(devinfolist, 0, 2048);

	for(pdev = global_searchedDeviceEntryList_head; pdev!= NULL; pdev=pdev->root.next)
	{
		devformat = "{\\\"devid\\\":\\\"%s\\\", \\\"devname\\\":\\\"%s\\\"}";
		sprintf(devinfo, devformat, pdev->deviceid, pdev->name);
		if (bfirst == 0)
		{
			strcpy(devinfolist, devinfo);
			bfirst = 1;
		}
		else
		{
			strcat(devinfolist, ",");			
			strcat(devinfolist, devinfo);
		}
	}

	sprintf(str, "\\\"devlist\\\":[%s]", devinfolist);
	
	dodevres(session, str, "ok", 0);	
	
	return 0;
}

int ProcgetFP(char *session,char * devid ,char *usrid)
{
	int i;
	int num;
	int bfirst = 0;
	char *format;
	char fpinfolist[2048];
	char fpinfo[128];
	char str[2048];
	struct FingerPrintEntry fplist[20];

	memset(fpinfolist, 0, 2048);

	num = GetFingerPrintsByUserIdDeviceId(devid, usrid, fplist);
	for (i = 0; i<num; i++)
	{
		format = "{\\\"fpid\\\":\\\"%s\\\", \\\"fpname\\\":\\\"%s\\\"}";
		sprintf(fpinfo, format, fplist[i].fingerprintid, fplist[i].name);
		if (bfirst == 0)
		{
			strcpy(fpinfolist, fpinfo);
			bfirst = 1;
		}
		else
		{
			strcat(fpinfolist, ",");			
			strcat(fpinfolist, fpinfo);
		}
	}
	
	sprintf(str, "\\\"fingerprintlist\\\":[%s]", fpinfolist);
		
	dodevres(session, str, "ok", 0);
}

//解析从websocket 获取的信息
static void dorouterdev(char *session, cJSON *data_value)
{
    DEBUG_PRINTF("==[%s]====session: %s ====\n",__func__,session);
	char *msg;
	cJSON *json, *data;
	char* devid;
	char* usrid;
	char* fpname;
	char *fingerprintid;
	char *target;	
	char *encrypt;
	char *keyId;
	char *devname;
    char *in_data;
	
	target = cJSON_GetObjectItem(data_value, "requestUrl")?cJSON_GetObjectItem(data_value, "requestUrl")->valuestring:"";
	data = cJSON_GetObjectItem(data_value, "data");
	
	if(!strcmp(target,"devadd"))
	{
        DEBUG_PRINTF("===[%s]====devadd====\n",__func__);
		devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
        dodevres(session, "", "add process start", 0);
		ZBDevAddReq(session, devid);		
	}
	else if (!strcmp(target, "devunbound"))
	{
		devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
        dodevres(session, "", "unbound process start", 0);
        ZB_DevDelReq(devid, 1);
		//ZBDevDelReq(session, devid);		
	}
    else if(!strcmp(target,"devcheck"))
    {   
        int ret=0;
        char str[256]={0};
        devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
        ret = ZB_DevCheck(devid);
        sprintf(str, "\\\"contained\\\":%d", ret);     
        dodevres(session, str, "ok", 0);
    }
    #if 0
	else if(!strcmp(target,"devsearch"))
	{
		ZBDevSearchReq(session);		
	}
	else if(!strcmp(target,"getdevsearch"))
	{
		ProcgetdevSearch(session);
	}
	else if (!strcmp(target, "devrename"))
	{
		devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
		devname = cJSON_GetObjectItem(data, "devname")?cJSON_GetObjectItem(data, "devname")->valuestring:"";
		ProcDevRename(session, devid, devname);
	}
	else if (!strcmp(target, "addfingerprint"))
	{
		devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
		usrid = cJSON_GetObjectItem(data, "usrid")?cJSON_GetObjectItem(data, "usrid")->valuestring:"";
		ZBAddFPReq(session, usrid, devid);				
	}
	else if (!strcmp(target, "delfingerprint"))
	{
		devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
		usrid = cJSON_GetObjectItem(data, "usrid")?cJSON_GetObjectItem(data, "usrid")->valuestring:"";
		fingerprintid = cJSON_GetObjectItem(data, "fingerprintid")?cJSON_GetObjectItem(data, "fingerprintid")->valuestring:"";
		ZBDelFPReq(session, devid, usrid, fingerprintid);				
	}
	else if (!strcmp(target, "getfingerprint"))
	{
		devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
		usrid = cJSON_GetObjectItem(data, "usrid")?cJSON_GetObjectItem(data, "usrid")->valuestring:"";
		ProcgetFP(session, devid, usrid);			
	}
	else if (!strcmp(target, "renamefingerprint"))
	{
		devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
		usrid = cJSON_GetObjectItem(data, "usrid")?cJSON_GetObjectItem(data, "usrid")->valuestring:"";
		fpname = cJSON_GetObjectItem(data, "name")?cJSON_GetObjectItem(data, "name")->valuestring:"";
		fingerprintid = cJSON_GetObjectItem(data, "fingerprintid")?cJSON_GetObjectItem(data, "fingerprintid")->valuestring:"";
		ProcrenameFP(session, devid, usrid, fingerprintid, fpname);		
	}
    #endif
    else if (!strcmp(target,"devaction"))
    {
        int len=0;
        int len2=0;
        char *encrypdata;
        unsigned char bindata[256]={0};
        devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
        len = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "len")->valueint:"";
        encrypdata= cJSON_GetObjectItem(data, "encrypdata")?cJSON_GetObjectItem(data, "encrypdata")->valuestring:"";
        DEBUG_PRINTF("[%s]======encrypdata: %s========\n",__func__,encrypdata);
        len2=base64_decode(encrypdata, bindata);
        if(len2 != len)
        {
            DEBUG_PRINTF("[%s]!!! error==== len: %d   len2: %d ====\n",__func__,len,len2);
        }
        ZB_DevAction(session,devid,bindata,len);
    }
    #if 0
	else if (!strcmp(target, "unlock"))
	{
		//usrid = cJSON_GetObjectItem(data, "usrid")?cJSON_GetObjectItem(data, "usrid")->valuestring:"";
		devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
        in_data = cJSON_GetObjectItem(data,"encryinfo")?cJSON_GetObjectItem(data, "encryinfo")->valuestring:"";
        ZB_DevUnlockReq(session, devid,in_data);
		//ZBDevUnlockReq(session, devid, usrid);
	}
	else if (!strcmp(target, "lockstatus"))
	{
		devid = cJSON_GetObjectItem(data, "devid")?cJSON_GetObjectItem(data, "devid")->valuestring:"";
        in_data = cJSON_GetObjectItem(data,"encryinfo")?cJSON_GetObjectItem(data, "encryinfo")->valuestring:"";
        ZB_DevStatusReq(session, devid, in_data);
		//ZBDevStatusReq(session, devid);
	}
    #endif 
    
    #if 0
	else if (!strcmp(target, "keypass"))
	{
		//....................................
	}	
    #endif
	else
	{
		dodevres(session, "", "invalid cmd", DEVMANAGER_INVALID_CMD);
	}


	return;
}

static void decodemessage(char *msg)
{
    DEBUG_PRINTF("[%s]=====msg:%s====\n",__func__,msg);
	cJSON *json, *data_value;
	char *url, *method;
	char *session;
	
	json = cJSON_Parse(msg);
	if (!json)
	{
		return; 
	}

	printf("dmreceivejsonmsg:%s\r\n", cJSON_Print(json));
	
	session = cJSON_GetObjectItem(json, "sessionId")?cJSON_GetObjectItem(json, "sessionId")->valuestring:"";
	data_value = cJSON_GetObjectItem(json, "data");

	dorouterdev(session, data_value);	
	cJSON_Delete(json);
	return;
}

static void *pthread_routine(void *arg)
{
	initGdata();

	//openTCP();

	openUART();
	
	readUART();

	closeUART();
}

#define devmanager_logfile "/tmp/.devmanager"

void init_log()
{
	FILE *f1;
	if(access(devmanager_logfile,F_OK) !=0)
	{	
		return;
	}
	f1 = open(devmanager_logfile, O_RDWR | O_APPEND);

	if(f1!=NULL)
	{
		dup2(f1,1);
		dup2(f1,2);

		close(f1);
	}
	
}




int main(int argc, char **argv)
{
	int ret=0;
	int n = -1;
	int commFd = -1;
	int maxFd = -1;
	int fd, rv;
	fd_set readFdsMaster,rfds;
	pthread_t pid;
    pthread_t pid2;
	VosMsgHeader *msg = NULL;
	struct VosMsgBody *body = NULL;

	init_log();
	vosLog_init(EID_QTECDEVICEMANAGER);
	vosLog_setDestination(VOS_LOG_DEST_STDERR);
	vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);

	ret=vosMsg_init(EID_QTECDEVICEMANAGER, &g_msgHandle);

	if(ret != VOS_RET_SUCCESS)
	{
		vosLog_error("dm msg initialization failed, ret= %d", ret);
		return ret;
	}
	
	pthread_create(&pid, NULL, pthread_routine, NULL);
	pthread_detach(pid);
	
	vosMsg_getEventHandle(g_msgHandle, &commFd);
	FD_ZERO(&readFdsMaster);
	FD_SET(commFd, &readFdsMaster);
	maxFd = commFd;

    //test code
    #if 0
    sleep(5);
    int test=0x01020304;
    char *a=&test;
    if(a[0]==0x01)
    {
        printf("===big===\n");
    }
    else 
    {
        printf("===little===\n");
    }
    printf("===short: %d ====\n",sizeof(short));
    printf("====int: %d======\n",sizeof(int));
    ZBDevAddReq("1234", "00124B000F8C8916");
    updatedevlist();
   // ZB_DevDelReq("00124B000F8C8916",1);
   #endif
    sleep(5);
    qtec_test();
   
	while(1)
	{
		rfds = readFdsMaster;
		n = select(maxFd+1, &rfds, NULL, NULL, NULL);
		if (n < 0)
		{
			continue;
		}
		
		if (FD_ISSET(commFd, &rfds))
		{
			ret = vosMsg_receive(g_msgHandle, &msg);
			if (ret != VOS_RET_SUCCESS)
			{
				continue;
			}
			switch(msg->type)
			{
				case 0x10009527:
				{
					body = msg;
					decodemessage(body->buf);
                    //pthread_create(&pid, NULL, decodemessage, (void *)(body->buf));
					break;
				}
				default:
					break;
			}
			if (NULL!= msg)
			{				
				VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);				
			}
		}
	}
	vosMsg_cleanup(&g_msgHandle);
    
	return ret;
}
