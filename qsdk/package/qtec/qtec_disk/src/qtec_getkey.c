#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sec_api.h>
#include <librtcfg.h>

#define PRINTF_RED(format,...) printf("\e[1;31m"format"\e[0m",##__VA_ARGS__)
#define PRINTF_GRE(format,...) printf("\e[1;32m"format"\e[0m",##__VA_ARGS__)

void *g_msgHandle;

//该进程调用spiinit 的LoadData 接口 获取16个字节的16进制数，然后将他转换成可读字符串，当做下一步磁盘加密的密码
int main()
{
    int ret=0;
    char mainKey[16]={0};
    char keystring[256]={0};
    char cmd[256]={0};
    int i=0;
    ret=vosMsg_init(EID_QTEC_GETKEY, &g_msgHandle);
    if (ret != VOS_RET_SUCCESS)
    {
        return -1;
    }
    if ((ret = QtGetSpiLock(g_msgHandle)) == 0)
    {
        ret = LoadData(mainKey, 16);
        QtReleaseSpiLock(g_msgHandle);
    }
    if(ret != 0)
    {
        PRINTF_RED("[%s]===qtec_getkey cannot get mainkey from spiint===\n",__func__);
        return -1;
    }


    for(i=0;i<16;i++)
    {
        PRINTF_GRE("%02x ",mainKey[i]);
    }

    snprintf(keystring,sizeof(keystring),"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",mainKey[0],mainKey[1],mainKey[2],mainKey[3],mainKey[4],mainKey[5],mainKey[6],mainKey[7],mainKey[8],mainKey[9],mainKey[10],mainKey[11],mainKey[12],mainKey[13],mainKey[14],mainKey[15]);
    if(strncmp(keystring,"01000000000000000000000000000000",strlen(keystring))==0)
    {
        PRINTF_RED("[%s]===qtec_getkey get wrong key from spiint===\n",__func__);
        return -1;
    }
    snprintf(cmd,256,"qtec_disk.status.status_key=%s",keystring);
    rtcfgUciSet(cmd);

    return 0;
    
    
}

