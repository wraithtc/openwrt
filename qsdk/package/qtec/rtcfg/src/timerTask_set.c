#include <stdio.h>
#include <string.h>
#include <time.h>
#include "timerTask_set.h"
#include "librtcfg.h"
#include "rtcfg_uci.h"

TIMER_TASK_MNG_STRU g_astTimerTask;


int TimerTaskGetTaskId()
{
    char cmd[128] = {0};
    char value[64] = {0};

    
    snprintf(cmd, sizeof(cmd), "wifitimerid.taskid.id");
    rtcfgUciGet(cmd, value);

    if (strlen(value) == 0 || atoi(value) == 0)
    {
        return 1;
    }
    else
    {
        return atoi(value);
    }
}

int TimerTaskSetTaskId(int id)
{
    char cmd[128] = {0};
    char value[64] = {0};
    if (access("/etc/config/wifitimerid", F_OK) != 0)
    {
        system("touch /etc/config/wifitimerid");
    }
    rtcfgUciSet("wifitimerid.taskid=idcount");
    snprintf(cmd, sizeof(cmd), "wifitimerid.taskid.id=%d", id);
    rtcfgUciSet(cmd);
    rtcfgUciCommit("wifitimerid");
}


/**********************************************
    函数名：TimerTaskEnable
    功能：  使能定时任务
    创建人：tongchao
    创建时间：2017/7/24
***********************************************/
UINT8 TimerTaskEnable(TIMER_TASK_STRU *taskInfo)
{
    UINT8 pos;

    if (NULL == taskInfo || !taskInfo->ucEnable)
    {
        printf("NULL pointer or disabled taskInfo\n");
        return RCT_ERROR;
    }
    pos = 0;
    if (pos >= TASK_TYPE_BUTT)
    {
        printf("Invalid task type!\n");
        return RCT_ERROR;
    }
    memcpy(&g_astTimerTask.stTaskInfo[pos], taskInfo, sizeof(TIMER_TASK_STRU));

    return RCT_OK;
}

/**********************************************
    函数名：TimerTaskDisable
    功能：  删除定时任务
    创建人：tongchao
    创建时间：2017/7/24
***********************************************/
UINT8 TimerTaskDisable(TIMER_TASK_STRU *taskInfo)
{
    UINT8 pos;

    if (NULL == taskInfo || taskInfo->ucEnable)
    {
        printf("NULL pointer or enabled taskInfo\n");
        return RCT_ERROR;
    }
    pos = 0;
    if (pos >= TASK_TYPE_BUTT)
    {
        printf("Invalid task type!\n");
        return RCT_ERROR;
    }
    g_astTimerTask.stTaskInfo[pos].ucEnable = 0;

    return RCT_OK;
}

/**********************************************
    函数名：TimerTaskGetTask
    功能：  获取定时任务
    创建人：tongchao
    创建时间：2017/7/24
***********************************************/

TIMER_TASK_STRU *TimerTaskGetTask(TASK_TYPE_E eTaskType)
{
    if (eTaskType >= TASK_TYPE_BUTT)
    {
        printf("Invalid task type!");
        return NULL;
    }
    TimerTaskLoadConfig();
    return &g_astTimerTask.stTaskInfo[0];
}



/**********************************************
    函数名：TimerTaskAdd
    功能：  添加定时任务
    创建人：tongchao
    创建时间：2017/6/27
***********************************************/
UINT8 TimerTaskAdd(TIMER_TASK_STRU *taskInfo)
{
    UINT8 pos;

    if (NULL == taskInfo)
    {
        printf("NULL pointer taskInfo\n");
        return RCT_ERROR;
    }

    if (g_astTimerTask.ulTaskNum == MAX_TIMER_TASK_NUM)
    {
        printf("Too many wifi timer rules!\n");
        return TASK_RET_TOO_MANY_RULES;
    }
    printf("task num :%d__%s__%d\n",g_astTimerTask.ulTaskNum, __FUNCTION__,__LINE__);
    pos = g_astTimerTask.ulTaskNum + 1;

    if( pos > (MAX_TIMER_TASK_NUM-1) )
    {
        printf("[%s] Too many rules!\n",__func__);
        return TASK_RET_TOO_MANY_RULES;
    }
    
    memcpy(&g_astTimerTask.stTaskInfo[pos], taskInfo, sizeof(TIMER_TASK_STRU));
    g_astTimerTask.ulTaskNum++;

    return RCT_OK;
}

