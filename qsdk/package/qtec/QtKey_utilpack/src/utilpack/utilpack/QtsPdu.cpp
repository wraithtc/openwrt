//$Id: CsPdu.cpp,v 1.87.4.1 2010/03/29 10:14:28 jerryh Exp $
#include "QtBase.h"
#include "QtsPdu.h"
#if !defined (_NEW_PROTO_TP)


///////////////////////////////////////////
//class CCsPduBase
///////////////////////////////////////////
CCsPduBase::CCsPduBase(BYTE byType)
{
	m_byType = byType;
	
	//QT_ASSERTE(GetFixLength(m_byType) > 0);//Ensure a valid PDU Type
}

CCsPduBase::~CCsPduBase()
{
	m_byType = CS_PDU_TYPE_HEAD;
}
	
void CCsPduBase::Decode(CQtMessageBlock& mb)	//mb >> m_xx
{
	CQtByteStreamNetwork bs(mb);

	bs >> m_byType;

	//QT_ASSERTE(GetFixLength(m_byType) > 0);//Ensure a valid PDU Type
}

void CCsPduBase::Encode(CQtMessageBlock& mb)	//mb << m_xx
{
	CQtByteStreamNetwork bs(mb);

	bs << m_byType;
}

DWORD CCsPduBase::GetFixLength(BYTE byType, BOOL bNeedACK)
{
	if(byType == CS_PDU_TYPE_HEAD)
		return 1;//PDU Head
	else if(byType == CS_PDU_TYPE_CONN_REQ || byType == CS_PDU_TYPE_CONN_RESP)
		return 8;
	else if(byType == CS_PDU_TYPE_DISCONN)
		return 3;
	else if(byType == CS_PDU_TYPE_KEEPALIVE)
		return 5;
	else if(byType == CS_PDU_TYPE_DATA_START || byType == CS_PDU_TYPE_DATA_NORMAL)
	{
		if(bNeedACK)
			return 9;
		else
			return 5;
	}
	else
	{
		QT_ERROR_TRACE("CCsPduBase::GetFixLength(BYTE byType, BOOL bNeedACK), Invalid PDU type, byType = " << byType);
		return 0;
	}
}

DWORD CCsPduBase::GetFixLength()
{
	return CCsPduBase::GetFixLength(m_byType, TRUE);
}

BYTE CCsPduBase::GetType()
{
	return m_byType;
}
///////////////////////////////////////////
//class CCsPduConnReqResp
///////////////////////////////////////////
CCsPduConnReqResp::CCsPduConnReqResp(DWORD dwACK, WORD wConTag, BYTE byType, BYTE byConnType) : CCsPduBase(byType)
{
	m_dwACK = dwACK;
	m_wConTag = wConTag;
	m_byConnType = byConnType;
}

CCsPduConnReqResp::~CCsPduConnReqResp()
{
	m_dwACK = 0;
	m_wConTag = 0;
}

void CCsPduConnReqResp::Decode(CQtMessageBlock& mb)	//mb >> m_xx
{
	CCsPduBase::Decode(mb);

	CQtByteStreamNetwork bs(mb);

	bs >> m_dwACK;
	bs >> m_wConTag;
	bs >> m_byConnType;
}

void CCsPduConnReqResp::Encode(CQtMessageBlock& mb)	//mb << m_xx
{
	CCsPduBase::Encode(mb);

	CQtByteStreamNetwork bs(mb);

	bs << m_dwACK;
	bs << m_wConTag;
	bs << m_byConnType;
}

DWORD CCsPduConnReqResp::GetACK()
{
	return m_dwACK;
}

WORD CCsPduConnReqResp::GetConTag()
{
	return m_wConTag;
}

BYTE CCsPduConnReqResp::GetConnType()
{
	return m_byConnType;
}

///////////////////////////////////////////
//class CCsPduSendItem
///////////////////////////////////////////
CCsPduSendItem::CCsPduSendItem(
							   BOOL bNeedACK,
							   DWORD dwACK, 
							   BYTE byType, 
							   DWORD dwDataLen, 
							   CQtMessageBlock *pData) : CCsPduBase(byType)
{
	m_dwACK = dwACK;
	m_dwDataLen = dwDataLen;
	m_pmbData = pData;

	m_dwSeqStart = 0;
	m_dwSeqEnd = 0;
	m_pNextPDU = NULL;

	m_bNeedACK = bNeedACK;
}

CCsPduSendItem::~CCsPduSendItem()
{
	m_dwACK = 0;
	m_dwDataLen = 0;
	if(m_pmbData)
	{
		m_pmbData->DestroyChained();
		m_pmbData = NULL;
	}

	m_dwSeqStart = 0;
	m_dwSeqEnd = 0;
	m_pNextPDU = NULL;
}

void CCsPduSendItem::Decode (CQtMessageBlock& mb) 
{ 
	CCsPduBase::Decode(mb); 
}	

void CCsPduSendItem::Encode (CQtMessageBlock& mb) 
{ 
	CCsPduBase::Encode(mb); 
}

void CCsPduSendItem::SetACK(DWORD dwACK) 
{ 
	m_dwACK = dwACK; 
}

DWORD CCsPduSendItem::GetACK() 
{ 
	return m_dwACK; 
}

DWORD CCsPduSendItem::GetDataLen() 
{ 
	return m_dwDataLen; 
}

CQtMessageBlock* CCsPduSendItem::GetData() 
{ 
	return m_pmbData; 
}

