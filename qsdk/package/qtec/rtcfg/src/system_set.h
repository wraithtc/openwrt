#ifndef __SYSTEM_SET_H__
#define __SYSTEM_SET_H_
#include <stdlib.h>
#include <stdio.h>
#include "fwk.h"

#define QT_MAX_SEC (60)
#define QT_MAX_MIN (60)
#define QT_MAX_HOUR (24)
#define QT_MAX_DAY_FEB (28)
#define QT_MAX_DAY_FEB_RUN (29)
#define QT_MAX_DAY_MONTH_BIG (31)
#define QT_MAX_DAY_MONTH_SMALL (30)
#define QT_MAX_MONTH (12)
#define QT_MAX_YEAR (9999)
#define QT_MIN_YEAR (1000)
#define QT_TIME_SET_RESULT_FILE    "/tmp/time_set_result"
#define QT_IMAGE_FILE_NAME    "/tmp/firmware.img"
#define QT_SW_UPGRADE_RES_FILE    "/tmp/upgraderes"
#define QT_IMG_CHK_RES_FILE    "/tmp/imgchkres"

#define ONE_KEY_SWITCH_USR_PWD_FILE    "/tmp/onekeychange"
#define ONE_KEY_SWITCH_RESULT_FILE    "/tmp/onekeyswitchres"

#define MAX_HOST_NUM   256

#define QT_SPI_LOCK_FILE "/tmp/spilock"

typedef struct{
    UINT8 ucSec;    /* 设置当前时间中的秒 */
    UINT8 ucMin;    /* 设置当前时间中的分钟 */
    UINT8 ucHour;    /* 设置当前时间中的小时 */
    UINT8 ucMonth;    /* 设置当前时间中的日期中的月份 */
    
    UINT8 ucDay;    /* 设置当前时间中的日期中的天 */
    UINT8 ucRsv;    /* 保留字段 */
    UINT16 usYear;    /* 设置当前时间中的年份 */
    
    
}QT_TIME_STRU;

typedef struct{
    char url[256];
    char ip[32];
}QT_HOSTS_CFG;

void QtReboot();

void QtRestore();

VOS_RET_E QtSetSysTime(QT_TIME_STRU *pstTimeInfo);

VOS_RET_E QtGetSysTime(QT_TIME_STRU *pstTimeInfo);

VOS_RET_E QtUpgradeSoftware(UINT8 isKeepConfig);

VOS_RET_E QtQueryUpgrade(char *outStr, int outLen);

VOS_RET_E QtOneKeySwitch();

VOS_RET_E QtGetOneKeySwitchStatus(int *status);

int QtSetHosts(QT_HOSTS_CFG *hosts);

int QtGetHosts(QT_HOSTS_CFG *hosts, int *len);

void QtEmptyHosts();
#endif /* __SYSTEM_SET_H__ */
