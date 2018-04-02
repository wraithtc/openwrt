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
#include "dbbasic.h"
#include <keyapi.h>
#include <basic.h>
#include <private-libwebsockets.h>
#include <sec_api.h>

#include "signal.h"





struct VosMsgBody
{
	VosMsgHeader stHead;
	char buf[4096];
};

typedef struct{
    char appUser[128];
    char appChannelId[128];
    char serverIp[128];
    int port;
}TCP_PROXY_INFO;

struct encodeinfo
{
	char sessionid[100];
	char devid[50];
	char usrid[50];
    unsigned char keyid[50];
    int  reuseflag;
};

typedef enum _KeyType
{
	POOL_TYPE_RAW = 1,
	POOL_TYPE_SYNC
}KeyType;

typedef enum _UpgradeFlag
{
    REQ_NO = 0,
	REQ_FROM_APP,
	REQ_FROM_SERVER
}UpgradeFlag;

typedef enum{
    DIST_CONNECT = 0,
    DIST_CONNECTED,
    SERVER_CONNECT,
    SERVER_CONNECTED,
    GAME_OVER
}PROC_STATUS_E;

static struct lws *wsi_lws = NULL;
static struct lws_client_connect_info dist_client_info;
static struct lws_client_connect_info client_info;

static long int timeold;
static unsigned int reqid = 1;
static char devserialnum[64];
static char sendbuf[41][81920];
static struct encodeinfo encodelist[40];
void *g_mcHandle =NULL;
void *g_msgHandle = NULL;
static int idblock = 0;

//global variables used for update info request
static int g_reuseflag = 0;
static char g_sessionid[64] = {0};
static char g_session[64] = {0};
static char g_devid[64] = {0};
static char g_usrid[64] = {0};
static char g_target[64] = {0};
static unsigned char g_keyid[64] = {0};
static UpgradeFlag g_upgeadeflag = REQ_NO;
static char g_token[256] = {0};
static char serverUrl[512] = {0};
static unsigned short serverPort = 0;
static PROC_STATUS_E g_status = DIST_CONNECT;
int g_serverConnCount = 0;
int g_demoServer = 0; //曹总专享服务器？1是0否
int g_testServer = 0; //测试服务器or现网服务器,1:测试服务器，0：现网服务器(阿里云)
struct lws_context *g_distContext;

void write_log(const char *fmt, ...)
{
    int ret = 0;
    char buf[4096] = {0};
    char *cmd = NULL;
    char *allocBuf = NULL;
    va_list paraList;
    FILE *fp;

    if (access("/etc/myclientprint", F_OK) != 0)
        return;
    
    va_start(paraList, fmt);
    vsnprintf(buf, sizeof(buf), fmt, paraList);
    va_end(paraList);

    fp = fopen("/etc/myclientprint", "a");
    fwrite(buf, sizeof(buf), 1, fp);
    fclose(fp);
}


static int myclient2server()
{
    //DEBUG_PRINTF("====[%s]=====\n",__func__);
    if(wsi_lws==NULL)
    {
        DEBUG_PRINTF("[%s]====wsi_lws is null=====\n",__func__);
        return -1;
    }
    DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
    if(wsi_lws->state == LWSS_ESTABLISHED)
    {
        lws_callback_on_writable(wsi_lws);
        return 0;
    }
    return -1;
}

#if defined(LWS_OPENSSL_SUPPORT) && defined(LWS_HAVE_SSL_CTX_set1_param)
char crl_path[1024] = "";
#endif

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
static char* getserialnum()
{
	if (0 == devserialnum[0])
	{
		struct systemInfo output;
		getSystemInfo(&output);
		memcpy(devserialnum, output.serialnum, 64);
	}
		
	return devserialnum;
}

static int lwswriteback(struct lws *wsi_in) 
{

	int n;
	int len;
	char *out = NULL;
	char *str = NULL;
	int i;
    int found = 0;
    int pos = 41;
	for (i = 0; i<41; i++)
	{
		if (sendbuf[i][0] != 0 && !found)
		{
			str = strdup(sendbuf[i]);
			found = 1;
            pos = i;
		}

        /* 保持紧凑排列，后面元素往前移一位 */
        //存储缓冲区时，只能填满前面40位
        if (i > pos)
        {
            memcpy(sendbuf[pos], sendbuf[i], strlen(sendbuf[i]) + 1);
            pos = i;
        }
	}

	if (str == NULL)
		return 0;
	
	len = strlen(str); 
    
	DEBUG_PRINTF("[%s]mw send msg buf[%d] 2 server (%d):%s\r\n",__func__, i, strlen(str)+1, str);

	out = (char *)malloc(sizeof(char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));
	memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );
	free(str);//memset(str, 0, len+1);
	n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);
	free(out);


    //??ô????Ϊ?˷?ֹlws_write??��?????????ݲ???һ????
    /**
    DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
    if(wsi_lws->state == LWSS_ESTABLISHED)
    {
        lws_callback_on_writable(wsi_lws);
    }
	*/
	myclient2server();

	return n;
}

static int addstr2buf(char *str)
{
	char * buf = NULL;
	int i = 0;
	for (i = 0;i<40; i++)
	{
		if (sendbuf[i][0]==0)
		{
			buf = sendbuf[i];
			break;
		}
	}       
	
	if(i >= 40)
	{
        DEBUG_PRINTF("[%s]====warning !!!! give up msg:%s\n",__func__,str);
		DEBUG_PRINTF("warning!!!!!!!!!!!!!!!!!!!!: give up msg:%s\r\n", str);
		return -1;
	}
    DEBUG_PRINTF("[%s] add 2 buf [%d] (%d): %s\n",__func__,i,strlen(str)+1,str);
	//DEBUG_PRINTF("add 2 buf[%d] (%d):%s\r\n", i, strlen(str)+1, str);
	memcpy(buf, str, strlen(str)+1);
	return 0;
}

static char *decodeencryptmsg(char * msg, char* decodemsg, unsigned char *keyId, char *userid, char* deviceid, int reuseflag)
{
	while (idblock == 1)
	{
		sleep(1);
	}
	idblock = 1;
	char basemsg[1024] = {0};
	char decodem[1024] = {0};
	struct tagCQtQkMangent *pstcqtqkmangent;
	struct tagCQtKeyEncrypt *pstcqtkeyencrypt;
	unsigned char key[17]={0};
	int msglen = 1024;
	int decodelen = 0;
    int basemsglen = 0;
	int ret = 0;
    cJSON * tmp_cjson=NULL;
    char * tmp_cjson_string=NULL;
	
	pstcqtqkmangent = GetCQtQkMangent();
	pstcqtkeyencrypt = GetCQtKeyEncrypt();
	DEBUG_PRINTF("[%s]: decodemsg pre:%s\r\n", __func__, msg);
	memset(basemsg, 0, 1024);
	decodelen = lws_b64_decode_string(msg, basemsg, 1024);
    basemsglen = (decodelen + 15) / 16 * 16;
	//DEBUG_PRINTF("decodemsg afterbase64:%s\r\n", basemsg);
	ret = C_GetKeyByIdNode(pstcqtqkmangent, POOL_TYPE_SYNC, 1, userid, deviceid, keyId, key);

	//DEBUG_PRINTF("decodemsg getkey(%d): %s\r\n", ret, key);
	//C_DeleteKeyById(pstcqtqkmangent, 1, POOL_TYPE_SYNC, keyId);
	memset(decodemsg, 0, 1024);
	ret = C_EncryptOrDecrypt(pstcqtkeyencrypt, decodem, &msglen, basemsg, basemsglen, 0, key);
    tmp_cjson=cJSON_Parse(decodem);
    if ( tmp_cjson!= NULL)
    {
        tmp_cjson_string=cJSON_PrintUnformatted(tmp_cjson);
		strcpy(decodemsg, tmp_cjson_string);
        if(tmp_cjson!=NULL)
        {
            cJSON_Delete(tmp_cjson);
        }
        if(tmp_cjson_string !=NULL)
        {
            free(tmp_cjson_string);
        }
    }
    DEBUG_PRINTF("[%s] decodemsg last(%d):%s\r\n",__func__, ret, decodemsg);
	
	
	ReleaseCQtKeyEncrypt(&pstcqtkeyencrypt);
	ReleaseCQtQkMangent(&pstcqtqkmangent);
	idblock = 0;
}

