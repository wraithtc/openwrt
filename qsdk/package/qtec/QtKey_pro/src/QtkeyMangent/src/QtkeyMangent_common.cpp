#include "QtkeyMangent_common.h"
#include "QtKeyEncrypt.h"


static int UserResult_GetTime(void *NotUsed, int argc, char **argv, char **azColName);

static int UserResult_Count(void *NotUsed, int argc, char **argv, char **azColName);

static int UserResult_GetRawKeyByNode(void *NotUsed, int argc, char **argv, char **azColName);

unsigned char g_key[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

typedef struct
{
	string key_id;
	int key_state;
	string key;
	string user_id;
	string device_id;
	int validity_time;
	string create_time;
	string modify_time;
	string peeridList;
}Table;

typedef struct
{
	string key_id;
	int key_state;
	string key;
	int validity_time;
	string create_time;
	string modify_time;
	string peeridList;
}RawKeyTable;

static int UserResult_GetTime(void *NotUsed, int argc, char **argv, char **azColName)
{
    strncpy((char *)NotUsed,argv[0],strlen(argv[0]));
	
    return 0;
}

static int UserResult_Count(void *NotUsed, int argc, char **argv, char **azColName)
{
	*(int *)NotUsed = atoi(argv[0]);
	
    return 0;
}

static int UserResult_GetRawKeyByNode(void *NotUsed, int argc, char **argv, char **azColName)
{
 	for(int i = 0 ; i < argc ; i++)  
    {  
        cout<<azColName[i]<<" = "<<(argv[i] ? argv[i] : "NULL")<<", " << endl;  
    }
   	
   	
	Table *table = (Table *)NotUsed;
    table->key_id = (argv[0] ? argv[0] : "NULL");
    table->key_state = atoi((argv[1] ? argv[1] : "NULL"));
    table->key = (argv[2] ? argv[2] : "NULL");
    table->user_id = (argv[3] ? argv[3] : "NULL");
    table->device_id = (argv[4] ? argv[4] : "NULL");
    table->validity_time = atoi((argv[5] ? argv[5] : "NULL"));
    table->create_time = (argv[6] ? argv[6] : "NULL");
    table->modify_time = (argv[7] ? argv[7] : "NULL");
    table->peeridList = (argv[8] ? argv[8] : "NULL");

    return 0;
}

static int UserResult_GetRawKey(void *NotUsed, int argc, char **argv, char **azColName)
{
 	for(int i = 0 ; i < argc ; i++)  
    {  
        cout<<azColName[i]<<" = "<<(argv[i] ? argv[i] : "NULL")<<", " << endl;  
    }
   	
   	
	RawKeyTable *table = (RawKeyTable *)NotUsed;
    table->key_id = (argv[0] ? argv[0] : "NULL");
    table->key_state = atoi((argv[1] ? argv[1] : "NULL"));
    table->key = (argv[2] ? argv[2] : "NULL");
    table->validity_time = atoi((argv[3] ? argv[3] : "NULL"));
    table->create_time = (argv[4] ? argv[4] : "NULL");
    table->modify_time = (argv[5] ? argv[5] : "NULL");
    table->peeridList = (argv[6] ? argv[6] : "NULL");

    return 0;
}

void CQtQkMangentCommon::HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
{
	char ddl,ddh;
	int i;

	for (i=0; i<nLen; i++)
	{
	ddh = 48 + pbSrc[i] / 16;
	ddl = 48 + pbSrc[i] % 16;
	if (ddh > 57) ddh = ddh + 7;
	if (ddl > 57) ddl = ddl + 7;
	pbDest[i*2] = ddh;
	pbDest[i*2+1] = ddl;
	}

	pbDest[nLen*2] = '\0';
}

void CQtQkMangentCommon::StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
{
	char h1,h2;
	BYTE s1,s2;
	int i;

	for (i=0; i<nLen; i++)
	{
		h1 = pbSrc[2*i];
		h2 = pbSrc[2*i+1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9) 
		s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9) 
		s2 -= 7;

		pbDest[i] = s1*16 + s2;
	}
}

