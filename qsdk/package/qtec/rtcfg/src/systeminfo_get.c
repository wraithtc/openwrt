#include <stdio.h>
#include <string.h>
#include "librtcfg.h"
#include "systeminfo_get.h"
#include "rtcfg_uci.h"

#define FILENAME "/etc/device_info"
#define MAXLINESIZE 256

/**
 *
 * func_name: getSystemInfo
 *            get basic system info:
 *                      a.product
 *                      b.serialnum
 *                      c.production version
 *                      d. whether the dut is configured 
 */
int getSystemInfo(struct systemInfo *output)
{
    FILE *fp=NULL;
	char tmp_mac[64]={0};
	char tmp1[64] = {0};
	char tmp2[64] = {0};
	char tmp3[64] = {0};
	char tmp4[64] = {0};
    char buffer[256] = {0};
    if( (fp=fopen(FILENAME,"r"))==NULL)
    {
        printf("===can't open FILE: "FILENAME"======\n");
        return -1;
    }
    
    char str[MAXLINESIZE];
    while((fgets(str,MAXLINESIZE,fp))!=NULL)
    {
        DEBUG_PRINTF("=====str : %s=====\n",str);
        if(strncmp(str,"DEVICE_PRODUCT",strlen("DEVICE_PRODUCT"))==0)
        {
            sscanf(str,"DEVICE_PRODUCT=%s",output->product);
            DEBUG_PRINTF("output->product: %s==\n",output->product);
        }
        else if(strncmp(str,"DEVICE_REVISION",strlen("DEVICE_REVISION"))==0)
        {
            sscanf(str,"DEVICE_REVISION=%s",output->productVersion);
            DEBUG_PRINTF("output->Device_REVISION; %s===\n",output->productVersion);
        }
    }
    
    fclose(fp);
	fp = NULL;

    system("ifconfig eth1 > /tmp/.lanmac");
    fp = fopen("/tmp/.lanmac","r");
	if(fp)
	{
		fgets(buffer,256,fp);
    	sscanf(buffer, "%s %s %s %s %s", tmp1,tmp2,tmp3,tmp4, tmp_mac);
		printf("buffer:%s, tmp_mac:%s", buffer, tmp_mac);
		fclose(fp);
	}
    printf("====tmp_mac: %s ====\n",tmp_mac);
    
    int i=0;
    int j=0;
    while( (tmp_mac[j]!='\0')&&(tmp_mac[j]!=0) )
    {
        if(tmp_mac[j]==':')
        {
            j++;
        }
        else
        {
            output->serialnum[i]=tmp_mac[j];
            i++;
            j++;
        }
    }
    
    output->serialnum[i] = '\0';

    printf("output->serialnum: %s ===\n", output->serialnum);

    char tmp_char[16]={0};
    rtcfgUciGet("system.@system[0].configured",tmp_char);
    output->configured=atoi(tmp_char);
    return 0;
}

int SetSystemConfigured(int value)
{
    char cmd[256]={0};
    snprintf(cmd,256,"system.@system[0].configured=%d",value);
    rtcfgUciSet(cmd);
	rtcfgUciCommit("system");
    return 0;

}

int GetSystemConfigured(int *output_value)
{
    char tmp_char[16]={0};
    rtcfgUciGet("system.@system[0].configured",tmp_char);
    *output_value = atoi(tmp_char);
    return 0;
}

int SetGuiPassword(char *password)
{
    DEBUG_PRINTF("======[%s]====password:%s==\n",__func__,password);
    char cmd[256]={0};
    snprintf(cmd,256,"system.@system[0].guipassword=%s",password);
    rtcfgUciSet(cmd);

    rtcfgUciCommit("system");
    return 0;
}

int GetGuiPassword(char *password)
{
    DEBUG_PRINTF("====[%s]==\n",__func__);
    int ret=0;
    ret=rtcfgUciGet("system.@system[0].guipassword",password);
    if(ret !=0)
    {
        DEBUG_PRINTF("===[%s]===cannt get gui password===\n",__func__);
        return -1;
    }
    else
    {
        return 0;
    }
    return 0;
}

int CheckGuiPassword(char *password)
{
    DEBUG_PRINTF("====[%s]====password:%s ==\n",__func__,password);
    char curPassword[64]={0};
    int ret=0;
    ret=rtcfgUciGet("system.@system[0].guipassword",curPassword);
    DEBUG_PRINTF("==[%s]===curPassword: %s====\n",__func__,curPassword);
    
    if(ret !=0)
    {
        DEBUG_PRINTF("===[%s]====cant get gui password===\n",__func__);
        return -1;
    }
    else 
    {
        if(strlen(password)!=strlen(curPassword))
        {
            DEBUG_PRINTF("===[%s]====password not match====\n",__func__);
            return -1;
        }
        else
        {   
            if(strncmp(password,curPassword,strlen(password)) !=0)
            {
                DEBUG_PRINTF("===[%s]===password not match===\n",__func__);
                return -1;
            }
            else
            {
                DEBUG_PRINTF("===[%s]===password match====\n",__func__);
                return 0;
            }
        }
    }
    return 0;
}

int setTokenId(char *inputTokenId)
{
    DEBUG_PRINTF("====[%s]====inputTokenId:%s====\n",__func__,inputTokenId);
    int ret=0;
    char cmd[256]={0};
    snprintf(cmd,256,"system.@system[0].guitokenid=%s",inputTokenId);
    ret = rtcfgUciSet(cmd);
    if(ret !=0)
    {
        DEBUG_PRINTF("===[%s]===cant set guitokenid into system\n",__func__);
        return -1;
    }
    else
    {
        rtcfgUciCommit("system");
        return 0;
    }
}


int getTokenId(char *outputTokenId)
{
    DEBUG_PRINTF("====[%s]==\n",__func__);
    int ret=0;
    ret=rtcfgUciGet("system.@system[0].guitokenid",outputTokenId);
    if(ret !=0)
    {
        DEBUG_PRINTF("===[%s]===cannt get gui token id===\n",__func__);
        return -1;
    }
    else
    {
        return 0;
    }
    return 0;
}

int checkTokenId(char *inputTokenId)
{
    DEBUG_PRINTF("===[%s]===inputTokenId:%s===\n",__func__,inputTokenId);
    
    int ret=0;
    char curTokenId[200]={0};
    ret=rtcfgUciGet("system.@system[0].guitokenid",curTokenId);
    DEBUG_PRINTF("==[%s]===curTokenId: %s====\n",__func__,curTokenId);
    
    if(ret !=0)
    {
        DEBUG_PRINTF("===[%s]====cant get Token Id===\n",__func__);
        return -1;
    }
    else 
    {
        if(strlen(inputTokenId)!=strlen(curTokenId))
        {
            DEBUG_PRINTF("===[%s]====TokenId not match====\n",__func__);
            return -1;
        }
        else
        {   
            if(strncmp(inputTokenId,curTokenId,strlen(inputTokenId)) !=0)
            {
                DEBUG_PRINTF("=====[%s]===token id not match===\n",__func__);
                return -1;
            }
            else
            {
                DEBUG_PRINTF("===[%s]===token id match====\n",__func__);
                return 0;
            }
        }
    }
    return 0;
}
