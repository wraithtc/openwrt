#include "basic.h"

//获取和修改密码

void proc_rsacfg_encrypt(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("===[%s]===\n",__func__);
    char *str=NULL;
    str=cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"";
    unsigned char *result=NULL;
    int result_len=0;
    result = (unsigned char *)my_encrypt(str, RSA_PUBLIC_KEY_FILE,&result_len);
    if(result == NULL)
    {
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("encrypt fail"));
    }
    else
    {
        char base64[2048]={0};
        base64_encode(result, base64, result_len);
        DEBUG_PRINTF("====base64: %s  ====\n", base64);
        
        cJSON_AddItemToObject(jsonOut, "result", cJSON_CreateString(base64));
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
    }
}


//void proc_rsacfg_decrypt(cJSON *jsonValue,cJSON *jsonOut)
void check_gui_password(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]=====\n",__func__);
    cJSON *obj = NULL;
	obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    char password[256]={0};
    UTIL_STRNCPY(password,cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"",sizeof(password));

    char username[64]={0};
    UTIL_STRNCPY(username,cJSON_GetObjectItem(jsonValue, "username")?cJSON_GetObjectItem(jsonValue, "username")->valuestring:"",sizeof(username));

    if( (strlen(password)==0) || (strlen(username)==0) )
    {
        global_weberrorcode = ERR_PARAMETER_MISS;
        return;
    }
    
#if 0
    char *target=NULL;
    target = cJSON_GetObjectItem(jsonValue, "target")?cJSON_GetObjectItem(jsonValue, "target")->valuestring:"";
#endif

   
    unsigned char bindata[2048]={0};
    int len_debug=base64_decode(password, bindata);
	//DEBUG_PRINTF("===[%s]====len_debug:%d====\n",__func__,len_debug);
	//DEBUG_PRINTF("===[%s]====strlen(bindata): %d=====\n",__func__,strlen(bindata));
	//char str_debug[256]={0};
    //base64_encode( bindata, str_debug, len_debug );
	//DEBUG_PRINTF("===[%s]====str_debug: %s====\n",__func__,str_debug);
    
    char *result=NULL;
    result=(char *)my_decrypt(len_debug, bindata, RSA_PRIVATE_KEY_FILE);
    if(result == NULL)
    {
        global_weberrorcode= ERR_DECRY_FAIL;
        return;
        #if 0
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("password decry fail"));
        #endif 
    }

    int ret=0;
    ret=CheckGuiPassword(result);

    if(ret !=0)
    {
        global_weberrorcode=ERR_PASSWORD_NOTMATCH;

    }
    else
    {
        char tokenid[MAX_TOKENID_LEN]={0};
        generatortokenid(tokenid);
        setTokenId(tokenid);
        cJSON_AddItemToObject(obj, "tokenid", cJSON_CreateString(tokenid));
     
    }
    if(result!=NULL)
    {
        free(result);
    }
}

//void proc_rsacfg_decrypt(cJSON *jsonValue,cJSON *jsonOut)
void reset_gui_password(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]=====\n",__func__);
    cJSON *obj = NULL;
	obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    char password[256]={0};
    UTIL_STRNCPY(password,cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"",sizeof(password));

    char username[64]={0};
    UTIL_STRNCPY(username,cJSON_GetObjectItem(jsonValue, "username")?cJSON_GetObjectItem(jsonValue, "username")->valuestring:"",sizeof(username));
    

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
    else
    {
        //在set之前需要检测老密码是否匹配
        int ret=0;
        char oldPassworld[64]={0};
        ret=GetGuiPassword(oldPassworld);
        //如果当前系统密码为空或不可取时，不检测老密码参数，这种情况应该发生在第一次配置
        if( (ret!=0) || (strlen(oldPassworld)==0) )
        {
            DEBUG_PRINTF("===[%s]====cant get current password from system, so ignore password match===\n",__func__);
        }
        else
        {
            char oldpassword[256]={0};
            UTIL_STRNCPY(oldpassword,cJSON_GetObjectItem(jsonValue, "oldpassword")?cJSON_GetObjectItem(jsonValue, "oldpassword")->valuestring:"",sizeof(oldpassword));
            unsigned char old_bindata[2048]={0};
            int len_debug=base64_decode(oldpassword, old_bindata);
            char *old_result=NULL;
            old_result=(char *)my_decrypt(len_debug,old_bindata, RSA_PRIVATE_KEY_FILE);
            if(old_result==NULL)
            {
                global_weberrorcode=ERR_DECRY_FAIL;
                return;
            }
            ret=CheckGuiPassword(old_result);
            if(old_result != NULL)
            {
                free(old_result);
            }
            if(ret !=0)
            {
                global_weberrorcode=ERR_PASSWORD_NOTMATCH;
                return;
            }
        }
        
        ret=SetGuiPassword(result);
        if(ret !=0)
        {
            global_weberrorcode= ERR_INTERNALLOGIC_WRONG;
            return;
        }
        else
        {   
            char tokenid[200]={0};
            generatortokenid(tokenid);
            setTokenId(tokenid);
            cJSON_AddItemToObject(obj, "tokenid", cJSON_CreateString(tokenid));
        }
        if(result!=NULL)
        {
            free(result);
        }
    }
}

