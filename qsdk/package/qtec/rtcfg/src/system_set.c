#include "system_set.h"
#include "network_set.h"
#include "sys/resource.h"



void QtReboot()
{
    UTIL_DO_SYSTEM_ACTION("killall consoled");
    UTIL_DO_SYSTEM_ACTION("reboot");
}

void QtRestore()
{
    UTIL_DO_SYSTEM_ACTION("firstboot -y");
    UTIL_DO_SYSTEM_ACTION("killall consoled");
    UTIL_DO_SYSTEM_ACTION("reboot");
}

VOS_RET_E QtSetSysTime(QT_TIME_STRU *pstTimeInfo)
{
    FILE *fp;
    char aucBuf[32] = {0};
    printf("===%s===\n",__func__);fflush(stdout);
    if (pstTimeInfo == NULL 
        || pstTimeInfo->ucSec >= QT_MAX_SEC
        || pstTimeInfo->ucMin >= QT_MAX_MIN
        || pstTimeInfo->ucHour >= QT_MAX_HOUR
        || pstTimeInfo->ucMonth > QT_MAX_MONTH
        || pstTimeInfo->ucDay > QT_MAX_DAY_MONTH_BIG
        || (pstTimeInfo->usYear < QT_MIN_YEAR || pstTimeInfo->usYear > QT_MAX_YEAR))
    {
        return VOS_RET_INVALID_ARGUMENTS;
    }
    printf("===%s=%d==\n",__func__,__LINE__);fflush(stdout);
    UTIL_DO_SYSTEM_ACTION("date -s %d.%d.%d-%d:%d:%d > %s", pstTimeInfo->usYear, pstTimeInfo->ucMonth, 
                                                pstTimeInfo->ucDay, pstTimeInfo->ucHour, 
                                                pstTimeInfo->ucMin, pstTimeInfo->ucSec, QT_TIME_SET_RESULT_FILE);
    if ((fp = fopen(QT_TIME_SET_RESULT_FILE, "r")) == NULL)
    {
        vosLog_error("Fail to open time set result file!");
        return VOS_RET_INTERNAL_ERROR;
    }
    printf("===%s=%d==\n",__func__,__LINE__);fflush(stdout);
    if (fread(aucBuf, 10, 1, fp) < 0)
    {
        vosLog_error("Fail to read from time set result file!");
        return VOS_RET_INTERNAL_ERROR;
    }
    printf("===%s=%d==\n",__func__,__LINE__);fflush(stdout);
    if (util_strstr(aucBuf, "BusyBox") != NULL)
    {
        vosLog_error("Fail to set time!");
        return VOS_RET_INVALID_ARGUMENTS;
    }
    printf("===%s=%d==\n",__func__,__LINE__);fflush(stdout);
    return VOS_RET_SUCCESS;
}

static UINT8 qtMonthEngToNum(const char *aucMonthEng)
{
    if (aucMonthEng == NULL)
    {
        return 0;
    }

    if (!util_strcmp(aucMonthEng, "Jan"))
    {
        return 1;
    }
    else if (!util_strcmp(aucMonthEng, "Feb"))
    {
        return 2;
    }
    else if (!util_strcmp(aucMonthEng, "Mar"))
    {
        return 3;
    }
    else if (!util_strcmp(aucMonthEng, "Apr"))
    {
        return 4;
    }
    else if (!util_strcmp(aucMonthEng, "May"))
    {
        return 5;
    }
    else if (!util_strcmp(aucMonthEng, "Jun"))
    {
        return 6;
    }
    else if (!util_strcmp(aucMonthEng, "Jul"))
    {
        return 7;
    }
    else if (!util_strcmp(aucMonthEng, "Aug"))
    {
        return 8;
    }
    else if (!util_strcmp(aucMonthEng, "Sep"))
    {
        return 9;
    }
    else if (!util_strcmp(aucMonthEng, "Oct"))
    {
        return 10;
    }
    else if (!util_strcmp(aucMonthEng, "Nov"))
    {
        return 11;
    }
    else if (!util_strcmp(aucMonthEng, "Dec"))
    {
        return 12;
    }
    else
    {
        return 0;
    }
}