int wifitimerRuleCheck(TIMER_TASK_STRU *startTaskInfo, TIMER_TASK_STRU *stopTaskInfo)
{
    int i = 0, j = 0, k = 0;
    TIMER_TASK_STRU *curStartTaskInfo, *curStopTaskInfo;
    time_t startTime, stopTime, curStartTime, curStopTime, tmpStartTime, tmpStopTime;
    char buf[128] = {0};
    struct tm tm_;
    int flag=0;

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "1970-01-0%c %02d:%02d:00",startTaskInfo->aucTimeWeek[k]+1, startTaskInfo->ucTimerHour, startTaskInfo->ucTimerMin);
    strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_);
    startTime= mktime(&tm_);

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "1970-01-0%c %02d:%02d:00", stopTaskInfo->aucTimeWeek[k]+1, stopTaskInfo->ucTimerHour, stopTaskInfo->ucTimerMin);
    strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_);
    stopTime= mktime(&tm_);
    if (stopTime < startTime)
    {
        stopTime += 24*60*60;
    }
    printf("startTime:%d, stopTime:%d\n", startTime, stopTime);
    if ((startTime - stopTime) >= -300)
    {
        printf("start time and stop time can not be equal!\n");
        return -1;
    }
    #if 0
    while (startTaskInfo->aucTimeWeek[k] != 0)
    {
        if (startTaskInfo->aucTimeWeek[k] != ',')
        {
            int step = 0;
            
            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "1970-01-0%c %02d:%02d:00",startTaskInfo->aucTimeWeek[k]+1, startTaskInfo->ucTimerHour, startTaskInfo->ucTimerMin);
            strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_);
            startTime= mktime(&tm_);

            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "1970-01-0%c %02d:%02d:00", stopTaskInfo->aucTimeWeek[k]+1, stopTaskInfo->ucTimerHour, stopTaskInfo->ucTimerMin);
            strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_);
            stopTime= mktime(&tm_);
            if (stopTime < startTime)
            {
                stopTime += 24*60*60;
            }
            printf("startTime:%d, stopTime:%d\n", startTime, stopTime);
            if ((startTime - stopTime) >= -300)
            {
                printf("start time and stop time can not be equal!\n");
                return -1;
            }
            
            printf("g_astTimerTask.ulTaskNum:%d\n", g_astTimerTask.ulTaskNum);
            for (i = 1; i < g_astTimerTask.ulTaskNum + 1; i+=2)
            {
                curStartTaskInfo= &g_astTimerTask.stTaskInfo[i];
                curStopTaskInfo = &g_astTimerTask.stTaskInfo[i+1];

                if (curStartTaskInfo->usTaskId == startTaskInfo->usTaskId)
                {
                    continue;
                }
                j=0;
                while (curStartTaskInfo->aucTimeWeek[j] != 0)
                {
                    if (curStartTaskInfo->aucTimeWeek[j] != ',')
                    {
                        memset(buf, 0, sizeof(buf));
                        snprintf(buf, sizeof(buf), "1970-01-0%c %02d:%02d:00", curStartTaskInfo->aucTimeWeek[j]+1, curStartTaskInfo->ucTimerHour, curStartTaskInfo->ucTimerMin);
                        strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_);
                        curStartTime= mktime(&tm_);

                        memset(buf, 0, sizeof(buf));
                        snprintf(buf, sizeof(buf), "1970-01-0%c %02d:%02d:00", curStopTaskInfo->aucTimeWeek[j]+1, curStopTaskInfo->ucTimerHour, curStopTaskInfo->ucTimerMin);
                        strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_);
                        curStopTime= mktime(&tm_);
                        tmpStartTime = startTime;
                        tmpStopTime = stopTime;
                        if (abs(curStartTaskInfo->aucTimeWeek[j] - startTaskInfo->aucTimeWeek[k]) == 6)
                        {
                            if (startTaskInfo->aucTimeWeek[k] == '0')
                            {
                                tmpStartTime += 604800;
                                tmpStopTime += 604800;
                            }
                            else
                            {
                                curStartTime +=  604800;
                                curStopTime +=  604800;
                            }
                        }
                        
                        if (curStopTime < curStartTime)
                        {
                            curStopTime += 24*60*60;
                            
                        }
                        printf("starttime:%d, stopTime:%d, curStartTime:%d, curStopTime:%d\n", tmpStartTime, tmpStopTime, curStartTime, curStopTime);
                        if (tmpStartTime > curStopTime || tmpStopTime < curStartTime)
                        {
                            j++;
                            continue;
                        }

                        printf("time conflict!\n");
                        return -1;
                    }
                    j++;
                }
            }   
            
        }
        k++;
    }
    #endif
    return 0;
}