static char *encodeencryptmsg(char * msg, char* encodemsg, unsigned char *keyId, char *userid, char* deviceid, int reuseflag)
{
    DEBUG_PRINTF("[%s]=====msg:%s  keyId:%s userid:%s deviceid:%s reuseflag:%d====\n",__func__,msg,keyId,userid,deviceid,reuseflag);
	while (idblock == 1)
	{
        DEBUG_PRINTF("[%s]====idblock is 1====\n",__func__);
		sleep(1);
	}
	idblock = 1;
    char *msgcpy, *basemsg;
    int msglen, basemsglen, encodemsglen;
	struct tagCQtQkMangent *pstcqtqkmangent;
	struct tagCQtKeyEncrypt *pstcqtkeyencrypt;
	unsigned char key[17]={0};
	int ret = 0;


	msglen = (strlen(msg) + 16) / 16 * 16;
    basemsglen = msglen;
    encodemsglen = 2 * msglen;

    msgcpy = malloc(msglen);
	memset(msgcpy, 0, msglen);
	memcpy(msgcpy, msg, strlen(msg));
    basemsg = malloc(basemsglen);
	memset(basemsg, 0, basemsglen);
	DEBUG_PRINTF("srclen = %d\r\n", msglen);
	pstcqtqkmangent = GetCQtQkMangent();
	pstcqtkeyencrypt = GetCQtKeyEncrypt();
	DEBUG_PRINTF("encodemsg pre:%s\r\n", msgcpy);	
    GetNextSynKeyId(keyId);
	ret = C_GetKeyByIdNode(pstcqtqkmangent, POOL_TYPE_SYNC, 1, userid, deviceid, keyId, key);

	//DEBUG_PRINTF("encodemsg getkey(%d): %s: %s\r\n", ret, keyId, key);
    
	ret = C_EncryptOrDecrypt(pstcqtkeyencrypt, basemsg, &basemsglen, msgcpy, msglen, 1, key);
	lws_b64_encode_string(basemsg, basemsglen, encodemsg, encodemsglen);
	DEBUG_PRINTF("encodemsg last:%s\r\n", encodemsg);

	ReleaseCQtKeyEncrypt(&pstcqtkeyencrypt);
	ReleaseCQtQkMangent(&pstcqtqkmangent);
	idblock = 0;
    free(msgcpy);
    free(basemsg);
	return;
}

static void sendupdatereq(int pid)
{
    cJSON *jsonOut, *obj;
    char session[64] = {0};
    char *buffer, *objbuf;
    DEBUG_PRINTF("enter sendupdatereq.\n");
    jsonOut = cJSON_CreateObject();
	obj = cJSON_CreateObject();
    sprintf(session, "%s%d", getserialnum(), reqid++);
    memcpy(g_sessionid, session, sizeof(g_sessionid));
    
    cJSON_AddItemToObject(jsonOut, "method", cJSON_CreateString("request"));
	cJSON_AddItemToObject(obj, "url", cJSON_CreateString("/version/routerupgrade"));
	cJSON_AddItemToObject(obj, "pid", cJSON_CreateNumber(pid));
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(jsonOut, "sessionId", cJSON_CreateString(session));
    buffer = cJSON_PrintUnformatted(jsonOut);
	DEBUG_PRINTF("sendupdatereq buffer is %s.\n", buffer);
    addstr2buf(buffer);
    #if 0
	DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
    if(wsi_lws->state == LWSS_ESTABLISHED)
    {
        lws_callback_on_writable(wsi_lws);
    }
    #endif 
    myclient2server();
    free(buffer);
	if(jsonOut)
	{
		cJSON_Delete(jsonOut);
	}
    return;
}

static void procupdateinfo(cJSON *data_value, cJSON *jsonOut)
{
    char *downloadUrl, *versionNo;
    char buffer[256] = {0};
	char cmd[256] = {0};
	char str[64];
    char localversion[64] = {0};
    int ret = 0;
    cJSON *obj;
    FILE *fp = NULL;

    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
   
    downloadUrl = cJSON_GetObjectItem(data_value, "downloadUrl")?cJSON_GetObjectItem(data_value, "downloadUrl")->valuestring:"";
    versionNo = cJSON_GetObjectItem(data_value, "versionNo")?cJSON_GetObjectItem(data_value, "versionNo")->valuestring:"";

    //淇濆瓨downloadurl淇℃伅
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
		system(cmd);
    }
    
}


//获取speedtest url 
static void sendspeedtesturlreq()
{
    cJSON *jsonOut, *obj;
    char session[64] = {0};
    char *buffer, *objbuf;
    DEBUG_PRINTF("enter sendspeedtesturlreq.\n");
    jsonOut = cJSON_CreateObject();
	obj = cJSON_CreateObject();
    sprintf(session, "%s%d", getserialnum(), reqid++);
    //不比较返回的sessionid
    //memcpy(g_sessionid, session, sizeof(g_sessionid));
    
    cJSON_AddItemToObject(jsonOut, "method", cJSON_CreateString("request"));
	cJSON_AddItemToObject(obj, "url", cJSON_CreateString("/route/routerupload/speedroadaddress"));
	
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(jsonOut, "sessionId", cJSON_CreateString(session));
    buffer = cJSON_PrintUnformatted(jsonOut);
	DEBUG_PRINTF("sendupdatereq buffer is %s.\n", buffer);
    addstr2buf(buffer);
    #if 0
	DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
    if(wsi_lws->state == LWSS_ESTABLISHED)
    {
        lws_callback_on_writable(wsi_lws);
    }
    #endif 
    myclient2server();
    free(buffer);
	if(jsonOut)
	{
		cJSON_Delete(jsonOut);
	}
    return;
}

static void prospeedtesturl(cJSON *data_value)
{
    DEBUG_PRINTF("====%s====",__func__);
    char *address;
    char cmd[256]={0};
    address = cJSON_GetObjectItem(data_value, "address")?cJSON_GetObjectItem(data_value, "address")->valuestring:"";
    if(strlen(address)!=0)
    {
        memset(cmd,0,256);
        snprintf(cmd,256,"system.@system[0].speedtesturl=%s",address);
        rtcfgUciSet(cmd);
    }


}

int dortcfgres(char *session, char * deviceid, char* userid, unsigned char* keyid, cJSON *jsonOut, int code, int reuseflag, char* url)
{
	char *format = NULL;
	char *encodemsg =NULL;
    char *source = NULL;
    char *str = NULL;
    int  encodemsglen;
	int encryption = 0;
	char *pData;
	char msg[256] = {0};
      

	err2msg(code,msg,sizeof(msg));
    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString(msg));
	cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(code));

	if (NULL == cJSON_GetObjectItem(jsonOut, "data"))
		cJSON_AddItemToObject(jsonOut, "data", cJSON_CreateObject());

    source = cJSON_PrintUnformatted(jsonOut);
    DEBUG_PRINTF("[%s]=====source is %s======\n",__func__,source);
	if (userid[0] != 0)
	{
        encodemsglen = 2 * strlen(source);
        encodemsg = malloc(encodemsglen);
		memset(encodemsg, 0, encodemsglen);
		encodeencryptmsg(source, encodemsg, keyid, userid, deviceid, reuseflag);
		encryption = 1;
		pData = encodemsg;
	}
	else
	{
		pData = source;
	}

    str = malloc(encodemsglen +1024);
    memset(str, 0, encodemsglen +1024);
	if(encryption)
	{
		format = "{\"sessionId\":\"%s\",\"method\":\"response\", \"data\":{\"encryptInfo\":\"%s\",\"encryption\":\"%d\",\"keyinvalid\":%d, \"keyId\":\"%s\", \"userid\":\"%s\", \"deviceid\":\"%s\"}}";
	}
	else
	{
		format = "{\"sessionId\":\"%s\",\"method\":\"response\", \"data\":{\"encryptInfo\":%s,\"encryption\":\"%d\",\"keyinvalid\":%d, \"keyId\":\"%s\", \"userid\":\"%s\", \"deviceid\":\"%s\"}}";
	}
	sprintf(str, format, session, pData, encryption, 0, keyid, userid, deviceid);

	//lwswriteback(wsi_lws, str);
	addstr2buf(str);
    #if 0
    DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
    if(wsi_lws->state == LWSS_ESTABLISHED)
    {
        lws_callback_on_writable(wsi_lws);
    }
    #endif
    myclient2server();

    if(str !=NULL)
    {   
        free(str);
    }
    if( encodemsg !=NULL)
    {
        free(encodemsg);
    }
    if( source !=NULL)
    {
        free(source);
    }


}


