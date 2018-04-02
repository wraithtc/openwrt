#include "timerTask_set.h"

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
    struct uci_package * pkg = NULL;  
    struct uci_element *e;  
    char *ip, *value;
  
  
    ctx = uci_alloc_context(); // 申请一个UCI上下文.  
    if (UCI_OK != uci_load(ctx, UCI_CONFIG_FILE, &pkg))  
        goto cleanup; //如果打开UCI文件失败,则跳到末尾 清理 UCI 上下文.  
  
  
    /*遍历UCI的每一个节*/  
    uci_foreach_element(&pkg->sections, e)  
    {  
        struct uci_section *s = uci_to_section(e);  
        TIMER_TASK_STRU taskInfo = {0};
        // 将一个 element 转换为 section类型, 如果节点有名字,则 s->anonymous 为 false.  
        // 此时通过 s->e->name 来获取.  
        // 此时 您可以通过 uci_lookup_option()来获取 当前节下的一个值.  
        if (NULL != s->e.name)  
        {  
            printf("Load, section name %s, type %s\n", s->e.name, s->type);
            taskInfo.usTaskId = atoi(s->type);  
        }  

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_TYPE_OPTION)))  
        {  
            taskInfo.ucTaskType = atoi(value);  
        } 

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_MIN_TYPE_OPTION)))  
        {  
            taskInfo.ucMinType = atoi(value);  
        } 

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_MIN_OPTION)))  
        {  
            taskInfo.ucTimerMin = atoi(value);  
        } 

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_HOUR_TYPE_OPTION)))  
        {  
            taskInfo.ucHourType = atoi(value);  
        } 

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_HOUR_OPTION)))  
        {  
            taskInfo.ucTimerHour = atoi(value);  
        } 

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_DAY_TYPE_OPTION)))  
        {  
            taskInfo.ucDayType = atoi(value);  
        } 

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_DAY_OPTION)))  
        {  
            taskInfo.ucTimerDay = atoi(value);  
        }

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_MONTH_TYPE_OPTION)))  
        {  
            taskInfo.ucMonthType = atoi(value);  
        } 

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_MONTH_OPTION)))  
        {  
            taskInfo.ucTimerMonth = atoi(value);  
        }

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_WEEK_TYPE_OPTION)))  
        {  
            taskInfo.ucWeekType = atoi(value);  
        } 

        if (NULL != (value = (char *)uci_lookup_option_string(ctx, s, TIMER_TASK_WEEK_OPTION)))  
        {  
            strncpy(taskInfo.aucTimeWeek, value, sizeof(taskInfo.aucTimeWeek));  
        }
        // 如果您不确定是 string类型 可以先使用 uci_lookup_option() 函数得到Option 然后再判断.  
        // Option 的类型有 UCI_TYPE_STRING 和 UCI_TYPE_LIST 两种.  
        taskInfo.ucEnable = 1;
        TimerTaskEnable(&taskInfo);
    }  
    uci_unload(ctx, pkg); // 释放 pkg   
cleanup:  
    uci_free_context(ctx);  
    ctx = NULL; 

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
    struct uci_context * ctx = uci_alloc_context(); //申请上下文  
    int num, i;
    bool ret = false;
    int res;
    struct uci_package *pkg = NULL;  
    TimerTaskEmptyConfig();

    if(UCI_OK != uci_load(ctx, "wifitimer", &pkg))
    {
        printf("Fail to open wifitimer config!\n");
        goto cleanup;
    }
    num = g_astTimerTask.ulTaskNum;

    if((pkg = uci_lookup_package(ctx, "wifitimer")) != NULL)  
    {  
        
        for (i = 0; i < TASK_TYPE_BUTT; i++)
        {
            char section[32] = {0};
            char uciOption[128];
            char value[32];
            TIMER_TASK_STRU *taskInfo = &g_astTimerTask.stTaskInfo[i];
            if (taskInfo->ucEnable)
            {
                struct uci_ptr ptr = {  
                .p = pkg  
                };
                
                snprintf(section, sizeof(section), "%d", taskInfo->usTaskId);
                printf("Add section %s\n",section);
                uci_add_section(ctx, pkg, section, &ptr.s);

                ptr.o = NULL;
                ptr.option = TIMER_TASK_TYPE_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucTaskType);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }
                
                printf("_______________________%s___%d__\n",__FUNCTION__,__LINE__);
                ptr.o = NULL;
                ptr.option = TIMER_TASK_MIN_TYPE_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucMinType);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }

                ptr.o = NULL;
                ptr.option = TIMER_TASK_MIN_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucTimerMin);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }

                ptr.o = NULL;
                ptr.option = TIMER_TASK_HOUR_TYPE_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucHourType);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }

                ptr.o = NULL;
                ptr.option = TIMER_TASK_HOUR_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucTimerHour);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }

                ptr.o = NULL;
                ptr.option = TIMER_TASK_DAY_TYPE_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucDayType);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }

                ptr.o = NULL;
                ptr.option = TIMER_TASK_DAY_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucTimerDay);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }

                ptr.o = NULL;
                ptr.option = TIMER_TASK_MONTH_TYPE_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucMonthType);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }

                ptr.o = NULL;
                ptr.option = TIMER_TASK_MONTH_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucTimerMonth);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }

                ptr.o = NULL;
                ptr.option = TIMER_TASK_WEEK_TYPE_OPTION;
                memset(value, 0, sizeof(value));
                snprintf(value, sizeof(value), "%d", taskInfo->ucWeekType);
                ptr.value = value;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                }

                ptr.o = NULL;
                ptr.option = TIMER_TASK_WEEK_OPTION;
                ptr.value = taskInfo->aucTimeWeek;
                res = uci_set(ctx,&ptr);
                if (res != 0)
                {
                    printf("uci set fail, res = %d\n",res);
                    goto cleanup;
                } 
                uci_commit(ctx, &ptr.p, false); //提交保存更改 
            }
             
        }
        
        
    }
    
            
cleanup:
    uci_unload(ctx,pkg); //卸载包     
    uci_free_context(ctx); //释放上下文  
    return ret;
}