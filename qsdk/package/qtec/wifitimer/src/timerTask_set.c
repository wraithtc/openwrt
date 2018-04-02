#include <stdio.h>
#include <string.h>
#include "timerTask_set.h"


TIMER_TASK_MNG_STRU g_astTimerTask;


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
    pos = taskInfo->ucTaskType;
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
    pos = taskInfo->ucTaskType;
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
    return &g_astTimerTask.stTaskInfo[eTaskType];
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
    pos = g_astTimerTask.ulTaskNum;
    memcpy(&g_astTimerTask.stTaskInfo[pos], taskInfo, sizeof(TIMER_TASK_STRU));
    g_astTimerTask.ulTaskNum++;

    return RCT_OK;
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
    UINT32 i;
    
    if (NULL == taskInfo)
    {
        printf("NULL pointer taskInfo\n");
        return RCT_ERROR;
    }

    for (i = 0; i < g_astTimerTask.ulTaskNum; i++)
    {
        if (taskInfo->usTaskId == g_astTimerTask.stTaskInfo[i].usTaskId)
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

    for (i = 0; i < g_astTimerTask.ulTaskNum; i++)
    {
        if (taskInfo->usTaskId == g_astTimerTask.stTaskInfo[i].usTaskId)
        {
            memcpy(&g_astTimerTask.stTaskInfo[i], taskInfo, sizeof(TIMER_TASK_STRU));
            break;
        }
    }

    if (i >= g_astTimerTask.ulTaskNum)
    {
        return RCT_ERROR;
    }
    

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


/**********************************************
    函数名： TimerTaskRun
    功能： 启动定时任务
    创建人：tongchao
    创建时间：2017/6/27
***********************************************/

UINT8 TimerTaskRun()
{
    UINT32 i;
    char aucCfgStr[4096] = {0};
    char aucDigStr[32] = {0};
    TIMER_TASK_STRU *taskInfo;
    FILE *fp;
    
    for (i = 0 ; i < TASK_TYPE_BUTT; i++)
    {
        taskInfo = &g_astTimerTask.stTaskInfo[i];

        if (taskInfo->ucEnable == 0)
        {
            continue;
        }

        if (0 != taskInfo->ucTimerMin)
        {
            memset(aucDigStr, 0, sizeof(aucDigStr));
            snprintf(aucDigStr, sizeof(aucDigStr), "%d", taskInfo->ucTimerMin);
            if (TIME_TYPE_NORMAL == taskInfo->ucMinType)
            {
                strncat(aucCfgStr, aucDigStr, sizeof(aucCfgStr) - strlen(aucCfgStr));
            }
            else if (TIME_TYPE_PER == taskInfo->ucMinType)
            {
                strncat(aucCfgStr, "/", sizeof(aucCfgStr) - strlen(aucCfgStr));
                strncat(aucCfgStr, aucDigStr, sizeof(aucCfgStr) - strlen(aucCfgStr));
            }
        }
        else
        {
            strncat(aucCfgStr, "*", sizeof(aucCfgStr) - strlen(aucCfgStr));
        }
        strncat(aucCfgStr, " ", sizeof(aucCfgStr) - strlen(aucCfgStr));

        if (0 != taskInfo->ucTimerHour)
        {
            memset(aucDigStr, 0, sizeof(aucDigStr));
            snprintf(aucDigStr, sizeof(aucDigStr), "%d", taskInfo->ucTimerHour);
            if (TIME_TYPE_NORMAL == taskInfo->ucHourType)
            {
                strncat(aucCfgStr, aucDigStr, sizeof(aucCfgStr) - strlen(aucCfgStr));
            }
            else if (TIME_TYPE_PER == taskInfo->ucHourType)
            {
                strncat(aucCfgStr, "/", sizeof(aucCfgStr) - strlen(aucCfgStr));
                strncat(aucCfgStr, aucDigStr, sizeof(aucCfgStr) - strlen(aucCfgStr));
            }
        }
        else
        {
            strncat(aucCfgStr, "*", sizeof(aucCfgStr) - strlen(aucCfgStr));
        }
        strncat(aucCfgStr, " ", sizeof(aucCfgStr) - strlen(aucCfgStr));

        if (0 != taskInfo->ucTimerDay)
        {
            memset(aucDigStr, 0, sizeof(aucDigStr));
            snprintf(aucDigStr, sizeof(aucDigStr), "%d", taskInfo->ucTimerDay);
            if (TIME_TYPE_NORMAL == taskInfo->ucDayType)
            {
                strncat(aucCfgStr, aucDigStr, sizeof(aucCfgStr));
            }
            else if (TIME_TYPE_PER == taskInfo->ucDayType)
            {
                strncat(aucCfgStr, "/", sizeof(aucCfgStr) - strlen(aucCfgStr));
                strncat(aucCfgStr, aucDigStr, sizeof(aucCfgStr) - strlen(aucCfgStr));
            }
        }
        else
        {
            strncat(aucCfgStr, "*", sizeof(aucCfgStr) - strlen(aucCfgStr));
        }
        strncat(aucCfgStr, " ", sizeof(aucCfgStr) - strlen(aucCfgStr));

        if (0 != taskInfo->ucTimerMonth)
        {
            memset(aucDigStr, 0, sizeof(aucDigStr));
            snprintf(aucDigStr, sizeof(aucDigStr), "%d", taskInfo->ucTimerMonth);
            if (TIME_TYPE_NORMAL == taskInfo->ucMonthType)
            {
                strncat(aucCfgStr, aucDigStr, sizeof(aucCfgStr) - strlen(aucCfgStr));
            }
            else if (TIME_TYPE_PER == taskInfo->ucMonthType)
            {
                strncat(aucCfgStr, "/", sizeof(aucCfgStr));
                strncat(aucCfgStr, aucDigStr, sizeof(aucCfgStr) - strlen(aucCfgStr));
            }
        }
        else
        {
            strncat(aucCfgStr, "*", sizeof(aucCfgStr) - strlen(aucCfgStr));
        }
        strncat(aucCfgStr, " ", sizeof(aucCfgStr) - strlen(aucCfgStr));

        if (0 != strlen(taskInfo->aucTimeWeek))
        {
            memset(aucDigStr, 0, sizeof(aucDigStr));

            if (TIME_TYPE_NORMAL == taskInfo->ucWeekType)
            {
                strncat(aucCfgStr, taskInfo->aucTimeWeek, sizeof(aucCfgStr) - strlen(aucCfgStr));
            }
            else if (TIME_TYPE_PER == taskInfo->ucWeekType)
            {
                strncat(aucCfgStr, "/", sizeof(aucCfgStr) - strlen(aucCfgStr));
                strncat(aucCfgStr, aucDigStr, sizeof(aucCfgStr) - strlen(aucCfgStr));
            }
        }
        else
        {
            strncat(aucCfgStr, "*", sizeof(aucCfgStr) - strlen(aucCfgStr));
        }
        strncat(aucCfgStr, " ", sizeof(aucCfgStr) - strlen(aucCfgStr));

        if (TASK_TYPE_WIFI_ON == taskInfo->ucTaskType)
        {
            strncat(aucCfgStr, "wifi up", sizeof(aucCfgStr) - strlen(aucCfgStr));
        }
        else if (TASK_TYPE_WIFI_OFF == taskInfo->ucTaskType)
        {
            strncat(aucCfgStr, "wifi down", sizeof(aucCfgStr) - strlen(aucCfgStr));
        }
        else if (TASK_TYPE_POWER_OFF == taskInfo->ucTaskType)
        {
            strncat(aucCfgStr, "poweroff", sizeof(aucCfgStr) - strlen(aucCfgStr));
        }
        strncat(aucCfgStr, "\n", sizeof(aucCfgStr) - strlen(aucCfgStr));
    }

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