QkPoolHandle CQtQkMangentCommon::QkPoolOpen()
{
	int nRet = 0;
	sqlite3 *dbHangle;

 	nRet = sqlite3_open("/etc/testDB.db", &dbHangle);
 	if(nRet){
		PRINT("CQtQkMangentCommon::QkPoolOpen, sqlite3_open error\n");
		return NULL;
 	}

 	cout << sqlite3_libversion() << endl;
 	 	
	PRINT("CQtQkMangentCommon::QkPoolOpen, sqlite3_open success\n");

	return (QkPoolHandle)dbHangle;
}

void CQtQkMangentCommon::QkPoolCreateTable(QkPoolHandle pDbQkPool)
{
	int nRet = 0;
	char szSql[MAX_SQL_LEN] = {0};
	char** pResult;
	char* cErrMsg;

	nRet = sqlite3_get_table(pDbQkPool, "select * from rawkey;", &pResult, 0, 0, &cErrMsg);
	if(SQLITE_OK != nRet){
		sprintf(szSql,"create table rawkey(key_id text primary key,key_state int,key blob, validity_time int,create_time text,modify_time text,peeridList text);");
		nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
		if (0 != nRet){
	        PRINT("CQtQkMangentCommon::GetSynKeyKeyByIdNode_common, sqlite3_exec error\n");
	        cout << cErrMsg << endl;
	        sqlite3_free_table(pResult);
			return;
	    }
	}
	sqlite3_free_table(pResult);
	memset(szSql, 0, MAX_SQL_LEN);
	
	nRet = sqlite3_get_table(pDbQkPool, "select * from synkey;", &pResult, 0, 0, &cErrMsg);
	if(SQLITE_OK != nRet){
		sprintf(szSql,"create table synkey(key_id text primary key,key_state int,key blob,user_id text,device_id text,validity_time int,create_time text,modify_time text,peeridList text);");
		nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
		if (0 != nRet){
	        PRINT("CQtQkMangentCommon::GetSynKeyKeyByIdNode_common, sqlite3_exec error\n");
	        cout << cErrMsg << endl;
	        sqlite3_free_table(pResult);
			return;
	    }
	}
	sqlite3_free_table(pResult);

	return;
}

void CQtQkMangentCommon::QkPoolClose(QkPoolHandle pQkPool)
{
	int nRet = 0;
	
	if(pQkPool == NULL)
	{
		PRINT("CQtQkMangentCommon::QkPoolClose, pQkPool is NULL!\n");
		return ;
	}
	
	nRet = sqlite3_close(pQkPool);
	if(nRet){
		PRINT("CQtQkMangentCommon::QkPoolOpen, sqlite3_open error\n");
		return;
 	}
 	
 	return;
}

int CQtQkMangentCommon::QkPoolGetNowTime(long *pTime)
{
	struct timeval struTV ;
	if(pTime == NULL)
	{
		return DBS_FAIL;
	}
	
	gettimeofday (&struTV , NULL);
	*pTime = struTV.tv_sec*1000 + struTV.tv_usec/1000;
	
	return DBS_SUC;
}

int CQtQkMangentCommon::QtMangentGetTime(QkPoolHandle pDbQkPool, void *time)
{
	int nRet = 0;
    int i = 0;
	long lNowTime;
	string strSql = "";
    char* cErrMsg;
	
	/* get current time */
	nRet = QkPoolGetNowTime(&lNowTime);
	if(nRet != DBS_SUC)
	{
		PRINT("CQtQkMangentCommon::QtMangentGetTime, QkPoolGetNowTime error\n");
		return DBS_FAIL;
	}
	strSql += "SELECT datetime(\'now\');";

	//nRet = sqlite3_exec(pDbQkPool, strSql.c_str(), UserResult_GetTime, time, NULL);
    while(i < RETRY_TIMES)
    {
		i++;
        nRet = sqlite3_exec(pDbQkPool, strSql.c_str(), UserResult_GetTime, time, NULL);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (DBS_SUC != nRet)
    {
        PRINT("CQtQkMangentCommon::QtMangentGetTime, sqlite3_exec error\n");
		return DBS_FAIL;
    }
    
    return 0;
}

int CQtQkMangentCommon::QtMangentGetRawKeyCount(QkPoolHandle pDbQkPool, int *pUsedCount, int *pUnusedCount, CQtUserId UserId, CQtDeviceId &DeviceId)
{
	int nRet = 0;
    int i = 0;
	char szSql[MAX_SQL_LEN] = {0};
    char* cErrMsg;

	if(pDbQkPool == NULL){
		PRINT("CQtQkMangentCommon::QtMangentGetCount, argument error\n");
		return DBS_FAIL;
	}

	sprintf(szSql,"select count(key_id) from rawkey where key_state=%d;",\
		KEY_UNUSED);

	//nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_Count, pUnusedCount, NULL);
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_Count, pUnusedCount, NULL);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::QtMangentGetCount, sqlite3_exec error\n");
		return DBS_FAIL;
    }

    memset(szSql, 0, MAX_SQL_LEN);
	sprintf(szSql,"select count(key_id) from rawkey where key_state=%d;",\
		KEY_USED);

	//nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_Count, pUsedCount, NULL);
    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_Count, pUsedCount, NULL);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::QtMangentGetCount, sqlite3_exec error\n");
		return DBS_FAIL;
    }	

	return DBS_SUC;
}

