#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libwebsockets.h>
#include <librtcfg.h>
#include "encryption.h"
#include "keyapi.h"
#include "cJSON.h"
#include <sys/time.h>
#include "basic.h"

typedef enum _Sm4Mode
{
	SM4_ECB_DEC = 0,
	SM4_ECB_ENC,
	SM4_CBC_ENC,
	SM4_CBC_DEC,
}Sm4Mode;

typedef enum _KeyType
{
	POOL_TYPE_RAW = 1,
	POOL_TYPE_SYNC
}KeyType;

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

static void DecodeFailReply(cJSON *jsonOut, unsigned char *keyid, char *userid, char *deviceid)
{
	struct systemInfo output = {0};
	
	(void)getSystemInfo(&output);
    cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(""));
    cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(1));
    cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(1));
	cJSON_AddItemToObject(jsonOut, "userid", cJSON_CreateString(userid));
	cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(deviceid));
	cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(keyid));
	cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));
}

static void EncryptJsonData(cJSON *jsonIn, cJSON *jsonOut, unsigned char *keyid, char *userid, char *deviceid, int ret, int reuseflag, int encryption)
{
    char *dest, *sourcebuffer, *outputdata, *source;
	char msg[256] = {0};
    char debug_msg[256]={0};
	int sourcelen, destlen, outputdatalen;
	unsigned char key[KEYLEN+1] = {0};
	struct tagCQtQkMangent *pstcqtqkmangent;
	struct tagCQtKeyEncrypt *pstcqtkeyencrypt;
	cJSON *data = NULL;
	cJSON *obj = NULL;
	struct systemInfo output = {0};
	
	(void)getSystemInfo(&output);
    
	data = cJSON_GetObjectItem(jsonIn, "data");
	if(NULL == data)
	{
		obj = cJSON_CreateObject();
		cJSON_AddItemToObject(jsonIn, "data", obj);
	}
    
#if 0
	if(ret)
	{
		cJSON_AddItemToObject(jsonIn, "msg", cJSON_CreateString("fail"));
		cJSON_AddItemToObject(jsonIn, "code", cJSON_CreateNumber(ret));
        cJSON_AddItemToObject(jsonIn, "debug_code", cJSON_CreateNumber(global_weberrorcode));
        char debug_msg[256]={0};
        err2msg(global_weberrorcode,debug_msg,sizeof(debug_msg));
        cJSON_AddItemToObject(jsonIn, "debug_msg", cJSON_CreateString(debug_msg));
	}
	else
	{
		cJSON_AddItemToObject(jsonIn, "msg", cJSON_CreateString("ok"));
		cJSON_AddItemToObject(jsonIn, "code", cJSON_CreateNumber(0));

        cJSON_AddItemToObject(jsonIn, "debug_code", cJSON_CreateNumber(global_weberrorcode));
        char debug_msg[256]={0};
        err2msg(global_weberrorcode,debug_msg,sizeof(debug_msg));
        cJSON_AddItemToObject(jsonIn, "debug_msg", cJSON_CreateString(debug_msg));
	}
#endif
    
	err2msg(ret,msg,sizeof(msg));
    cJSON_AddItemToObject(jsonIn, "msg", cJSON_CreateString(msg));
	cJSON_AddItemToObject(jsonIn, "code", cJSON_CreateNumber(ret));
    cJSON_AddItemToObject(jsonIn, "debug_code", cJSON_CreateNumber(global_weberrorcode));
    err2msg(global_weberrorcode,debug_msg,sizeof(debug_msg));
    cJSON_AddItemToObject(jsonIn, "debug_msg", cJSON_CreateString(debug_msg));
	
	source = cJSON_PrintUnformatted(jsonIn);
    
	/* jiami */
	if(encryption)
	{
		pstcqtqkmangent = GetCQtQkMangent();
		pstcqtkeyencrypt = GetCQtKeyEncrypt();
		sourcelen = (strlen(source) + 16) / 16 * 16;
		destlen = sourcelen;
		outputdatalen = 2 * destlen;
		
		dest = malloc(destlen);
		sourcebuffer = malloc(sourcelen); 
		outputdata = malloc(outputdatalen);

		memset(dest, 0, destlen);
		memset(sourcebuffer, 0, sourcelen);
		memset(outputdata, 0, outputdatalen);
		
		memcpy(sourcebuffer, source, strlen(source));
		
		//encrypt

		GetNextSynKeyId(keyid);
		C_GetKeyByIdNode(pstcqtqkmangent, POOL_TYPE_SYNC, 1, userid, deviceid, keyid, key);
		int i = 0;
		printf("decode key:");
		for(i = 0; i < 16;i++)
		{
			printf("0x%2x ",key[i]);
		}
		printf("\n");
		
		/* delete key */
		C_EncryptOrDecrypt(pstcqtkeyencrypt, dest, &destlen, sourcebuffer, sourcelen, 1, key);

		//base64
		lws_b64_encode_string(dest, destlen, outputdata, outputdatalen);
		cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(outputdata));
		ReleaseCQtKeyEncrypt(&pstcqtkeyencrypt);
		ReleaseCQtQkMangent(&pstcqtqkmangent);
		free(sourcebuffer);
		free(dest);
		free(outputdata);
	}
	else
	/* mingwen */
	{
		cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(source));
	}
	printf("source is:%s.\n", source);	
	cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(encryption));
    cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(0));
	cJSON_AddItemToObject(jsonOut, "userid", cJSON_CreateString(userid));
	cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(deviceid));
	cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(keyid));
	cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));
	free(source);

	return;
}

