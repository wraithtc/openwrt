#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "timerTask_set.h"

/**********************************************
    函数名：TimerTaskDecodeMsg
    功能： 解析json字符串
    创建人：tongchao
    创建时间：2017/7/21
***********************************************/

int TimerTaskDecodeMsg(const char *inputStr)
{
    cJSON *json, *jsonValue, *jsonOut, *jsonObj, *dataObj = NULL;
    int f0, f1, f2, lTaskType;
    char *outBuff, *target, *method;
    TIMER_TASK_STRU taskInfo = {0}, *pTaskInfo;
    int taskType, timeMin, timeHour, ret = 0;
    char *timeDay, *outStr;

    f0 = dup(STDOUT_FILENO);
	f1 = open("/dev/null", O_RDWR);
	f2 = dup2(f1, STDOUT_FILENO);
	close(f1);
    
    printf("%s\n",inputStr);
    json = cJSON_Parse(inputStr);
    if (!json)
	{
		printf("parse input str error!\n");
        outStr = "parse input str error!";
        ret = -1;
		goto out;		
	}

    target = cJSON_GetObjectItem(json, "requestUrl")?cJSON_GetObjectItem(json, "requestUrl")->valuestring:"";
    method = cJSON_GetObjectItem(json, "method")?cJSON_GetObjectItem(json, "method")->valuestring:"";
	jsonValue = cJSON_GetObjectItem(json, "data");
    if (!jsonValue)
    {
        printf("No data!\n");
        outStr = "No data!";
        ret = -1;
		goto out;	
    }
    
    
    if (!strcmp(method, "post"))
    {
        if (!cJSON_GetObjectItem(jsonValue, "tasktype") 
            || !cJSON_GetObjectItem(jsonValue, "minute") 
            || !cJSON_GetObjectItem(jsonValue, "hour")
            || !cJSON_GetObjectItem(jsonValue, "enable")
            || !cJSON_GetObjectItem(jsonValue, "day"))
        {
            printf("Miss argument to set timer task!\n");
            outStr = "Miss argument to set timer task!";
            ret = -1;
		    goto out;	
        }
        taskInfo.ucTaskType = cJSON_GetObjectItem(jsonValue, "tasktype")->valueint;
        taskInfo.ucTimerMin = cJSON_GetObjectItem(jsonValue, "minute")->valueint;
        taskInfo.ucTimerHour = cJSON_GetObjectItem(jsonValue, "hour")->valueint;
        taskInfo.ucEnable = cJSON_GetObjectItem(jsonValue, "enable")->valueint;
        taskInfo.usTaskId = taskInfo.ucTaskType;
        strncpy(taskInfo.aucTimeWeek, cJSON_GetObjectItem(jsonValue, "day")->valuestring, sizeof(taskInfo.aucTimeWeek));
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
    }
    else if (!strcmp(method, "get"))
    {
        if (!cJSON_GetObjectItem(jsonValue, "tasktype"))
        {
            printf("Miss argument to get timer task!\n");
            outStr = "Miss argument to get timer task!";
            ret = -1;
		    goto out;	
        }
        lTaskType = cJSON_GetObjectItem(jsonValue, "tasktype")->valueint;
        if ((pTaskInfo = TimerTaskGetTask(lTaskType)) == NULL)
        {
            outStr = "Fail to get timer task!";
            ret = -1;
		    goto out;
        }
        dataObj = cJSON_CreateObject();
        if (dataObj == NULL)
        {
            printf("Fail to create data json obj!\n");
            outStr = "Fail to create data json obj!";
            ret = -1;
		    goto out;	
        }
        cJSON_AddItemToObject(dataObj, "minute", cJSON_CreateNumber(pTaskInfo->ucTimerMin));
    	cJSON_AddItemToObject(dataObj, "hour", cJSON_CreateNumber(pTaskInfo->ucTimerHour));
    	cJSON_AddItemToObject(dataObj, "enable", cJSON_CreateNumber(pTaskInfo->ucEnable));
        cJSON_AddItemToObject(dataObj, "day", cJSON_CreateString(pTaskInfo->aucTimeWeek));
    }

out:    
	jsonOut = cJSON_CreateObject();
    if (0 == ret)
    {
        outStr = "OK";
    }

    if (NULL != dataObj)
    {
        cJSON_AddItemToObject(jsonOut, "data", dataObj);
    }
	cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString(outStr));
	cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(ret));
    fflush(stdout);
	dup2(f0, f2);
	outBuff = cJSON_Print(jsonOut);
	printf("%s\n",outBuff);
	free(outBuff);

	cJSON_Delete(jsonOut);
	cJSON_Delete(json);
}

int main()
{

	int length;
	char *method;
	char *inputstring;
	
	printf("content-type:application/json\r\n\r\n");
    
	method = getenv("REQUEST_METHOD"); 
	if(method == NULL)
	{
	    printf("method is null!\n");
		return 1;   
	}

	//post method,read from stdin
	if(!strcmp(method, "POST")) 
	{

		length = atoi(getenv("CONTENT_LENGTH")); 
		if(length != 0)
		{
			inputstring = malloc(sizeof(char)*length + 1); 
			if (0 != fread(inputstring, sizeof(char), length, stdin)) 
			{
				TimerTaskDecodeMsg(inputstring);
			}
	    }
	}

	//get method
	else if(!strcmp(method, "GET"))
	{

		inputstring = getenv("QUERY_STRING");   
		length = strlen(inputstring);

	}
	return 0;
}