int CQtQkMangentCommon::InsertRawKey_common(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId, 
		ListKeyID &lKeyId, 
		ListKey &lKey,
		BYTE *mainKey)
{
	int nRet = 0;
    int i = 0;
	char strTime[64] = {0};
	char szSql[MAX_SQL_LEN] = {0};
	char szSql_tmp[MAX_SQL_LEN_TMP] = {0};
	char* cErrMsg;
	unsigned char input[ENCRYPT_KEY_LEN];
	unsigned char output[ENCRYPT_KEY_LEN];
	unsigned char output_str[2 * ENCRYPT_KEY_LEN + 1];
	int destLen;
	ListKeyID::iterator KeyIdItor;
	ListKey::iterator KeyItor;

	if(pDbQkPool == NULL)
	{
		PRINT("CQtQkMangentCommon::InsertRawKey_common, argument error\n");
		return DBS_FAIL;
	}

	nRet = QtMangentGetTime(pDbQkPool, &strTime);
	if(DBS_SUC != nRet)
	{
		PRINT("CQtQkMangentCommon::InsertRawKey_common, QtMangentGetTime error\n");
		return DBS_FAIL;
	}

	strcat(szSql, "BEGIN;insert into rawkey ( key_id, key_state, key, validity_time, create_time,modify_time, peeridList) values");

	for(KeyIdItor = lKeyId.begin(),KeyItor = lKey.begin(); \
				KeyIdItor != lKeyId.end(),KeyItor != lKey.end(); \
				)
	{
		memset(input, 0, ENCRYPT_KEY_LEN);
		memset(output, 0, ENCRYPT_KEY_LEN);
		memcpy(input, (KeyItor->GetBuffer()), ENCRYPT_KEY_LEN);
		if(0 != m_KeyEncrypt.EncryptOrDecrypt(output, destLen, input, ENCRYPT_KEY_LEN, SM4_ECB_ENC, mainKey))
		{
			PRINT("qkencrypt.EncryptOrDecrypt error\n");
			return DBS_FAIL;
		}
		
		memset(output_str, 0, 2 * ENCRYPT_KEY_LEN + 1);
		HexToStr(output_str, output, QT_MAX_KEY_LEN);
		
		memset(szSql_tmp, 0, MAX_SQL_LEN_TMP);
		sprintf(szSql_tmp, "('%.16s', %d, '%.32s', %d, '%s', '%s', '%s')",\
			KeyIdItor->GetBuffer(), KEY_UNUSED, \
			output_str, \
			DEFAULT_VAILDITY_TIME, strTime, \
			strTime, "");
		//cout << strlen(szSql_tmp) << endl;
		strcat(szSql,szSql_tmp);
		//cout << KeyId.GetBuffer() << endl;
		//cout << Key.GetBuffer() << endl;
		++KeyIdItor;
		++KeyItor;
		if(KeyIdItor != lKeyId.end() && KeyItor != lKey.end())
			strcat(szSql,",");
	}
	strcat(szSql, ";COMMIT;");
	

	//nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
        if(nRet && (strstr(cErrMsg, "database is locked") || strstr(cErrMsg, "cannot start a transaction within a transaction")))
        {
			cout << cErrMsg << endl;
                        sqlite3_exec(pDbQkPool, "ROLLBACK;", NULL, NULL, NULL);
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (0 != nRet)
    {
        PRINT("CQtQkMangentCommon::InsertRawKey_common, sqlite3_exec error\n");
        cout << cErrMsg << endl;
		return DBS_FAIL;
    }
    
	return DBS_SUC;
}

int CQtQkMangentCommon::GetRawKeyKeyByNode_common(
		QkPoolHandle pDbQkPool, 
		CQtKeyId &KeyId, 
		CQtKey &Key,
		BYTE *mainKey)
{
	int nRet = 0;
    int i = 0;
	char szSql[MAX_SQL_LEN] = {0};
	RawKeyTable *table = new RawKeyTable();
	char* cErrMsg;
	unsigned char input[ENCRYPT_KEY_LEN];
	unsigned char output[ENCRYPT_KEY_LEN+1];
	unsigned char output_str[ QT_MAX_KEY_LEN + 1];
	int destLen;
	
	sprintf(szSql,"select key_id, key_state, key, validity_time, create_time, modify_time, peeridList from rawkey where key_state=%d limit %d;",\
		KEY_UNUSED, 1);

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKey, table, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
			cout << cErrMsg << endl;
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	//nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKey, table, NULL);
	if (SQLITE_OK != nRet){
        PRINT("CQtQkMangentCommon::GetRawKeyKeyByNode_common, sqlite3_exec error\n");
        delete(table);
		return DBS_FAIL;
    }

	memset(output_str, 0, QT_MAX_KEY_LEN + 1);
	StrToHex(output_str, (BYTE *)table->key.c_str(), QT_MAX_KEY_LEN);

	memset(input, 0, ENCRYPT_KEY_LEN);
	memset(output, 0, ENCRYPT_KEY_LEN+1);
	memcpy(input, output_str, ENCRYPT_KEY_LEN);

	if(0 != m_KeyEncrypt.EncryptOrDecrypt(output, destLen, input, ENCRYPT_KEY_LEN, SM4_ECB_DEC, mainKey))
	{
		PRINT("qkencrypt.EncryptOrDecrypt error\n");
		return DBS_FAIL;
	}

	//cout << "key2:" << output << endl;
    KeyId.SetValue((BYTE *)table->key_id.c_str(), BYTE(QT_MAX_KEY_ID_LEN));
    Key.SetValue((BYTE *)output, BYTE(QT_MAX_KEY_LEN));
	delete(table);
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql,"update rawkey set key_state = %d where key_id = '%.16s';", \
		KEY_USED, KeyId.GetBuffer());

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
			cout << cErrMsg << endl;
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	//nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
	if (SQLITE_OK != nRet){
        PRINT("CQtQkMangentCommon::GetRawKeyKeyByNode_common, sqlite3_exec delete error\n");
        cout << cErrMsg << endl;
		return DBS_FAIL;
    }

	return DBS_SUC;
}