static int sendmsg2dm(char *msg)
{
	struct VosMsgBody stMsg = {0};

	//DEBUG_PRINTF("mwsendmsg2dm:%s\r\n", cJSON_Print(cJSON_Parse(msg)));
    DEBUG_PRINTF("[%s]mwsendmsg2dm:%s\r\n",__func__,msg);
    
    stMsg.stHead.dataLength = strlen(msg)+1;
	stMsg.stHead.dst = EID_QTECDEVICEMANAGER;
	stMsg.stHead.src = EID_MYWEBSOCKET;
	stMsg.stHead.type = 0x10009527;
	stMsg.stHead.flags_request = 1;
	memcpy(stMsg.buf, msg, strlen(msg)+1);
	vosMsg_send(g_mcHandle, (VosMsgHeader *)&stMsg);
	
}

static void sendmsg2cgi(char *msg, unsigned int pid)
{
	struct VosMsgBody stMsg = {0};

	//DEBUG_PRINTF("mwsendmsg2cgi:%s\r\n", cJSON_Print(cJSON_Parse(msg)));
	DEBUG_PRINTF("mwsendmsg2cgi:%s\r\n", (msg));
	stMsg.stHead.dataLength = strlen(msg)+1;
	stMsg.stHead.dst = pid;
	stMsg.stHead.src = EID_MYWEBSOCKET;
	stMsg.stHead.type = VOS_MSG_UPDATE_GETINFO;
	stMsg.stHead.flags_response = 1;
	memcpy(stMsg.buf, msg, strlen(msg)+1);
	vosMsg_send(g_mcHandle, (VosMsgHeader *)&stMsg);	
    return;
}

//special care changed \CC\E1\D0\D1
int update_specialcare(char *macaddress,char *hostname, char *devicetype, int operatetype)
{
    DEBUG_PRINTF("[%s]=====macaddress:%s  hostname:%s devicetype:%s===== operatetype: %d===\n",__func__,macaddress,hostname, devicetype, operatetype);
    char str[2048];
	int len;
	int iRet = 0;

	char *format = "{\"sessionId\":\"%s%d\",\"method\":\"request\", \"data\":{\"url\":\"/route/routerupload/specialfocus\", \"routeSerialNo\":\"%s\", \"mac\":\"%s\", \"hostname\":\"%s\",\"devicetype\":\"%s\",\"oprateType\":\"%d\"}}";
	sprintf(str, format, getserialnum(), reqid++, getserialnum(), macaddress, hostname,devicetype,operatetype);

	//len = lwswriteback(wsi_lws, str);
	//if (len < 0)
	//{
		//savelog2local(time, opratetype, usrid, devid);
		//iRet = -1;
	//}
	addstr2buf(str);
    #if 0
	DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
    if(wsi_lws->state == LWSS_ESTABLISHED)
    {
        lws_callback_on_writable(wsi_lws);
    }
    #endif

    myclient2server();

	return iRet;
}

int updatelog(char * time, int opratetype, char* usrid, int code, char *devid)
{	  
    DEBUG_PRINTF("[%s]===time:%s opratetype:%d usrid:%s code:%d, devid:%s====\n",__func__,time,opratetype,usrid,code,devid);
	char str[2048];
	int len;
	int iRet = 0;

	char *format = "{\"sessionId\":\"%s%d\",\"method\":\"request\", \"data\":{\"url\":\"/route/routerupload/informationreport\", \"routeSerialNo\":\"%s\", \"occurTime\":\"%s\", \"oprateType\":\"%d\", \"userUniqueKey\":\"%s\", \"operateCode\":\"%d\", \"deviceSerialNo\":\"%s\"}}";
	sprintf(str, format, getserialnum(), reqid++, getserialnum(), time, opratetype, usrid, code, devid);

	//len = lwswriteback(wsi_lws, str);
	//if (len < 0)
	//{
		//savelog2local(time, opratetype, usrid, devid);
		//iRet = -1;
	//}
	addstr2buf(str);
    #if 0
	DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
    if(wsi_lws->state == LWSS_ESTABLISHED)
    {
        lws_callback_on_writable(wsi_lws);
    }
    #endif

    myclient2server();

	return iRet;
}

int updateoldlog()
{
    DEBUG_PRINTF("[%s]=====\n",__func__);
	int iRet;
	struct LogEntry *plog=NULL;
	
	for (plog = getFirstLog();plog != NULL; DelFirstLog(),plog = getFirstLog())
	{
		iRet = updatelog(plog->time, plog->opratetype,plog->userid, 0, plog->devid);
		if (iRet == -1)
			break;
	}
	
	return 0;	
}

void setencodeinfobysession(char *sessionid, char *usrid, char *devid, unsigned char *keyid, int reuseflag)
{
	int i;
	for (i = 0; i < 40; i ++)
	{
		if (encodelist[i].sessionid[0]==0)
		{
			strcpy(encodelist[i].sessionid, sessionid);
			strcpy(encodelist[i].devid, devid);
			strcpy(encodelist[i].usrid, usrid);
            strcpy(encodelist[i].keyid, keyid);
            //encodelist[i].reuseflag = 0;
            encodelist[i].reuseflag=reuseflag;
			break;
		}
	}
	
}

int  getencodeinfobysession(char *sessionid, char *usrid, char *devid, unsigned char *keyid, int *reuseflag)
{
	int i;
	for (i = 0; i < 40; i ++)
	{
		if (strcmp(encodelist[i].sessionid, sessionid)==0)
		{
			strcpy(devid, encodelist[i].devid);
			strcpy(usrid, encodelist[i].usrid);
            strcpy(keyid, encodelist[i].keyid);
            *reuseflag = encodelist[i].reuseflag;
			memset(&encodelist[i], 0, 200);
			break;
		}
	}

	return i>=40?1:0;
}