void CCsPduSendItem::SetSeqStart(DWORD dwSeqStart) 
{
	if(m_byType == CS_PDU_TYPE_DATA_START || m_byType == CS_PDU_TYPE_DATA_NORMAL)
	{
		m_dwSeqStart = dwSeqStart;
		m_dwSeqEnd = m_dwSeqStart + (m_dwDataLen + GetFixLength()) - 1;
	}
	else
	{
		QT_ERROR_TRACE_THIS("CCsPduSendItem::SetSeqStart, Only Data PDU can be set Seq#");
	}
}

DWORD CCsPduSendItem::GetSeqStart()
{
	return m_dwSeqStart;
}

DWORD CCsPduSendItem::GetSeqEnd()
{
	return m_dwSeqEnd;
}

void CCsPduSendItem::SetNext(CCsPduSendItem *pNextPDU) 
{ 
	m_pNextPDU = pNextPDU; 
}

CCsPduSendItem* CCsPduSendItem::GetNext()
{
	return m_pNextPDU;
}

DWORD CCsPduSendItem::GetFixLength()
{
	return CCsPduBase::GetFixLength(m_byType, m_bNeedACK);
}

///////////////////////////////////////////
//class CCsPduDisconn
///////////////////////////////////////////
CCsPduDisconn::CCsPduDisconn(WORD wReason, BYTE byType) 
	: CCsPduSendItem(TRUE, 0, byType, 0, NULL)
{
	m_wReason = wReason;
}

CCsPduDisconn::~CCsPduDisconn()
{
	m_wReason = 0;
}

void CCsPduDisconn::Decode(CQtMessageBlock& mb)	//mb >> m_xx
{
	CCsPduSendItem::Decode(mb);

	CQtByteStreamNetwork bs(mb);

	bs >> m_wReason;
}

void CCsPduDisconn::Encode(CQtMessageBlock& mb)	//mb << m_xx
{
	CCsPduSendItem::Encode(mb);

	CQtByteStreamNetwork bs(mb);

	bs << m_wReason;
}

WORD CCsPduDisconn::GetReason()
{
	return m_wReason;
}

///////////////////////////////////////////
//class CCsPduKeepAlive
///////////////////////////////////////////
CCsPduKeepAlive::CCsPduKeepAlive(DWORD dwACK, BYTE byType) 
	: CCsPduSendItem(TRUE, dwACK, byType, 0, NULL)
{
}

CCsPduKeepAlive::~CCsPduKeepAlive()
{
}

void CCsPduKeepAlive::Decode(CQtMessageBlock& mb)	//mb >> m_xx
{
	CCsPduSendItem::Decode(mb);

	CQtByteStreamNetwork bs(mb);

	bs >> m_dwACK;
}

void CCsPduKeepAlive::Encode(CQtMessageBlock& mb)	//mb << m_xx
{
	CCsPduSendItem::Encode(mb);

	CQtByteStreamNetwork bs(mb);

	bs << m_dwACK;
}

///////////////////////////////////////////
//class CCsPduData
///////////////////////////////////////////
CCsPduData::CCsPduData(BOOL bNeedACK, DWORD dwACK, DWORD dwDataLen, BYTE byType, CQtMessageBlock *pData) 
	: CCsPduSendItem(bNeedACK, dwACK, byType, dwDataLen, pData)
{
}

CCsPduData::~CCsPduData()
{
}

void CCsPduData::Decode(CQtMessageBlock& mb)	//mb >> m_xx
{
	CCsPduSendItem::Decode(mb);

	CQtByteStreamNetwork bs(mb);

	if(m_bNeedACK)
		bs >> m_dwACK;
	
	bs >> m_dwDataLen;

	QT_ASSERTE(m_pmbData == NULL);
	QT_ASSERTE(mb.GetChainedLength() >= m_dwDataLen);
	
	if(mb.GetChainedLength() == m_dwDataLen)
	{
		m_pmbData = mb.DuplicateChained();
		mb.AdvanceChainedReadPtr(m_dwDataLen);
	}
	else
	{
		QT_INFO_TRACE_THIS("CCsPduData::Decode(), need Disjoint mb.");
		CQtMessageBlock *pTmp = mb.Disjoint(m_dwDataLen);
		m_pmbData = mb.DuplicateChained();
		mb.AdvanceChainedReadPtr(m_dwDataLen);
		mb.Append(pTmp);
	}
}

void CCsPduData::Encode(CQtMessageBlock& mb)	//mb << m_xx
{
	CCsPduSendItem::Encode(mb);

	CQtByteStreamNetwork bs(mb);

	if(m_bNeedACK)
		bs << m_dwACK;
	
	bs << m_dwDataLen;

	QT_ASSERTE(m_pmbData != NULL);
	
	mb.Append(m_pmbData);
}

void CCsPduData::DecodeWithOutData(CQtMessageBlock& mb)	//mb >> m_xx
{
	CCsPduSendItem::Decode(mb);

	CQtByteStreamNetwork bs(mb);

	if(m_bNeedACK)
		bs >> m_dwACK;

	bs >> m_dwDataLen;
}

void CCsPduData::EncodeWithOutData(CQtMessageBlock& mb)	//mb >> m_xx
{
	CCsPduSendItem::Encode(mb);

	CQtByteStreamNetwork bs(mb);

	if(m_bNeedACK)
		bs << m_dwACK;

	bs << m_dwDataLen;
}
#endif
