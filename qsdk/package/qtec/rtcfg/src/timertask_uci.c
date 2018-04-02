#include "timerTask_set.h"
#include "rtcfg_uci.h"
#include "librtcfg.h"

extern TIMER_TASK_MNG_STRU g_astTimerTask;
static struct uci_context * ctx = NULL;

void TimerTaskPrintConfig()
{
    TIMER_TASK_STRU *taskInfo;
    int i = 0;
    printf("---------------------------------------\n");
    for (i = 0; i < g_astTimerTask.ulTaskNum; i ++)
    {
        taskInfo = &g_astTimerTask.stTaskInfo[i];
        printf("Task id: %d\n", taskInfo->usTaskId);
        printf("MinType: %d\n", taskInfo->ucMinType);
        printf("Min: %d\n", taskInfo->ucTimerMin);
        printf("HourType: %d\n", taskInfo->ucHourType);
        printf("Hour: %d\n", taskInfo->ucTimerHour);
        printf("DayType: %d\n", taskInfo->ucDayType);
        printf("Day: %d\n", taskInfo->ucTimerDay);
        printf("MonthType: %d\n", taskInfo->ucMonthType);
        printf("Month: %d\n", taskInfo->ucTimerMonth);
        printf("WeekType: %d\n", taskInfo->ucWeekType);
        printf("Week: %s\n", taskInfo->aucTimeWeek);
        printf("\n");
    }
    printf("---------------------------------------\n");
}

/**********************************************
    函数名：TimerTaskLoadConfig
    功能：  从uci中读取配置
    创建人：tongchao
    创建时间：2017/7/20
***********************************************/
bool TimerTaskLoadConfig()
{
    int ret, i = 0;
    char cmd[128] = {0};
    char value[128] = {0};

    memset(&g_astTimerTask, 0, sizeof(g_astTimerTask));
    snprintf(cmd, sizeof(cmd), "wifitimer.@rule[%d]", i);
    while ((ret = rtcfgUciGet(cmd, value)) == 0)
    {
        TIMER_TASK_STRU taskInfo = {0};
        
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[%d].%s", i, TIMER_TASK_TYPE_OPTION);
        memset(value, 0, sizeof(value));
        rtcfgUciGet(cmd, value);
        taskInfo.ucTaskType = atoi(value);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[%d].%s", i, TIMER_TASK_ENABLE_OPTION);
        memset(value, 0, sizeof(value));
        rtcfgUciGet(cmd, value);
        taskInfo.ucEnable = atoi(value);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[%d].%s", i, TIMER_TASK_MIN_OPTION);
        memset(value, 0, sizeof(value));
        rtcfgUciGet(cmd, value);
        taskInfo.ucTimerMin = atoi(value);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[%d].%s", i, TIMER_TASK_HOUR_OPTION);
        memset(value, 0, sizeof(value));
        rtcfgUciGet(cmd, value);
        taskInfo.ucTimerHour = atoi(value);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[%d].%s", i, TIMER_TASK_WEEK_OPTION);
        memset(value, 0, sizeof(value));
        rtcfgUciGet(cmd, value);
        strncpy(taskInfo.aucTimeWeek, value, sizeof(taskInfo.aucTimeWeek));

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[%d].%s", i, TIMER_TASK_NAME_OPTION);
        memset(value, 0, sizeof(value));
        rtcfgUciGet(cmd, value);
        strncpy(taskInfo.aucName, value, sizeof(taskInfo.aucName));

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[%d].%s", i, TIMER_TASK_ID_OPTION);
        memset(value, 0, sizeof(value));
        rtcfgUciGet(cmd, value);
        taskInfo.usTaskId = atoi(value);

        if (taskInfo.ucTaskType == TASK_TYPE_POWER_OFF)
        {
            taskInfo.ucEnable?TimerTaskEnable(&taskInfo):TimerTaskDisable(&taskInfo);
        }
        else
        {
            TimerTaskAdd(&taskInfo);
        }
        i++;
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[%d]", i);
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "wifitimer.global.globalenable");
    memset(value, 0, sizeof(value));
    rtcfgUciGet(cmd, value);
    g_astTimerTask.ulEnable = atoi(value); 

    return true;
}

/**********************************************
    函数名：TimerTaskEmptyConfig
    功能：  清空uci配置
    创建人：tongchao
    创建时间：2017/7/20
***********************************************/
void TimerTaskEmptyConfig()
{
    FILE *fp = fopen(UCI_CONFIG_FILE, "rb");
    FILE *fp1;

    if (fp == NULL)
    {
        fp1 = fopen(UCI_CONFIG_FILE, "ab+");
        if (fp1 != NULL)
        {
            fclose(fp1);
        }
    }
    else
    {
        fclose(fp);
        fp1 = fopen(UCI_CONFIG_FILE, "w");
        if (fp1 != NULL)
        {
            fclose(fp1);
        }
    }
}

/**********************************************
    函数名：TimerTaskSaveConfig
    功能：  写入到uci配置
    创建人：tongchao
    创建时间：2017/7/20
***********************************************/
bool TimerTaskSaveConfig()
{
    int num, i;
    bool ret = false;
    int res; 
    char cmd[128] = {0};
    TimerTaskEmptyConfig();

    num = g_astTimerTask.ulTaskNum;   
    for (i = 0; i < g_astTimerTask.ulTaskNum + 1; i++)
    {
        TIMER_TASK_STRU *taskInfo = &g_astTimerTask.stTaskInfo[i];
        if (i == 0 && !taskInfo->ucEnable)
        {
            continue;
        }
        
        rtcfgUciAdd("wifitimer", "rule");

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[-1].%s=%d", TIMER_TASK_TYPE_OPTION, taskInfo->ucTaskType);
        rtcfgUciSet(cmd);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[-1].%s=%d", TIMER_TASK_ENABLE_OPTION, taskInfo->ucEnable);
        rtcfgUciSet(cmd);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[-1].%s=%d", TIMER_TASK_MIN_OPTION, taskInfo->ucTimerMin);
        rtcfgUciSet(cmd);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[-1].%s=%d", TIMER_TASK_HOUR_OPTION, taskInfo->ucTimerHour);
        rtcfgUciSet(cmd);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[-1].%s=%s", TIMER_TASK_WEEK_OPTION, taskInfo->aucTimeWeek);
        rtcfgUciSet(cmd);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[-1].%s=%d", TIMER_TASK_ID_OPTION, taskInfo->usTaskId);
        rtcfgUciSet(cmd);

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "wifitimer.@rule[-1].%s=%s", TIMER_TASK_NAME_OPTION, taskInfo->aucName);
        rtcfgUciSet(cmd);
        
         
    }
    
            
cleanup:

    snprintf(cmd, sizeof(cmd), "wifitimer.global.globalenable=%d", g_astTimerTask.ulEnable);
    rtcfgUciSet("wifitimer.global=switch");
    rtcfgUciSet(cmd);
    rtcfgUciCommit("wifitimer");
    
    return ret;
}