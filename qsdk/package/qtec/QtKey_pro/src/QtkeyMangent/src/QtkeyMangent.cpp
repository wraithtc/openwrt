#include "QtkeyMangent.h"
#include "QtkeyMangent_common.h"
#include "QtKeyEncrypt.h"

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

typedef struct
{
    RawKeyTable table[300];
    int index;
}RawKeyTableList;

static int UserResult_GetRawKeyList(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i = 0;
 	for(i = 0 ; i < argc ; i++)  
    {  
        cout<<azColName[i]<<" = "<<(argv[i] ? argv[i] : "NULL")<<", " << endl;  
    }
   	    
	RawKeyTableList *tablelist = (RawKeyTableList *)NotUsed;

	int index = tablelist->index;

    tablelist->table[index].key_id = (argv[0] ? argv[0] : "NULL");
    tablelist->table[index].key_state = atoi((argv[1] ? argv[1] : "NULL"));
    tablelist->table[index].key = (argv[2] ? argv[2] : "NULL");
    tablelist->table[index].validity_time = atoi((argv[3] ? argv[3] : "NULL"));
    tablelist->table[index].create_time = (argv[4] ? argv[4] : "NULL");
    tablelist->table[index].modify_time = (argv[5] ? argv[5] : "NULL");
    tablelist->table[index].peeridList = (argv[6] ? argv[6] : "NULL");
	tablelist->index++;
    return 0;
}


static void StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
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
int CQtQkPoolBase::Initialize()
{
	m_pQkPoolHandle = m_QkMangentCommon.QkPoolOpen();
	if(m_pQkPoolHandle == NULL)
	{
		PRINT("Init QkPool failed!\n");
		return QKPOOL_FAIL;
	}

	//m_QkMangentCommon.QkPoolCreateTable(m_pQkPoolHandle);

	PRINT("Init QkPool success!\n");
	
	return QKPOOL_SUC;
}

void CQtQkPoolBase::UnInitialize()
{
	m_QkMangentCommon.QkPoolClose((QkPoolHandle)m_pQkPoolHandle);
}

CQtQkMangent::CQtQkMangent()
{
	Initialize();
}

CQtQkMangent::~CQtQkMangent()
{
	UnInitialize();
}

int CQtQkMangent::AddKey(
	BYTE nPoolType, 
	CQtUserId UserId, 
	CQtDeviceId &DeviceId, 
	ListKeyID &lKeyId, 
	ListKey &lKey,
	BYTE *mainKey)
{
	int nRet = 0;
	ListKeyID::iterator KeyIdItor;
	ListKey::iterator KeyItor;
	ListKeyID lKeyId_tmp;
	ListKey lKey_tmp;
	int count = 0;
	
	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::AddKey, m_pQkPoolHandle is NULL!\n");
		return QKPOOL_FAIL;
	}

	if(POOL_TYPE_RAW == nPoolType)
	{
		lKeyId_tmp.clear();
		lKey_tmp.clear();
		for(KeyIdItor = lKeyId.begin(),KeyItor = lKey.begin(); \
					KeyIdItor != lKeyId.end(),KeyItor != lKey.end();)
		{
			lKeyId_tmp.push_back(*KeyIdItor);
			lKey_tmp.push_back(*KeyItor);
			count++;
			++KeyIdItor;
			++KeyItor;
			
			if(count >= 10 || KeyIdItor == lKeyId.end() || KeyItor == lKey.end()){
				nRet = m_QkMangentRawKey.QkRawKeyInsert(m_pQkPoolHandle, UserId, DeviceId, lKeyId_tmp, lKey_tmp, mainKey);
				if(QKPOOL_SUC != nRet){
					PRINT("CQtQkMangent::AddKey, insert rawkey error!\n");
					return QKPOOL_FAIL;
				}
				count = 0;
				lKeyId_tmp.clear();
				lKey_tmp.clear();
			}
		}
	}
	else if(POOL_TYPE_SYNC == nPoolType)
	{
		lKeyId_tmp.clear();
		lKey_tmp.clear();
		for(KeyIdItor = lKeyId.begin(),KeyItor = lKey.begin(); \
					KeyIdItor != lKeyId.end(),KeyItor != lKey.end();)
		{
			lKeyId_tmp.push_back(*KeyIdItor);
			lKey_tmp.push_back(*KeyItor);
			count++;
			++KeyIdItor;
			++KeyItor;
			
			if(count >= 10 || KeyIdItor == lKeyId.end() || KeyItor == lKey.end()){
				nRet = m_QkMangentSynKey.QkSynKeyInsert(m_pQkPoolHandle, UserId, DeviceId, lKeyId_tmp, lKey_tmp, mainKey);
				if(QKPOOL_SUC != nRet){
					PRINT("CQtQkMangent::AddKey, insert synkey error!\n");
					return QKPOOL_FAIL;
				}
				count = 0;
				lKeyId_tmp.clear();
				lKey_tmp.clear();			
			}
		}
	}
	else
	{
		PRINT("CQtQkMangent::AddKey, nPoolType error!\n");
		return QKPOOL_FAIL;
	}

	return QKPOOL_SUC;
}