/**********************************************
    函数名：WifiTimerTaskAdd
    功能：  添加wifi定时任务
    创建人：tongchao
    创建时间：2017/9/8
***********************************************/
UINT8 WifiTimerTaskAdd(TIMER_TASK_STRU *startTaskInfo, TIMER_TASK_STRU *stopTaskInfo)
{
    UINT8 pos;
    FILE *fp;
    UINT8 ret;
    unsigned short taskId;
    
    if (NULL == startTaskInfo || NULL == stopTaskInfo)
    {
        printf("NULL pointer taskInfo\n");
        return RCT_ERROR;
    }

    if (0 != wifitimerRuleCheck(startTaskInfo, stopTaskInfo))
    {
        printf("Fail to add wifitimer, time error!\n");
        return TASK_RET_INVALID_ARG;
    }


    startTaskInfo->usTaskId = TimerTaskGetTaskId();
    stopTaskInfo->usTaskId = startTaskInfo->usTaskId;
    TimerTaskSetTaskId(startTaskInfo->usTaskId+1);
    #if 0
    if (access(WIFI_TIMER_ID_FILE, F_OK) != 0)
    {
        startTaskInfo->usTaskId = 1;
        stopTaskInfo->usTaskId = 1;
        fp = fopen(WIFI_TIMER_ID_FILE, "w");
        fprintf(fp, "%d", 1);
        fclose(fp);
    }
    else
    {
        fp = fopen(WIFI_TIMER_ID_FILE, "r");
        if (fscanf(fp, "%d", &taskId) != 1)
        {
            printf("Fail to get wifi timer rule id\n");
            return 1;
        }
        taskId++;
        fclose(fp);
        startTaskInfo->usTaskId = taskId;
        stopTaskInfo->usTaskId = taskId;

        fp = fopen(WIFI_TIMER_ID_FILE, "w");
        fprintf(fp, "%d", taskId);
        fclose(fp);
    }
    #endif
    ret = TimerTaskAdd(startTaskInfo);
    if (ret != TASK_RET_OK)
    {
        printf("Fail to add task , ret = %d\n");
        return ret;
    }
    ret = TimerTaskAdd(stopTaskInfo);
    if (ret != TASK_RET_OK)
    {
        printf("Fail to add task , ret = %d\n");
        return ret;
    }
    
    return TASK_RET_OK;
}


/**********************************************
    函数名： TimerTaskDel
    功能：    删除定时任务
    创建人：tongchao
    创建时间：2017/6/27
***********************************************/
UINT8 TimerTaskDel(TIMER_TASK_STRU *taskInfo)
{
    UINT8 pos = MAX_TIMER_TASK_NUM;
    UINT32 i, num = g_astTimerTask.ulTaskNum + 1;
    
    if (NULL == taskInfo)
    {
        printf("NULL pointer taskInfo\n");
        return RCT_ERROR;
    }
    printf("##################TASK INFO###################\n");
    for (i = 1; i < g_astTimerTask.ulTaskNum + 1; i++)
    {

        printf("id:%d-name:%s-type:%d\n", g_astTimerTask.stTaskInfo[i].usTaskId, g_astTimerTask.stTaskInfo[i].aucName, g_astTimerTask.stTaskInfo[i].ucTaskType);
    }
    printf("##################TASK INFO END###################\n");
    for (i = 1; i < num; i++)
    {
        if (taskInfo->usTaskId == g_astTimerTask.stTaskInfo[i].usTaskId && taskInfo->ucTaskType == g_astTimerTask.stTaskInfo[i].ucTaskType)
        {
            memset(&g_astTimerTask.stTaskInfo[i], 0, sizeof(TIMER_TASK_STRU));
            pos = i;
            g_astTimerTask.ulTaskNum--;
        }
        

        /* 保持紧凑排列，后面元素往前移一位 */
        if (i > pos)
        {
            memcpy(&g_astTimerTask.stTaskInfo[pos], &g_astTimerTask.stTaskInfo[i], sizeof(g_astTimerTask.stTaskInfo[pos]));
            pos = i;
        }
    }
    printf("##################TASK INFO###################\n");
    for (i = 1; i < g_astTimerTask.ulTaskNum + 1; i++)
    {

        printf("id:%d-name:%s-type:%d\n", g_astTimerTask.stTaskInfo[i].usTaskId, g_astTimerTask.stTaskInfo[i].aucName, g_astTimerTask.stTaskInfo[i].ucTaskType);
    }
    printf("##################TASK INFO END###################\n");
    return RCT_OK;
}

