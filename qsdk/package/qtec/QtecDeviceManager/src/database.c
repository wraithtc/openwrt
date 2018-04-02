#include "basic.h"
#include <sqlite3.h>

#if 0
struct DeviceManagerEntry *globalDeviceManagerTable =NULL;
int globalDeviceManagerTable_num=0;

struct DeviceEntry *globalDeviceEntryTable=NULL;
int globalDeviceEntryTable_num=0;
#endif
#define DB_FILE "/etc/qtec.db"

//全局变量保存打开的数据库
static sqlite3 *db =NULL;

#if 0
pthread_mutex_t mut;
pthread_t workthread;

struct UserEntry *global_usertable_head=NULL;
struct UserEntry *global_usertable_tail=NULL;
struct DeviceEntry *global_devicetable_head=NULL;
struct DeviceEntry *global_devicetable_tail=NULL;
#endif 

/**
* 在数据库中，这边存储的表暂时先定为4张:
*  user_table:    id(primary key), name,  grade
*  managedDevice_table:  id(primary key), type, name, status
*  password_table: id(primary key), content, userId, deviceId
*  fingerPrint_table: id(primary key), name, userId, deviceId
**/

//DEBUG 展示表项内容
int Debug_ShowTableLine(int n_column, char ** column_value, char ** column_name)
{
    int i;
    DEBUG_PRINTF("this table has %d column\n", n_column);
    for(i=0;i<n_column;i++)
    {
        DEBUG_PRINTF("column_name: %s ====> column value: %s \n", column_name[i], column_value[i]);
    }
    printf("========\n");
}


//managedDevice_table 的回调函数
int LoadManagedDeviceTable(void *para, int n_column, char ** column_value, char ** column_name)
{
    DEBUG_PRINTF("===========%s=======\n",__func__);
    Debug_ShowTableLine(n_column, column_value, column_name);

    //input the managedDevice_table info into global_ManagedDeviceEntryList
    struct simpleDeviceEntry *newEntry = malloc_simpleDeviceEntry();
    strcpy(newEntry->deviceid, column_value[ManagedDeviceEntryId_index]);
    newEntry->type = atoi (column_value[ManagedDeviceEntryType_index]);
    strcpy(newEntry->name, column_value[ManagedDeviceEntryName_index]);
    newEntry->status = atoi(column_value[ManagedDeviceEntryStatus_index]);
    strcpy(newEntry->ieee_addr, column_value[ManagedDeviceEntryIeee_addr_index]);
    newEntry->nw_addr =atoi(column_value[ManagedDeviceEntryNw_addr_index]);
    strcpy(newEntry->version,column_value[ManagedDeviceEntryVersion_index]);
    strcpy(newEntry->model,column_value[ManagedDeviceEntryModel_index]);
    strcpy(newEntry->seq,column_value[ManagedDeviceEntrySeq_index]);

    //将newEntry 添加到 global_ManagedDeviceEntryList

    qtec_insert_list((struct qtec_list *) newEntry, (struct qtec_list * *) &global_ManagedDeviceEntryList_head, (struct qtec_list * *) &global_ManagedDeviceEntryList_tail);
    return 0;
}

int AddNewEntrytoManagedDeviceTable(struct simpleDeviceEntry *input)
{
    DEBUG_PRINTF("======[%s]======\n", __func__);
    char cmd[256]={0};
    int rc=0;
    char *errmsg =NULL;
    snprintf(cmd,256,"insert into managedDevice_table values('%s','%d','%s','%d','%s','%d','%s','%s','%s')",input->deviceid,input->type, input->name,input->status,input->ieee_addr,input->nw_addr,input->version,input->model,input->seq);
    rc=sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINTF("[%s] add new entry into user_table fail, error code is %d, reason is %s \n",__func__, rc, errmsg);
        if(errmsg !=NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;    
    }

    return 0;
}

