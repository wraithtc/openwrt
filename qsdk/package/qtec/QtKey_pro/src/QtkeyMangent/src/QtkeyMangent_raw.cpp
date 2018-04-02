#include "QtkeyMangent_raw.h"

int CQtQkMangentRawKey::QkRawKeyInsert(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId, 
		ListKeyID &KeyId, 
		ListKey &Key,
		BYTE *mainKey)
{
	int nRet = 0;
	
    nRet = m_QkMangentCommon.InsertRawKey_common(pDbQkPool, UserId, DeviceId, KeyId, Key, mainKey);
    if (0 != nRet)
    {
        PRINT("CQtQkMangentRawKey::QkRawKeyInsert, InsertRawKey_common error\n");
		return DBS_FAIL;
    }
    
	return DBS_SUC;
}

int CQtQkMangentRawKey::GetRawKeyCount(
		QkPoolHandle pDbQkPool, 
		int *pUsedCount,
		int *pUnusedCount, 
		CQtUserId UserId,
		CQtDeviceId &DeviceId)
{
	int nRet = 0;
	
    nRet = m_QkMangentCommon.QtMangentGetRawKeyCount(pDbQkPool, pUsedCount, pUnusedCount, UserId, DeviceId);
    if (0 != nRet)
    {
        PRINT("CQtQkMangentRawKey::GetRawKeyCount, QtMangentGetCount error\n");
		return DBS_FAIL;
    }
    
	return DBS_SUC;
}

int CQtQkMangentRawKey::GetRawKeyKeyByNode(
		QkPoolHandle pDbQkPool, 
		CQtKeyId &KeyId, 
		CQtKey &Key,
		BYTE *mainKey)
{
	int nRet = 0;
	
	nRet = m_QkMangentCommon.GetRawKeyKeyByNode_common(pDbQkPool, KeyId, Key, mainKey);
	if (0 != nRet){
        PRINT("CQtQkMangentRawKey::GetRawKeyKeyByNode, GetRawKeyKeyByNode_common error\n");
		return DBS_FAIL;
    }

	return DBS_SUC;
}

int CQtQkMangentRawKey::GetRawKeyKeyById(
		QkPoolHandle pDbQkPool, 
		CQtKeyId &KeyId, 
		CQtKey &Key)
{
	int nRet = 0;
	
	nRet = m_QkMangentCommon.GetRawKeyKeyById_common(pDbQkPool, KeyId, Key);
	if (0 != nRet){
        PRINT("CQtQkMangentRawKey::GetRawKeyKeyById, GetRawKeyKeyById_common error\n");
		return DBS_FAIL;
    }
	
	return DBS_SUC;
}


int CQtQkMangentRawKey::GetRawKeyKeyByIdNode(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtKeyId &KeyId, 
		CQtKey &Key)
{
	int nRet = 0;
	
	nRet = m_QkMangentCommon.GetRawKeyKeyByIdNode_common(pDbQkPool, UserId, KeyId, Key);
	if (0 != nRet){
        PRINT("CQtQkMangentRawKey::GetRawKeyKeyByIdNode, GetRawKeyKeyByIdNode_common error\n");
		return DBS_FAIL;
    }
	
	return DBS_SUC;
}

int CQtQkMangentRawKey::DeleteRawKeyByNode(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId)
{
	int nRet = 0;
	
	nRet = m_QkMangentCommon.DeleteRawKeyByNode_common(pDbQkPool, UserId);
	if (0 != nRet){
        PRINT("CQtQkMangentRawKey::DeleteRawKeyByNode, DeleteRawKeyByNode_common error\n");
		return DBS_FAIL;
    }
	
	return DBS_SUC;
}

int CQtQkMangentRawKey::DeleteRawKeyById(
		QkPoolHandle pDbQkPool, 
		ListKeyID &KeyId)
{
	int nRet = 0;
	
	nRet = m_QkMangentCommon.DeleteRawKeyById_common(pDbQkPool, KeyId);
	if (0 != nRet){
        PRINT("CQtQkMangentRawKey::DeleteRawKeyById, DeleteRawKeyById_common error\n");
		return DBS_FAIL;
    }
	
	return DBS_SUC;
}