void ProcRouterDiscovery(cJSON *jsonValue, cJSON *jsonOut)
{
	struct systemInfo output = {0};
	cJSON *obj = NULL;
	cJSON *encryptinfo = NULL;
	char *source;


	obj = cJSON_CreateObject();
	encryptinfo = cJSON_CreateObject();
	if(NULL == obj)
	{
		return;
	}
	
	(void)getSystemInfo(&output);
	cJSON_AddItemToObject(encryptinfo, "data", obj);
	cJSON_AddItemToObject(obj, "hostname", cJSON_CreateString(output.product));
	cJSON_AddItemToObject(obj, "version", cJSON_CreateString(output.productVersion));
	cJSON_AddItemToObject(obj, "serialnum", cJSON_CreateString(output.serialnum));
	cJSON_AddItemToObject(obj, "devmodel", cJSON_CreateString("QTEC"));
	cJSON_AddItemToObject(obj, "configured", cJSON_CreateNumber(output.configured));
	cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("success"));
	cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(0));
	
	source = cJSON_PrintUnformatted(encryptinfo);
	cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(source));
	cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(0));
	cJSON_AddItemToObject(jsonOut, "userid", cJSON_CreateString(""));
	cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(""));
	cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(""));
	cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));
	free(source);
	
	return;
}

void ProcRouterAdd(cJSON *jsonValue, cJSON *jsonOut)
{
	char password[BUFLEN_64] = {0};
	char userbound[BUFLEN_64] = {0};
	char staMac[BUFLEN_64] = {0};
	char staName[BUFLEN_64] = {0};
	char staSysInfo[BUFLEN_64] = {0};
	char userName[BUFLEN_64] = {0};
	char linebuffer[BUFLEN_64] = {0};
	char buf[BUFLEN_64] = {0};
	int  len = 0;
	int  ret = 0;
	int  linelen = 0;
	cJSON *obj = NULL;
	cJSON *encryptinfo = NULL;
	char *source;
	FILE *fp1 = NULL;
	FILE *fp2 = NULL;
	struct systemInfo output = {0};
	
	(void)getSystemInfo(&output);

	obj = cJSON_CreateObject();
	encryptinfo = cJSON_CreateObject();
	if(NULL == obj)
	{
		return;
	}

#if 0
	//user bound check
	fp1 = fopen("/etc/info/user_info", "r+");
	while(NULL != fgets(linebuffer, 64, fp1))
	{
		if(0 == strncmp(linebuffer, "USERBOUND", strlen("USERBUND")))
		{
			sscanf(linebuffer, "USERBOUND:%s", userbound);
			break;
		}			
	}
	fclose(fp1);
	if(0 == strcmp(userbound, "1"))
	{
		//json output data
		cJSON_AddItemToObject(jsonOut, "data", obj);
		cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("rounter is bound already"));
		cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
		return;
	}
