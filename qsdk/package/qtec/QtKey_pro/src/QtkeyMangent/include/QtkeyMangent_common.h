/********************************************************
*	Filename:	QtkeyMangent.h	                    	*
*	Author	:	lirui(lir@qtec.cn)                    	*
*                                                      	*
* 	History                                         	*
*		2017/06/19	Create								*
********************************************************/

#ifndef __QTMANAGENT_COMMON_H__
#define __QTMANAGENT_COMMON_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sqlite3.h>
#include <iostream>
#include "QtManagent_defines.h"
#include "QtKeyEncrypt.h"


#ifndef NULL
#define NULL	0
#endif
#ifndef MAX_NODE_ID_LEN
#define MAX_NODE_ID_LEN			8
#endif

#define DBS_SUC    0
#define DBS_FAIL   -1

#define RETRY_TIMES  10
#define WAIT_TIME    300000


typedef  sqlite3* QkPoolHandle;

class CQtQkMangentCommon
{
public:
	virtual QkPoolHandle QkPoolOpen();

	virtual void QkPoolCreateTable(QkPoolHandle pDbQkPool);

	virtual void QkPoolClose(QkPoolHandle pQkPool);

	virtual int QkPoolGetNowTime(long *pTime);

	virtual int QtMangentGetTime(QkPoolHandle pDbQkPool, void *time);

	virtual int QtMangentGetRawKeyCount(QkPoolHandle pDbQkPool, int *pUsedCount, int *pUnusedCount, CQtUserId UserId, CQtDeviceId &DeviceId);

	virtual int InsertRawKey_common(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId, ListKeyID &KeyId, ListKey &Key, BYTE *mainKey);

	virtual int GetRawKeyKeyByNode_common(QkPoolHandle pDbQkPool, CQtKeyId &KeyId, CQtKey &Key, BYTE *mainKey);

	virtual int GetRawKeyKeyById_common(QkPoolHandle pDbQkPool, CQtKeyId &KeyId, CQtKey &Key);

	virtual int GetRawKeyKeyByIdNode_common(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtKeyId &KeyId, CQtKey &Key);

	virtual int DeleteRawKeyByNode_common(QkPoolHandle pDbQkPool, CQtUserId UserId);

	virtual int DeleteRawKeyById_common(QkPoolHandle pDbQkPool, ListKeyID &KeyId);

	virtual int InsertSynKey_common(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId, ListKeyID &KeyId, ListKeyID &Key, BYTE *mainKey);

	virtual int QtMangentGetSynKeyCount(QkPoolHandle pDbQkPool, int *pUsedCount, int *pUnusedCount, CQtUserId UserId, CQtDeviceId &DeviceId);

	virtual int GetSynKeyKeyByNode_common(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId, CQtKeyId &KeyId, CQtKey &Key, BYTE *mainKey);

	virtual int GetSynKeyKeyById_common(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId, CQtKeyId &KeyId, CQtKey &Key);

	virtual int GetSynKeyKeyByIdNode_common(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId, CQtKeyId &KeyId, CQtKey &Key, BYTE *mainKey);

	virtual int DeleteSynKeyByNode_common(QkPoolHandle pDbQkPool, CQtUserId UserId, CQtDeviceId &DeviceId);

	virtual int DeleteSynKeyById_common(QkPoolHandle pDbQkPool, ListKeyID &KeyId);

	virtual int UpdateSynPeerById_common(QkPoolHandle pDbQkPool, string &lUserId, CQtKeyId &KeyId);

public:
	void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen);
	void StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen);

private:
	CQtKeyEncrypt m_KeyEncrypt;
};

#endif

