/*------------------------------------------------------*/
/* Connection Service PDU classes                       */
/*                                                      */
/* CsPdu.h                                              */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*    zhubin (zhubin@qtec.cn)                           */
/*                                                      */
/* History                                              */
/*    2017/02/15  Create                                */
/*                                                      */
/*------------------------------------------------------*/


#if !defined CS_PDU_H  && !defined (_NEW_PROTO_TP)
#define CS_PDU_H

#include "QtDefines.h"
#include "QtDebug.h"
#include "QtMessageBlock.h"
#include "QtByteStream.h"

#define CS_PDU_TYPE_NONE		0xF
#define CS_PDU_TYPE_HEAD		0x0
#define CS_PDU_TYPE_CONN_REQ	0x1
#define CS_PDU_TYPE_CONN_RESP	0x2
#define CS_PDU_TYPE_DISCONN		0x3
#define CS_PDU_TYPE_KEEPALIVE	0x4
#define CS_PDU_TYPE_DATA_START	0x5	//Indicate Seq# rewound
#define CS_PDU_TYPE_DATA_NORMAL	0x6

#define CS_CONN_TYPE_NONE		0xF
#define CS_CONN_TYPE_RLB		0x1
#define CS_CONN_TYPE_PKG		0x2

#define PDU_VERSION				1

class QT_OS_EXPORT CCsPduBase
{
public:
	CCsPduBase(BYTE byType = CS_PDU_TYPE_HEAD);
	virtual ~CCsPduBase();
	
	virtual void Decode(CQtMessageBlock& mb);	//mb >> m_xx
	virtual void Encode(CQtMessageBlock& mb);	//mb << m_xx

	virtual DWORD GetFixLength();
	static DWORD GetFixLength(BYTE byType, BOOL bNeedACK = TRUE);

	BYTE GetType();
protected:
	BYTE m_byType;
};

class QT_OS_EXPORT CCsPduConnReqResp : public CCsPduBase
{
public:
	//byType = CS_PDU_TYPE_CONN_REQ || CS_PDU_TYPE_CONN_RESP
	CCsPduConnReqResp(
		DWORD dwACK = 0, 
		WORD wConTag = 0, 
		BYTE byType = CS_PDU_TYPE_CONN_REQ, 
		BYTE byConnType = CS_CONN_TYPE_NONE);
	virtual ~CCsPduConnReqResp();

	virtual void Decode (CQtMessageBlock& mb);	//mb >> m_xx
	virtual void Encode (CQtMessageBlock& mb);	//mb << m_xx

	DWORD GetACK();
	WORD GetConTag();
	BYTE GetConnType();
protected:
	DWORD m_dwACK;
	WORD m_wConTag;
	BYTE m_byConnType;
};

//////////////////////////////
//PDUs can be add in SendBuf
//////////////////////////////

//SendBuf items, Base class for Data PDU & KeepAlive PDU
class QT_OS_EXPORT CCsPduSendItem : public CCsPduBase
{
public:
	CCsPduSendItem(
		BOOL bNeedACK, 
		DWORD dwACK = 0, 
		BYTE byType = CS_PDU_TYPE_DATA_NORMAL, 
		DWORD dwDataLen = 0,
		CQtMessageBlock *pData = NULL);
	virtual ~CCsPduSendItem();

	virtual void Decode (CQtMessageBlock& mb);	//mb >> m_xx
	virtual void Encode (CQtMessageBlock& mb);	//mb << m_xx

	void SetACK(DWORD dwACK);
	DWORD GetACK();
	DWORD GetDataLen();
	CQtMessageBlock *GetData();
public:
	void SetSeqStart(DWORD dwSeqStart);
	DWORD GetSeqStart();
	DWORD GetSeqEnd();
	void SetNext(CCsPduSendItem *pNextPDU);
	CCsPduSendItem* GetNext();

	virtual DWORD GetFixLength();
protected:
	DWORD m_dwACK;
	DWORD m_dwDataLen;
	CQtMessageBlock *m_pmbData;
protected:
	DWORD m_dwSeqStart;	//start sequence of this PDU
	DWORD m_dwSeqEnd;	//end sequence of this PDU

	CCsPduSendItem *m_pNextPDU;	//point to next CCsPduSendItem obj in the list

	BOOL m_bNeedACK;
};

class QT_OS_EXPORT CCsPduDisconn : public CCsPduSendItem
{
public:
	CCsPduDisconn(
		WORD wReason = 0, 
		BYTE byType = CS_PDU_TYPE_DISCONN);
	virtual ~CCsPduDisconn();

	virtual void Decode (CQtMessageBlock& mb);	//mb >> m_xx
	virtual void Encode (CQtMessageBlock& mb);	//mb << m_xx

	WORD GetReason();
protected:
	WORD m_wReason;
};

class QT_OS_EXPORT CCsPduKeepAlive : public CCsPduSendItem
{
public:
	CCsPduKeepAlive(DWORD dwACK = 0, BYTE byType = CS_PDU_TYPE_KEEPALIVE);
	virtual ~CCsPduKeepAlive();

	virtual void Decode (CQtMessageBlock& mb);	//mb >> m_xx
	virtual void Encode (CQtMessageBlock& mb);	//mb << m_xx
};

class QT_OS_EXPORT CCsPduData : public CCsPduSendItem
{
public:
	//byType = CS_PDU_TYPE_DATA_START || CS_PDU_TYPE_DATA_NORMAL
	CCsPduData(
		BOOL bNeedACK = TRUE,
		DWORD dwACK = 0, 
		DWORD dwDataLen = 0, 
		BYTE byType = CS_PDU_TYPE_DATA_NORMAL, 
		CQtMessageBlock *pData = NULL);
	virtual ~CCsPduData();

	virtual void Decode (CQtMessageBlock& mb);	//mb >> m_xx
	virtual void Encode (CQtMessageBlock& mb);	//mb << m_xx

	void DecodeWithOutData(CQtMessageBlock& mb);//mb >> m_xx, only wanna get true data len
	void EncodeWithOutData(CQtMessageBlock& mb);//mb << m_xx, only wanna put true data len
};
#endif // CS_PDU_H

