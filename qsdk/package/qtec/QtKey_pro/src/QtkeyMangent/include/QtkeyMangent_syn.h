/********************************************************
*	Filename:	QtkeyMangent.h	                    	*
*	Author	:	lirui(lir@qtec.cn)                    	*
*                                                      	*
* 	History                                         	*
*		2017/06/19	Create								*
********************************************************/

#ifndef __QKKEYMANGENT_SYN_H__
#define __QKKEYMANGENT_SYN_H__

#include "QtkeyMangent_common.h"


class CQtQkMangentSynKey
{
public:
	virtual int QkSynKeyInsert(QkPoolHandle pDbQkPool,CQtUserId UserId, CQtDeviceId &DeviceId, ListKeyID &KeyId, ListKey &Key, BYTE *mainKey);
	virtual int GetSynKeyCount(QkPoolHandle pDbQkPool, int *pUsedCount,int *pUnusedCount, CQtUserId UserId, CQtDeviceId &DeviceId);
	virtual int GetSynKeyKeyByNode(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId, CQtKeyId &KeyId, CQtKey &Key, BYTE *mainKey);
	virtual int GetSynKeyKeyById(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId, CQtKeyId &KeyId, CQtKey &Key);
	virtual int GetSynKeyKeyByIdNode(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId, CQtKeyId &KeyId, CQtKey &Key, BYTE *mainKey);
	virtual int DeleteSynKeyByNode(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId);
	virtual int DeleteSynKeyById(QkPoolHandle pDbQkPool, ListKeyID &KeyId);
	virtual int UpdateSynPeerById(QkPoolHandle pDbQkPool,string &lUserId, CQtKeyId &KeyId);
	
private:
	CQtQkMangentCommon m_QkMangentCommon;
};



#endif