int CQtQkMangent::GetCount(
	BYTE nPoolType, 
	int *pUsedCount,
	int *pUnusedCount, 
	CQtUserId UserId,
	CQtDeviceId &DeviceId)
{
	int nRet = 0;

	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::GetCount, m_pQkPoolHandle is NULL!\n");
		return QKPOOL_FAIL;
	}
	
	if(POOL_TYPE_RAW == nPoolType)
	{
		nRet = m_QkMangentRawKey.GetRawKeyCount(m_pQkPoolHandle, pUsedCount, pUnusedCount, UserId, DeviceId);
		if(QKPOOL_SUC != nRet)
		{
			PRINT("CQtQkMangent::GetCount, get rawkey count error!\n");
			return QKPOOL_FAIL;
		}
	}
	else if(POOL_TYPE_SYNC == nPoolType)
	{
		nRet = m_QkMangentSynKey.GetSynKeyCount(m_pQkPoolHandle, pUsedCount, pUnusedCount, UserId, DeviceId);
		if(QKPOOL_SUC != nRet)
		{
			PRINT("CQtQkMangent::GetCount, get synkey count error!\n");
			return QKPOOL_FAIL;
		}
	}
	else
	{
		PRINT("CQtQkMangent::GetCount, nPoolType error!\n");
		return QKPOOL_FAIL;
	}

	return QKPOOL_SUC;
}

int CQtQkMangent::GetKeyByNode(
	BYTE nPoolType, 
	BYTE nNumber, 
	CQtUserId UserId, 
	CQtDeviceId &DeviceId,
	ListKeyID &lKeyId, 
	ListKey &lKey,
	BYTE *mainKey)
{
	int nRet = 0;
	CQtKeyId KeyId_tmp;
	CQtKey Key_tmp;

	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::GetKeyByNode, m_pQkPoolHandle is NULL!\n");
		return QKPOOL_FAIL;
	}

	if(POOL_TYPE_RAW == nPoolType)
	{
		for(int i = 0; i < nNumber; i++)
		{
			KeyId_tmp.Clear();
			Key_tmp.Clear();
			nRet = m_QkMangentRawKey.GetRawKeyKeyByNode(m_pQkPoolHandle, KeyId_tmp, Key_tmp, mainKey);
			if(QKPOOL_SUC != nRet)
			{
				PRINT("CQtQkMangent::GetKeyByNode, get synkey error!\n");
				return QKPOOL_FAIL;
			}
			lKeyId.push_back(KeyId_tmp);
			lKey.push_back(Key_tmp);
		}
	}
	else if(POOL_TYPE_SYNC == nPoolType)
	{
		for(int i = 0; i < nNumber; i++)
		{
			KeyId_tmp.Clear();
			Key_tmp.Clear();
			nRet = m_QkMangentSynKey.GetSynKeyKeyByNode(m_pQkPoolHandle, UserId, DeviceId, KeyId_tmp, Key_tmp, mainKey);
			if(QKPOOL_SUC != nRet)
			{
				PRINT("CQtQkMangent::GetKeyByNode, get synkey error!\n");
				return QKPOOL_FAIL;
			}
			lKeyId.push_back(KeyId_tmp);
			lKey.push_back(Key_tmp);
		}
	}
	else
	{
		PRINT("CQtQkMangent::GetKeyByNode, nPoolType error!\n");
		return QKPOOL_FAIL;
	}

	return QKPOOL_SUC;

}

