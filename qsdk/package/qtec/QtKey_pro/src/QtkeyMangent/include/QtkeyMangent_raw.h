/********************************************************
*	Filename:	QtkeyMangent.h	                    	*
*	Author	:	lirui(lir@qtec.cn)                    	*
*                                                      	*
* 	History                                         	*
*		2017/06/19	Create								*
********************************************************/

#ifndef __QKKEYMANGENT_RAW_H__
#define __QKKEYMANGENT_RAW_H__

#include "QtkeyMangent_common.h"


class CQtQkMangentRawKey
{
public:
	virtual int QkRawKeyInsert(QkPoolHandle pDbQkPool,CQtUserId UserId, CQtDeviceId &DeviceId, ListKeyID &KeyId, ListKey &Key, BYTE *mainKey);
	virtual int GetRawKeyCount(QkPoolHandle pDbQkPool, int *pUsedCount,int *pUnusedCount, CQtUserId UserId, CQtDeviceId &DeviceId);
	virtual int GetRawKeyKeyByNode(QkPoolHandle pDbQkPool, CQtKeyId &KeyId, CQtKey &Key, BYTE *mainKey);
	virtual int GetRawKeyKeyById(QkPoolHandle pDbQkPool, CQtKeyId &KeyId, CQtKey &Key);
	virtual int GetRawKeyKeyByIdNode(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtKeyId &KeyId, CQtKey &Key);
	virtual int DeleteRawKeyByNode(QkPoolHandle pDbQkPool, CQtUserId UserId);
	virtual int DeleteRawKeyById(QkPoolHandle pDbQkPool, ListKeyID &KeyId);

private:
	CQtQkMangentCommon m_QkMangentCommon;
};



#endif

