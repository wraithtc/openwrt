#ifndef WIFITIMER_SET_H
#define WIFITIMER_SET_H
#include<unistd.h>
#include<string.h>
#include<uci.h>

#define MAX_TIMER_TASK_NUM  (128)
#define CROND_CONFIG_FILE_NAME  "/etc/crontabs/root"
#define UCI_CONFIG_FILE    "/etc/config/wifitimer"
#define RCT_OK (0)
#define RCT_ERROR (1)

#define TIMER_TASK_TYPE_OPTION    "type"
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
}TIMER_TASK_STRU;

typedef struct{
    UINT32 ulTaskNum;

    TIMER_TASK_STRU stTaskInfo[MAX_TIMER_TASK_NUM];
}TIMER_TASK_MNG_STRU;


UINT8 TimerTaskAdd(TIMER_TASK_STRU *taskInfo);

UINT8 TimerTaskDel(TIMER_TASK_STRU *taskInfo);

TIMER_TASK_MNG_STRU *TimerTaskGet();

bool TimerTaskLoadConfig();

bool TimerTaskSaveConfig();



#endif