#endif

	//password check
	ret = ProcPasswordCheckByApp(jsonValue, jsonOut);
	if(ret)
	{
		cJSON_AddItemToObject(encryptinfo, "data", obj);
		cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("password not match"));
		cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(ret));

		source = cJSON_PrintUnformatted(encryptinfo);
		cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(source));
		cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(0));
		cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(0));
		cJSON_AddItemToObject(jsonOut, "userid", cJSON_CreateString(""));
		cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(""));
		cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(""));
		cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));
		free(source);
		return;
	}

	//get sta info
	strcpy(staMac, (cJSON_GetObjectItem(jsonValue, "stamac")?cJSON_GetObjectItem(jsonValue, "stamac")->valuestring:""));
	strcpy(staName, (cJSON_GetObjectItem(jsonValue, "staname")?cJSON_GetObjectItem(jsonValue, "staname")->valuestring:""));
	strcpy(staSysInfo, (cJSON_GetObjectItem(jsonValue, "stasysinfo")?cJSON_GetObjectItem(jsonValue, "stasysinfo")->valuestring:""));
	strcpy(userName, (cJSON_GetObjectItem(jsonValue, "username")?cJSON_GetObjectItem(jsonValue, "username")->valuestring:""));

	//save stainfo
	fp1 = NULL;
	fp1 = fopen("/etc/info/user_info", "w+");
	if(NULL == fp1)
	{
		printf("open file failed");
		return;
	}

	snprintf(linebuffer, BUFLEN_64, "STAMAC:%s\n",staMac);
	fprintf(fp1, "%s", linebuffer);
	snprintf(linebuffer, BUFLEN_64, "STANAME:%s\n",staName);
	fprintf(fp1, "%s", linebuffer);
	snprintf(linebuffer, BUFLEN_64, "STASYSINFO:%s\n",staSysInfo);
	fprintf(fp1, "%s", linebuffer);
	snprintf(linebuffer, BUFLEN_64, "USERNAME:%s\n",userName);
	fprintf(fp1, "%s", linebuffer);
	snprintf(linebuffer, BUFLEN_64, "USERBOUND:1\n");
	fprintf(fp1, "%s", linebuffer);
    fclose(fp1);

	//json output data
	cJSON_AddItemToObject(encryptinfo, "data", obj);
	cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("ok"));
	cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(0));

	source = cJSON_PrintUnformatted(encryptinfo);
	cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(source));
	cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(0));
	cJSON_AddItemToObject(jsonOut, "userid", cJSON_CreateString(""));
	cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(""));
	cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(""));
	cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));
	free(source);

	return;
}

void ProcdevSearch(cJSON *jsonValue, cJSON *jsonOut, char *keyid, char *userid, char *deviceid, int reuseflag)
{
	int index = 0;
	int devnum = 0;	
	cJSON *obj = NULL;
	cJSON *array = NULL;
	cJSON *encryptinfo = NULL;
	cJSON *subJson = NULL;
	struct devInfo stDevInfo[10] = {0};
	char *source;
	int sourcelen, destlen;
	char key[KEYLEN+1] = {0};
	char dest[BUFLEN_1024] = {0};
	char sourcebuffer[BUFLEN_1024] = {0};
	char outputdata[BUFLEN_2048] = {0};
	struct tagCQtQkMangent *pstcqtqkmangent;
	struct tagCQtKeyEncrypt *pstcqtkeyencrypt;
	struct systemInfo output = {0};
	
	(void)getSystemInfo(&output);

	pstcqtqkmangent = GetCQtQkMangent();
	pstcqtkeyencrypt = GetCQtKeyEncrypt();

	devnum = searchDevList(stDevInfo);
	printf("devnum is %d\n", devnum);
	fflush(stdout);

	encryptinfo = cJSON_CreateObject();
	subJson = cJSON_CreateObject(); 
	cJSON_AddItemToObject(encryptinfo, "data", subJson);
	cJSON_AddItemToObject(subJson,"devlist",array=cJSON_CreateArray());

	for(index = 0; index < devnum; index++)
	{
		cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
		cJSON_AddItemToObject(obj,"devid",cJSON_CreateString("123"));
		cJSON_AddItemToObject(obj,"devname",cJSON_CreateString(stDevInfo[index].name));
	}
		

	cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("ok"));
	cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(0));

	source = cJSON_PrintUnformatted(encryptinfo);
	printf("source is %s", source);
	fflush(stdout);
	//encrypt
	sourcelen = strlen(source);
	memcpy(sourcebuffer, source, sourcelen);
    GetNextSynKeyId(keyid);
	C_GetKeyByIdNode(pstcqtqkmangent, POOL_TYPE_SYNC, 1, userid, deviceid, keyid, key);

	C_EncryptOrDecrypt(pstcqtkeyencrypt, dest, &destlen, sourcebuffer, BUFLEN_1024, 1, key);
	//base64
	lws_b64_encode_string(dest, BUFLEN_1024, outputdata, BUFLEN_2048);

	cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(outputdata));
	cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(1));
    cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(0));
	cJSON_AddItemToObject(jsonOut, "userid", cJSON_CreateString(userid));
	cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(deviceid));
	cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(keyid));
	cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));

	ReleaseCQtKeyEncrypt(&pstcqtkeyencrypt);
	ReleaseCQtQkMangent(&pstcqtqkmangent);
	free(source);	
	printf("exit ProcdevSearch.\n");
	fflush(stdout);
}