int CQtQkMangent::GetKeyById(
	BYTE nPoolType, 
	BYTE nNumber, 
	CQtUserId UserId, 
	CQtDeviceId &DeviceId,
	ListKeyID &lKeyId, 
	ListKey &lKey)
{
	int nRet = 0;
	int i;
	CQtKey Key_tmp;
	ListKeyID::iterator KeyIdItor;

	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::GetKeyById, m_pQkPoolHandle is NULL!\n");
		return QKPOOL_FAIL;
	}
	
	if(POOL_TYPE_RAW == nPoolType)
	{
		for(i = 0, KeyIdItor = lKeyId.begin(); \
			i < nNumber, KeyIdItor != lKeyId.end(); \
			++i, ++KeyIdItor)
		{
			Key_tmp.Clear();
			nRet = m_QkMangentRawKey.GetRawKeyKeyById(m_pQkPoolHandle, *KeyIdItor, Key_tmp);
			if(QKPOOL_SUC != nRet)
			{
				PRINT("CQtQkMangent::GetKeyById, get rawkey error!\n");
				return QKPOOL_FAIL;
			}
			lKey.push_back(Key_tmp);
		}
	}
	else if(POOL_TYPE_SYNC == nPoolType)
	{
		for(i = 0, KeyIdItor = lKeyId.begin(); \
			i < nNumber, KeyIdItor != lKeyId.end(); \
			i++, ++KeyIdItor)
		{
			Key_tmp.Clear();
			nRet = m_QkMangentSynKey.GetSynKeyKeyById(m_pQkPoolHandle, UserId, DeviceId, *KeyIdItor, Key_tmp);
			if(QKPOOL_SUC != nRet)
			{
				PRINT("CQtQkMangent::GetKeyById, get synkey error!\n");
				return QKPOOL_FAIL;
			}
			lKey.push_back(Key_tmp);
		}
	}
	else
	{
		PRINT("CQtQkMangent::GetKeyById, nPoolType error!\n");
		return QKPOOL_FAIL;
	}

	return QKPOOL_SUC;
}

int CQtQkMangent::GetKeyByIdNode(
	BYTE nPoolType, 
	BYTE nNumber, 
	CQtUserId UserId,
	CQtDeviceId &DeviceId,
	ListKeyID &lKeyId, 
	ListKey &lKey,
	BYTE *mainKey)
{
	int nRet = 0;
	int i;
	CQtKey Key_tmp;
	ListKeyID::iterator KeyIdItor;

	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::GetKeyByIdNode, m_pQkPoolHandle is NULL!\n");
		return QKPOOL_FAIL;
	}
	
	if(POOL_TYPE_RAW == nPoolType)
	{

	}
	else if(POOL_TYPE_SYNC == nPoolType)
	{
		for(i = 0, KeyIdItor = lKeyId.begin(); \
			i < nNumber, KeyIdItor != lKeyId.end(); \
			i++, ++KeyIdItor)
		{
			nRet = m_QkMangentSynKey.GetSynKeyKeyByIdNode(m_pQkPoolHandle, UserId, DeviceId, *KeyIdItor, Key_tmp, mainKey);
			if(QKPOOL_SUC != nRet)
			{
				PRINT("CQtQkMangent::GetKeyByIdNode, get synkey error!\n");
				return QKPOOL_FAIL;
			}
			lKey.push_back(Key_tmp);
		}
	}
	else
	{
		PRINT("CQtQkMangent::GetKeyByIdNode, nPoolType error!\n");
		return QKPOOL_FAIL;
	}

	return QKPOOL_SUC;
}

int CQtQkMangent::DeleteKeyByNode(
	BYTE nPoolType, 
	CQtUserId UserId,
	CQtDeviceId &DeviceId)
{
	int nRet = 0;

	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::DeleteKeyByNode, m_pQkPoolHandle is NULL!\n");
		return QKPOOL_FAIL;
	}
	
	if(POOL_TYPE_RAW == nPoolType)
	{
		nRet = m_QkMangentRawKey.DeleteRawKeyByNode(m_pQkPoolHandle, UserId);
		if(QKPOOL_SUC != nRet)
		{
			PRINT("CQtQkMangent::DeleteKeyByNode, delete rawkey error!\n");
			return QKPOOL_FAIL;
		}
	}
	else if(POOL_TYPE_SYNC == nPoolType)
	{
		nRet = m_QkMangentSynKey.DeleteSynKeyByNode(m_pQkPoolHandle, UserId, DeviceId);
		if(QKPOOL_SUC != nRet)
		{
			PRINT("CQtQkMangent::DeleteKeyByNode, delete synkey error!\n");
			return QKPOOL_FAIL;
		}
	}
	else
	{
		PRINT("CQtQkMangent::DeleteKeyByNode, nPoolType error!\n");
		return QKPOOL_FAIL;
	}

	return QKPOOL_SUC;
}