int CQtQkMangentCommon::GetRawKeyKeyById_common(
		QkPoolHandle pDbQkPool, 
		CQtKeyId &KeyId, 
		CQtKey &Key)
{
	int nRet = 0;
    int i = 0;
	char szSql[MAX_SQL_LEN] = {0};
	RawKeyTable *table = new RawKeyTable();
	char* cErrMsg;
	unsigned char input[ENCRYPT_KEY_LEN];
	unsigned char output[ENCRYPT_KEY_LEN+1];
	unsigned char output_str[ QT_MAX_KEY_LEN+1];
	int destLen;

	sprintf(szSql,"select key_id, key_state, key, validity_time, create_time, modify_time, peeridList from rawkey where key_id=%.16s  limit %d;",\
		KeyId.GetBuffer(), 1);

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKey, table, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
			cout << cErrMsg << endl;
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	//nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKey, table, &cErrMsg);
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::GetRawKeyKeyById_common, sqlite3_exec error\n");
    	cout << cErrMsg << endl;
		return DBS_FAIL;
    }

	memset(output_str, 0, QT_MAX_KEY_LEN + 1);
	StrToHex(output_str, (BYTE *)table->key.c_str(), QT_MAX_KEY_LEN);
    
    memset(input, 0, ENCRYPT_KEY_LEN);
	memset(output, 0, ENCRYPT_KEY_LEN);
	memcpy(input, output_str, ENCRYPT_KEY_LEN);
	if(0 != m_KeyEncrypt.EncryptOrDecrypt(output, destLen, input, ENCRYPT_KEY_LEN, SM4_ECB_DEC, g_key))
	{
		PRINT("qkencrypt.EncryptOrDecrypt error\n");
		return DBS_FAIL;
	}
    
	Key.SetValue((BYTE *)output, BYTE(QT_MAX_KEY_LEN));
	delete(table);
	return DBS_SUC;
}