void proc_firstconfigure_set(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]=====\n",__func__);

    //处理wifi 
   // proc_wireless_set(jsonValue,jsonOut);

    char ssid[256]={0};
    UTIL_STRNCPY(ssid,cJSON_GetObjectItem(jsonValue, "ssid")?cJSON_GetObjectItem(jsonValue, "ssid")->valuestring:"",sizeof(ssid));
    if(strlen(ssid) == 0)
    {
        global_weberrorcode = ERR_PARAMETER_MISS;
        return;
    }
    
    char rsa_key[256]={0};
    UTIL_STRNCPY(rsa_key,cJSON_GetObjectItem(jsonValue, "key")?cJSON_GetObjectItem(jsonValue, "key")->valuestring:"",sizeof(rsa_key));
    if(strlen(rsa_key)==0)
    {
        global_weberrorcode = ERR_PARAMETER_MISS;
        return;
    }

    unsigned char bindata[2048]={0};
    int len_debug=base64_decode(rsa_key,bindata);

    char *wifi_key=NULL;
    wifi_key=(char *)my_decrypt(len_debug,bindata,RSA_PRIVATE_KEY_FILE);
    if(wifi_key == NULL)
    {
        global_weberrorcode = ERR_DECRY_FAIL;
        return;
    }
    
    WifiConfig  stConfig = {0};
    UTIL_STRNCPY(stConfig.Ssid1,ssid,sizeof(stConfig.Ssid1));
    UTIL_STRNCPY(stConfig.Ssid2,ssid,sizeof(stConfig.Ssid2));
    UTIL_STRNCPY(stConfig.Key1,wifi_key,sizeof(stConfig.Key1));
    UTIL_STRNCPY(stConfig.Key2,wifi_key,sizeof(stConfig.Key2));
    UTIL_STRNCPY(stConfig.Channel1, "auto", sizeof(stConfig.Channel1));
    UTIL_STRNCPY(stConfig.Channel2, "auto", sizeof(stConfig.Channel2));
    //设置加密类型默认值
    strcpy(stConfig.Encryption1,"psk-mixed");
    strcpy(stConfig.Encryption2,"psk-mixed");
    
    int ret=0;
    ret = setWifiConfig(&stConfig);
    if(wifi_key !=NULL)
    {
        free(wifi_key);
    }
	if(ret !=0)
	{
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;  
        return;
	}
	else
	{
		ProcWifiMsgReq();
	}
    
    cJSON *obj = NULL;
	obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    char password[256]={0};
    UTIL_STRNCPY(password,cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"",sizeof(password));

    if(strlen(password)==0)
    {
        global_weberrorcode = ERR_PARAMETER_MISS;
        return;
    }
    DEBUG_PRINTF("===[%s]===str: %s ===\n",__func__,password);
    memset(bindata,0,sizeof(bindata));
    len_debug=base64_decode(password, bindata);
    
    char *result=NULL;
    result=(char *)my_decrypt(len_debug,bindata, RSA_PRIVATE_KEY_FILE);
    if(result == NULL)
    {
        global_weberrorcode=ERR_DECRY_FAIL;
        return;
    }
    else
    {
        //在set之前需要检测老密码是否匹配
        int ret=0;
           
        ret=SetGuiPassword(result);
        if(result !=NULL)
        {
            free(result);
        }
        
        if(ret !=0)
        {
           global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
           return;
        }
        else
        {   
            char tokenid[200]={0};
            generatortokenid(tokenid);
            setTokenId(tokenid);
            cJSON_AddItemToObject(obj, "tokenid", cJSON_CreateString(tokenid));

            //设置系统配置为1
            SetSystemConfigured(1);

        }
     
    }
    
}