static int UserResult_Count(void *NotUsed, int argc, char **argv, char **azColName)
{
	*(int *)NotUsed = atoi(argv[0]);
	
    return 0;
}

int CQtQkMangent::DeleteSynKeyByNodeId(
	BYTE *keyid,
	CQtUserId UserId,
	CQtDeviceId &DeviceId)
{
	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::DeleteSynKeyByNodeId, m_pQkPoolHandle is NULL!\n");
		return QKPOOL_FAIL;
	}

    int nRet = 0;
	int i = 0;
    int pUnusedCount = 0;
    char* cErrMsg;
	char szSql[MAX_SQL_LEN] = {0};

    sprintf(szSql,"select count(key_id) from synkey where user_id='%.32s' and device_id='%.32s' and key_id < '%.16s';", UserId.GetBuffer(), DeviceId.GetBuffer(), keyid);

	i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(m_pQkPoolHandle, szSql, UserResult_Count, &pUnusedCount, &cErrMsg);
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
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::DeleteSynKeyByNode_common, sqlite3_exec error\n");
		cout << cErrMsg << endl;
		return DBS_FAIL;
    }    

    if(pUnusedCount <= 20)
    {
        return DBS_SUC;
    }
    else
    {
        pUnusedCount = pUnusedCount - 20;
    }
    
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql,"delete from synkey where key_id in (select key_id from synkey where user_id = '%.32s' and device_id = '%.32s' limit %d);", UserId.GetBuffer(), DeviceId.GetBuffer(), pUnusedCount);

	i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(m_pQkPoolHandle, szSql, NULL, 0, &cErrMsg);
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
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::DeleteSynKeyByNode_common, sqlite3_exec error\n");
		cout << cErrMsg << endl;
		return DBS_FAIL;
    }

	return DBS_SUC;
}

int CQtQkMangent::DeleteKeyById(
	BYTE nPoolType, 
	ListKeyID &lKeyId)
{
	int nRet = 0;
	ListKeyID::iterator KeyIdItor;
	ListKeyID lKeyId_tmp;
	int count = 0;

	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::DeleteKeyById, m_pQkPoolHandle is NULL!\n");
		return QKPOOL_FAIL;
	}

	if(POOL_TYPE_RAW == nPoolType)
	{
		lKeyId_tmp.clear();
 		for(KeyIdItor = lKeyId.begin(); KeyIdItor != lKeyId.end();)
 		{
			lKeyId_tmp.push_back(*KeyIdItor);
			count++;
			++KeyIdItor;
			if(count >= 10 || KeyIdItor == lKeyId.end()){
				nRet = m_QkMangentRawKey.DeleteRawKeyById(m_pQkPoolHandle, lKeyId_tmp);
				if(QKPOOL_SUC != nRet)
				{
					PRINT("CQtQkMangent::DeleteKeyById, delete rawkey error!\n");
					return QKPOOL_FAIL;
				}
				count = 0;
				lKeyId_tmp.clear();
			}
		}
	}
	else if(POOL_TYPE_SYNC == nPoolType)
	{
		lKeyId_tmp.clear();	
		for(KeyIdItor = lKeyId.begin(); KeyIdItor != lKeyId.end();)
 		{
			lKeyId_tmp.push_back(*KeyIdItor);
			count++;
			++KeyIdItor;
			if(count >= 10 || KeyIdItor == lKeyId.end()){
				nRet = m_QkMangentSynKey.DeleteSynKeyById(m_pQkPoolHandle, lKeyId_tmp);
				if(QKPOOL_SUC != nRet)
				{
					PRINT("CQtQkMangent::DeleteKeyById, delete synkey error!\n");
					return QKPOOL_FAIL;
				}
				count = 0;
				lKeyId_tmp.clear();
			}
		} 
	}
	else
	{
		PRINT("CQtQkMangent::DeleteKeyById, nPoolType error!\n");
		return QKPOOL_FAIL;
	}

	return QKPOOL_SUC;
}

