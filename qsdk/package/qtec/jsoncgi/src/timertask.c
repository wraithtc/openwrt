#include "basic.h"


void proc_timertask_set(cJSON *jsonValue,cJSON *jsonOut)
{
    int f0, f1, f2, lTaskType;
    char *outBuff;
    TIMER_TASK_STRU taskInfo = {0}, *pTaskInfo;
    int taskType, timeMin, timeHour, ret = 0;
    char *timeDay, *outStr;

    if (!cJSON_GetObjectItem(jsonValue, "tasktype") 
        || !cJSON_GetObjectItem(jsonValue, "minute") 
        || !cJSON_GetObjectItem(jsonValue, "hour")
        || !cJSON_GetObjectItem(jsonValue, "enable")
        || !cJSON_GetObjectItem(jsonValue, "day"))
    {
        printf("Miss argument to set timer task!\n");
        outStr = "Miss argument to set timer task!";
        return;	
    }
    taskInfo.ucTaskType = cJSON_GetObjectItem(jsonValue, "tasktype")->valueint;
    taskInfo.ucTimerMin = cJSON_GetObjectItem(jsonValue, "minute")->valueint;
    taskInfo.ucTimerHour = cJSON_GetObjectItem(jsonValue, "hour")->valueint;
    taskInfo.ucEnable = cJSON_GetObjectItem(jsonValue, "enable")->valueint;
    taskInfo.usTaskId = taskInfo.ucTaskType;
    UTIL_STRNCPY(taskInfo.aucTimeWeek, cJSON_GetObjectItem(jsonValue, "day")->valuestring, sizeof(taskInfo.aucTimeWeek));
    if (taskInfo.ucEnable)
    {
        TimerTaskLoadConfig();
        TimerTaskEnable(&taskInfo);
        
    }
    else
    {
        TimerTaskLoadConfig();
        TimerTaskDisable(&taskInfo);
    }
    TimerTaskSaveConfig();
    TimerTaskRun();

    return;
}

void proc_timertask_detect(cJSON *jsonValue,cJSON *jsonOut)
{
    TIMER_TASK_STRU *pTaskInfo;
    char *timeDay, *outStr;
    int lTaskType;
    cJSON *dataObj;
    
    if (!cJSON_GetObjectItem(jsonValue, "tasktype"))
    {
        printf("Miss argument to get timer task!\n");
        outStr = "Miss argument to get timer task!";
        return ;	
    }
    lTaskType = cJSON_GetObjectItem(jsonValue, "tasktype")->valueint;
    if ((pTaskInfo = TimerTaskGetTask(lTaskType)) == NULL)
    {
        outStr = "Fail to get timer task!";
        return;
    }
    dataObj = cJSON_CreateObject();
    if (dataObj == NULL)
    {
        printf("Fail to create data json obj!\n");
        outStr = "Fail to create data json obj!";
        return;	
    }
    cJSON_AddItemToObject(dataObj, "minute", cJSON_CreateNumber(pTaskInfo->ucTimerMin));
	cJSON_AddItemToObject(dataObj, "hour", cJSON_CreateNumber(pTaskInfo->ucTimerHour));
	cJSON_AddItemToObject(dataObj, "enable", cJSON_CreateNumber(pTaskInfo->ucEnable));
    cJSON_AddItemToObject(dataObj, "day", cJSON_CreateString(pTaskInfo->aucTimeWeek));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    return;
}

