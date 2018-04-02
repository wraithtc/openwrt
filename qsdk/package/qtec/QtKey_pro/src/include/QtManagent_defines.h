/********************************************************
*	Filename:	QtkeyMangent.h	                    	*
*	Author	:	lirui(lir@qtec.cn)                    	*
*                                                      	*
* 	History                                         	*
*		2017/06/19	Create								*
********************************************************/
#ifndef __QTMANAGENT_DEFINS_H__
#define __QTMANAGENT_DEFINS_H__

#include "QtDebug.h"
#include "QtServerSessionApi.h"

#define PRINT printf

#ifndef NULL
#define NULL	0
#endif
#define MAX_SQL_LEN 			4096
#define MAX_SQL_LEN_TMP			400
#define DEFAULT_USERID			1

#define MAX_SYN_NUMBER	5
#define MIN_SYN_NUMBER	1

#define QT_MAX_KEY_ID_LEN		16
#define QT_MAX_KEY_LEN			16
#define QT_MAX_DEVICEID_LEN		32

#define KEY_UNUSED	1
#define KEY_USED	2

#define DEFAULT_VAILDITY_TIME	24*60*60


#define QKPOOL_SUC    0
#define QKPOOL_FAIL   -1

enum{
	POOL_TYPE_RAW = 1,
	POOL_TYPE_SYNC
};


typedef std::list <CQtKeyId> ListKeyID;
typedef std::list <CQtKey> ListKey;
typedef std::list <CQtUserId> ListUserId;
typedef std::list <CQtDeviceId> ListDeviceId;

// ----------------------- Session PDU type -----------------------------

const BYTE QT_SESSION_PDU_TYPE_BASE					= 10;

const BYTE QT_KEY_SYN_PDU_REQ				= QT_SESSION_PDU_TYPE_BASE + 91;
const BYTE QT_KEY_SYN_PDU_RESP				= QT_SESSION_PDU_TYPE_BASE + 92;
#if 0
const BYTE QT_KEY_SYN_PDU_CONF				= QT_SESSION_PDU_TYPE_BASE + 93;
const BYTE QT_KEY_SYN_PDU_PUSH_REQ			= QT_SESSION_PDU_TYPE_BASE + 94;
const BYTE QT_KEY_SYN_PDU_PUSH_ANOTHER_RESP	= QT_SESSION_PDU_TYPE_BASE + 95;
const BYTE QT_KEY_SYN_PDU_PUSH_ANOTHER_CONF	= QT_SESSION_PDU_TYPE_BASE + 96;
const BYTE QT_KEY_SYN_PDU_PUSH_RESP			= QT_SESSION_PDU_TYPE_BASE + 97;
#endif

#endif