int CQtQkMangent::UpdatePeerById(
	BYTE nPoolType, 
	string &lUserId,
	ListKeyID &lKeyId)
{
	int nRet = 0;
	ListKeyID::iterator KeyIdItor;

	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::UpdatePeerById, m_pQkPoolHandle is NULL!\n");
		return QKPOOL_FAIL;
	}

	if(POOL_TYPE_RAW == nPoolType)
	{
 		
	}
	else if(POOL_TYPE_SYNC == nPoolType)
	{
		for(KeyIdItor = lKeyId.begin(); KeyIdItor != lKeyId.end(); ++KeyIdItor)
 		{
			nRet = m_QkMangentSynKey.UpdateSynPeerById(m_pQkPoolHandle, lUserId, *KeyIdItor);
			if(QKPOOL_SUC != nRet)
			{
				PRINT("CQtQkMangent::UpdatePeerById, delete synkey error!\n");
				return QKPOOL_FAIL;
			}
		}
	}
	else
	{
		PRINT("CQtQkMangent::UpdatePeerById, nPoolType error!\n");
		return QKPOOL_FAIL;
	}

	return QKPOOL_SUC;
}

int CQtQkMangent::GetRawKeyByNode(
	BYTE nPoolType, 
	BYTE nNumber, 
	CQtUserId UserId, 
	CQtDeviceId &DeviceId,
	ListKeyID &lKeyId, 
	ListKey &lKey,
	BYTE *mainKey)
{
    int nRet = 0;
	CQtKeyId KeyId_tmp;
	CQtKey Key_tmp;
    int i = 0;
    struct timeval tv;
	char szSql[MAX_SQL_LEN] = {0};
	RawKeyTableList *rawtablelist = new RawKeyTableList();
	char* cErrMsg;
	unsigned char input[ENCRYPT_KEY_LEN];
	unsigned char output[ENCRYPT_KEY_LEN+1];
	unsigned char output_str[ QT_MAX_KEY_LEN + 1];
    unsigned char g_key[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	int destLen;

	rawtablelist->index = 0;

	if(m_pQkPoolHandle == NULL)
	{
		PRINT("CQtQkMangent::GetKeyByNode, m_pQkPoolHandle is NULL!\n");
		delete(rawtablelist);
		return QKPOOL_FAIL;
	}

	sprintf(szSql,"select key_id, key_state, key, validity_time, create_time, modify_time, peeridList from rawkey where key_state=%d limit %d;",\
		KEY_UNUSED, nNumber);

	cout << szSql << endl;

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
        nRet = sqlite3_exec(m_pQkPoolHandle, szSql, UserResult_GetRawKeyList, rawtablelist, &cErrMsg);
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
		delete(rawtablelist);
		return DBS_FAIL;
    }

    for(i = 0; i < nNumber; i++)
    {
    	memset(output_str, 0, QT_MAX_KEY_LEN + 1);
    	StrToHex(output_str, (BYTE *)rawtablelist->table[i].key.c_str(), QT_MAX_KEY_LEN);

    	memset(input, 0, ENCRYPT_KEY_LEN);
    	memset(output, 0, ENCRYPT_KEY_LEN+1);
    	memcpy(input, output_str, ENCRYPT_KEY_LEN);

    	if(0 != m_KeyEncrypt.EncryptOrDecrypt(output, destLen, input, ENCRYPT_KEY_LEN, SM4_ECB_DEC, mainKey))
    	{
    		PRINT("qkencrypt.EncryptOrDecrypt error\n");
			delete(rawtablelist);
    		return DBS_FAIL;
    	}
        KeyId_tmp.Clear();
		Key_tmp.Clear();
        KeyId_tmp.SetValue((BYTE *)rawtablelist->table[i].key_id.c_str(), BYTE(QT_MAX_KEY_ID_LEN));
        Key_tmp.SetValue((BYTE *)output, BYTE(QT_MAX_KEY_LEN));
        lKeyId.push_back(KeyId_tmp);
		lKey.push_back(Key_tmp);
    }

    memset(szSql, 0, sizeof(szSql));
	sprintf(szSql,"update rawkey set key_state = %d where key_id <= '%.16s';", \
		KEY_USED, KeyId_tmp.GetBuffer());

    i = 0;
    while(i < RETRY_TIMES)
    {
		i++;
		cout << szSql << endl;
        nRet = sqlite3_exec(m_pQkPoolHandle, szSql, NULL, 0, &cErrMsg);
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
	if (0 != nRet){
        PRINT("CQtQkMangentCommon::GetRawKeyKeyByNode_common, sqlite3_exec delete error\n");
        cout << cErrMsg << endl;
		delete(rawtablelist);
		return DBS_FAIL;
    }
	
	delete(rawtablelist);
	return DBS_SUC;
}