void ProcDevAdd(cJSON *jsonValue, cJSON *jsonOut, char *keyid, char *userid, char *deviceid, int reuseflag)
{
	char password[BUFLEN_64] = {0};
	char passwordStore[BUFLEN_64] = {0};
	char userName[BUFLEN_64] = {0};
	char mac[BUFLEN_64] = {0};
	char buf[BUFLEN_64] = {0};
	char key[KEYLEN+1] = {0};
	char* devId = NULL;
	char *source;
	int sourcelen, destlen;
	cJSON *obj = NULL;
	cJSON *encryptinfo = NULL;
	FILE *fp = NULL;
	char dest[BUFLEN_1024] = {0};
	char sourcebuffer[BUFLEN_1024] = {0};
	char outputdata[BUFLEN_2048] = {0};
	struct tagCQtQkMangent *pstcqtqkmangent;
	struct tagCQtKeyEncrypt *pstcqtkeyencrypt;
	struct systemInfo output = {0};
	
	(void)getSystemInfo(&output);

	pstcqtqkmangent = GetCQtQkMangent();
	pstcqtkeyencrypt = GetCQtKeyEncrypt();

	obj = cJSON_CreateObject();
	encryptinfo = cJSON_CreateObject();
	if(NULL == obj)
	{
		return;
	}

	//get info
	strcpy(password, (cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:""));
	strcpy(userName, (cJSON_GetObjectItem(jsonValue, "username")?cJSON_GetObjectItem(jsonValue, "username")->valuestring:""));
	strcpy(mac, (cJSON_GetObjectItem(jsonValue, "mac")?cJSON_GetObjectItem(jsonValue, "mac")->valuestring:""));
	devId = cJSON_GetObjectItem(jsonValue, "devid")?cJSON_GetObjectItem(jsonValue, "devid")->valuestring:"0";

	//json output data
	cJSON_AddItemToObject(jsonOut, "data", obj);
	fp = fopen("/html/www/lighttpd.user", "r");
	fgets(buf,BUFLEN_64,fp);
	sscanf(buf,"admin:%s", passwordStore);
	if(strncmp(password,passwordStore,strlen(password))==0)
	{
		(void)addBandDev(atoi(devId));
		system("ubus call mywebsocket updatedevinfo");
		cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("ok"));
		cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(0));
	}
	else
	{
		cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("password error!"));
		cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(1));
	}

	source = cJSON_PrintUnformatted(encryptinfo);
	//encrypt
	sourcelen = strlen(source);
	memcpy(sourcebuffer, source, sourcelen);
    GetNextSynKeyId(keyid);
	C_GetKeyByIdNode(pstcqtqkmangent, POOL_TYPE_SYNC, 1, userid, deviceid, keyid, key);

	C_EncryptOrDecrypt(pstcqtkeyencrypt, dest, &destlen, sourcebuffer, BUFLEN_1024, 1, key);
	//base64
	lws_b64_encode_string(dest, BUFLEN_1024, outputdata, BUFLEN_2048);
	cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(outputdata));
	cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(1));
    cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(0));
	cJSON_AddItemToObject(jsonOut, "userid", cJSON_CreateString(userid));
	cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(deviceid));
	cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(keyid));
	cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));

	ReleaseCQtKeyEncrypt(&pstcqtkeyencrypt);
	ReleaseCQtQkMangent(&pstcqtqkmangent);
	free(source);	
}