int CQtQkMangentCommon::GetRawKeyKeyByIdNode_common(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtKeyId &KeyId, 
		CQtKey &Key)
{

	return DBS_SUC;
}

int CQtQkMangentCommon::DeleteRawKeyByNode_common(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId)
{
	int nRet = 0;
    int i = 0;
    char* cErrMsg;
	char szSql[MAX_SQL_LEN] = {0};

	sprintf(szSql,"delete from rawkey;", UserId.GetBuffer());

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
			cout << cErrMsg << endl;
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	//nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, NULL);
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::DeleteRawKeyByNode_common, sqlite3_exec error\n");
		return DBS_FAIL;
    }

	return DBS_SUC;
}

int CQtQkMangentCommon::DeleteRawKeyById_common(
		QkPoolHandle pDbQkPool, 
		ListKeyID &lKeyId)
{
	int nRet = 0;
    int i = 0;
	char szSql[MAX_SQL_LEN] = {0};
	char szSql_tmp[MAX_SQL_LEN_TMP] = {0};
	char* cErrMsg;
	ListKeyID::iterator KeyIdItor;

	strcat(szSql, "BEGIN;");
	for(KeyIdItor = lKeyId.begin(); KeyIdItor != lKeyId.end();++KeyIdItor)
	{
		memset(szSql_tmp, 0, MAX_SQL_LEN_TMP);
		sprintf(szSql_tmp,"delete from rawkey where key_id = '%.16s';", KeyIdItor->GetBuffer());
		strcat(szSql,szSql_tmp);
	}
	strcat(szSql, "COMMIT;");
    
	//nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
        if(nRet && (strstr(cErrMsg, "database is locked") || strstr(cErrMsg, "cannot start a transaction within a transaction")))
        {
			cout << cErrMsg << endl;
                        sqlite3_exec(pDbQkPool, "ROLLBACK;", NULL, NULL, NULL);
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::DeleteRawKeyById_common, sqlite3_exec error\n");
        cout << cErrMsg << endl;
		return DBS_FAIL;
    }

	return DBS_SUC;
}

int CQtQkMangentCommon::QtMangentGetSynKeyCount(QkPoolHandle pDbQkPool, int *pUsedCount, int *pUnusedCount, CQtUserId UserId, CQtDeviceId &DeviceId)
{
	int nRet = 0;
	char szSql[MAX_SQL_LEN] = {0};
    char* cErrMsg;
    int i = 0;
    
	if(pDbQkPool == NULL){
		PRINT("CQtQkMangentCommon::QtMangentGetCount, argument error\n");
		return DBS_FAIL;
	}

	sprintf(szSql,"select count(key_id) from synkey where user_id='%.32s' and device_id='%.32s' and key_state=%d;",\
		UserId.GetBuffer(), DeviceId.GetBuffer(), KEY_UNUSED);

	//nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_Count, pUnusedCount, NULL);
	i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_Count, pUnusedCount, NULL);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::QtMangentGetCount, sqlite3_exec error\n");
		return DBS_FAIL;
    }

    memset(szSql, 0, MAX_SQL_LEN);
	sprintf(szSql,"select count(key_id) from synkey where user_id='%.32s' and device_id='%.32s' and key_state=%d;",\
		UserId.GetBuffer(), DeviceId.GetBuffer(), KEY_USED);

	//nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_Count, pUsedCount, NULL);
    i = 0;
    while(i < RETRY_TIMES)
    {
	    i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_Count, pUsedCount, NULL);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::QtMangentGetCount, sqlite3_exec error\n");
		return DBS_FAIL;
    }	

	return DBS_SUC;
}

