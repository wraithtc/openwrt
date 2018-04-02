#include "basic.h"

struct specialcareEntry_json{
    char macaddr[64];
    int flag;
};

int check_specialcare_value(char *input_string)
{
    DEBUG_PRINTF("[%s]====input_string:%s====\n",__func__,input_string);
    char tmpchar[128]={0};
    char *delim=" ";
    char *p;
    int index=0;
    int i=0;
    int j=0;

    //check emty string
    if(strlen(input_string)==0)
    {
        return 0;
    }
    struct specialcareEntry_json specialcareArray[64]={0};
    p=strtok(input_string,delim);
    DEBUG_PRINTF("[%s]====p:%s=====\n",__func__,p);

    //check only space string
    if(p==NULL)
    {
        return 0;
    }
    
    memset(tmpchar,0,sizeof(tmpchar));
    UTIL_STRNCPY(tmpchar,p,sizeof(tmpchar)-1);
    specialcareArray[index].flag=atoi(&(tmpchar[strlen(tmpchar)-1]));
    tmpchar[strlen(tmpchar)-2]='\0';
    UTIL_STRNCPY(specialcareArray[index].macaddr,tmpchar,sizeof(specialcareArray[index].macaddr)-1);
    index++;

    while((p=strtok(NULL,delim)))
    {
        memset(tmpchar,0,sizeof(tmpchar));
        UTIL_STRNCPY(tmpchar,p,sizeof(tmpchar));
        specialcareArray[index].flag=atoi(&(tmpchar[strlen(tmpchar)-1]));
        tmpchar[strlen(tmpchar)-2]='\0';
        UTIL_STRNCPY(specialcareArray[index].macaddr,tmpchar,sizeof(specialcareArray[index].macaddr)-1);
        index++;
    }

    //至此获取到特殊关注的设备及对应的flag
    for(i=0;i<index;i++)
    {
        DEBUG_PRINTF("[%s]  specialcareArray[%d] macaddr:%s flag:%d====\n",__func__,i,specialcareArray[i].macaddr,specialcareArray[i].flag);
        for(j=0;j<index;j++)
        {
            if( (i!=j) && (strncmp(specialcareArray[i].macaddr, specialcareArray[j].macaddr, sizeof(specialcareArray[i].macaddr)) == 0) )
            {
                global_weberrorcode = ERR_VALUE_WRONG;
                return -1;
            }
        }
        
    }

    return 0;
   
}

void proc_specialcare_cfg_set(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]=======\n",__func__);

    char *specialcare = NULL;
    int ret;
    char cmd[2048]={0};
    char tmp_specialcare[256]={0};
    specialcare = cJSON_GetObjectItem(jsonValue, "specialcare")?cJSON_GetObjectItem(jsonValue, "specialcare")->valuestring:"";
    UTIL_STRNCPY(tmp_specialcare,specialcare,256);
  
    ret=check_specialcare_value(specialcare);
    DEBUG_PRINTF("==[%s]===ret %d===\n",__func__,ret);
    if(ret == -1)
    {
        global_weberrorcode = ERR_VALUE_WRONG;
        return ;
    }

    snprintf(cmd,sizeof(cmd),"system.@system[0].specialcare=%s",tmp_specialcare);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("system");

    
    return;
    
}

void proc_specialcare_cfg_get(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);
    int ret=0;
    char tmp_char[2048]={0};
    
    cJSON *obj = NULL;
    ret = rtcfgUciGet("system.@system[0].specialcare",tmp_char);
    
    obj = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "specialcare", cJSON_CreateString(tmp_char));
    return;
}

int proc_specialcare(cJSON *jsonValue, cJSON *jsonOut)
{
    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_specialcare_cfg_get(jsonValue, jsonOut);
    }
    else if  ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
    	proc_specialcare_cfg_set(jsonValue, jsonOut);
    }
	else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
        return ERR_METHOD_NOT_SUPPORT;
    }

    return 0;
}