void ProcFirstKeyReq(cJSON *jsonValue, cJSON *jsonOut, char *userid, char *deviceid)
{
	char devicename[32] = {0};
    char serialnumber[64] = {0};
	unsigned char keyid[KEYIDLEN+1] = {0};
	unsigned char key[KEYLEN+1] = {0};
    unsigned char keybased[32] = {0};
	SrcMessage stSrcMessage = {0};
	DestMessage stDestMessage = {0};
	int destlen = 0;
	int sourcelen = 0;
	int outputdatalen = 0;
	int usedcount = 0;
	int unusedcount = 0;
	int index, ret;
	int i = 0;
    unsigned char *keytemp, *keyidtemp;
	char *source, *dest, *sourcebuffer, *outputdata;
	cJSON *encryptinfo = NULL;
	cJSON *data = NULL;
	cJSON *obj = NULL;
	cJSON *array = NULL;
	struct tagCQtQkMangent *pstcqtqkmangent;
	struct tagCQtKeyEncrypt *pstcqtkeyencrypt;
	struct systemInfo output = {0};

    data = cJSON_CreateObject();
	encryptinfo = cJSON_CreateObject();
    
	(void)getSystemInfo(&output);
    strcpy(serialnumber, (cJSON_GetObjectItem(jsonValue, "serialnumber")?cJSON_GetObjectItem(jsonValue, "serialnumber")->valuestring:""));
    /* check serialnumber */
    if(0 != strncmp(serialnumber, output.serialnum, 64))
    {
        printf("this serial number mismatch!\n");
        cJSON_AddItemToObject(encryptinfo, "data", data);
		cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("this serial number mismatch!"));
		cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(ERR_SERIAL_NUMBER_MISMARCH));
        source = cJSON_PrintUnformatted(encryptinfo);
        printf("source is: %s.\r\n", source);
        cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(source));
    	cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(0));
        cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(0));
    	cJSON_AddItemToObject(jsonOut, "userid",  cJSON_CreateString(userid));
    	cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(deviceid));
    	cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(""));
    	cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));
        free(source);
        return;
    }

	pstcqtqkmangent = GetCQtQkMangent();
	pstcqtkeyencrypt = GetCQtKeyEncrypt();

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

	//check rawkey num
	C_GetCount(pstcqtqkmangent, 1, &usedcount, &unusedcount, userid, deviceid);
	printf("unusedcount is %d.\n", unusedcount);
	if(unusedcount < stSrcMessage.keyNumber)
	{
        ProcRawkeyAddReq();
        printf("rawkey is not enough.\n");
        cJSON_AddItemToObject(encryptinfo, "data", data);
		cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("rawkey is not enough! try again later."));
		cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(ERR_RAWKEY_NOT_ENOUGH));
        source = cJSON_PrintUnformatted(encryptinfo);
        printf("source is: %s.\r\n", source);
        cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(source));
    	cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(0));
        cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(0));
    	cJSON_AddItemToObject(jsonOut, "userid",  cJSON_CreateString(userid));
    	cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(deviceid));
    	cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(""));
    	cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));
        free(source);
        ReleaseCQtKeyEncrypt(&pstcqtkeyencrypt);
    	ReleaseCQtQkMangent(&pstcqtqkmangent);
		free(stDestMessage.keyId);
		free(stDestMessage.key);
        return;
	}

	//clean synkey table first
	C_DeleteKeyByNode(pstcqtqkmangent, POOL_TYPE_SYNC, userid, deviceid);

	//get synkey
	ret = C_DealClientSynReq(&stDestMessage, &stSrcMessage);

	//jsonout	
	if(!ret)
	{
		keytemp = stDestMessage.key;
		keyidtemp = stDestMessage.keyId;
		cJSON_AddItemToObject(data, "keynumber", cJSON_CreateNumber(stDestMessage.keyNumber));
		cJSON_AddItemToObject(data, "deviceid", cJSON_CreateString(stDestMessage.pushDeviceId));
		cJSON_AddItemToObject(data, "devicename", cJSON_CreateString(devicename));
		cJSON_AddItemToObject(data,"keylist",array=cJSON_CreateArray());
		for(index = 0; index < stSrcMessage.keyNumber; index++)
		{
			memset(keyid, 0 ,KEYIDLEN+1);
			memset(key, 0 ,KEYLEN+1);
			memcpy(keyid, keyidtemp, KEYIDLEN);
			memcpy(key, keytemp, KEYLEN);
            lws_b64_encode_string(key, 16, keybased, 32);	
			cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
			cJSON_AddItemToObject(obj,"keyid",cJSON_CreateString(keyid));
			cJSON_AddItemToObject(obj,"key",cJSON_CreateString(keybased));
			printf("keyid:%s, synkey:",keyid);
			for(i = 0; i < 16;i++)
			{
				printf("0x%2x ",key[i]);
			}
			printf("\n");
			keytemp += KEYLEN;
			keyidtemp += KEYIDLEN;
		}
		cJSON_AddItemToObject(encryptinfo, "data", data);

		cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("ok"));
		cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(0));
	}
	else
	{
		cJSON_AddItemToObject(encryptinfo, "data", data);
		cJSON_AddItemToObject(encryptinfo, "msg", cJSON_CreateString("fail"));
		cJSON_AddItemToObject(encryptinfo, "code", cJSON_CreateNumber(1));
	}
		
	source = cJSON_PrintUnformatted(encryptinfo);
	sourcelen = (strlen(source) + 16) /16 * 16;
    destlen = sourcelen;
    outputdatalen = 2 * destlen;
    
    dest = malloc(destlen);
	sourcebuffer = malloc(sourcelen); 
	outputdata = malloc(outputdatalen);

    memset(dest, 0, destlen);
	memset(sourcebuffer, 0, sourcelen);
	memset(outputdata, 0, outputdatalen);
    
    memcpy(sourcebuffer, source, strlen(source));
    
	printf("source is: %s.\r\n", source);

	//encrypt
	memset(keyid, 0 ,KEYIDLEN+1);
	memset(key, 0 ,KEYLEN+1);
	
	GetFirstEncodeKey(key);
	printf("decode key:");
	for(i = 0; i < 16;i++)
	{
		printf("0x%02x ",key[i]);
	}
	printf("\n");
	C_EncryptOrDecrypt(pstcqtkeyencrypt, dest, &destlen, sourcebuffer, sourcelen, 1, key);
	//C_EncryptOrDecrypt(pstcqtkeyencrypt, dest, &destlen, sourcebuffer, sourcelen, 1, "1111111111111111");
	//base64
	lws_b64_encode_string(dest, destlen, outputdata, outputdatalen);
			
	cJSON_AddItemToObject(jsonOut, "encryptinfo", cJSON_CreateString(outputdata));
	cJSON_AddItemToObject(jsonOut, "encryption", cJSON_CreateNumber(1));
    cJSON_AddItemToObject(jsonOut, "keyinvalid", cJSON_CreateNumber(0));
	cJSON_AddItemToObject(jsonOut, "userid",  cJSON_CreateString(userid));
	cJSON_AddItemToObject(jsonOut, "deviceid", cJSON_CreateString(deviceid));
	cJSON_AddItemToObject(jsonOut, "keyid", cJSON_CreateString(""));
	cJSON_AddItemToObject(jsonOut, "serialnumber", cJSON_CreateString(output.serialnum));

	ReleaseCQtKeyEncrypt(&pstcqtkeyencrypt);
	ReleaseCQtQkMangent(&pstcqtqkmangent);
	free(source);
	free(sourcebuffer);
	free(dest);
	free(outputdata);
	free(stDestMessage.keyId);
	free(stDestMessage.key);

	ProcRawkeyAddReq();
	return;	
}