int CQtQkMangentCommon::InsertSynKey_common(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId,
		ListKeyID &lKeyId, 
		ListKeyID &lKey,
		BYTE *mainKey)
{
	int nRet = 0;
    int i = 0;
	char strTime[64] = {0};
	char szSql[MAX_SQL_LEN] = {0};
	char szSql_tmp[MAX_SQL_LEN_TMP] = {0};
	unsigned char input[ENCRYPT_KEY_LEN];
	unsigned char output[ENCRYPT_KEY_LEN];
	unsigned char output_str[2 * ENCRYPT_KEY_LEN + 1];
	int destLen;
	char* cErrMsg;
	ListKeyID::iterator KeyIdItor;
	ListKey::iterator KeyItor;

	nRet = QtMangentGetTime(pDbQkPool, &strTime);
	if(DBS_SUC != nRet)
	{
		PRINT("QkPoolRawKeyInsert QtMangentGetTime error\n");
		return DBS_FAIL;
	}

	strcat(szSql, "BEGIN;insert into synkey ( key_id, key_state, key, user_id, device_id, validity_time, create_time,modify_time, peeridList) values");
	for(KeyIdItor = lKeyId.begin(),KeyItor = lKey.begin(); \
				KeyIdItor != lKeyId.end(),KeyItor != lKey.end();)
	{
		memset(input, 0, ENCRYPT_KEY_LEN);
		memset(output, 0, ENCRYPT_KEY_LEN);
		memcpy(input, KeyItor->GetBuffer(), ENCRYPT_KEY_LEN);
		if(0 != m_KeyEncrypt.EncryptOrDecrypt(output, destLen, input, ENCRYPT_KEY_LEN, SM4_ECB_ENC, mainKey))
		{
			PRINT("qkencrypt.EncryptOrDecrypt error\n");
			return DBS_FAIL;
		}

		memset(output_str, 0, 2 * ENCRYPT_KEY_LEN + 1);
		HexToStr(output_str, output, QT_MAX_KEY_LEN);
		
		memset(szSql_tmp, 0, MAX_SQL_LEN_TMP);
		sprintf(szSql_tmp,"('%.16s', %d, '%.32s', '%.32s', '%.32s', %d, '%s', '%s', '%s')",\
			KeyIdItor->GetBuffer(), KEY_UNUSED, \
			output_str, UserId.GetBuffer(), DeviceId.GetBuffer(), \
			DEFAULT_VAILDITY_TIME, strTime, \
			strTime, "");
		strcat(szSql,szSql_tmp);

		++KeyIdItor;
		++KeyItor;
		
		if(KeyIdItor != lKeyId.end() && KeyItor != lKey.end())
			strcat(szSql,",");
	}
	strcat(szSql, ";COMMIT;");

	//nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
       	nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
        if(nRet && (strstr(cErrMsg, "database is locked") || strstr(cErrMsg, "cannot start a transaction within a transaction")))
        {
			cout << cErrMsg << endl;
                        sqlite3_exec(pDbQkPool, "ROLLBACK;", NULL, NULL, NULL);
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (0 != nRet)
    {
        PRINT("CQtQkMangentCommon::InsertSynKey_common, sqlite3_exec error\n");
        cout << cErrMsg << endl;
		return DBS_FAIL;
    }
    
	return DBS_SUC;
}

int CQtQkMangentCommon::GetSynKeyKeyByNode_common(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId,
		CQtKeyId &KeyId, 
		CQtKey &Key,
		BYTE *mainKey)
{
	int nRet = 0;
	char szSql[MAX_SQL_LEN] = {0};
	Table *table = new Table();
	char* cErrMsg;
	unsigned char input[ENCRYPT_KEY_LEN];
	unsigned char output[ENCRYPT_KEY_LEN+1];
	unsigned char output_str[ QT_MAX_KEY_LEN + 1];
	int destLen;
    int i = 0;

	sprintf(szSql,"select key_id, key_state, key, user_id, device_id, validity_time, create_time, modify_time, peeridList from synkey where user_id='%.32s' and device_id='%.32s' and key_state=%d limit %d;",\
		UserId.GetBuffer(), DeviceId.GetBuffer(), KEY_UNUSED, 1);

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKeyByNode, table, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
			cout << cErrMsg << endl;
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	//nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKeyByNode, table, NULL);
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::GetSynKeyKeyByNode_common, sqlite3_exec error\n");
        delete(table);
		return DBS_FAIL;
    }

	memset(output_str, 0, QT_MAX_KEY_LEN + 1);
	StrToHex(output_str, (BYTE *)table->key.c_str(), QT_MAX_KEY_LEN);

    memset(input, 0, ENCRYPT_KEY_LEN);
	memset(output, 0, ENCRYPT_KEY_LEN+1);
	memcpy(input, output_str, ENCRYPT_KEY_LEN);
	if(0 != m_KeyEncrypt.EncryptOrDecrypt(output, destLen, input, ENCRYPT_KEY_LEN, SM4_ECB_DEC, mainKey))
	{
		PRINT("qkencrypt.EncryptOrDecrypt error\n");
		return DBS_FAIL;
	}

    KeyId.SetValue((BYTE *)table->key_id.c_str(), BYTE(QT_MAX_KEY_ID_LEN));
    Key.SetValue((BYTE *)output, BYTE(QT_MAX_KEY_LEN));
	delete(table);
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql,"update synkey set key_state = %d where key_id = '%.16s' and device_id = '%.32s';", \
		KEY_USED, KeyId.GetBuffer(), DeviceId.GetBuffer());

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
			cout << cErrMsg << endl;
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	//nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
	if (SQLITE_OK != nRet){
        PRINT("CQtQkMangentCommon::GetRawKeyKeyByNode_common, sqlite3_exec delete error\n");
        cout << cErrMsg << endl;
		return DBS_FAIL;
    }
	return DBS_SUC;
}

