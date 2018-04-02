#ifndef BASIC_H
#define BASIC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include "qtecstruct.h"
#include "error.h"
#include "fwk.h"





#define DEBUG 1

#ifdef DEBUG 
#define DEBUG_PRINTF(format,...) printf(format, ##__VA_ARGS__);fflush(stdout);
#define DEBUG_PRINTF_RED(format,...) printf("\e[1;31m"format"\e[0m",##__VA_ARGS__);fflush(stdout);
#define DEBUG_PRINTF_GRE(format,...) printf("\e[1;32m"format"\e[0m",##__VA_ARGS__);fflush(stdout);

#else
#define DEBUG_PRINTF(format,...)
#endif


#if 0
extern struct UserEntry *global_usertable_head;
extern struct UserEntry *global_usertable_tail;
extern struct DeviceEntry *global_devicetable_head;
extern struct DeviceEntry *global_devicetable_tail;
#endif

enum DeviceManager_error_code {
    DEVMANAGER_DEVICE_NOT_FOUND = 101,
    DEVMANAGER_INVALID_CMD = 102,
    DEVMANAGER_WRONG_CONNECT = 103,
    DEVMANAGER_INTER_WRONG= 104,
    DEVMANAGER_KEY_WRONG=105,
    
   
};




/**
* 在数据库中，这边存储的表暂时先定为4张:
*  user_table:    id(primary key), name,  grade
*  managedDevice_table:  id(primary key), type, name,  status
*  password_table: id(primary key), content, userId, deviceId
*  fingerPrint_table: id(primary key), name, userId, deviceId
**/
//user_table
#define UserEntryId_index 0
#define UserEntryName_index 1
#define UserEntryGrade_index 2

//managedDevice_table
#define ManagedDeviceEntryId_index 0
#define ManagedDeviceEntryType_index 1
#define ManagedDeviceEntryName_index 2
#define ManagedDeviceEntryStatus_index 3
#define ManagedDeviceEntryIeee_addr_index 4
#define ManagedDeviceEntryNw_addr_index 5
#define ManagedDeviceEntryVersion_index 6
#define ManagedDeviceEntryModel_index 7
#define ManagedDeviceEntrySeq_index 8

//fingerPrint_table
#define FingerPrintEntryId_index 0
#define FingerPrintEntryName_index 1
#define FingerPrintEntryDeviceId_index 2
#define FingerPrintEntryUserId_index 3

//log_table
#define LogEntryTime_index 0
#define LogEntryOperatetype_index 1
#define LogEntryUserId_index 2
#define LogEntryDevId_index 3

//******************************************************************//
//defined in logic.c
//搜寻到的设备列表
extern struct simpleDeviceEntry *global_searchedDeviceEntryList_head;
extern struct simpleDeviceEntry *global_searchedDeviceEntryList_tail;

//添加管理的设备列表
extern struct simpleDeviceEntry *global_ManagedDeviceEntryList_head;
extern struct simpleDeviceEntry *global_ManagedDeviceEntryList_tail;

//用户列表
extern struct UserEntry *global_UserEntryList_head;
extern struct UserEntry *global_UserEntryList_tail;

//指纹管理表
extern struct FingerPrintEntry * global_FingerPrintEntryList_head ;
extern struct FingerPrintEntry * global_FingerPrintEntryList_tail ;

//日志列表
extern struct LogEntry* global_LogEntryList_head;
extern struct LogEntry* global_LogEntryList_tail;
//*****************************************************************//

//defined in connect.c
extern void *g_msgHandle ;

#endif