VOS_RET_E QtGetSysTime(QT_TIME_STRU *pstTimeInfo)
{
    FILE *fp;
    char aucBuf[128] = {0},aucMonth[8] = {0}, aucWeekDay[8] = {0}, aucDay[8] = {0}, aucHour[8] = {0}, aucMin[8] = {0};
    UINT8 ucMonth;
    int ret;

    if (pstTimeInfo == NULL)
    {
        vosLog_error("pstTimeInfo is NULL!");
        return VOS_RET_INVALID_ARGUMENTS;
    }

    UTIL_DO_SYSTEM_ACTION("date > %s", QT_TIME_SET_RESULT_FILE);

    if ((fp = fopen(QT_TIME_SET_RESULT_FILE, "r")) == NULL)
    {
        vosLog_error("Fail to open time set result file!");
        return VOS_RET_INTERNAL_ERROR;
    }

    if (fread(aucBuf, sizeof(aucBuf), 1, fp) < 0)
    {
        vosLog_error("Fail to read from time set result file!");
        fclose(fp);
        return VOS_RET_INTERNAL_ERROR;
    }

    ret = sscanf(aucBuf,"%s %s %s %[^:]:%[^:]:%d UTC %d", aucWeekDay, aucMonth, aucDay, aucHour, 
                                                    aucMin, &pstTimeInfo->ucSec, &pstTimeInfo->usYear);
    printf("=ret=%d=month:%s==hour:%s==min:%s==%s=%d==\n",ret, aucDay, aucHour, aucMin, __func__,__LINE__);fflush(stdout);
    /* 7表示7个字段和sscanf的字符串全都匹配上 */
    if (ret < 7)
    {
        vosLog_error("time result format wrong!");
        fclose(fp);
        return VOS_RET_INTERNAL_ERROR;
    }
 
    ucMonth = qtMonthEngToNum(aucMonth);
    if (ucMonth == 0)
    {
        vosLog_error("Month is wrong!");
        fclose(fp);
        return VOS_RET_INVALID_ARGUMENTS;
    }
    pstTimeInfo->ucMonth = ucMonth;
    pstTimeInfo->ucDay = atoi(aucDay);
    pstTimeInfo->ucHour = atoi(aucHour);
    pstTimeInfo->ucMin = atoi(aucMin);

    fclose(fp);
    return VOS_RET_SUCCESS;
}


VOS_RET_E QtUpgradeSoftware(UINT8 isKeepConfig)
{
    FILE *fp;
    char aucBuf[1024] = {0};
	
	UTIL_DO_SYSTEM_ACTION("chmod 777 %s", QT_IMAGE_FILE_NAME);
    UTIL_DO_SYSTEM_ACTION("echo \"IMG CHECKING\" > %s", QT_SW_UPGRADE_RES_FILE);
    
    UTIL_DO_SYSTEM_ACTION("sysupgrade -T %s > %s", QT_IMAGE_FILE_NAME, QT_IMG_CHK_RES_FILE);

    fp = fopen(QT_IMG_CHK_RES_FILE, "r");
    if (fp == NULL)
    {
        vosLog_error("Fail to open img check result file!");
        UTIL_DO_SYSTEM_ACTION("echo \"IMG CHECK FAILED\" > %s", QT_SW_UPGRADE_RES_FILE);
        return VOS_RET_INTERNAL_ERROR;
    }

    if (fread(aucBuf, 1024, 1, fp) < 0)
    {
        vosLog_error("Fail to read from img check result file!");
        UTIL_DO_SYSTEM_ACTION("echo \"IMG CHECK FAILED\" > %s", QT_SW_UPGRADE_RES_FILE);
        fclose(fp);
        return VOS_RET_INTERNAL_ERROR;
    }

    if (util_strstr(aucBuf, "Image check 'platform_check_image' failed") != NULL)
    {
        vosLog_error("Invalid img file!");
        UTIL_DO_SYSTEM_ACTION("echo \"IMG CHECK FAILED\" > %s", QT_SW_UPGRADE_RES_FILE);
        fclose(fp);
        return VOS_RET_INVALID_ARGUMENTS;
    }
    fclose(fp);

    UTIL_DO_SYSTEM_ACTION("echo \"IMG FLASHING\" > %s", QT_SW_UPGRADE_RES_FILE);
    if (isKeepConfig)
    {
        UTIL_DO_SYSTEM_ACTION("touch /tmp/upgrade_save_config");
    }
    UTIL_DO_SYSTEM_ACTION("touch /tmp/img_download_ok");
    

    #if 0
    if (fork() == 0)
    {
        printf("pid = %d\n", getpid());
        if(isKeepConfig == 1)
        {
            char *newargv[]= {"sh","/sbin/sysupgrade", "-c",QT_IMAGE_FILE_NAME};
            char *newenviron[]={NULL};
            execv("/bin/sh",newargv);

        }
        else
        {
            char *newargv[]= {"sh","/sbin/sysupgrade","-n",QT_IMAGE_FILE_NAME};
            char *newenviron[]={NULL};
            execv("/bin/sh",newargv);
        }
        vosLog_error("sysupgrade fail");
        //exit(127);
    }
   
    
    #endif
    #if 0
    struct rlimit    rl;
    int i;
    if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        perror("getrlimit(RLIMIT_NOFILE, &rl)");
        return -1;
    }
    if(rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }
    for(i = 0; i < rl.rlim_max; i++)
    {
        close(i);
    }
    #endif
    return VOS_RET_SUCCESS;

    
}