int proc_wifitimer_set(cJSON *jsonValue,cJSON *jsonOut)
{
    int f0, f1, f2, lTaskType;
    char *outBuff;
    TIMER_TASK_STRU startTaskInfo = {0}, stopTaskInfo = {0}, *pTaskInfo;
    int taskType, timeMin, timeHour, ret = 0;
    char *timeDay, *outStr;
    int isAdd;
    
    
    if (!cJSON_GetObjectItem(jsonValue, "rule_enable") 
        || !cJSON_GetObjectItem(jsonValue, "start_hour") 
        || !cJSON_GetObjectItem(jsonValue, "start_min")
        || !cJSON_GetObjectItem(jsonValue, "stop_hour")
        || !cJSON_GetObjectItem(jsonValue, "stop_min")
        || !cJSON_GetObjectItem(jsonValue, "week_day")
        || !cJSON_GetObjectItem(jsonValue, "name"))
    {
        printf("Miss argument to set wifi timer task!\n");
        outStr = "Miss argument to set wifi timer task!";
        global_weberrorcode=ERR_PARAMETER_MISS;
        return ERR_PARAMETER_MISS;		
    }
    isAdd = cJSON_GetObjectItem(jsonValue, "id")?0:1;
    startTaskInfo.ucTaskType = TASK_TYPE_WIFI_OFF;
    startTaskInfo.ucTimerMin = cJSON_GetObjectItem(jsonValue, "start_min")->valueint;
    startTaskInfo.ucTimerHour = cJSON_GetObjectItem(jsonValue, "start_hour")->valueint;
    startTaskInfo.ucEnable = cJSON_GetObjectItem(jsonValue, "rule_enable")->valueint;
    
    if (!isAdd)
    {
        startTaskInfo.usTaskId = cJSON_GetObjectItem(jsonValue, "id")->valueint;
    }
    UTIL_STRNCPY(startTaskInfo.aucTimeWeek, cJSON_GetObjectItem(jsonValue, "week_day")->valuestring, sizeof(startTaskInfo.aucTimeWeek));
    UTIL_STRNCPY(startTaskInfo.aucName, cJSON_GetObjectItem(jsonValue, "name")->valuestring, sizeof(startTaskInfo.aucName));
    
    stopTaskInfo.ucTaskType = TASK_TYPE_WIFI_ON;
    stopTaskInfo.ucTimerMin = cJSON_GetObjectItem(jsonValue, "stop_min")->valueint;
    stopTaskInfo.ucTimerHour = cJSON_GetObjectItem(jsonValue, "stop_hour")->valueint;
    stopTaskInfo.ucEnable = cJSON_GetObjectItem(jsonValue, "rule_enable")->valueint;
    if (!isAdd)
    {
        stopTaskInfo.usTaskId = cJSON_GetObjectItem(jsonValue, "id")->valueint;
    }
    UTIL_STRNCPY(stopTaskInfo.aucTimeWeek, cJSON_GetObjectItem(jsonValue, "week_day")->valuestring, sizeof(stopTaskInfo.aucTimeWeek));
    UTIL_STRNCPY(stopTaskInfo.aucName, cJSON_GetObjectItem(jsonValue, "name")->valuestring, sizeof(stopTaskInfo.aucName));
    printf("startTaskInfo.aucName %s   stopTaskInfo.aucName  %s\n", startTaskInfo.aucName, stopTaskInfo.aucName);
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    TimerTaskLoadConfig();
    if (isAdd)
    {
        ret = WifiTimerTaskAdd(&startTaskInfo, &stopTaskInfo);
        if (ret == 0)
        {
            TASK_INFO_STRU stTaskInfo;
            stTaskInfo.ucEnable = startTaskInfo.ucEnable;
            stTaskInfo.usTaskId = startTaskInfo.usTaskId;
            stTaskInfo.ucStartHour = startTaskInfo.ucTimerHour;
            stTaskInfo.ucStartMin = startTaskInfo.ucTimerMin;
            stTaskInfo.ucStopHour = stopTaskInfo.ucTimerHour;
            stTaskInfo.ucStopMin = stopTaskInfo.ucTimerMin;
            UTIL_STRNCPY(stTaskInfo.aucWeekDay, startTaskInfo.aucTimeWeek, sizeof(stTaskInfo.aucWeekDay));
            ProcAddWifiTimer(&stTaskInfo);
        }
        global_weberrorcode = ret;
    }
    else
    {
        ret = WifiTimerTaskEdit(&startTaskInfo, &stopTaskInfo);
        if (ret == 0)
        {
            TASK_INFO_STRU stTaskInfo;
            stTaskInfo.ucEnable = startTaskInfo.ucEnable;
            stTaskInfo.usTaskId = startTaskInfo.usTaskId;
            stTaskInfo.ucStartHour = startTaskInfo.ucTimerHour;
            stTaskInfo.ucStartMin = startTaskInfo.ucTimerMin;
            stTaskInfo.ucStopHour = stopTaskInfo.ucTimerHour;
            stTaskInfo.ucStopMin = stopTaskInfo.ucTimerMin;
            UTIL_STRNCPY(stTaskInfo.aucWeekDay, startTaskInfo.aucTimeWeek, sizeof(stTaskInfo.aucWeekDay));
            ProcEditWifiTimer(&stTaskInfo);
        }
        global_weberrorcode = ret;
    }
    
    if (ret == 0)
    {
        TimerTaskSaveConfig();
        //TimerTaskRun();
    }
    QtReleaseUciLock();
    return ret;
}

void proc_wifitimer_del(cJSON *jsonValue,cJSON *jsonOut)
{
    TIMER_TASK_STRU startTaskInfo = {0}, stopTaskInfo = {0};
    int ret = 0;
    char *outStr;

    if (!cJSON_GetObjectItem(jsonValue, "id"))
    {
        printf("Miss argument to del wifi timer task!\n");
        outStr = "Miss argument to del wifi timer task!";
        return;	
    }
    startTaskInfo.usTaskId = cJSON_GetObjectItem(jsonValue, "id")->valueint;
    startTaskInfo.ucTaskType = TASK_TYPE_WIFI_OFF;
    stopTaskInfo.usTaskId = cJSON_GetObjectItem(jsonValue, "id")->valueint;
    stopTaskInfo.ucTaskType = TASK_TYPE_WIFI_ON;
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return;
    }
    TimerTaskLoadConfig();
    WifiTimerTaskDel(&startTaskInfo, &stopTaskInfo);
    ProcDelWifiTimer(startTaskInfo.usTaskId);
    TimerTaskSaveConfig();
    //TimerTaskRun();
    QtReleaseUciLock();
    
    return;
}