/**********************************************
    函数名：WifiTimerTaskDel
    功能：  删除wifi定时任务
    创建人：tongchao
    创建时间：2017/9/8
***********************************************/
UINT8 WifiTimerTaskDel(TIMER_TASK_STRU *startTaskInfo, TIMER_TASK_STRU *stopTaskInfo)
{
    UINT8 ret;
    
    ret = TimerTaskDel(startTaskInfo);
    if (ret != RCT_OK)
        return ret;
    ret = TimerTaskDel(stopTaskInfo);
    if (ret != RCT_OK)
        return ret;

    
    return RCT_OK;
}


/**********************************************
    函数名： TimerTaskDel
    功能：    删除定时任务
    创建人：tongchao
    创建时间：2017/6/27
***********************************************/
UINT8 TimerTaskEdit(TIMER_TASK_STRU *taskInfo)
{
    UINT8 pos = MAX_TIMER_TASK_NUM;
    UINT32 i;
    
    if (NULL == taskInfo)
    {
        printf("NULL pointer taskInfo\n");
        return RCT_ERROR;
    }

    for (i = 1; i < g_astTimerTask.ulTaskNum + 1; i++)
    {
        if (taskInfo->usTaskId == g_astTimerTask.stTaskInfo[i].usTaskId && taskInfo->ucTaskType == g_astTimerTask.stTaskInfo[i].ucTaskType)
        {
            memcpy(&g_astTimerTask.stTaskInfo[i], taskInfo, sizeof(TIMER_TASK_STRU));
            break;
        }
    }

    if (i >= g_astTimerTask.ulTaskNum + 1)
    {
        return RCT_ERROR;
    }
    

    return RCT_OK;
}

/**********************************************
    函数名：WifiTimerTaskEdit
    功能：  删除wifi定时任务
    创建人：tongchao
    创建时间：2017/9/8
***********************************************/
UINT8 WifiTimerTaskEdit(TIMER_TASK_STRU *startTaskInfo, TIMER_TASK_STRU *stopTaskInfo)
{
    UINT8 ret;

    if (0 != wifitimerRuleCheck(startTaskInfo, stopTaskInfo))
    {
        printf("Fail to add wifitimer, time error!\n");
        return TASK_RET_INVALID_ARG;
    }
    ret = TimerTaskEdit(startTaskInfo);
    if (ret != RCT_OK)
        return ret;
    ret = TimerTaskEdit(stopTaskInfo);
    if (ret != RCT_OK)
        return ret;

    return RCT_OK;
}

/**********************************************
    函数名： TimerTaskSetRuleSw
    功能： 设置wifitimer规则开关
    创建人：tongchao
    创建时间：2017/9/8
***********************************************/

int TimerTaskSetRuleSw(int id, int enable)
{
    int i;
    
    for (i = 1; i < g_astTimerTask.ulTaskNum + 1; i++)
    {
        if (id == g_astTimerTask.stTaskInfo[i].usTaskId)
        {
            g_astTimerTask.stTaskInfo[i].ucEnable = enable;
            g_astTimerTask.stTaskInfo[i+1].ucEnable = enable;
            break;
        }
    }

    if (i >= g_astTimerTask.ulTaskNum + 1)
    {
        return RCT_ERROR;
    }

    return RCT_OK;
}

