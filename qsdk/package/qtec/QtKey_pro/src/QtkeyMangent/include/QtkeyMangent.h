/********************************************************
*	Filename:	QtkeyMangent.h	                    	*
*	Author	:	lirui(lir@qtec.cn)                    	*
*                                                      	*
* 	History                                         	*
*		2017/06/19	Create								*
********************************************************/

#ifndef __QKKEYMANGENT_H__
#define __QKKEYMANGENT_H__

#include "QtManagent_defines.h"
#include "QtkeyMangent_raw.h"
#include "QtkeyMangent_syn.h"
#include "QtKeyEncrypt.h"

class CQtQkPoolBase
{
public:
	CQtQkPoolBase()
	{
		m_pQkPoolHandle = 0;
	}
	~CQtQkPoolBase(){}
	
public:
	int Initialize();
	void UnInitialize();
	
protected:
	sqlite3 *m_pQkPoolHandle;
	CQtQkMangentCommon m_QkMangentCommon;
};

class CQtQkMangent : public CQtQkPoolBase
{
public:
	CQtQkMangent();
	~CQtQkMangent();

public:
	/* 添加密钥 */
	virtual int AddKey(BYTE nPoolType, CQtUserId UserId, CQtDeviceId &DeviceId, ListKeyID &lKeyId, ListKey &lKey, BYTE *mainKey);

	/* 获取当前表中某节点密钥的数量(使用/未使用) */
	virtual int GetCount(BYTE nPoolType, int *pUsedCount, int *pUnusedCount, CQtUserId UserId, CQtDeviceId &DeviceId);

	/* 根据user_id获取指定数量密钥 */
	virtual int GetKeyByNode(BYTE nPoolType, BYTE nNumber, CQtUserId UserId, CQtDeviceId &DeviceId, ListKeyID &lKeyId, ListKey &lKey, BYTE *mainKey);

    /* 根据user_id获取指定数量原始密钥 */
	virtual int GetRawKeyByNode(BYTE nPoolType, BYTE nNumber, CQtUserId UserId, CQtDeviceId &DeviceId, ListKeyID &lKeyId, ListKey &lKey, BYTE *mainKey);

	/* 根据key_id获取指定数量密钥 */
	virtual int GetKeyById(BYTE nPoolType, BYTE nNumber, CQtUserId UserId, CQtDeviceId &DeviceId, ListKeyID &lKeyId, ListKey &lKey);

	/* 根据key_id、user_id获取指定数量密钥 */
	virtual int GetKeyByIdNode(BYTE nPoolType, BYTE nNumber, CQtUserId UserId, CQtDeviceId &DeviceId, ListKeyID &lKeyId, ListKey &lKey, BYTE *mainKey);

	/* 根据user_id删除密钥 */
	virtual int DeleteKeyByNode(BYTE nPoolType, CQtUserId UserId, CQtDeviceId &DeviceId);

	/* 根据key_id删除密钥 */
	virtual int DeleteKeyById(BYTE nPoolType, ListKeyID &lKeyId);

    /* 删除比keyid小的同步秘钥 */
    virtual int DeleteSynKeyByNodeId(BYTE *keyid, CQtUserId UserId, CQtDeviceId &DeviceId);

	/* 根据key_id更新peer */
	virtual int UpdatePeerById(BYTE nPoolType, string &lUserId, ListKeyID &lKeyId);    

private:
	CQtQkMangentRawKey m_QkMangentRawKey;
	CQtQkMangentSynKey m_QkMangentSynKey;
    CQtKeyEncrypt m_KeyEncrypt;
};

#endif