int proc_wifitimer_get(cJSON *jsonValue,cJSON *jsonOut)
{
    TIMER_TASK_MNG_STRU *pTaskMngInfo;
    TIMER_TASK_STRU *pStartTaskInfo, *pStopTaskInfo;
    int ret = 0, pos;
    cJSON *dataArray;
    cJSON *dataObj;

    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    TimerTaskLoadConfig();
    QtReleaseUciLock();
    pTaskMngInfo = TimerTaskGet();
    
    dataArray = cJSON_CreateArray();
    if (!dataArray)
    {
        printf("Fail to create data array!\n");
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    dataObj = cJSON_CreateObject();
    if (!dataObj)
    {
        printf("Fail to create data array!\n");
		global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return ERR_INTERNALLOGIC_WRONG;
    }
    for (pos = 1; pos < pTaskMngInfo->ulTaskNum + 1; pos += 2)
    {
        cJSON *itemObj = cJSON_CreateObject();
        if (!itemObj)
        {
            printf("Fail to create data object!\n");
			global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
            return ERR_INTERNALLOGIC_WRONG;
        }
        pStartTaskInfo = &pTaskMngInfo->stTaskInfo[pos];
        pStopTaskInfo = &pTaskMngInfo->stTaskInfo[pos+1];

        cJSON_AddItemToObject(itemObj, "rule_enable", cJSON_CreateNumber(pStartTaskInfo->ucEnable));
    	cJSON_AddItemToObject(itemObj, "id", cJSON_CreateNumber(pStartTaskInfo->usTaskId));
    	cJSON_AddItemToObject(itemObj, "name", cJSON_CreateString(pStartTaskInfo->aucName));
        cJSON_AddItemToObject(itemObj, "start_hour", cJSON_CreateNumber(pStartTaskInfo->ucTimerHour));
        cJSON_AddItemToObject(itemObj, "start_min", cJSON_CreateNumber(pStartTaskInfo->ucTimerMin));
        cJSON_AddItemToObject(itemObj, "stop_hour", cJSON_CreateNumber(pStopTaskInfo->ucTimerHour));
        cJSON_AddItemToObject(itemObj, "stop_min", cJSON_CreateNumber(pStopTaskInfo->ucTimerMin));
        cJSON_AddItemToObject(itemObj, "week_day", cJSON_CreateString(pStartTaskInfo->aucTimeWeek));
        cJSON_AddItemToArray(dataArray, itemObj);
    }
    cJSON_AddItemToObject(dataObj, "rules", dataArray);
    cJSON_AddItemToObject(dataObj, "enable", cJSON_CreateNumber(pTaskMngInfo->ulEnable));
    cJSON_AddItemToObject(jsonOut, "data", dataObj);
    
    return 0;
}


int proc_wifitimer_sw_set(cJSON *jsonValue,cJSON *jsonOut)
{
    cJSON *dataObj;
    cJSON *ruleArray;
    int arraysize;
    int i = 0, id, rule_enable, enable;
    TASK_SW_INFO stSwInfo;
    
    if (QtGetUciLock() != VOS_RET_SUCCESS)
    {
        printf("Fail to get uci lock, %s\n", __func__);
        global_weberrorcode = ERR_GET_UCI_LOCK_FAIL;
        return ERR_GET_UCI_LOCK_FAIL;
    }
    TimerTaskLoadConfig();
    if (!cJSON_GetObjectItem(jsonValue, "enable")
        || !cJSON_GetObjectItem(jsonValue, "rules"))
    {
        printf("Miss argument to set wifi timer sw task!\n");
		global_weberrorcode=ERR_PARAMETER_MISS;
        QtReleaseUciLock();
        return ERR_PARAMETER_MISS;
    }
    enable = cJSON_GetObjectItem(jsonValue, "enable")->valueint;
    TimerTaskSetSw(enable);
    stSwInfo.ucGlobalSw = (UINT8)enable;

    ruleArray = cJSON_GetObjectItem(jsonValue, "rules");

    if (ruleArray)
    {
        
        arraysize = cJSON_GetArraySize(ruleArray);
        stSwInfo.ucTaskNum = (UINT8)arraysize;
        for (i = 0; i < arraysize; i++)
        {
            dataObj = cJSON_GetArrayItem(ruleArray, i);
            if (!cJSON_GetObjectItem(dataObj, "id")
                || !cJSON_GetObjectItem(dataObj, "rule_enable"))
            {
                printf("Miss argument to set wifi timer sw task!\n");
                global_weberrorcode=ERR_PARAMETER_MISS;
                QtReleaseUciLock();
                return ERR_PARAMETER_MISS;	
            }
            id = cJSON_GetObjectItem(dataObj, "id")->valueint;
            rule_enable = cJSON_GetObjectItem(dataObj, "rule_enable")->valueint;
            TimerTaskSetRuleSw(id ,rule_enable);
            stSwInfo.stRuleSwInfo[i].usTaskId = (UINT16)id;
            stSwInfo.stRuleSwInfo[i].ucEnable = (UINT8)rule_enable;
            
        }
    }
    TimerTaskSaveConfig();
    ProcSetWifiTimerSw(&stSwInfo);
    //TimerTaskRun();
    QtReleaseUciLock();
    return 0;
}