int CQtQkMangentCommon::GetSynKeyKeyById_common(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId,
		CQtKeyId &KeyId, 
		CQtKey &Key)
{
	int nRet = 0;
	int i = 0;
	char szSql[MAX_SQL_LEN] = {0};
    char* cErrMsg;
	Table *table = new Table();
	unsigned char input[ENCRYPT_KEY_LEN];
	unsigned char output[ENCRYPT_KEY_LEN+1];
	unsigned char output_str[ QT_MAX_KEY_LEN+1];
	int destLen;

	sprintf(szSql,"select key_id, key_state, key, user_id, validity_time, create_time, modify_time, peeridList from synkey where key_state=%d and key_id=%.16s  limit %d;",\
		KEY_UNUSED, KeyId.GetBuffer(), 1);

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKeyByNode, table, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
			cout << cErrMsg << endl;
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	//nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKeyByNode, table, NULL);
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::GetSynKeyKeyById_common, sqlite3_exec error\n");
        delete(table);
		return DBS_FAIL;
    }

	memset(output_str, 0, QT_MAX_KEY_LEN + 1);
	StrToHex(output_str, (BYTE *)table->key.c_str(), QT_MAX_KEY_LEN);

    memset(input, 0, ENCRYPT_KEY_LEN);
	memset(output, 0, ENCRYPT_KEY_LEN+1);
	memcpy(input, output_str, ENCRYPT_KEY_LEN);
	if(0 != m_KeyEncrypt.EncryptOrDecrypt(output, destLen, input, ENCRYPT_KEY_LEN, SM4_ECB_DEC, g_key))
	{
		PRINT("qkencrypt.EncryptOrDecrypt error\n");
		return DBS_FAIL;
	}

	Key.SetValue((BYTE *)output, BYTE(QT_MAX_KEY_LEN));

	delete(table);
	return DBS_SUC;
}

