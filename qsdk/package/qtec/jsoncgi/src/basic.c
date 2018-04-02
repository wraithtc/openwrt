#include "basic.h"

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//define global variable
int global_weberrorcode=0;
char global_requesturl[64]={0};
int request_method=0;

//生成tokenid
int generatortokenid(char *tokenid)
{
    srand((int)time(0));
    int i =rand()%1000;
    //计算MD5值
    MD5_CTX ctx;
    unsigned char md[16];
    char buf[33]={'\0'};
    char tmp[3]={'\0'};
    MD5_Init(&ctx);
    MD5_Update(&ctx,&i,sizeof(i));
    MD5_Final(md,&ctx);

    for(i=0;i<16;i++){
        sprintf(tmp,"%02x",md[i]);
        strcat(buf,tmp);
    }
  
    snprintf(tokenid,MAX_TOKENID_LEN,"%s",buf);
  
    return 0;
}



//base64 加码
char * base64_encode( const unsigned char * bindata, char * base64, int binlength )
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}


//base64解码
int base64_decode(char * base64, unsigned char * bindata )
{

    //gui 前端 可能会把+替换成空格，所以我们先要把它转回来
    //假设正常的base64应该不包含空格
    int length=0;
    for(length=0;length<strlen(base64);length++)
    {
        if(base64[length]==' ')
            base64[length]='+';
    }
    //end 转换空格到+
    
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}


//检测tokenid 
//若检验成功则返回0，否则返回非0
int webCheckTokenId()
{
    //如果是第一次配置，或者系统内tokenid存值为0， 则跳过检验直接返回成功
    int system_configured=0;
    int ret=0;
    GetSystemConfigured(&system_configured);
    if(system_configured == 0)
    {
        DEBUG_PRINTF("[%s]====current system is not configured====\n",__func__);
        return 0;
    }

    //获取系统token id
    char tokenid[MAX_TOKENID_LEN]={0};
    ret=getTokenId(tokenid);
    if( (ret!=0) || (strlen(tokenid)==0) )
    {
        DEBUG_PRINTF("[%s]only when system is unconfigured, tokenid is null, but if this line is printed, so bad thing occur===\n",__func__);
        return -1;  
    }

    DEBUG_PRINTF("[%s]====system token id: %s ====\n",__func__,tokenid);
    
    //获取http头的tokenid
    char *httptokenid=NULL;
    httptokenid=getenv("HTTP_TOKEN_ID");

    DEBUG_PRINTF("[%s]===http token id: %s ====\n", __func__,httptokenid);

    if(httptokenid == NULL)
    {
        DEBUG_PRINTF("[%s] not have http token id===\n",__func__);
        return -1;
    }
    
    if(strlen(httptokenid) != strlen(tokenid) )
    {
        DEBUG_PRINTF("[%s] token id not match===\n",__func__);
        return -1;
    }

    if(strncmp(tokenid,httptokenid,strlen(tokenid)) != 0)
    {
        DEBUG_PRINTF("[%s] token id not match===\n",__func__);
        return -1;
    }
    else
    {
        DEBUG_PRINTF("[%s] token id match ===\n",__func__);
        return 0;
    }
    
}

/*
//在这里定义错误类型
enum web_error_code {
     ERR_NO_LOGIN = -100,
     ERR_URL_NOT_SUPPORT= -99,
     ERR_METHOD_NOT_SUPPORT=-98,
     ERR_PARAMETER_MISS=-97,
     ERR_VALUE_WRONG=-96,
     ERR_INTERNALLOGIC_WRONG=-95,
     ERR_OTHER=0
};

*/