VOS_RET_E QtQueryUpgrade(char *outStr, int outLen)
{
    FILE *fp;
    char aucBuf[32] = {0};
	char buffer[256] = {0};

    fp = fopen("/tmp/downloadrate", "r");
    if(fp)
    {
	    while(fgets(buffer, 256, fp))
	    {
			if(strstr(buffer, "server returned error"))
			{
                printf("img upload failed!");
                UTIL_STRNCPY(outStr, "IMG UPLOADING FAILED", outLen);
                fclose(fp);
				return VOS_RET_DOWNLOAD_FAILURE;
			}
	    }
		fclose(fp);
    }
    if (access(QT_SW_UPGRADE_RES_FILE, F_OK) != 0)
    {
        printf("upgrade result file is not exists, img uplogding!");
        UTIL_STRNCPY(outStr, "IMG UPLOADING", outLen);
        return VOS_RET_SUCCESS;
    }

    fp = fopen(QT_SW_UPGRADE_RES_FILE, "r");
    if (fp == NULL)
    {
        printf("upgrade unknow error!\n");
        return VOS_RET_OPEN_FILE_ERROR;
    }

    if (fread(aucBuf, 32, 1, fp) < 0)
    {
        printf("read upgrade res file error!\n");
        fclose(fp);
        return VOS_RET_INTERNAL_ERROR;
    }

    UTIL_STRNCPY(outStr, aucBuf, outLen);
	fclose(fp);
    return VOS_RET_SUCCESS;
}