int CQtQkMangentCommon::GetSynKeyKeyByIdNode_common(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId,
		CQtKeyId &KeyId, 
		CQtKey &Key,
		BYTE *mainKey)
{
	int nRet = 0;
	char szSql[MAX_SQL_LEN] = {0};
	Table *table = new Table();
	char* cErrMsg;
	unsigned char input[ENCRYPT_KEY_LEN];
	unsigned char output[ENCRYPT_KEY_LEN+1];
	unsigned char output_str[ QT_MAX_KEY_LEN + 1];
	int destLen;
    int i = 0;
	
	sprintf(szSql,"select key_id, key_state, key, user_id, device_id, validity_time, create_time, modify_time, peeridList from synkey where key_id=%.16s and user_id = '%.32s' and device_id = '%.32s' limit %d;",\
		KeyId.GetBuffer(), UserId.GetBuffer(), DeviceId.GetBuffer(), 1);

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKeyByNode, table, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
			cout << cErrMsg << endl;
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
  
	//nRet = sqlite3_exec(pDbQkPool, szSql, UserResult_GetRawKeyByNode, table, &cErrMsg);
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::GetSynKeyKeyByIdNode_common, sqlite3_exec error\n");
        cout << cErrMsg << endl;
        delete(table);
		return DBS_FAIL;
    }

	memset(output_str, 0, QT_MAX_KEY_LEN + 1);
	StrToHex(output_str, (BYTE *)table->key.c_str(), QT_MAX_KEY_LEN);

    memset(input, 0, ENCRYPT_KEY_LEN);
	memset(output, 0, ENCRYPT_KEY_LEN+1);
	memcpy(input, output_str, ENCRYPT_KEY_LEN);

	if(0 != m_KeyEncrypt.EncryptOrDecrypt(output, destLen, input, ENCRYPT_KEY_LEN, SM4_ECB_DEC, mainKey))
	{
		PRINT("qkencrypt.EncryptOrDecrypt error\n");
		return DBS_FAIL;
	}

	Key.SetValue((BYTE *)output, BYTE(QT_MAX_KEY_LEN));

	delete(table);
	return DBS_SUC;
}

int CQtQkMangentCommon::DeleteSynKeyByNode_common(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId,
		CQtDeviceId &DeviceId)
{
	int nRet = 0;
    int i = 0;
    char* cErrMsg;
	char szSql[MAX_SQL_LEN] = {0};

	sprintf(szSql,"delete from synkey where user_id = '%.32s' and device_id = '%.32s';", UserId.GetBuffer(), DeviceId.GetBuffer());

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
			cout << cErrMsg << endl;
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	//nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, NULL);
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::DeleteSynKeyByNode_common, sqlite3_exec error\n");
		return DBS_FAIL;
    }

	return DBS_SUC;
}

int CQtQkMangentCommon::DeleteSynKeyById_common(
		QkPoolHandle pDbQkPool, 
		ListKeyID &lKeyId)
{
	int nRet = 0;
    int i = 0;
	char szSql[MAX_SQL_LEN] = {0};
	char szSql_tmp[MAX_SQL_LEN_TMP] = {0};
	char* cErrMsg;
	ListKeyID::iterator KeyIdItor;

	strcat(szSql, "BEGIN;");
	for(KeyIdItor = lKeyId.begin(); KeyIdItor != lKeyId.end();++KeyIdItor)
	{
		memset(szSql_tmp, 0, MAX_SQL_LEN_TMP);
		sprintf(szSql_tmp,"delete from synkey where key_id = '%.16s';", KeyIdItor->GetBuffer());
		strcat(szSql,szSql_tmp);
	}
	strcat(szSql, "COMMIT;");

	//nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
        if(nRet && (strstr(cErrMsg, "database is locked") || strstr(cErrMsg, "cannot start a transaction within a transaction")))
        {
			cout << cErrMsg << endl;
                        sqlite3_exec(pDbQkPool, "ROLLBACK;", NULL, NULL, NULL);
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::DeleteSynKeyById_common, sqlite3_exec error\n");
        cout << cErrMsg << endl;
		return DBS_FAIL;
    }

	return DBS_SUC;
}

int CQtQkMangentCommon::UpdateSynPeerById_common(
		QkPoolHandle pDbQkPool, 
		string &lUserId,
		CQtKeyId &KeyId)
{
	int nRet = 0;
    int i = 0;
	char szSql[MAX_SQL_LEN] = {0};
	char* cErrMsg;

	sprintf(szSql,"update synkey set peeridList='%s' where key_id='%.16s';", lUserId.c_str(), KeyId.GetBuffer());

	//nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, NULL);
    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(pDbQkPool, szSql, NULL, 0, &cErrMsg);
        if(nRet && strstr(cErrMsg, "database is locked"))
        {
            usleep(WAIT_TIME);
            continue;
        }
        else
        {
            break;
        }
    }
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::UpdateSynPeerById_common, sqlite3_exec error\n");
		return DBS_FAIL;
    }

	return DBS_SUC;
}