void decode_message(char *msg)
{
	cJSON *json, *jsonValue, *jsonOut, *encryptinfo, *encryptinfoOut;
	char *target, *buffer, *dest, *deviceid, *userid;
    unsigned char *keyid;
	int ret = 0, method, f0, f1, f2, sourcelen, destlen, decodelen, originlen, encryption, reuseflag;
	struct tagCQtQkMangent *pstcqtqkmangent;
	struct tagCQtKeyEncrypt *pstcqtkeyencrypt;
	unsigned char key[KEYLEN+1] = {0};
	char origin[BUFLEN_1024] = {0};
	char source[BUFLEN_1024] = {0};
    char *dupfile = (access("/etc/info/huangkai", F_OK) == 0)?"/etc/info/huangkai":"/dev/null";
    jsonOut = cJSON_CreateObject();
	json = cJSON_Parse(msg);
	if (!json)
	{
		printf("error!");
		return;		
	}
	
	f0 = dup(STDOUT_FILENO);
	f1 = open(dupfile, O_RDWR | O_APPEND);
	f2 = dup2(f1, STDOUT_FILENO);
	close(f1);
	
	pstcqtqkmangent = GetCQtQkMangent();

	pstcqtkeyencrypt = GetCQtKeyEncrypt();  
	dest = cJSON_GetObjectItem(json, "encryptinfo")?cJSON_GetObjectItem(json, "encryptinfo")->valuestring:"";
	keyid = cJSON_GetObjectItem(json, "keyid")?cJSON_GetObjectItem(json, "keyid")->valuestring:"";
	deviceid = cJSON_GetObjectItem(json, "deviceid")?cJSON_GetObjectItem(json, "deviceid")->valuestring:"";
	encryption = cJSON_GetObjectItem(json, "encryption")?cJSON_GetObjectItem(json, "encryption")->valueint:0;
	userid = cJSON_GetObjectItem(json, "userid")?cJSON_GetObjectItem(json, "userid")->valuestring:"";
    reuseflag = cJSON_GetObjectItem(json, "reuse")?cJSON_GetObjectItem(json, "reuse")->valueint:0;

	printf("encryption:%d, keyid:%s,devideid:%s,userid:%s.\n", encryption, keyid, deviceid, userid);
	destlen = sizeof(dest);

	if(encryption)
	{
		//decrypt
		//base64 
		decodelen = lws_b64_decode_string(dest, origin, BUFLEN_1024);
		originlen = (decodelen + 15) /16 * 16;
		ret = C_GetKeyByIdNode(pstcqtqkmangent, POOL_TYPE_SYNC, 1, userid, deviceid, keyid, key);
		int i = 0;
		printf("decode key:");
		for(i = 0; i < 16;i++)
		{
			printf("0x%2x ",key[i]);
		}
		printf("\n");
        
		ret += C_EncryptOrDecrypt(pstcqtkeyencrypt, source, &sourcelen, origin, originlen, 0, key);
        DEBUG_PRINTF("decodelen:%d, strlen(origin):%d, originlen:%d.\n",decodelen, strlen(origin), originlen);
		DEBUG_PRINTF("encryption is enable,source is :%s\r\n", source);
		if(cJSON_Parse(source))
		{
			encryptinfo = cJSON_Parse(source);
		}
		else
		{
			/* 通知秘钥失效 */
			printf("Decrypt failed.\n");
            DecodeFailReply(jsonOut, keyid, userid, deviceid);
            fflush(stdout);  
	        dup2(f0, f2);
	        buffer = cJSON_Print(jsonOut);
	        printf("%s\n",buffer);
	        free(buffer);
	        cJSON_Delete(jsonOut);
	        cJSON_Delete(json);
            return;
		}

		target = cJSON_GetObjectItem(encryptinfo, "requestUrl")?cJSON_GetObjectItem(encryptinfo, "requestUrl")->valuestring:"";
		jsonValue = cJSON_GetObjectItem(encryptinfo, "data");
	}
	else
	{
		encryptinfo = cJSON_Parse(dest);
		target = cJSON_GetObjectItem(encryptinfo, "requestUrl")?cJSON_GetObjectItem(encryptinfo, "requestUrl")->valuestring:"";
		jsonValue = cJSON_GetObjectItem(encryptinfo, "data");
	}
	
	DEBUG_PRINTF("target is %s.\r\n", target);
	encryptinfoOut = cJSON_CreateObject();
    global_weberrorcode=0;
	if(!strcmp(target,"routerinfo"))
	{
		ProcRouterDiscovery(jsonValue, jsonOut);
	}
	else if(!strcmp(target,"set_firstconfigure_cfg"))
	{
		ret = ProcFirstconfigureSetByApp(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"routeradd"))
	{
		ProcRouterAdd(jsonValue, jsonOut);
	}
	else if(!strcmp(target,"routerstatus"))
	{
		ret = proc_stalist_get(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"devsearch"))
	{
		ProcdevSearch(jsonValue, jsonOut, keyid, userid, deviceid, reuseflag);
	}
	else if(!strcmp(target,"devadd"))
	{
		ProcDevAdd(jsonValue, jsonOut, keyid, userid, deviceid, reuseflag);
	}
	else if(!strcmp(target,"firstkeyrequest"))
	{
		ProcFirstKeyReq(jsonValue, jsonOut, userid, deviceid);
	}
	else if(!strcmp(target,"keyrequest"))
	{
		ret = ProcKeyReq(jsonValue, encryptinfoOut, userid, deviceid, keyid);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_routerbasicinfo"))
	{
		GetRouterBasicInfoByApp(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"upgrade"))
	{
		ret = ProcUpdateReq(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"check_password_cfg"))
	{
		ret = ProcPasswordCheckByApp(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_password_cfg"))
	{
		ret = ProcPasswordSetByApp(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"cfgrestore"))
	{
		ProcFirstbootReq(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"routerrestart"))
	{
		proc_reboot(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_wireless_cfg"))
	{
		ret = ProcWifiSetByApp(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_wireless_cfg"))
	{
		ProcWifiGetByApp(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_basicwan_cfg"))
	{
		ret = ProcWanSetByApp(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_wantype_cfg"))
	{
		ret = ProcWanGetByApp(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_timertask_cfg"))
	{
		proc_timertask_set(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_timertask_cfg"))
	{
		proc_timertask_detect(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
    else if(!strcmp(target,"get_smb_pwd"))
	{
		ret = proc_get_smb_pwd(jsonValue, encryptinfoOut);
		EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_update_version"))
	{	
		ret = ProcUpdateMsgGet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_update_rate"))
	{	
		ret = proc_queryupgrade(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
    else if(!strcmp(target,"get_wifi_txpower"))
	{	
		ret = procWifiGetTxpower(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
    else if(!strcmp(target,"set_wifi_txpower"))
	{	
		ret = procWifiSetTxpower(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
    //儿童模式
    else if(!strcmp(target,"proc_childrule_cfg"))
    {
        method = cJSON_GetObjectItem(encryptinfo, "method")?cJSON_GetObjectItem(encryptinfo, "method")->valuestring:""; 
        request_method = methodstr2int(method);
        ret = proc_childrule(jsonValue,encryptinfoOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
    }
    else if(!strcmp(target,"proc_wan_speedtest_cfg"))
    {
        method = cJSON_GetObjectItem(encryptinfo, "method")?cJSON_GetObjectItem(encryptinfo, "method")->valuestring:""; 
        DEBUG_PRINTF("[%s]====method:%s===\n",__func__,method);
        request_method = methodstr2int(method);
        DEBUG_PRINTF("[%s]===request_method:%d===\n",__func__,request_method);
        ret = proc_wanspeed(jsonValue,encryptinfoOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
        
    }
    else if(!strcmp(target,"proc_specialcare_cfg"))
    {
        method = cJSON_GetObjectItem(encryptinfo, "method")?cJSON_GetObjectItem(encryptinfo, "method")->valuestring:""; 
        DEBUG_PRINTF("[%s]====method:%s===\n",__func__,method);
        request_method = methodstr2int(method);
        
        ret = proc_specialcare(jsonValue,encryptinfoOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
    }
    else if(!strcmp(target,"proc_qos_cfg"))
    {
        method = cJSON_GetObjectItem(encryptinfo, "method")?cJSON_GetObjectItem(encryptinfo, "method")->valuestring:""; 
        DEBUG_PRINTF("[%s]====method:%s===\n",__func__,method);
        request_method = methodstr2int(method);
        
        ret = proc_qos(jsonValue,encryptinfoOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
    }
	else if(!strcmp(target,"proc_macblock_cfg"))
    {
        method = cJSON_GetObjectItem(encryptinfo, "method")?cJSON_GetObjectItem(encryptinfo, "method")->valuestring:""; 
        DEBUG_PRINTF("[%s]====method:%s===\n",__func__,method);
        request_method = methodstr2int(method);
        
        ret = proc_macblock(jsonValue,encryptinfoOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
    }
    else if(!strcmp(target,"proc_qtec_disk_cfg"))
    {
        method = cJSON_GetObjectItem(encryptinfo, "method")?cJSON_GetObjectItem(encryptinfo, "method")->valuestring:""; 
        DEBUG_PRINTF("[%s]====method:%s===\n",__func__,method);
        request_method = methodstr2int(method);
        
        ret = proc_qtec_disk(jsonValue,encryptinfoOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
    }
    else if(!strcmp(target,"proc_speedlimit_cfg"))
    {
        method = cJSON_GetObjectItem(encryptinfo, "method")?cJSON_GetObjectItem(encryptinfo, "method")->valuestring:""; 
        DEBUG_PRINTF("[%s]====method:%s===\n",__func__,method);
        request_method = methodstr2int(method);
        
        ret = proc_speedlimit(jsonValue,encryptinfoOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
    }
	else if(!strcmp(target,"ebtables_proc_speedlimit_cfg"))
    {
        method = cJSON_GetObjectItem(encryptinfo, "method")?cJSON_GetObjectItem(encryptinfo, "method")->valuestring:""; 
        DEBUG_PRINTF("[%s]====method:%s===\n",__func__,method);
        request_method = methodstr2int(method);
        
        ebtables_proc_speedlimit(jsonValue,encryptinfoOut);
        if( (ret==0) && (global_weberrorcode !=0))
        {
            ret=global_weberrorcode;
        }
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
    }
	//wifianti
	else if(!strcmp(target,"set_antiwifi"))
	{	
		ProcAntiwifiSet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_antiwifi_status"))
	{	
		ProcAntiwifiStatusGet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_antiwifi_question"))
	{	
		ProcQuestionSet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_antiwifi_admin_forbidden"))
	{	
		ProcAntiWifiAdminSet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_antiwifi_dev_list"))
	{	
		ProcAntiWifiDevGet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_authed_antiwifi_dev"))
	{	
		ret = ProcAntiWifiAuthSet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_antiwifi_authinfo"))
	{	
		ProcAntiWifiQueandAswGet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
    else if(!strcmp(target,"get_guest_wifi"))
	{	
		procGetGuestWifi(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_guest_wifi"))
	{	
		procSetGuestWifi(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
    else if(!strcmp(target,"set_wifi_timer"))
	{	
		ret = proc_wifitimer_sw_set(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_wifi_timer"))
	{	
	    
		ret = proc_wifitimer_get(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
    else if(!strcmp(target,"set_wifi_timer_rule"))
	{
		ret = proc_wifitimer_set(jsonValue, encryptinfoOut); 
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"del_wifi_timer_rule"))
	{	
		proc_wifitimer_del(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
    else if(!strcmp(target,"get_wds_cfg"))
	{	
		ret = proc_wdscfg_get(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
    else if(!strcmp(target,"get_wds_wifi_scan"))
	{	
		ret = proc_wds_scan(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_wds_cfg"))
	{	
		ret = proc_wdscfg_set(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
    else if(!strcmp(target,"set_up_wds"))
	{	
		ret = proc_wds_setup(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_wds_status"))
	{	
		ret = proc_wds_status_get(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"quicklycheck"))
	{	
		ProcQuicklyCheck(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
    else if(!strcmp(target,"add_vpn_cfg"))
	{	
		ret = proc_add_vpn(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
    else if(!strcmp(target,"set_vpn_cfg"))
	{	
		ret = proc_edit_vpn(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"del_vpn_cfg"))
	{	
		ret = proc_del_vpn(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
    else if(!strcmp(target,"get_vpn_cfg"))
	{	
		ret = proc_get_vpn(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_vpn_sw"))
	{	
		ret = proc_set_vpn_sw(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, ret, reuseflag, encryption);
	}
	else if(!strcmp(target,"get_wifi_firewall_cfg"))
	{	
		ProcWifiFirewallGet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}
	else if(!strcmp(target,"set_wifi_firewall_cfg"))
	{	
		ProcWifiFirewallSet(jsonValue, encryptinfoOut);
        EncryptJsonData(encryptinfoOut, jsonOut, keyid, userid, deviceid, 0, reuseflag, encryption);
	}


	ReleaseCQtKeyEncrypt(&pstcqtkeyencrypt);
	ReleaseCQtQkMangent(&pstcqtqkmangent);
	fflush(stdout);  
	dup2(f0, f2);
	buffer = cJSON_Print(jsonOut);
	printf("%s\n",buffer);
	free(buffer);
	
	cJSON_Delete(jsonOut);
	cJSON_Delete(json);
	cJSON_Delete(encryptinfo);

	return;
}

int main()
{
	int length;
	char *method;
	char *inputstring;
	
	printf("content-type:application/json\r\n\r\n");
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