VOS_RET_E QtOneKeySwitch()
{
    FILE *fp;
    char aucBuf[64] = {0};
    int n=0;
    struct wanPppoeConfig pppCfg = {0};
    

    remove(ONE_KEY_SWITCH_USR_PWD_FILE);

    UTIL_DO_SYSTEM_ACTION("pppoe-server -I eth0 -L 192.168.1.1 -R 192.168.1.2 -N 10");
    UTIL_DO_SYSTEM_ACTION("echo 1 > %s", ONE_KEY_SWITCH_RESULT_FILE);
    while(1){
        if (access(ONE_KEY_SWITCH_USR_PWD_FILE,F_OK) == 0){
            fp = fopen(ONE_KEY_SWITCH_USR_PWD_FILE, "r");
            if (fp != NULL){
                fread(aucBuf, 64, 1, fp);
                printf("-----------%s----------\n",aucBuf);
                if (sscanf(aucBuf, "%s %s", pppCfg.username, pppCfg.password) != 2)
                {
                    printf("Get username and password error!\n");
                    UTIL_DO_SYSTEM_ACTION("echo 4 > %s", ONE_KEY_SWITCH_RESULT_FILE);
                    fclose(fp);
                    break;
                }
                printf("Set pppoe username<%s>, password<%s>!\n", pppCfg.username, pppCfg.password);
                UTIL_DO_SYSTEM_ACTION("echo 2 > %s", ONE_KEY_SWITCH_RESULT_FILE);
                wanPppoeConfigSet(&pppCfg);
                UTIL_DO_SYSTEM_ACTION("echo 3 > %s", ONE_KEY_SWITCH_RESULT_FILE);
                UTIL_DO_SYSTEM_ACTION("killall pppoe-server");
                UTIL_DO_SYSTEM_ACTION("echo 6 > %s", ONE_KEY_SWITCH_RESULT_FILE);
                fclose(fp);
                break;
            }

            
        }

        if (n >= 30){
            UTIL_DO_SYSTEM_ACTION("killall pppoe-server");
            UTIL_DO_SYSTEM_ACTION("echo 5 > %s", ONE_KEY_SWITCH_RESULT_FILE);
            break;
        }
        n++;
        sleep(1);
    }

}

VOS_RET_E QtGetOneKeySwitchStatus(int *status)
{
    FILE *fp;
    char aucBuf[64] = {0};
    int ret;

    fp = fopen(ONE_KEY_SWITCH_RESULT_FILE, "r");
    if (fp == NULL)
    {
        *status = 1;
        return VOS_RET_SUCCESS;
    }

    if (fread(aucBuf, 64, 1, fp) < 0)
    {
        printf("read from one key switch result file fail!\n");
        return -1;
    }
    fclose(fp);
    *status = atoi(aucBuf);

    return VOS_RET_SUCCESS;
}

void QtEmptyHosts()
{
    FILE *fp = fopen("/etc/hosts", "rb");
    FILE *fp1;

    if (fp == NULL)
    {
        fp1 = fopen("/etc/hosts", "ab+");
        if (fp1 != NULL)
        {
            fclose(fp1);
        }
    }
    else
    {
        fclose(fp);
        fp1 = fopen("/etc/hosts", "w");
        if (fp1 != NULL)
        {
            fclose(fp1);
        }
    }
}

int QtSetHosts(QT_HOSTS_CFG *hosts)
{
    char cmd[128] = {0};

    snprintf(cmd, sizeof(cmd), "echo '%s %s' >> /etc/hosts", hosts->ip, hosts->url);
    printf("%s\n",cmd);
    system(cmd);
#if 0
    FILE *fp;

    fp = fopen("/etc/hosts", "a");
    if (fp == NULL)
    {
        printf("Fail to open hosts file\n");
        return -1;
    }

    fprintf(fp, "%s %s", hosts->ip, hosts->url);
    fclose(fp);
#endif

    return 0;
}

int QtGetHosts(QT_HOSTS_CFG *hosts, int *len)
{
    FILE *fp;
    char line[1024] = {0};
    int i = 0;

    fp = fopen("/etc/hosts", "r");
    if (fp == NULL)
    {
        printf("Fail to open hosts file\n");
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL && i < *len)
    {
        sscanf(line, "%s %s", hosts[i].ip, hosts[i].url);
        i++;
    }
    *len = i;
    fclose(fp);

    return 0;
}

size_t get_executable_path( char* processdir,char* processname, size_t len)
{
        char* path_end;
        if(readlink("/proc/self/exe", processdir,len) <=0)
                return -1;
        path_end = strrchr(processdir,  '/');
        if(path_end == NULL)
                return -1;
        ++path_end;
        strcpy(processname, path_end);
        *path_end = '\0';
        return (size_t)(path_end - processdir);
}


int QtGetSpiLock(void *msgHandle)
{
    return UTIL_sendRequestToSmd(msgHandle, VOS_MSG_GET_LOCK, 0, NULL, 0); 
}

int QtReleaseSpiLock(void *msgHandle)
{
    return UTIL_sendRequestToSmd(msgHandle, VOS_MSG_RELEASE_LOCK, 0, NULL, 0);        
}

