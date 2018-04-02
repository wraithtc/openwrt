#include <stdio.h>
#include <string.h>
#include "timerTask_set.h"
#include <sys/select.h>
#include <unistd.h>

void *g_msgHandle;
extern void TimerTaskPrintConfig();

int main(int argc, const char *argv[])
{
    TIMER_TASK_MNG_STRU *pstTimerTaskMng = TimerTaskGet();
    int num=pstTimerTaskMng->ulTaskNum;
    int i =0, ret;

    printf("==========Timer Task Test========\n");
    
    if (1 >= argc)
    {
        system("cat /etc/crontabs/root");
        printf("============================\n");
        printf("taskNum = %d\n", pstTimerTaskMng->ulTaskNum);
        printf("task id:\n");
        for (i = 0; i < num; i++)
        {
            printf("%d\n", pstTimerTaskMng->stTaskInfo[i].usTaskId);
        }
        return;
    }

    if (1 == atoi(argv[1]))
    {
        printf("==========ADD ONE TASK========\n");
        printf("3:40 turn on wifi\n");
        TimerTaskLoadConfig();
        TIMER_TASK_STRU stTaskInfo = {0};
        stTaskInfo.ucTaskType = TASK_TYPE_WIFI_ON;
        stTaskInfo.ucMinType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerMin = 40;
        stTaskInfo.ucHourType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerHour = 3;
        strncpy(stTaskInfo.aucTimeWeek, "1,2", sizeof(stTaskInfo.aucTimeWeek));
        stTaskInfo.usTaskId = 0;

        TimerTaskAdd(&stTaskInfo);
        system("date -s 3:39:40");
        TimerTaskRun();
        TimerTaskSaveConfig();
    }
    else if (2 == atoi(argv[1]))
    {
        printf("==========ADD TWO TASK========\n");
        printf("3:40 turn on wifi, 3:41 turn off wifi\n");
        TIMER_TASK_STRU stTaskInfo = {0};
        TimerTaskLoadConfig();
        TimerTaskPrintConfig();
        stTaskInfo.ucTaskType = TASK_TYPE_WIFI_ON;
        stTaskInfo.ucMinType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerMin = 42;
        stTaskInfo.ucHourType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerHour = 3;
        stTaskInfo.usTaskId = 2;
        TimerTaskAdd(&stTaskInfo);
        
        stTaskInfo.ucTaskType = TASK_TYPE_WIFI_OFF;
        stTaskInfo.ucMinType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerMin = 41;
        stTaskInfo.ucHourType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerHour = 3;
        stTaskInfo.usTaskId = 1;
        TimerTaskAdd(&stTaskInfo);
        TimerTaskSaveConfig();
        system("date -s 3:39:40");
        TimerTaskRun();
    }
    else if (3 == atoi(argv[1]))
    {
        printf("==========ADD TWO TASK========\n");
        printf("3:40 turn on wifi, 3:41 turn off wifi\n");
        TIMER_TASK_STRU stTaskInfo = {0};
        TimerTaskLoadConfig();
        TimerTaskPrintConfig();
        stTaskInfo.ucTaskType = TASK_TYPE_WIFI_ON;
        stTaskInfo.ucMinType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerMin = 42;
        stTaskInfo.ucHourType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerHour = 3;
        stTaskInfo.usTaskId = 2;
        TimerTaskAdd(&stTaskInfo);
        
        stTaskInfo.ucTaskType = TASK_TYPE_WIFI_OFF;
        stTaskInfo.ucMinType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerMin = 41;
        stTaskInfo.ucHourType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerHour = 3;
        stTaskInfo.usTaskId = 1;
        TimerTaskAdd(&stTaskInfo);
        
        system("date -s 3:39:40");
        TimerTaskRun();
        
        printf("==========DEL ONE TASK========\n");
        stTaskInfo.ucTaskType = TASK_TYPE_WIFI_ON;
        stTaskInfo.ucMinType = TIME_TYPE_NORMAL;
        stTaskInfo.ucTimerMin = 40;
        stTaskInfo.ucHourType = TIME_TYPE_NORMAL;
        stTaskInfo.usTaskId = 1;
        TimerTaskDel(&stTaskInfo);
        TimerTaskSaveConfig();
        TimerTaskRun();
    }

}