/**********************************************
    函数名： TimerTaskSetSw
    功能： 设置wifitimer总开关
    创建人：tongchao
    创建时间：2017/9/8
***********************************************/

int TimerTaskSetSw(int enable)
{

    g_astTimerTask.ulEnable = enable;

    return RCT_OK;
}


/**********************************************
    函数名： TimerTaskGet
    功能：  查询定时任务
    创建人：tongchao
    创建时间：2017/6/27
***********************************************/
TIMER_TASK_MNG_STRU *TimerTaskGet()
{
    return &g_astTimerTask;
}


void TimerTaskAddRuleToCrond(TIMER_TASK_STRU *taskInfo, char *aucCfgStr, int len)
{
    char aucDigStr[32] = {0};

    memset(aucDigStr, 0, sizeof(aucDigStr));
    snprintf(aucDigStr, sizeof(aucDigStr), "%d", taskInfo->ucTimerMin);
    if (TIME_TYPE_NORMAL == taskInfo->ucMinType)
    {
        strncat(aucCfgStr, aucDigStr, len - strlen(aucCfgStr));
    }
    else if (TIME_TYPE_PER == taskInfo->ucMinType)
    {
        strncat(aucCfgStr, "/", len - strlen(aucCfgStr));
        strncat(aucCfgStr, aucDigStr, len - strlen(aucCfgStr));
    }
    strncat(aucCfgStr, " ", len - strlen(aucCfgStr));

    memset(aucDigStr, 0, sizeof(aucDigStr));
    snprintf(aucDigStr, sizeof(aucDigStr), "%d", taskInfo->ucTimerHour);
    if (TIME_TYPE_NORMAL == taskInfo->ucHourType)
    {
        strncat(aucCfgStr, aucDigStr, len - strlen(aucCfgStr));
    }
    else if (TIME_TYPE_PER == taskInfo->ucHourType)
    {
        strncat(aucCfgStr, "/", len - strlen(aucCfgStr));
        strncat(aucCfgStr, aucDigStr, len - strlen(aucCfgStr));
    }
    strncat(aucCfgStr, " ", len - strlen(aucCfgStr));

    if (0 != taskInfo->ucTimerDay)
    {
        memset(aucDigStr, 0, sizeof(aucDigStr));
        snprintf(aucDigStr, sizeof(aucDigStr), "%d", taskInfo->ucTimerDay);
        if (TIME_TYPE_NORMAL == taskInfo->ucDayType)
        {
            strncat(aucCfgStr, aucDigStr, len);
        }
        else if (TIME_TYPE_PER == taskInfo->ucDayType)
        {
            strncat(aucCfgStr, "/", len - strlen(aucCfgStr));
            strncat(aucCfgStr, aucDigStr, len - strlen(aucCfgStr));
        }
    }
    else
    {
        strncat(aucCfgStr, "*", len - strlen(aucCfgStr));
    }
    strncat(aucCfgStr, " ", len - strlen(aucCfgStr));

    if (0 != taskInfo->ucTimerMonth)
    {
        memset(aucDigStr, 0, sizeof(aucDigStr));
        snprintf(aucDigStr, sizeof(aucDigStr), "%d", taskInfo->ucTimerMonth);
        if (TIME_TYPE_NORMAL == taskInfo->ucMonthType)
        {
            strncat(aucCfgStr, aucDigStr, len - strlen(aucCfgStr));
        }
        else if (TIME_TYPE_PER == taskInfo->ucMonthType)
        {
            strncat(aucCfgStr, "/", len);
            strncat(aucCfgStr, aucDigStr, len - strlen(aucCfgStr));
        }
    }
    else
    {
        strncat(aucCfgStr, "*", len - strlen(aucCfgStr));
    }
    strncat(aucCfgStr, " ", len - strlen(aucCfgStr));

    if (0 != strlen(taskInfo->aucTimeWeek))
    {
        memset(aucDigStr, 0, sizeof(aucDigStr));

        if (TIME_TYPE_NORMAL == taskInfo->ucWeekType)
        {
            strncat(aucCfgStr, taskInfo->aucTimeWeek, len - strlen(aucCfgStr));
        }
        else if (TIME_TYPE_PER == taskInfo->ucWeekType)
        {
            strncat(aucCfgStr, "/", len - strlen(aucCfgStr));
            strncat(aucCfgStr, aucDigStr, len - strlen(aucCfgStr));
        }
    }
    else
    {
        strncat(aucCfgStr, "*", len - strlen(aucCfgStr));
    }
    strncat(aucCfgStr, " ", len - strlen(aucCfgStr));

    if (TASK_TYPE_WIFI_ON == taskInfo->ucTaskType)
    {
        strncat(aucCfgStr, "wifi up;ifup wan;ifup wwan", len - strlen(aucCfgStr));
    }
    else if (TASK_TYPE_WIFI_OFF == taskInfo->ucTaskType)
    {
        strncat(aucCfgStr, "wifi down", len - strlen(aucCfgStr));
    }
    else if (TASK_TYPE_POWER_OFF == taskInfo->ucTaskType)
    {
        strncat(aucCfgStr, "/etc/poweroff.sh", len - strlen(aucCfgStr));
    }
    strncat(aucCfgStr, "\n", len - strlen(aucCfgStr));
    printf("aucCfgStr:%s,%s--%d\n", aucCfgStr, __FUNCTION__,__LINE__);
}

