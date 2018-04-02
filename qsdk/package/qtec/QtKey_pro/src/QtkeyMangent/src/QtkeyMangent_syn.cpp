#include "QtkeyMangent_syn.h"

int CQtQkMangentSynKey::QkSynKeyInsert(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId,
		ListKeyID &KeyId, 
		ListKeyID &Key,
		BYTE *mainKey)
{
	int nRet = 0;
	
    nRet = m_QkMangentCommon.InsertSynKey_common(pDbQkPool, UserId, DeviceId, KeyId, Key, mainKey);
    if (0 != nRet)
    {
        PRINT("CQtQkMangentSynKey::QkSynKeyInsert, InsertSynKey_common error\n");
		return DBS_FAIL;
    }
    
	return DBS_SUC;
}

int CQtQkMangentSynKey::GetSynKeyCount(
		QkPoolHandle pDbQkPool, 
		int *pUsedCount,
		int *pUnusedCount, 
		CQtUserId UserId,
		CQtDeviceId &DeviceId)
{
	int nRet = 0;
	
    nRet = m_QkMangentCommon.QtMangentGetSynKeyCount(pDbQkPool, pUsedCount, pUnusedCount, UserId, DeviceId);
    if (0 != nRet)
    {
        PRINT("CQtQkMangentSynKey::GetSynKeyCount, QtMangentGetCount error\n");
		return DBS_FAIL;
    }
    
	return DBS_SUC;
}

int CQtQkMangentSynKey::GetSynKeyKeyByNode(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId,
		CQtKeyId &KeyId, 
		CQtKey &Key,
		BYTE *mainKey)
{
	int nRet = 0;
	
	nRet = m_QkMangentCommon.GetSynKeyKeyByNode_common(pDbQkPool, UserId, DeviceId, KeyId, Key, mainKey);
	if (0 != nRet){
        PRINT("CQtQkMangentSynKey::GetSynKeyKeyByNode, GetSynKeyKeyByNode_common error\n");
		return DBS_FAIL;
    }

	return DBS_SUC;
}

int CQtQkMangentSynKey::GetSynKeyKeyById(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId,
		CQtKeyId &KeyId, 
		CQtKey &Key)
{
	int nRet = 0;

	nRet = m_QkMangentCommon.GetSynKeyKeyById_common(pDbQkPool, UserId, DeviceId, KeyId, Key);
	if (0 != nRet){
        PRINT("CQtQkMangentSynKey::GetSynKeyKeyById, GetSynKeyKeyById_common error\n");
		return DBS_FAIL;
    }
	
	return DBS_SUC;
}

int CQtQkMangentSynKey::GetSynKeyKeyByIdNode(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId, 
		CQtDeviceId &DeviceId,
		CQtKeyId &KeyId, 
		CQtKey &Key,
		BYTE *mainKey)
{
	int nRet = 0;
	
	nRet = m_QkMangentCommon.GetSynKeyKeyByIdNode_common(pDbQkPool, UserId, DeviceId, KeyId, Key, mainKey);
	if (0 != nRet){
        PRINT("CQtQkMangentSynKey::GetSynKeyKeyByIdNode, GetSynKeyKeyByIdNode_common error\n");
		return DBS_FAIL;
    }
	
	return DBS_SUC;
}

int CQtQkMangentSynKey::DeleteSynKeyByNode(
		QkPoolHandle pDbQkPool, 
		CQtUserId UserId,
		CQtDeviceId &DeviceId)
{
	int nRet = 0;

	nRet = m_QkMangentCommon.DeleteSynKeyByNode_common(pDbQkPool, UserId, DeviceId);
	if (0 != nRet){
        PRINT("CQtQkMangentSynKey::DeleteSynKeyByNode, DeleteSynKeyByNode_common error\n");
		return DBS_FAIL;
    }
	
	return DBS_SUC;
}

int CQtQkMangentSynKey::DeleteSynKeyById(
		QkPoolHandle pDbQkPool, 
		ListKeyID &KeyId)
{
	int nRet = 0;
	
	nRet = m_QkMangentCommon.DeleteSynKeyById_common(pDbQkPool, KeyId);
	if (0 != nRet){
        PRINT("CQtQkMangentSynKey::DeleteSynKeyById, DeleteSynKeyById_common error\n");
		return DBS_FAIL;
    }
	
	return DBS_SUC;
}

int CQtQkMangentSynKey::UpdateSynPeerById(
		QkPoolHandle pDbQkPool, 
		string &lUserId,
		CQtKeyId &KeyId)
{
	int nRet = 0;
	
	nRet = m_QkMangentCommon.UpdateSynPeerById_common(pDbQkPool, lUserId, KeyId);
	if (0 != nRet){
        PRINT("CQtQkMangentSynKey::UpdateSynPeerById, DeleteSynKeyById_common error\n");
		return DBS_FAIL;
    }
	
	return DBS_SUC;
}


