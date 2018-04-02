#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <librtcfg.h>
#include <libubox/uloop.h>
#include <libubox/list.h>  
#include <libubox/blobmsg_json.h>  
#include "signal.h"

#define DEBUG_PRINTF(format,...)   printf(format, ##__VA_ARGS__);
#define DEBUG_PRINTF_RED(format,...) printf("\e[1;31m"format"\e[0m",##__VA_ARGS__)
#define DEBUG_PRINTF_GRE(format,...) printf("\e[1;32m"format"\e[0m",##__VA_ARGS__)




enum status_code {
    STATUS_CODE_INIT =               3000,   //初始状态
    STATUS_CODE_KEY_FAIL=            3001,   //加密芯片问题，获取不到32位KEY
    STATUS_CODE_DISK_FAIL=           3002,   //未发现磁盘
    STATUS_CODE_OPEN_FAIL=           3003,   //不能用密码打开磁盘，此刻需提示用户是否格式化磁盘，和加密磁盘，下一步动作需等待用户确认

    STATUS_CODE_SUCCESS=                0,   //以正确的姿势打开磁盘
};

enum process_code {
    PROCESS_CODE_INIT   =             4000,
    PROCESS_CODE_FORMATING  =         4001,  //正在格式化...
    PROCESS_CODE_ENCRYING   =         4002,  //正在加密中...
    PROCESS_CODE_CREATING   =         4003,  //正在创建文件系统（ext4)中...

    PROCESS_CODE_DONE       =            0,   //成功完成磁盘格式化加密和文件系统创建
    PROCESS_CODE_FAIL       =         4999,   //磁盘未能成功的格式化加密和文件系统创建 （不是-1，是怕app那边未能正确识别-1，且会与其他功能冲突）
};

char err_msg[256]={0}; //保存process_code err_msg
char status_msg[256]={0}; //保存status_code msg

//现在的信号处理函数会自动重新安装信号处理
static void qtec_diskmanager_sig_handler(int sig_no)
{
    DEBUG_PRINTF_RED("[%s]====sign_no:%d======\n",__func__,sig_no);
    if(sig_no==SIGUSR1)
    {
        DEBUG_PRINTF_RED("[%s]===recive SIGUSR1 to start disk re-format===\n",__func__);
        
    }
    else if(sig_no == SIGUSR2)
    {
        DEBUG_PRINTF("[%s]====recive SIGUSR2 to get the info that disk remove===\n",__func__);
    }
}

//改进程不设置成守护进程
//该进程设置成，确保磁盘正确打开，正确打开后则退出，否则循环尝试
void main()
{
    DEBUG_PRINTF("===qtec_diskmanager start=======\n");
    signal(SIGUSR1,qtec_diskmanager_sig_handler);
    signal(SIGUSR2,qtec_diskmanager_sig_handler);
    while(1)
    {
        DEBUG_PRINTF("[%s]===while loop====\n",__func__);
        sleep(1);
    }
       
    
}