int ProcFirstconfigureSetByApp(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]=====\n",__func__);
	int  ret = 0;
    char ssid[64] = {0};
    char password[64] = {0};
    char key[64] = {0};
    WifiConfig  stConfig = {0};
    
    UTIL_STRNCPY(ssid,cJSON_GetObjectItem(jsonValue, "ssid")?cJSON_GetObjectItem(jsonValue, "ssid")->valuestring:"",sizeof(ssid));
    UTIL_STRNCPY(key,cJSON_GetObjectItem(jsonValue, "key")?cJSON_GetObjectItem(jsonValue, "key")->valuestring:"",sizeof(key));
    UTIL_STRNCPY(password,cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"",sizeof(password));
    
    if((0 == ssid[0]) || (0 == key[0]) || (0 == password[0]))
    {
        return ERR_INTERNALLOGIC_WRONG;
    }
    
    strcpy(stConfig.Key1, key);
    strcpy(stConfig.Key2, key);
    strcpy(stConfig.Encryption1, "psk-mixed");
    strcpy(stConfig.Encryption2, "psk-mixed");
    sprintf(stConfig.Ssid1, "%s", ssid);
    sprintf(stConfig.Ssid2, "%s", ssid);
    UTIL_STRNCPY(stConfig.Channel1, "auto", sizeof(stConfig.Channel1));
    UTIL_STRNCPY(stConfig.Channel2, "auto", sizeof(stConfig.Channel2));
    //set wifi
    ret = setWifiConfig(&stConfig);
    if(ret)
    {
        printf("wifi set fail.");
        return ERR_INTERNALLOGIC_WRONG;
    }
    else
	{
		ProcWifiMsgReq();
	}

    //set guipassword
    SetGuiPassword(password);

    //set router configured
    SetSystemConfigured(1);

    return 0;
}

int ProcPasswordCheckByApp(cJSON *jsonValue,cJSON *jsonOut)
{
	int ret = 0;
	char oldPassword[64]={0};	
	char storePassworld[64]={0};

	UTIL_STRNCPY(oldPassword,cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"",sizeof(oldPassword));

    ret=GetGuiPassword(storePassworld);
    //如果当前系统密码为空或不可取时，不检测老密码参数，这种情况应该发生在第一次配置
    if( (ret!=0) || (strlen(storePassworld)==0) )
    {
        DEBUG_PRINTF("===[%s]====cant get current password from system, so ignore password match===\n",__func__);
		return 0;
    }
	else
	{
		ret=CheckGuiPassword(oldPassword);
	}	
	if(ret)
    {
        return ERR_PASSWORD_NOTMATCH;
    }
    else
    {
        return 0;
    }
}

int ProcPasswordSetByApp(cJSON *jsonValue,cJSON *jsonOut)
{
	int ret = 0;
	char password[64] = {0};
    char oldPassword[64] = {0};
    char storePassword[64] = {0};
	cJSON *obj = NULL;
	cJSON *encryptinfo = NULL;

	obj = cJSON_CreateObject();
	encryptinfo = cJSON_CreateObject();
    
	UTIL_STRNCPY(password,cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"",sizeof(password));
    UTIL_STRNCPY(oldPassword,cJSON_GetObjectItem(jsonValue, "oldpassword")?cJSON_GetObjectItem(jsonValue, "oldpassword")->valuestring:"",sizeof(oldPassword));
	if((0 == password[0]) || (0 == oldPassword[0]))
	{
		return ERR_PARAMETER_MISS;
	}
	
    ret=GetGuiPassword(storePassword);
    //如果当前系统密码为空或不可取时，不检测老密码参数，这种情况应该发生在第一次配置
    if( (ret!=0) || (strlen(storePassword)==0) )
    {
        DEBUG_PRINTF("===[%s]====cant get current password from system, so ignore password match===\n",__func__);
		ret = 0;
    }
	else
	{
		ret=CheckGuiPassword(oldPassword);
	}

    //if password match,set newpassword.
    if(0 == ret)
    {
        SetGuiPassword(password);
        return 0;
    }
    else
    {
        return ERR_PASSWORD_NOTMATCH;
    }
}