/**********************************************
    函数名： TimerTaskRun
    功能： 启动定时任务
    创建人：tongchao
    创建时间：2017/6/27
***********************************************/

UINT8 TimerTaskRun()
{
    UINT32 i,j;
    char aucCfgStr[1024] = {0};
    char aucDigStr[32] = {0};
    TIMER_TASK_STRU *taskInfo, *startTaskInfo, *stopTaskInfo;
    FILE *fp;
    time_t startTime, stopTime, curStartTime, curStopTime;
    char buf[128] = {0};
    struct tm tm_;
    printf("___num<%d>,globalenable<%d>__%s___%d__\n",g_astTimerTask.ulTaskNum, g_astTimerTask.ulEnable, __FUNCTION__,__LINE__);
    taskInfo = &g_astTimerTask.stTaskInfo[0];

    if (taskInfo->ucEnable)
    {
        TimerTaskAddRuleToCrond(taskInfo, aucCfgStr, sizeof(aucCfgStr));
    }
    
    for (i = 1 ; i < g_astTimerTask.ulTaskNum + 1; i+=2)
    {
        startTaskInfo = &g_astTimerTask.stTaskInfo[i];
        stopTaskInfo = &g_astTimerTask.stTaskInfo[i+1];

        if (!g_astTimerTask.ulEnable || startTaskInfo->ucEnable == 0)
        {
            continue;
        }
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "1970-01-01 %02d:%02d:00", startTaskInfo->ucTimerHour, startTaskInfo->ucTimerMin);
        strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_);
        startTime= mktime(&tm_);

        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "1970-01-01 %02d:%02d:00", stopTaskInfo->ucTimerHour, stopTaskInfo->ucTimerMin);
        strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_);
        stopTime= mktime(&tm_);
        if (startTime > stopTime)
        {
            j=0;
            while (stopTaskInfo->aucTimeWeek[j] != 0)
            {
                if (stopTaskInfo->aucTimeWeek[j] != ',')
                {
                    stopTaskInfo->aucTimeWeek[j] = '0' + (stopTaskInfo->aucTimeWeek[j] - '0' + 1)%7;      
                }
                j++;
            }
        }
        printf("stopTaskInfo->aucTimeWeek:%s\n", stopTaskInfo->aucTimeWeek);
        TimerTaskAddRuleToCrond(startTaskInfo, aucCfgStr, sizeof(aucCfgStr));
        TimerTaskAddRuleToCrond(stopTaskInfo, aucCfgStr, sizeof(aucCfgStr));
    }
    printf("aucCfgStr:\n%s\n", aucCfgStr);
    fp = fopen(CROND_CONFIG_FILE_NAME, "w");
    if (NULL == fp)
    {
        printf("Fail to open %s \n", CROND_CONFIG_FILE_NAME);
        return RCT_ERROR;
    }

    fprintf(fp, "%s", aucCfgStr);
    fclose(fp);

    system("/etc/init.d/cron restart");
    system("/etc/init.d/cron enable");

    return RCT_OK;
}



