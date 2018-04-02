#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "keyapi.h"
#include "encryption.h"
#include <sqlite3.h>
#include <sec_api.h>

#define MAXFILE 65535

static void daemonize()
{
	pid_t pc;
	int i;

	pc = fork(); 
	if(pc<0)
	{
		printf("error fork\n");
		exit(1);
	}
	else if(pc > 0)
	{
		exit(0);
	}
	setsid();
	chdir("/");
	umask(0);
	//for(i=0;i<MAXFILE;i++)
	//{
	//	close(i);
	//}
}
int main(void)
{
	int usedcount = 0;
    int nRet = 0;
	int i = 0;
	int randomReadyFlag = 0;
	int unusedcount = 0;
	char userid[32] = {0};
	char deviceid[32] = {0};
	char randBuf[16] = {0};
    char szSql[4096] = {0};
    char** pResult;
	char* cErrMsg;
	struct tagCQtQkMangent *pstcqtqkmangent;
    sqlite3 *dbHangle;

	daemonize();
	//freopen("/tmp/qtkeyoutput.txt","w",stdout);
	printf("enter qtkeymanger.\r\n");

	nRet = sqlite3_open("/etc/testDB.db", &dbHangle);
 	if(nRet){
		printf("CQtQkMangentCommon::QkPoolOpen, sqlite3_open error\n");
 	}

	while(0 != sqlite3_get_table(dbHangle, "select * from rawkey;", &pResult, 0, 0, &cErrMsg))
	{
        memset(szSql, 0, 4096);
		sprintf(szSql,"create table rawkey(key_id text primary key,key_state int,key blob, validity_time int,create_time text,modify_time text,peeridList text);");
		nRet = sqlite3_exec(dbHangle, szSql, NULL, 0, &cErrMsg);
		if (0 != nRet){
	        printf("CQtQkMangentCommon::GetSynKeyKeyByIdNode_common, sqlite3_exec error\n");
	        sqlite3_free_table(pResult);
	    }
        sleep(1);
	}
	sqlite3_free_table(pResult);
	
	if(0 != sqlite3_get_table(dbHangle, "select * from synkey;", &pResult, 0, 0, &cErrMsg))
	{
        memset(szSql, 0, 4096);
		sprintf(szSql,"create table synkey(key_id text primary key,key_state int,key blob,user_id text,device_id text,validity_time int,create_time text,modify_time text,peeridList text);");
		nRet = sqlite3_exec(dbHangle, szSql, NULL, 0, &cErrMsg);
		if (0 != nRet){
	        printf("CQtQkMangentCommon::GetSynKeyKeyByIdNode_common, sqlite3_exec error\n");
	        sqlite3_free_table(pResult);
	    }
        sleep(1);
	}
	sqlite3_free_table(pResult);
	sqlite3_close(dbHangle);

	while(0 == randomReadyFlag)
	{
		memset(randBuf, 0, 16);
    	GetRandom(randBuf, 16);
		for(i = 0; i < 16; i++)
		{
			if(randBuf[i] != 0)
			{
				randomReadyFlag = 1;
				printf("random data is ok!\n");
				break;
			}
		}
		sleep(1);
	}
 

	while(1)
	{
		pstcqtqkmangent = GetCQtQkMangent();
		//if rawkeynumber <50, addkey
		C_GetCount(pstcqtqkmangent, 1, &usedcount, &unusedcount, userid, deviceid);
		printf("unusedcount is %d.\n", unusedcount);
		if(unusedcount <= MIXRAWKEYNUM)
		{
			AddRawKey();
		}
		ReleaseCQtQkMangent(&pstcqtqkmangent);
		sleep(5);
	}
}