int err2msg(int errcode,char *input_msg, int msg_len)
{
    DEBUG_PRINTF("[%s]===errcode: %d  msg_len: %d====\n",__func__,errcode,msg_len);
    switch(errcode)
    {
        case 0: //因为正常情况下应该都返回成功
            UTIL_STRNCPY(input_msg,"success",msg_len);
            break;
        case ERR_NO_LOGIN:
            UTIL_STRNCPY(input_msg,"not login",msg_len);
            break;
        case ERR_URL_NOT_SUPPORT:
            UTIL_STRNCPY(input_msg,"url not support",msg_len);
            break;
        case ERR_METHOD_NOT_SUPPORT:
            UTIL_STRNCPY(input_msg,"method not support",msg_len);
            break;
        case ERR_PARAMETER_MISS:
            UTIL_STRNCPY(input_msg,"key parameter miss",msg_len);
            break;
        case ERR_VALUE_WRONG:
            UTIL_STRNCPY(input_msg,"value illegal",msg_len);
            break;
        case ERR_INTERNALLOGIC_WRONG:
            UTIL_STRNCPY(input_msg, "internal logic wrong", msg_len);
            break;
        case ERR_DECRY_FAIL:
            UTIL_STRNCPY(input_msg, "password decry fail", msg_len);
            break;
        case ERR_PASSWORD_NOTMATCH:
            UTIL_STRNCPY(input_msg, "password not match", msg_len);
            break;
		case ERR_PASSWORD_TOO_SIMPLE:
			UTIL_STRNCPY(input_msg, "wifi key too simple", msg_len);
			break;
        case ERR_WAN_IPWRONG:
            UTIL_STRNCPY(input_msg, "wan miss ipaddress", msg_len);
            break;
        case ERR_WAN_GWWRONG:
            UTIL_STRNCPY(input_msg, "wan gw wrong or cannot access gw", msg_len);
            break;
        case ERR_WAN_DNSWRONG:
            UTIL_STRNCPY(input_msg, "wan cannot get DNS", msg_len);
            break;
        case ERR_WAN_INTERNETWRONG:
            UTIL_STRNCPY(input_msg, "wan cannot access internet", msg_len);
            break;
        case ERR_WAN_NOTUP:
            UTIL_STRNCPY(input_msg, "WAN not up or can not access server", msg_len);
            break;
        case ERR_NO_PERMIT:
            UTIL_STRNCPY(input_msg, "current no permit to handle this url", msg_len);
            break;
        case ERR_NO_MATCH:
            UTIL_STRNCPY(input_msg, "no find the match entry rule", msg_len);
            break;
		case ERR_NO_SPEEDTEST:
			UTIL_STRNCPY(input_msg, "speed test not started", msg_len);
			break;
		case ERR_GET_SMB_PSW_FAIL:
			UTIL_STRNCPY(input_msg, "get smb password failed", msg_len);
			break;
		case ERR_GET_UPGRADE_RATE_FAIL:
			UTIL_STRNCPY(input_msg, "get update rate failed", msg_len);
			break;
        case ERR_IP_NOT_INTER:
            UTIL_STRNCPY(input_msg, "ip not in lan subnet", msg_len);
			break;
        case ERR_RULE_CONFLICT:
            UTIL_STRNCPY(input_msg, "rule conflict",msg_len);
            break;
		case ERR_GET_UPDATE_INFO_FAIL:
            UTIL_STRNCPY(input_msg, "get update info failed",msg_len);
            break;
        case ERR_OFFLINE_OR_IN_MACBLOCK:
            UTIL_STRNCPY(input_msg, "device offline or in macblock",msg_len);
            break;
        default:
            UTIL_STRNCPY(input_msg, "unknown reason error", msg_len);
            break;
    }

    return 0;
}

//根据错误码返回jason报文
int err2replymsg(int errcode, char *input_url, cJSON * jsonOut)
{
    DEBUG_PRINTF("[%s]===errcode: %d====\n",__func__,errcode);
    cJSON_AddItemToObject(jsonOut, "errorcode", cJSON_CreateNumber(errcode));
    cJSON_AddItemToObject(jsonOut, "requestedurl", cJSON_CreateString(input_url));
    char msg[256]={0};
    err2msg(errcode, msg, sizeof(msg));
	cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString(msg));
    return 0;
}

//测试ping 结果
//return 0 :表示 能ping 通
//ping 192.168.1.1 -c 4  -I eth0.2 -q
int checkPingResult(char* input_destip, char* input_ifname)
{
    DEBUG_PRINTF("[%s]====input_destip: %s===input_ifname: %s\n",__func__,input_destip,input_ifname);
    FILE *fp;
    char cmd[256]={0};
    snprintf(cmd,256,"ping %s -c 4 -I %s -q",input_destip,input_ifname);
   
    fp=popen(cmd,"r");
    char line[256];
    if(fp != NULL)
    {
        while (fgets(line,sizeof(line),fp) != NULL)
        {
            if(strstr(line,"round-trip")!=NULL)
            {
                return 0;
            }
           
        }

        return 1;
    }
    else
    {
        DEBUG_PRINTF("[%s]====popen fail===\n",__func__);
        return -1;
    }
     
}

//GetNextSynKeyId id+1
void GetNextSynKeyId(unsigned char *keyid)
{
    unsigned long int nextkeyid = 0;
    sscanf(keyid, "10000000%d", &nextkeyid);
    nextkeyid++;
    printf("nettkeyid is %d.\n", nextkeyid);
    memset(keyid, 0, 16);
    snprintf(keyid, 17, "10000000%8ld", nextkeyid);
    return;
}

void GetFirstEncodeKey(unsigned char *key)
{
	int i, random;
	char randbuffer[16] = {0};
	char cmd[256] = {0};

    srand((int)time(0));
    random = rand()%1000000;

    printf("rand index is:%d.\n", random);
	fflush(stdout);
	snprintf(randbuffer, 16, "%06d", random);
    ProcDisplayFirstKeyMsgReq(randbuffer);
	//snprintf(cmd, 256, "i2c_ctrl %s", randbuffer);
	//system(cmd);
	printf("randbuffer:%s,cmd:%s.\n", randbuffer, cmd);
	memset(cmd, 0, 256);
	snprintf(cmd, 256, "echo %06d > /tmp/firstencodekey", random);
	system(cmd);

    //计算MD5值
    MD5_CTX ctx;
    unsigned char md[16] = {0};
    MD5_Init(&ctx);
    MD5_Update(&ctx,randbuffer,strlen(randbuffer));
    MD5_Final(md,&ctx);

    printf("md:");
	for(i = 0; i < 16;i++)
	{
		printf("0x%02x ",md[i]);  
	}
	printf("\n");  

    memcpy(key, md, 16); 

    return;
}