//通过指定device id 来删除对应的managed Device table entry
//cmd: delete from managedDevice_table where id='%s' 
int DelEntryFromManagedDeviceTable(char *input_deviceid)
{
    DEBUG_PRINTF("======[%s]====input: %s=====\n",__func__,input_deviceid);
    char cmd[256]={0};
    int rc =0;
    char *errmsg = NULL;
    snprintf(cmd, 256, "delete from managedDevice_table where id='%s'",input_deviceid);
    rc = sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if(rc != SQLITE_OK )
    {
        DEBUG_PRINTF("[%s] delete entry from managedDevice_table fail, error code is %d, reason is %s \n",__func__, rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;
    }
    return 0;
    
}


//根据input entry 的device id 来更新指定的ManagedDevice table entry, 因为 id 是不能改变的。
//cmd : update managedDevice_table type='%d',name='%s',status='%d' where id='%s' 
int UpdateEntryToManagedDeviceTable(struct simpleDeviceEntry *input)
{
    DEBUG_PRINTF("====%s===input entry user id: %s====\n", __func__,input->deviceid);
    char cmd[256]={0};
    int rc =0;
    char *errmsg =NULL;
    snprintf(cmd,256, "update managedDevice_table set type='%d',name='%s',status='%d',ieee_addr='%s',nw_addr='%d',version='%s',model='%s',seq='%s'  where id='%s'",input->type,input->name,input->status,input->ieee_addr,input->nw_addr,input->version,input->model,input->seq,input->deviceid);
    rc=sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        DEBUG_PRINTF("[%s] update entry to managedDevice_table fail, error code is %d, reason is %s \n",__func__, rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;
    }
    return 0;
}

//user_table 的回调函数
int LoadUserTable(void * para, int n_column, char ** column_value, char ** column_name)
{
    DEBUG_PRINTF("========%s=====\n",__func__);
    Debug_ShowTableLine(n_column, column_value, column_name);

    //input the database info into global_usertable entry 

    struct UserEntry *newEntry= malloc_userEntry();
    strcpy(newEntry->username, column_value[UserEntryName_index]);
    strcpy(newEntry->userid,column_value[UserEntryId_index]);
    newEntry->grade =atoi(column_value[UserEntryGrade_index]);

    qtec_insert_list((struct qtec_list *) newEntry, (struct qtec_list * *) &global_UserEntryList_head, (struct qtec_list * *) &global_UserEntryList_tail);
    return 0;
}


int AddNewEntryToUserTable(struct UserEntry *input)
{
    DEBUG_PRINTF("======%s======\n", __func__);
    char cmd[256]={0};
    int rc=0;
    char *errmsg =NULL;
    snprintf(cmd,256,"insert into user_table values('%s','%s','%d')",input->userid,input->username,input->grade);
    rc=sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINTF("add new entry into user_table fail, error code is %d, reason is %s \n", rc, errmsg);
        if(errmsg !=NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;    
    }

    return 0;
}


//通过指定user id 来删除对应的user table entry
//cmd: delete from user_table where id='%s' 
int DelEntryFromUserTable(char *input_userid)
{
    DEBUG_PRINTF("======%s====input: %s=====\n",__func__,input_userid);
    char cmd[256]={0};
    int rc =0;
    char *errmsg = NULL;
    snprintf(cmd, 256, "delete from user_table where id='%s'",input_userid);
    rc = sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if(rc != SQLITE_OK )
    {
        DEBUG_PRINTF("delete entry from user_table fail, error code is %d, reason is %s \n", rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;
    }
    return 0;
    
}

//根据input entry 的user id 来更新指定的user table entry, 因为 id 是不能改变的。
//cmd : update user_table name='%s',grade='%d' where id='%s' 
int UpdateEntryToUserTable(struct UserEntry *input)
{
    DEBUG_PRINTF("====%s===input entry user id: %s====\n", __func__,input->userid);
    char cmd[256]={0};
    int rc =0;
    char *errmsg =NULL;
    snprintf(cmd,256, "update user_table set name='%s',grade='%d' where id='%s'",input->username,input->grade,input->userid);
    rc=sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        DEBUG_PRINTF("update entry to user_table fail, error code is %d, reason is %s \n", rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;
    }
    return 0;
}

//fingerPrint_table 的回调函数
int LoadFingerPrintTable(void * para, int n_column, char ** column_value, char ** column_name)
{
    DEBUG_PRINTF("========%s=====\n",__func__);
    Debug_ShowTableLine(n_column, column_value, column_name);

    //input the database info into global_usertable entry 

    struct FingerPrintEntry* newEntry= malloc_fingerPrintEntry();

    strcpy(newEntry->fingerprintid,column_value[FingerPrintEntryId_index]);
    strcpy(newEntry->name,column_value[FingerPrintEntryName_index]);
    strcpy(newEntry->deviceid,column_value[FingerPrintEntryDeviceId_index]);
    strcpy(newEntry->userid,column_value[FingerPrintEntryUserId_index]);
    
    qtec_insert_list((struct qtec_list *) newEntry, (struct qtec_list * *) &global_FingerPrintEntryList_head, (struct qtec_list * *) &global_FingerPrintEntryList_tail);
    return 0;
}


int AddNewEntryToFingerPrintTable(struct FingerPrintEntry* input)
{
    DEBUG_PRINTF("======%s======\n", __func__);
    char cmd[256]={0};
    int rc=0;
    char *errmsg =NULL;
    snprintf(cmd,256,"insert into fingerPrint_table values('%s','%s','%s','%s')",input->fingerprintid,input->name,input->deviceid,input->userid);
    rc=sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINTF("add new entry into fingerPrint_table fail, error code is %d, reason is %s \n", rc, errmsg);
        if(errmsg !=NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;    
    }

    return 0;
}


//通过指定fingerPrintId 和 deviceid 来删除对应的FingerPrint table entry
//cmd: delete from FingerPrintTable where fingerPrintid='2' and deviceid='2' ;
int DelEntryFromFingerPrintTable(char *input_fingerPrintid, char *input_deviceid)
{
    DEBUG_PRINTF("======%s====input_fingerPrintid: %s,  input_deviceid: %s =====\n",__func__,input_fingerPrintid, input_deviceid);
    char cmd[256]={0};
    int rc =0;
    char *errmsg = NULL;
    snprintf(cmd, 256, "delete from fingerPrint_table where fingerPrintid='%s' and deviceid='%s' ",input_fingerPrintid, input_deviceid);
    rc = sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if(rc != SQLITE_OK )
    {
        DEBUG_PRINTF("delete entry from fingerPrint_table fail, error code is %d, reason is %s \n", rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;
    }
    return 0;
    
}

//根据指定的fingerPrintId 和 deviceid 来更新指定的FingerPrint table entry,
//cmd : update fingerPrint_table set name='%s',userid='%s' where fingerPrintid='%s' and deviceid='%s' 
int UpdateEntryToFingerPrintTable(struct FingerPrintEntry* input)
{
    DEBUG_PRINTF("====%s===input entry fingerPrintid: %s, deviceid: %s====\n", __func__,input->fingerprintid,input->deviceid);
    char cmd[256]={0};
    int rc =0;
    char *errmsg =NULL;
    snprintf(cmd,256, "update fingerPrint_table set name='%s',userid='%s' where fingerPrintid='%s' and deviceid='%s'",input->name,input->userid,input->fingerprintid,input->deviceid);
    rc=sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        DEBUG_PRINTF("update entry to fingerPrint_table fail, error code is %d, reason is %s \n", rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;
    }
    return 0;
}


//log_table 的回调函数
int LoadLogTable(void * para, int n_column, char ** column_value, char ** column_name)
{
    DEBUG_PRINTF("========%s=====\n",__func__);
    Debug_ShowTableLine(n_column, column_value, column_name);

    //input the database info into global_usertable entry 

    struct LogEntry* newEntry= malloc_logEntry();

    strcpy(newEntry->time,column_value[LogEntryTime_index]);
    newEntry->opratetype = column_value[LogEntryOperatetype_index];
    strcpy(newEntry->userid,column_value[LogEntryUserId_index]);
    strcpy(newEntry->devid,column_value[LogEntryDevId_index]);
    
    qtec_insert_list((struct qtec_list *) newEntry, (struct qtec_list * *) &global_LogEntryList_head, (struct qtec_list * *) &global_LogEntryList_tail);
    return 0;
}


int AddNewEntryToLogTable(struct LogEntry* input)
{
    DEBUG_PRINTF("======%s======\n", __func__);
    char cmd[256]={0};
    int rc=0;
    char *errmsg =NULL;
    snprintf(cmd,256,"insert into log_table values('%s','%d','%s','%s')",input->time,input->opratetype,input->userid,input->devid);
    rc=sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if( rc != SQLITE_OK )
    {
        DEBUG_PRINTF("add new entry into fingerPrint_table fail, error code is %d, reason is %s \n", rc, errmsg);
        if(errmsg !=NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;    
    }

    return 0;
}


//每次删除只是删除table的第一条
//delete from log_table where rowid=(select min(rowid) from log_table);
int DelFirstEntryFromLogTable()
{
    DEBUG_PRINTF("======%s====\n",__func__);
    char cmd[256]={0};
    int rc =0;
    char *errmsg = NULL;
    snprintf(cmd, 256, "delete from log_table where rowid=(select min(rowid) from log_table) ");
    rc = sqlite3_exec(db,cmd,NULL,NULL,&errmsg);
    if(rc != SQLITE_OK )
    {
        DEBUG_PRINTF("delete entry from log_table fail, error code is %d, reason is %s \n", rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);

        global_error = ERR_DB_CMD;
        return global_error;
    }
    return 0;
    
}


//获取指定表中最大的rowid
int getMaxRowIdFromDatabase(char * input_tablename)
{
    DEBUG_PRINTF("====%s====\n",__func__);
    int rowid=0;
    char cmd[256] = {0};
    int rc =0;
    char *errmsg = NULL;
    char **result=NULL;
    snprintf(cmd, 256, "select max(rowid) from %s",input_tablename);
    rc = sqlite3_get_table(db,cmd,&result,NULL,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        DEBUG_PRINTF("[%s] get max row id from talbe %s  fail, error code is %d, reason is %s \n", __func__,input_tablename, rc, errmsg);
        if(errmsg!=NULL)
            free(errmsg);

        global_error= ERR_DB_CMD;
        return rowid;
    }
    else
    {
        if(result[1] == NULL)
        {
            return 0;
        }
        rowid= atoi(result[1]);
        sqlite3_free_table(result);
    }

    return rowid;

}
//暂时先把每个devicemanagerentry 存储到不同的文件里
void reloadDataFromSystem()
{
    //to do
}


void saveDataToSystem()
{
    //to do
}

void initData()
{
    DEBUG_PRINTF("===========%s=======\n",__func__);

    int rc =0;
    char * errmsg=NULL;
    rc =sqlite3_open( DB_FILE, &db);

    //创建 user_table, 若已存在，则报错，但这个错误可以接受
    rc = sqlite3_exec(db, "create table user_table(ID primary key, name, grade)",NULL,NULL,&errmsg);
    if( rc != SQLITE_OK)
    {
        DEBUG_PRINTF("create user_table fail， error code is %d, reason is %s\n",rc,errmsg);
        if (errmsg !=NULL)
            free(errmsg);
    }

    //创建managedDevice_table, 若已存在，则报错，但这个错误可以接受
    rc = sqlite3_exec(db, "create table managedDevice_table(ID primary key, type,  name, status,ieee_addr,nw_addr,version,model,seq)",NULL,NULL,&errmsg);
    if( rc != SQLITE_OK)
    {
        DEBUG_PRINTF("create managedDevice_table fail, error code is %d, reason is %s\n", rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);
    }

    //创建fingerPrintDevice_table, 若已存在，则报错，但这个错误可以接受
    rc = sqlite3_exec(db, "create table fingerPrint_table(fingerPrintid, name,  deviceid, userid, primary key (fingerPrintid, deviceid))",NULL,NULL,&errmsg);
    if( rc != SQLITE_OK)
    {
        DEBUG_PRINTF("create fingerPrintDevice_table fail, error code is %d, reason is %s\n", rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);
    }

    //创建log_table, 若已存在，则报错，但这个错误可以接受
    rc = sqlite3_exec(db, "create table log_table(time, opratetype,userid,devid)",NULL,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        DEBUG_PRINTF("create log_table fail, error code is %d, reason is %s \n", rc, errmsg);
        if(errmsg !=NULL)
            free(errmsg);
    }

    
     //从数据库加载信息到内存
     //加载  用户表
    rc = sqlite3_exec(db, "select * from user_table", LoadUserTable,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        DEBUG_PRINTF("[%s] load user table fail, error code is %d, reason is %s \n ",__func__, rc, errmsg);
        if(errmsg !=NULL)
            free(errmsg);
    }

    //加载 设备管理表
    rc = sqlite3_exec(db, "select * from managedDevice_table", LoadManagedDeviceTable,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        DEBUG_PRINTF("[%s] load managedDevice_table fail, error code is %d, reason is %s \n ",__func__, rc, errmsg);
        if(errmsg !=NULL)
            free(errmsg);
    }

    //加载 指纹管理表
    rc = sqlite3_exec(db, "select * from fingerPrint_table",LoadFingerPrintTable,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        DEBUG_PRINTF("[%s] load managedDevice_table fail, error code is %d, reason is %s \n ",__func__, rc, errmsg);
        if(errmsg !=NULL)
            free(errmsg);
    }  

    //加载日志表
    rc = sqlite3_exec(db, "select * from log_table", LoadLogTable,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        DEBUG_PRINTF("[%s] load log_table fail, error code is %d, reason is %s \n", __func__, rc, errmsg);
        if(errmsg != NULL)
            free(errmsg);
    }
}