static void dorouterdev(struct lws *wsi, char *session, cJSON *data_value)
{
	char *msg;
	cJSON *json, *data, *jsonOut;
	char* devid = "";
	char* usrid = "";
	char* fpname;
	char *fingerprintid;
	char *target;	
	char *encrypt;
	char decodemsg[1024];
	unsigned char *keyId;
	char *devname;
	char *method;
    int reuseflag = 0;
	int ret = 0;
    char str[1024] = {0};
	char *format = NULL;
	
	msg = cJSON_GetObjectItem(data_value, "encryptInfo")?cJSON_GetObjectItem(data_value, "encryptInfo")->valuestring:"";
	encrypt = cJSON_GetObjectItem(data_value, "encryption")?cJSON_GetObjectItem(data_value, "encryption")->valuestring:"0";	

	if (atoi(encrypt) == 1)
	{
		keyId = cJSON_GetObjectItem(data_value, "keyId")?cJSON_GetObjectItem(data_value, "keyId")->valuestring:"";
		devid = cJSON_GetObjectItem(data_value, "deviceId")?cJSON_GetObjectItem(data_value, "deviceId")->valuestring:"";
		usrid = cJSON_GetObjectItem(data_value, "userId")?cJSON_GetObjectItem(data_value, "userId")->valuestring:"";
        reuseflag = cJSON_GetObjectItem(data_value, "reuse")?cJSON_GetObjectItem(data_value, "reuse")->valueint:0;
		decodeencryptmsg(msg, decodemsg, keyId, usrid, devid, reuseflag);
		if (decodemsg[0] == 0)
		{
            /* 解码失败直接返回密码失效 */
			format = "{\"sessionId\":\"%s\",\"method\":\"response\", \"data\":{\"encryptInfo\":\"%s\",\"encryption\":\"%d\",\"keyinvalid\":%d, \"keyId\":\"%s\", \"userid\":\"%s\", \"deviceid\":\"%s\"}}";
	        sprintf(str, format, session, "", 1, 1, keyId, usrid, devid);
            addstr2buf(str);
            #if 0
	        DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
            if(wsi_lws->state == LWSS_ESTABLISHED)
            {
                lws_callback_on_writable(wsi_lws);
            }
            #endif
            myclient2server();
            return;
		}
		else
		{
			msg = decodemsg;
		}
	}

    DEBUG_PRINTF("[%s] decmsgfromserver:%s \n",__func__,msg);
	//DEBUG_PRINTF("decmsgfromserver:%s\r\n", msg);
	
	json = cJSON_Parse(msg);
	if (!json)
	{
		return;	
	}
	
	target = cJSON_GetObjectItem(json, "requestUrl")?cJSON_GetObjectItem(json, "requestUrl")->valuestring:"";
	method = cJSON_GetObjectItem(json, "method")?cJSON_GetObjectItem(json, "method")->valuestring:"";
	data = cJSON_GetObjectItem(json, "data");
    DEBUG_PRINTF("[%s]===requestUrl====%s====method:%s=====data:%s====\n",__func__,target,method,data);
    
	jsonOut = cJSON_CreateObject();
	global_weberrorcode =0;
    
	if((!strcmp(target,"devadd"))    
	  ||(!strcmp(target, "devunbound"))
	  ||(!strcmp(target,"devsearch"))
	  ||(!strcmp(target,"getdevsearch"))
	  ||(!strcmp(target, "devrename"))	
	  ||(!strcmp(target, "addfingerprint"))	
	  ||(!strcmp(target, "delfingerprint"))	
	  ||(!strcmp(target, "getfingerprint"))	
	  ||(!strcmp(target, "renamefingerprint"))	
	  ||(!strcmp(target, "unlock"))	
	  ||(!strcmp(target, "lockstatus"))
	  ||(!strcmp(target, "devcheck"))
	  ||(!strcmp(target, "devaction")))
	{
		if (atoi(encrypt) == 1)
			setencodeinfobysession(session, usrid, devid, keyId, reuseflag);
		char msg2dm[2048];
		sprintf(msg2dm, "{\"sessionId\":\"%s\",\"data\":%s}", session, msg);
		sendmsg2dm(msg2dm);
	}	
	else if(!strcmp(target,"routerstatus"))
	{	
		ret = proc_stalist_get(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"keyrequest"))
	{	
		ret = ProcKeyReq(data, jsonOut, usrid, devid, keyId);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"get_routerbasicinfo"))
	{	
		GetRouterBasicInfoByApp(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"upgrade"))
	{	
		ret = ProcUpdateReq(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"get_update_rate"))
	{	
		proc_queryupgrade(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"check_password_cfg"))
	{
		ret = ProcPasswordCheckByApp(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"set_password_cfg"))
	{	
		ret = ProcPasswordSetByApp(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"cfgrestore"))
	{	
		ProcFirstbootReq(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"routerrestart"))
	{	
		proc_reboot(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"set_wireless_cfg"))
	{	
		ret = ProcWifiSetByApp(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"get_wireless_cfg"))
	{	
		ProcWifiGetByApp(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"set_basicwan_cfg"))
	{	
		ret = ProcWanSetByApp(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}	
	else if(!strcmp(target,"get_wantype_cfg"))
	{	
		ret = ProcWanGetByApp(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"get_timertask_cfg"))
	{	
		proc_timertask_detect(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"set_timertask_cfg"))
	{	
		proc_timertask_set(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
    else if(!strcmp(target,"get_smb_pwd"))
	{	
		ret = proc_get_smb_pwd(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"get_wifi_txpower"))
	{	
		ret = procWifiGetTxpower(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"set_wifi_txpower"))
	{	
		ret = procWifiSetTxpower(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"get_guest_wifi"))
	{	
		ret = procGetGuestWifi(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"set_guest_wifi"))
	{	
		ret = procSetGuestWifi(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"get_update_version"))
	{	
        memset(g_session, 0, 64);
        memset(g_devid, 0, 64);
        memset(g_usrid, 0, 64);
        memset(g_target, 0, 64);
        memset(g_keyid, 0, 64);
        memcpy(g_session, session, 64);
        memcpy(g_devid, devid, 64);
        memcpy(g_usrid, usrid, 64);
        memcpy(g_target, target, 64);
        memcpy(g_keyid, keyId, 64);
        g_reuseflag = reuseflag;
        g_upgeadeflag = REQ_FROM_SERVER;
        sendupdatereq(0);
	}
    else if(!strcmp(target,"proc_childrule_cfg"))
    {
        request_method = methodstr2int(method);
        ret = proc_childrule(data,jsonOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
    }
    else if(!strcmp(target,"proc_wan_speedtest_cfg"))
    {
        request_method = methodstr2int(method);
        ret = proc_wanspeed(data,jsonOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
    }
    else if(!strcmp(target,"proc_specialcare_cfg"))
    {
        request_method = methodstr2int(method);
        ret = proc_specialcare(data,jsonOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
    }
    else if(!strcmp(target,"proc_qos_cfg"))
    {
        request_method = methodstr2int(method);
        ret = proc_qos(data,jsonOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
    }
	else if(!strcmp(target,"proc_macblock_cfg"))
    { 
        request_method = methodstr2int(method);
        ret = proc_macblock(data,jsonOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
    }
    else if(!strcmp(target,"proc_qtec_disk_cfg"))
    { 
        request_method = methodstr2int(method);
        ret = proc_qtec_disk(data,jsonOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
    }
    else if(!strcmp(target,"proc_speedlimit_cfg"))
    {
        request_method = methodstr2int(method);
        ret = proc_speedlimit(data,jsonOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
    }
    else if(!strcmp(target,"ebtables_proc_speedlimit_cfg"))
    {
        request_method = methodstr2int(method);
        ebtables_proc_speedlimit(data,jsonOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
    }
	else if(!strcmp(target,"set_antiwifi"))
	{	
		ProcAntiwifiSet(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"get_antiwifi_status"))
	{	
		ProcAntiwifiStatusGet(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"set_antiwifi_question"))
	{	
		ProcQuestionSet(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"set_antiwifi_admin_forbidden"))
	{	
		ProcAntiWifiAdminSet(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"get_antiwifi_dev_list"))
	{	
		ProcAntiWifiDevGet(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"set_authed_antiwifi_dev"))
	{	
		ProcAntiWifiAuthSet(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
	else if(!strcmp(target,"get_antiwifi_authinfo"))
	{	
		ProcAntiWifiQueandAswGet(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
    else if(!strcmp(target,"set_wifi_timer"))
	{	
		ret = proc_wifitimer_sw_set(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"get_wifi_timer"))
	{	
		ret = proc_wifitimer_get(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"set_wifi_timer_rule"))
	{	
		ret = proc_wifitimer_set(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"del_wifi_timer_rule"))
	{	
		proc_wifitimer_del(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
    else if(!strcmp(target,"get_wds_cfg"))
	{	
		ret = proc_wdscfg_get(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"get_wds_wifi_scan"))
	{	
		ret = proc_wds_scan(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"set_wds_cfg"))
	{	
		ret = proc_wdscfg_set(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"set_up_wds"))
	{	
		ret = proc_wds_setup(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"get_wds_status"))
	{	
		ret = proc_wds_status_get(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"quicklycheck"))
	{	
		ProcQuicklyCheck(data, jsonOut);
		dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
    else if(!strcmp(target,"add_vpn_cfg"))
	{	
		ret = proc_add_vpn(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"set_vpn_cfg"))
	{	
		ret = proc_edit_vpn(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"del_vpn_cfg"))
	{	
		ret = proc_del_vpn(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
    else if(!strcmp(target,"get_vpn_cfg"))
	{	
		ret = proc_get_vpn(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"set_vpn_sw"))
	{	
		ret = proc_set_vpn_sw(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, ret, reuseflag, target);
	}
	else if(!strcmp(target,"get_wifi_firewall_cfg"))
	{	
		ProcWifiFirewallGet(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
    else if(!strcmp(target,"set_wifi_firewall_cfg"))
	{	
		ProcWifiFirewallSet(data, jsonOut);
        dortcfgres(session, devid, usrid, keyId, jsonOut, 0, reuseflag, target);
	}
 
	else
	{
		dortcfgres(session, devid, "", keyId, jsonOut, ERR_URL_NOT_SUPPORT, reuseflag, target);
		DEBUG_PRINTF("invalid requesturl!!!\r\n");		
	}


    if(json!=NULL)
    {
        cJSON_Delete(json);
    }


    if(jsonOut !=NULL)
    {
        cJSON_Delete(jsonOut);
    }

	return;
}

static int doheartbeat(struct lws *wsi, char *session)
{
	char str[100];
	char *format = "{\"sessionId\":\"%s\",\"method\":\"response\", \"data\":{}}";
	sprintf(str, format, session);

	//lwswriteback2(wsi, str);
	addstr2buf(str);
    #if 0
	DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
    if(wsi_lws->state == LWSS_ESTABLISHED)
    {
        lws_callback_on_writable(wsi_lws);
    }
    #endif 
    myclient2server();
}

static int doSetUpTcpProxy(struct lws *wsi, char *session, cJSON *data_value)
{
    char msgBuf[128] = {0};
    DEBUG_PRINTF("enter %s.\n", __FUNCTION__);
    char server_ip[128] = {0};
    int port = 0;
    struct VosMsgBody msg = {0};
    TCP_PROXY_INFO proxyInfo = {0};
    if (cJSON_GetObjectItem(data_value, "port") == NULL
        || cJSON_GetObjectItem(data_value, "tcpServer") == NULL)
    {
        write_log("Argument missing!\n");
        return -1;
    }
    rtcfgUciGet("system.@system[0].tcpproxyip",server_ip);
    if (strlen(server_ip) == 0)
    {
        UTIL_STRNCPY(server_ip ,cJSON_GetObjectItem(data_value, "tcpServer")->valuestring, sizeof(server_ip));
    }
    port = cJSON_GetObjectItem(data_value, "port")->valueint;  
    UTIL_STRNCPY(proxyInfo.serverIp, server_ip, sizeof(proxyInfo.serverIp));
    proxyInfo.port = port; 
    memcpy(msg.buf, &proxyInfo, sizeof(msg.buf));
    msg.stHead.dataLength = sizeof(proxyInfo); 
    msg.stHead.dst = EID_TCP_PROXY;
    msg.stHead.src = EID_MYWEBSOCKET;
    msg.stHead.type = VOS_MSG_TCPPROXY_CONNET_SERVER_REQ;
    msg.stHead.flags_request = 1;
    vosMsg_send(g_mcHandle, (VosMsgHeader *)&msg); 
    #if 0
    char *format = "{\"sessionId\":\"%s\",\"method\":\"response\", \"data\":{}}";
    snprintf(msgBuf, sizeof(msgBuf), format, session);
    addstr2buf(msgBuf);
    myclient2server();
    #endif
}


static int douploadrawkey(struct lws *wsi, char *session)
{
    char keyStrBuf[32] = {0};
    char keyBuf[8] = {0};
    char msgBuf[128] = {0};
    cJSON *jsonOut = NULL, *obj = NULL;
    char *buffer, *objbuf;
    DEBUG_PRINTF("enter douploadrawkey.\n");
    jsonOut = cJSON_CreateObject();
    obj = cJSON_CreateObject();
    int i = 0;
    if (!jsonOut || !obj)
    {
        DEBUG_PRINTF("memory not enough!\n");
        goto cleanup;
    }
    while (i < 100)
    {
        DEBUG_PRINTF("prepare get lock.\n");
        if (QtGetSpiLock(g_msgHandle) == 0)
        {
            DEBUG_PRINTF("Got lock.\n");
            LoadData(keyBuf, (int)sizeof(keyBuf));
            DEBUG_PRINTF("Got key.\n");
            QtReleaseSpiLock(g_msgHandle);
            DEBUG_PRINTF("released lock.\n");
            break;
        }
        i++;
        sleep(1);
    }
    
    snprintf(keyStrBuf, sizeof(keyStrBuf), "%02x%02x%02x%02x%02x%02x%02x%02x", (unsigned int)keyBuf[0], (unsigned int)keyBuf[1], (unsigned int)keyBuf[2], (unsigned int)keyBuf[3], (unsigned int)keyBuf[4], (unsigned int)keyBuf[5], (unsigned int)keyBuf[6], (unsigned int)keyBuf[7]);
    DEBUG_PRINTF("keyStrBuf is %s.\n", keyStrBuf);
    cJSON_AddItemToObject(jsonOut, "method", cJSON_CreateString("request"));
    cJSON_AddItemToObject(obj, "url", cJSON_CreateString("/route/routerupload/uploadrouterrootpass"));
    cJSON_AddItemToObject(obj, "routeSerialNo", cJSON_CreateString(getserialnum()));
    cJSON_AddItemToObject(obj, "routerRootPass", cJSON_CreateString(keyStrBuf));
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(jsonOut, "sessionId", cJSON_CreateString(session));
    buffer = cJSON_PrintUnformatted(jsonOut);
    DEBUG_PRINTF("douploadrawkey buffer is %s.\n", buffer);
    addstr2buf(buffer);
    myclient2server();

cleanup:
    if (jsonOut)
    {
        cJSON_Delete(jsonOut);
    }

    if (buffer)
    {
        free(buffer);
    }
}

void procDistSrvResp(cJSON *data_value)
{
    DEBUG_PRINTF("enter procDistSrvResp.\n");
    if (!cJSON_GetObjectItem(data_value, "uri") 
        || !cJSON_GetObjectItem(data_value, "token"))   
    {
        DEBUG_PRINTF("miss argument from dist server!\n");
        g_status = DIST_CONNECT;
        return;
    }
    
    UTIL_STRNCPY(serverUrl, cJSON_GetObjectItem(data_value, "uri")->valuestring, sizeof(serverUrl));
    UTIL_STRNCPY(g_token, cJSON_GetObjectItem(data_value, "token")->valuestring, sizeof(g_token)); 
    DEBUG_PRINTF("serverUrl:%s, token:%s\n", serverUrl, g_token);
    g_status = SERVER_CONNECT;
}

static void decodeMsgFromDistSrv(struct lws *wsi, char *msg)
{
    DEBUG_PRINTF("[%s]===\n",__func__);
	cJSON *json, *data_value, *jsonOut;
	char *url, *method;
	char *session;
	char *buffer;
    unsigned int pid;
	
	json = cJSON_Parse(msg);
	if (!json)
	{
		return;
	}

	jsonOut = cJSON_CreateObject();

	method = cJSON_GetObjectItem(json, "method")?cJSON_GetObjectItem(json, "method")->valuestring:"";
	session = cJSON_GetObjectItem(json, "sessionId")?cJSON_GetObjectItem(json, "sessionId")->valuestring:"";
    
	data_value = cJSON_GetObjectItem(json, "data");

	if (!strcmp(method,"response"))//response
	{
		DEBUG_PRINTF("[%s] lwsreceivemsgfromserver:%s\r\n",__func__, msg);
        
        procDistSrvResp(data_value);  
	}
	
	cJSON_Delete(json);
	if(jsonOut)
	{
		cJSON_Delete(jsonOut);
	}
	return;
}


static void decodemessage(struct lws *wsi, char *msg)
{
    DEBUG_PRINTF("[%s]===\n",__func__);
	cJSON *json, *data_value, *jsonOut;
	char *url, *method;
	char *session;
	char *buffer;
    unsigned int pid;
	
	json = cJSON_Parse(msg);
	if (!json)
	{
		return;
	}

	jsonOut = cJSON_CreateObject();

	method = cJSON_GetObjectItem(json, "method")?cJSON_GetObjectItem(json, "method")->valuestring:"";
	session = cJSON_GetObjectItem(json, "sessionId")?cJSON_GetObjectItem(json, "sessionId")->valuestring:"";
    
	data_value = cJSON_GetObjectItem(json, "data");

	if (!strcmp(method,"response"))//response
	{
		DEBUG_PRINTF("[%s] lwsreceivemsgfromserver:%s\r\n",__func__, msg);
        char *responseurl;
        responseurl=cJSON_GetObjectItem(data_value, "url")?cJSON_GetObjectItem(data_value, "url")->valuestring:"";
		if(!strcmp(g_sessionid, session))
        {
            if(g_upgeadeflag == REQ_FROM_APP)
            {
                pid = cJSON_GetObjectItem(data_value, "pid")?cJSON_GetObjectItem(data_value, "pid")->valueint:0;
				DEBUG_PRINTF("pid is :%d.\n", pid);
                buffer = cJSON_PrintUnformatted(data_value);
				DEBUG_PRINTF("receive buffer from server:%s.\n", buffer);
                sendmsg2cgi(buffer, pid);
                g_upgeadeflag = REQ_NO;
                if(buffer !=NULL)
                {
                    free(buffer);
                }
            }
            else if(g_upgeadeflag == REQ_FROM_SERVER)
            {
                procupdateinfo(data_value, jsonOut);
                dortcfgres(g_session, g_devid, g_usrid, g_keyid,jsonOut, 0, g_reuseflag, g_target);
                g_upgeadeflag = REQ_NO;
            }   
        }	

        if(!strcmp(responseurl,"speedroadaddress"))
        {
            prospeedtesturl(data_value);
        }
	}
	else//request
	{
		url = cJSON_GetObjectItem(data_value, "url")?cJSON_GetObjectItem(data_value, "url")->valuestring:"";
		if (!strcmp(url, "dorouter"))
		{
			DEBUG_PRINTF("[%s]lwsreceivemsgfromserver:%s\r\n",__func__, msg);
			dorouterdev(wsi, session, data_value);
		}
		else if (!strcmp(url, "heartbeat"))
        {   
            DEBUG_PRINTF("[%s]lwsreceivemsgfromserver:%s\r\n",__func__, msg);
			doheartbeat(wsi, session);
        }
        else if (!strcmp(url, "uploadrouterrootpass"))
        {
            DEBUG_PRINTF("[%s]lwsreceivemsgfromserver:%s\r\n",__func__, msg);
            douploadrawkey(wsi, session);
        }
        else if (!strcmp(url, "qtecCreateTunnelReq"))
        {
            DEBUG_PRINTF("[%s]lwsreceivemsgfromserver:%s\r\n",__func__, msg);
            doSetUpTcpProxy(wsi, session, data_value);
        }
	}
	
	cJSON_Delete(json);
	if(jsonOut)
	{
		cJSON_Delete(jsonOut);
	}
	return;
}

static int lwscallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
   // DEBUG_PRINTF("[%s]===reason:%d====\n",__func__,reason);
	unsigned char buf[LWS_PRE + 2048];
	unsigned int rands[4];
	int l = 0;
	int n;

	switch (reason) {
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lwsl_notice("LWS_CALLBACK_CLIENT_ESTABLISHED\r\n");
		char str[200];
        if (g_status == DIST_CONNECTED)
        {
    		char *format = "{\"method\":\"request\", \"sessionId\":\"%s%d\", \"data\":{\"routeSerialNo\":\"%s\", \"url\":\"/route/routerupload/createlink\"}}";
    		sprintf(str, format, getserialnum(), reqid++, getserialnum());
        }
        else if (g_status == SERVER_CONNECTED)
        {
            char *format = "{\"method\":\"request\", \"sessionId\":\"%s%d\", \"data\":{\"routeSerialNo\":\"%s\", \"token\":\"%s\",\"url\":\"/route/routerupload/createlink\", \"serverUrl\":\"%s\"}}";
    		sprintf(str, format, getserialnum(), reqid++, getserialnum(), g_token, serverUrl);
        }
            
		addstr2buf(str);
		//lws_callback_on_writable(wsi_lws);
		myclient2server();
        if (g_status == SERVER_CONNECTED)
        {
            updateoldlog();
            sendspeedtesturlreq();
        }
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_notice("LWS_CALLBACK_CLIENT_CONNECTION_ERROR\r\n");
        if (g_status == DIST_CONNECTED)
        {
            g_status = DIST_CONNECT;
        }
        else if (g_status == SERVER_CONNECTED)
        {
            g_status = SERVER_CONNECT;
        }
		break;

	case LWS_CALLBACK_CLOSED:
		lwsl_notice("LWS_CALLBACK_CLOSED\r\n");
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		lwsl_notice("LWS_CALLBACK_CLIENT_WRITEABLE\r\n");
		lwswriteback(wsi_lws);
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		lwsl_notice("LWS_CALLBACK_CLIENT_RECEIVE\r\n");
		struct timeval tv;	
		gettimeofday(&tv, NULL);
		timeold = tv.tv_sec;
        g_serverConnCount = 0;
        if (g_status == DIST_CONNECTED)
        {
            decodeMsgFromDistSrv(wsi, (char *)in);
        }
        else if (g_status == SERVER_CONNECTED)
        {
		    decodemessage(wsi, (char *)in);
        }
		break;

	default:
		break;
	}

	return 0;
}

/* list of supported protocols and callbacks */
static const struct lws_protocols protocols[] = {
	{
		NULL,
		lwscallback,
		0,
		2048,
	},
	{ NULL, NULL, 0, 0 } /* end */
};

static const struct lws_extension exts[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_no_context_takeover"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL /* terminator */ }
};

static void decdmmsg(char *msg)
{
    
    
	cJSON *json, *data_value;
	char *method;
	char session[100];
	char *sessionid;
	char *encryptinfo;
	char encodemsg[8192] = {0};
	char *format;
	unsigned char keyid[50] = {0};
	int len, reuseflag;
	char usrid[50] = {0};
	char devid[50] = {0};
	char strmsg[8192] = {0};
    char *out;

	json = cJSON_Parse(msg);
	if (!json)
	{
        DEBUG_PRINTF("[%s]====can't prase msg===\n",__func__);
		return;
	}

    out=cJSON_Print(json);
	DEBUG_PRINTF("[%s] mwrcvmsgfromdm:%s\r\n", __func__, out);
    if(out)
    {
        free(out);
    }
	method = cJSON_GetObjectItem(json, "method")?cJSON_GetObjectItem(json, "method")->valuestring:"";
	data_value = cJSON_GetObjectItem(json, "data");

    DEBUG_PRINTF("[%s]===method:%s=====\n",__func__,method);
    
	if (!strcmp(method,"request"))//request
	{
		sprintf(session, "%s%d", getserialnum(), reqid++);
		cJSON_AddStringToObject(json, "sessionId", session);
		cJSON_AddStringToObject(data_value, "routeSerialNo", getserialnum());
		out=cJSON_PrintUnformatted(json);
		addstr2buf(out);
        #if 0
        if( (wsi_lws != NULL)
        {
            DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
            if(wsi_lws->state == LWSS_ESTABLISHED)
		    {
                lws_callback_on_writable(wsi_lws);
            }
        }
        #endif
        myclient2server();
	}
	else//response
	{
		sessionid = cJSON_GetObjectItem(json, "sessionId")?cJSON_GetObjectItem(json, "sessionId")->valuestring:"";
		encryptinfo = cJSON_GetObjectItem(data_value, "encryptInfo")?cJSON_GetObjectItem(data_value, "encryptInfo")->valuestring:"";
		
		if (getencodeinfobysession(sessionid, usrid, devid, keyid, &reuseflag) == 0)
		{
			encodeencryptmsg(encryptinfo, encodemsg, keyid, usrid, devid, reuseflag);
			format = "{\"sessionId\":\"%s\",\"method\":\"response\", \"data\":{\"encryptInfo\":\"%s\",\"encryption\":\"%d\",\"keyinvalid\":%d, \"keyId\":\"%s\", \"userid\":\"%s\", \"deviceid\":\"%s\"}}";
		    sprintf(strmsg, format, sessionid, encodemsg, 1, 0, keyid, usrid, devid);	
		}
		else
		{
			strcpy(strmsg, msg);
		}
        addstr2buf(strmsg);
        #if 0
		//lwswriteback(wsi_lws, msg);
		addstr2buf(strmsg);
        DEBUG_PRINTF("====%s===wsi_lws state:=%d====\n",__func__,(int)(wsi_lws->state)); //5 means LWSS_ESTABLISHED
        if(wsi_lws->state == LWSS_ESTABLISHED)
		{
            lws_callback_on_writable(wsi_lws);
        }
        #endif
        myclient2server();

	}

	cJSON_Delete(json);
}

//\B4\A6\C0\EDsmd\B4\AB\B9\FD\C0\B4\B5\C4\D0\C5Ϣ
static void *pthread_routine(void *arg)
{

    DEBUG_PRINTF("[%x][%s]=====\n",pthread_self(),__func__);
	int ret = -1;
	int n = -1;
	int commFd = -1;
	int maxFd = -1;
	int fd, rv;
	fd_set readFdsMaster,rfds;
	VosMsgHeader *msg = NULL;
	struct VosMsgBody *body = NULL;
	ret = vosMsg_init(EID_MYWEBSOCKET, &g_mcHandle);

    if (ret != VOS_RET_SUCCESS)
    {
        return NULL;
    }
    
	vosMsg_getEventHandle(g_mcHandle, &commFd);
	FD_ZERO(&readFdsMaster);
	FD_SET(commFd, &readFdsMaster);
	maxFd = commFd;
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
			ret = vosMsg_receive(g_mcHandle, &msg);
			if (ret != VOS_RET_SUCCESS)
			{
				continue;
			}

			switch(msg->type)
			{				
				case 0x10009527:
				{
					body = (struct VosMsgBody *)msg;
					decdmmsg(body->buf);
					break;
				}
				case VOS_MSG_UPDATE_GETINFO:
                {
                    //鍙戦€佹秷鎭粰server锛屾帴鍙楀悗锛屾妸鏀跺埌鐨勬秷鎭洖缁檆gi
                    sendupdatereq(msg->src);
                    g_upgeadeflag = REQ_FROM_APP;
                    break;
                }
                case VOS_MSG_SPECIALCARE_NOTICE:
                {
                    DEBUG_PRINTF("[%s]====recevied special care notice=====\n",__func__);
                    body= (struct VosMsgBody *)msg;
                    
                    //\CAյ\BD\CC\D8\CA\E2\B9\D8ע\B5\C4\CC\E1\D0ѣ\AC\D0\E8Ҫ\CF\F2\D4ƶ˷\A2\CB\CD
                    DEBUG_PRINTF("[%s]====msg is %s====\n",__func__,body->buf);

                    char input_mac[64]={0};
                    char hostname[64]={0};
                    char devicetype[64]={0};
                    int input_flag=0;
                    sscanf(body->buf,"%s %s %s %d",input_mac,hostname,devicetype,&input_flag);

                    update_specialcare(input_mac, hostname,devicetype,input_flag);
                    break;
                    
                }
				default:
				    DEBUG_PRINTF("[%s]unrecognized msg\n",__func__);
					break;
			}
			if (NULL!= msg)
			{
				VOS_MEM_FREE_BUF_AND_NULL_PTR(msg);
			}
		}
	}
	vosMsg_cleanup(&g_mcHandle);
	return;
}

#define myclient_logfile "/tmp/.myclient"

void init_log()
{
	int f1;
    system("ulimit -c unlimited");
    if(access(myclient_logfile,F_OK) !=0)
	{	
		return;
	}
	f1 = open(myclient_logfile, O_RDWR | O_APPEND);

	if(f1)
	{
		dup2((int)f1,1);
		dup2((int)f1,2);

		close((int)f1);
	}
	
}

static void myclient_sig_hangdler(int sign_no)
{
    DEBUG_PRINTF("[%s]=====sign_no:%d====\n",__func__,sign_no);
    if(sign_no==SIGUSR1)
    {
        sleep(1);
        //reconnect server
        DEBUG_PRINTF("[%s]====recive sig to re-connect1 to server... ...\n",__func__);
        #if 0
        if(wsi_lws!=NULL)
        {
            free(wsi_lws);
            wsi_lws=NULL;
        }
        #endif
	    wsi_lws = lws_client_connect_via_info(&client_info);
        while(wsi_lws == NULL)
        {
            sleep(1);
            DEBUG_PRINTF("[%s]====recive sig to re-connect2 to server... ...\n",__func__);
            wsi_lws = lws_client_connect_via_info(&client_info);
        }
    }
}


int procConnDistSrv(char *uri)
{
    struct lws_context_creation_info info;
	//struct lws_client_connect_info i;
	struct lws_context *context;
	const char *prot, *p;
    int use_ssl=0; // support ssl;
	char path[300];	
    //ssl
    char cert_path[1024] = "";
	char key_path[1024] = "";
	char ca_path[1024] = "/usr/bin/ca-certificates.crt";
    char testurl[1024]={0};
    char tmpuri[512] = {0};
    int serverConnCount = 0;
    
    memset(&info, 0, sizeof info);	
    memset(g_token, 0, sizeof(g_token));
    memset(serverUrl, 0, sizeof(serverUrl));
	memset(&dist_client_info, 0, sizeof(dist_client_info));
    UTIL_STRNCPY(tmpuri, uri, sizeof(tmpuri));
	(void)lws_parse_uri(tmpuri, &prot, &dist_client_info.address, &dist_client_info.port, &p);  
    
	/* add back the leading / on path */
	path[0] = '/';
	strncpy(path + 1, p, sizeof(path) - 2);
	path[sizeof(path) - 1] = '\0';
	dist_client_info.path = path;

    if (!strcmp(prot, "http") || !strcmp(prot, "ws"))
		use_ssl = 0;
	if (!strcmp(prot, "https") || !strcmp(prot, "wss"))
		use_ssl = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;

	/*
	 * create the websockets context.  This tracks open connections and
	 * knows how to route any traffic and which protocol version to use,
	 * and if each connection is client or server side.
	 *
	 * For this client-only demo, we tell it to not listen on any port.
	 */
    info.port = CONTEXT_PORT_NO_LISTEN;
	info.iface = NULL;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	//info.options = 0;
	info.extensions = exts;
	//info.ssl_cert_filepath = NULL;
	//info.ssl_private_key_filepath = NULL;
#if defined(LWS_OPENSSL_SUPPORT)
	info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif

	if (use_ssl) {
		/*
		 * If the server wants us to present a valid SSL client certificate
		 * then we can set it up here.
		 */

		if (cert_path[0])
			info.ssl_cert_filepath = cert_path;
		if (key_path[0])
			info.ssl_private_key_filepath = key_path;

		/*
		 * A CA cert and CRL can be used to validate the cert send by the server
		 */
		if (ca_path[0])
			info.ssl_ca_filepath = ca_path;

#if defined(LWS_OPENSSL_SUPPORT) && defined(LWS_HAVE_SSL_CTX_set1_param)
		else if (crl_path[0])
			lwsl_notice("WARNING, providing a CRL requires a CA cert!\n");
#endif
	}



	context = lws_create_context(&info);
	if (context == NULL) {
		DEBUG_PRINTF("create context err!\r\n");
		return -1;
	}

    
	dist_client_info.context = context;
    dist_client_info.ssl_connection = use_ssl;
	dist_client_info.host = dist_client_info.address;
	dist_client_info.origin = dist_client_info.address;
	dist_client_info.ietf_version_or_minus_one = -1;
	dist_client_info.protocol = protocols[0].name;

	wsi_lws = lws_client_connect_via_info(&dist_client_info);
    serverConnCount = 0;
	while(wsi_lws == NULL && serverConnCount < 3)
	{
		sleep(10);
        if (wsi_lws)
        {
            break;
        }
		wsi_lws = lws_client_connect_via_info(&dist_client_info);
        
        serverConnCount++;
	}

    if (serverConnCount == 3)
    {
        g_status = DIST_CONNECT;
        lws_context_destroy(context);
    }
    else
    {
        g_status = DIST_CONNECTED;
    }
    while (wsi_lws!= NULL && g_status == DIST_CONNECTED)
	{	
		lws_service(context, 100);
	}

    DEBUG_PRINTF("distribute service client work done, clean up\n");
    if (context)
    {
        lws_context_destroy(context);
    }
}

int procConnServiceSrv()
{
    struct lws_context_creation_info info;
	//struct lws_client_connect_info i;
	struct lws_context *context;
	const char *prot, *p;
    int use_ssl=0; // support ssl;
	char path[300];	
    //ssl
    char cert_path[1024] = "";
	char key_path[1024] = "";
	char ca_path[1024] = "/usr/bin/ca-certificates.crt";
    char testurl[1024]={0};
    char tmpuri[512] = {0};
    struct timeval tv;
    int serverConnCount = 0;
    char tmpUrl[BUFLEN_256] = {0};
    
    memset(&info, 0, sizeof info);	
    memset(&client_info, 0, sizeof(client_info));
    UTIL_STRNCPY(tmpUrl, serverUrl, sizeof(tmpUrl));
	(void)lws_parse_uri(tmpUrl, &prot, &client_info.address, &client_info.port, &p);

	/* add back the leading / on path */
	path[0] = '/';
	strncpy(path + 1, p, sizeof(path) - 2);
	path[sizeof(path) - 1] = '\0';
	client_info.path = path;

    if (!strcmp(prot, "http") || !strcmp(prot, "ws"))
		use_ssl = 0;
	if (!strcmp(prot, "https") || !strcmp(prot, "wss"))
		use_ssl = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;

	/*
	 * create the websockets context.  This tracks open connections and
	 * knows how to route any traffic and which protocol version to use,
	 * and if each connection is client or server side.
	 *
	 * For this client-only demo, we tell it to not listen on any port.
	 */
    info.port = CONTEXT_PORT_NO_LISTEN;
	info.iface = NULL;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	//info.options = 0;
	info.extensions = exts;
	//info.ssl_cert_filepath = NULL;
	//info.ssl_private_key_filepath = NULL;
#if defined(LWS_OPENSSL_SUPPORT)
	info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif

	if (use_ssl) {
		/*
		 * If the server wants us to present a valid SSL client certificate
		 * then we can set it up here.
		 */

		if (cert_path[0])
			info.ssl_cert_filepath = cert_path;
		if (key_path[0])
			info.ssl_private_key_filepath = key_path;

		/*
		 * A CA cert and CRL can be used to validate the cert send by the server
		 */
		if (ca_path[0])
			info.ssl_ca_filepath = ca_path;

#if defined(LWS_OPENSSL_SUPPORT) && defined(LWS_HAVE_SSL_CTX_set1_param)
		else if (crl_path[0])
			lwsl_notice("WARNING, providing a CRL requires a CA cert!\n");
#endif
	}



	context = lws_create_context(&info);
	if (context == NULL) {
		DEBUG_PRINTF("create context err!\r\n");
		return -1;
	}

    
	client_info.context = context;
    client_info.ssl_connection = use_ssl;
	client_info.host = client_info.address;
	client_info.origin = client_info.address;
	client_info.ietf_version_or_minus_one = -1;
	client_info.protocol = protocols[0].name;

	wsi_lws = lws_client_connect_via_info(&client_info);
    serverConnCount = 0;
	while(wsi_lws == NULL && serverConnCount < 3)
	{
		sleep(10);
        if (wsi_lws)
        {
            break;
        }
		wsi_lws = lws_client_connect_via_info(&client_info);
        
        serverConnCount++;
	}

    if (serverConnCount == 3)
    {
        g_status = DIST_CONNECT;
        lws_context_destroy(context);
    }
    g_status = SERVER_CONNECTED;
	gettimeofday(&tv, NULL);
	timeold = tv.tv_sec;

	while (wsi_lws!= NULL)
	{	
		lws_service(context, 100);
		
		gettimeofday(&tv, NULL);
		if(((tv.tv_sec-timeold)>30) || ((tv.tv_sec<timeold)&&((tv.tv_sec+86400-timeold)>30)))
		{
			DEBUG_PRINTF("[%s]====disconnect to server... ...\n",__func__);
            DEBUG_PRINTF("[%s]====re-connect to server... ...\n",__func__);
			wsi_lws = lws_client_connect_via_info(&client_info);
            DEBUG_PRINTF("[%s]====re-connect to server2... ...\n",__func__);
            g_serverConnCount++;
            if (g_serverConnCount == 3)
            {
                g_status = DIST_CONNECT;
                
                g_serverConnCount = 0;
                break;
            }
			while(wsi_lws == NULL)
			{
				sleep(10);
                
				wsi_lws = lws_client_connect_via_info(&client_info);
			}
		}
	}
    DEBUG_PRINTF("service client work done, clean up\n");
    if (context)
    {
        lws_context_destroy(context);
    }
}
    	

int main(int argc, char **argv)
{
  
	struct lws_context_creation_info info;
	//struct lws_client_connect_info i;
	struct lws_context *context;
	const char *prot, *p;
    int use_ssl=0; // support ssl;
	char path[300];	
    //ssl
    char cert_path[1024] = "";
	char key_path[1024] = "";
	char ca_path[1024] = "/usr/bin/ca-certificates.crt";
    char testurl[1024]={0};
    int serverConnCount = 0;
	char *default_url = g_testServer?(g_demoServer?"ws://192.168.92.71:10087/websocket":"ws://192.168.92.59:10087/websocket"):"wss://balance.3caretec.com:10087/websocket";
    char tmpuri[512] = {0};
    VOS_RET_E ret = VOS_RET_SUCCESS;

    //smd
    vosLog_init(EID_MYWEBSOCKET);
    vosLog_setLevel(VOS_LOG_LEVEL_DEBUG);
    vosLog_setDestination(VOS_LOG_DEST_STDERR);
	ret = vosMsg_init(EID_MYCLIENT_FOR_LOCK, &g_msgHandle);
    if (ret != VOS_RET_SUCCESS)
    {
       DEBUG_PRINTF("Fail to connect to smd, exit\n"); 
       vosLog_cleanup();
       return -1;
    }
    
    init_log();

    rtcfgUciGet("system.@system[0].myserverurl",testurl);
    if(strlen(testurl) == 0 )
    {
        snprintf(testurl,strlen(default_url)+1,default_url);
    }

    DEBUG_PRINTF("[%s]====testurl:%s===",__func__,testurl);


	char *uri=(argc == 1)?testurl:argv[1];
	pthread_t pid;
	struct timeval tv;

    signal(SIGUSR1,myclient_sig_hangdler);
	
	


	memset(sendbuf, 0, 40*81920);
	memset(encodelist, 0, 8000);

	pthread_create(&pid, NULL, pthread_routine, NULL);
    pthread_detach(pid);
	//end

    while (1)
    {
        if (g_status == DIST_CONNECT)
        {
            procConnDistSrv(uri);
        }

        if (g_status == SERVER_CONNECT)
        {
            procConnServiceSrv();
        }

    	usleep(500000);
    }
	vosMsg_cleanup(&g_msgHandle);

	return 0;
}

