#ifndef WIFITIMER_SET_H
#define WIFITIMER_SET_H
#include<unistd.h>
#include<string.h>
#include<uci.h>

#define MAX_TIMER_TASK_NUM  (16)
#define CROND_CONFIG_FILE_NAME  "/etc/crontabs/root"
#define UCI_CONFIG_FILE    "/etc/config/wifitimer"
#define RCT_OK (0)
#define RCT_ERROR (1)

#define TIMER_TASK_TYPE_OPTION    "type"
#define TIMER_TASK_ENABLE_OPTION    "enable"
#define TIMER_TASK_MIN_TYPE_OPTION    "mintype"
#define TIMER_TASK_MIN_OPTION    "min"
#define TIMER_TASK_HOUR_TYPE_OPTION    "hourtype"
#define TIMER_TASK_HOUR_OPTION    "hour"
#define TIMER_TASK_DAY_TYPE_OPTION    "daytype"
#define TIMER_TASK_DAY_OPTION    "day"
#define TIMER_TASK_MONTH_TYPE_OPTION    "monthtype"
#define TIMER_TASK_MONTH_OPTION    "month"
#define TIMER_TASK_WEEK_TYPE_OPTION    "weektype"
#define TIMER_TASK_WEEK_OPTION    "week"
#define TIMER_TASK_ID_OPTION    "id"
#define TIMER_TASK_NAME_OPTION    "name"

#define TIMER_TASK_GLOBAL_ENABLE_OPTION    "globalenable"

#define WIFI_TIMER_ID_FILE    "/etc/info/wifitimerid"

typedef unsigned char UINT8;
typedef char INT8;
typedef unsigned short UINT16;
typedef short INT16;
typedef unsigned int UINT32;
typedef int INT32;


typedef enum{
    TASK_TYPE_WIFI_ON = 0,
    TASK_TYPE_WIFI_OFF,
    TASK_TYPE_POWER_OFF,
    TASK_TYPE_BUTT
}TASK_TYPE_E;

typedef enum{
    TIME_TYPE_NORMAL = 0,
    TIME_TYPE_PER,
    TIME_TYPE_BUTT
}TIME_TYPE_E;

typedef enum{
    TASK_RET_OK = 0,
    TASK_RET_INVALID_ARG,
    TASK_RET_MISS_ARG,
    TASK_RET_TOO_MANY_RULES,
    TASK_RET_BUTT
}TASK_RET_E;    

typedef struct{
    UINT8 ucTaskType;   /* 任务类型，0代表WIFI打开，1代表WIFI关闭，2代表定时关机 */
	UINT8 ucMinType;    /* 分钟任务类型，0表示特定分钟，1表示每X分钟 */	
	UINT8 ucTimerMin;    /* 定时器分钟 */
	UINT8 ucHourType;   /* 小时任务类型，0表示特定小时，1表示每X小时 */	
	
	UINT8 ucTimerHour;  /* 定时器小时 */
	UINT8 ucDayType;    /* 天任务类型，0表示特定日期(如每个月几号)，1表示每X天 */	
	UINT8 ucTimerDay;  /* 定时器天 */
	UINT8 ucMonthType;  /* 月任务类型，0表示特定日期(如每年几月)，1表示每X月 */	
    
	UINT8 ucTimerMonth; /* 定时器月 */
	UINT8 ucWeekType;   /* 周任务类型，0表示星期X，1表示每周X */	
    UINT8 ucEnable;
    UINT8 ucRsv;    /* 保留字段 */	
    
    char aucTimeWeek[32]; /* 定时器周，0表示周日，1表示周一，依次类推,   (0,1)表示星期一星期日 */
    
    UINT16 usTaskId;    /* task id，方便规则删除时索引 */
    UINT16 usRsv;  /* 保留字段 */

    char aucName[64];
}TIMER_TASK_STRU;

typedef struct{
    UINT8 ucStartHour;
    UINT8 ucStartMin;
    UINT8 ucStopHour;
    UINT8 ucStopMin;

    UINT16 usTaskId;
    UINT8 ucEnable;
    UINT8 ucRsv;
    
    char aucWeekDay[16];
}TASK_INFO_STRU;

typedef struct{
    UINT32 ulTaskNum;
    UINT32 ulEnable;

    TIMER_TASK_STRU stTaskInfo[MAX_TIMER_TASK_NUM];
}TIMER_TASK_MNG_STRU;

typedef struct{
    UINT16 usTaskId;
    UINT8  ucEnable;
    UINT8  ucRsv;
}TASK_RULE_SW_INFO;

typedef struct{
    UINT8 ucGlobalSw;
    UINT8 ucTaskNum;
    UINT16 usRsv;
    
    TASK_RULE_SW_INFO stRuleSwInfo[MAX_TIMER_TASK_NUM];
}TASK_SW_INFO;

UINT8 TimerTaskAdd(TIMER_TASK_STRU *taskInfo);

UINT8 TimerTaskDel(TIMER_TASK_STRU *taskInfo);

TIMER_TASK_MNG_STRU *TimerTaskGet();

TIMER_TASK_STRU *TimerTaskGetTask(TASK_TYPE_E eTaskType);

bool TimerTaskLoadConfig();

bool TimerTaskSaveConfig();

UINT8 TimerTaskEnable(TIMER_TASK_STRU *taskInfo);

UINT8 TimerTaskDisable(TIMER_TASK_STRU *taskInfo);

int TimerTaskSetSw(int enable);

int TimerTaskSetRuleSw(int id, int enable);

UINT8 WifiTimerTaskAdd(TIMER_TASK_STRU *startTaskInfo, TIMER_TASK_STRU *stopTaskInfo);

UINT8 WifiTimerTaskDel(TIMER_TASK_STRU *startTaskInfo, TIMER_TASK_STRU *stopTaskInfo);

UINT8 WifiTimerTaskEdit(TIMER_TASK_STRU *startTaskInfo, TIMER_TASK_STRU *stopTaskInfo);

#endif

