
/*------------------------------------------------------*/
/* the reconnect connection class definition		    */
/*                                                      */
/* CsReconn.cpp							                */
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

#ifdef _NEW_PROTO_TP

#include "QtsRlb.h"

#define		PERSIST_SEND_INTERVAL			(LONG)(50 * 1000)	//ms

#define SEND_CACHED_DATA(ClassName, Status) do{\
	if(LINK_ACTIVE_STATUS == Status){\
		CQtMessageBlock *pmbBlock;\
		BOOL bIsPart;\
		do{\
			pmbBlock = m_CachedSendData.GetData(bIsPart);\
			if(pmbBlock)\
			{\
				if(!bIsPart)\
				{\
					if(QT_FAILED(PktConnectionWithKeepAlive_T::SendData(*pmbBlock))){\
						break;\
					}\
				}\
				else\
				{\
					if(!m_pmbSendData)\
						m_pmbSendData = pmbBlock->DuplicateChained();\
					else\
						m_pmbSendData->Append(pmbBlock->DuplicateChained());\
					if(QT_FAILED(PktConnectionWithKeepAlive_T::SendData()))\
					{\
						break;\
					}\
				}\
				m_CachedSendData.Next();\
			}\
		}while(pmbBlock);\
		}\
	}while(0);
#define RLB_SENDDATA(ClassName)		do{\
	if(m_CachedSendData.GetCachedDataSize() > m_dwMaxBuffLen) {\
		QT_ERROR_TRACE_THIS(ClassName << "::SendData the cache data already out of max allowed!");\
		if(LINK_ACTIVE_STATUS == m_LinkStatus)	{\
			m_PersistTimer.Schedule(this, CQtTimeValue(0, PERSIST_SEND_INTERVAL));\
		}\
		m_LinkStatus = LINK_BLOCK_STATUS;\
		return QT_ERROR_FAILURE;\
	}\
	if(aData.GetChainedLength() > 0)\
		m_CachedSendData.AddData(aData);\
	SEND_CACHED_DATA(ClassName, m_LinkStatus);\
	DWORD dwDataLen = aData.GetChainedLength();\
	aData.AdvanceChainedReadPtr(dwDataLen);\
	if(pParameter)\
		pParameter->m_dwHaveSent = dwDataLen;\
	return QT_OK;\
} while(0);

#define ACK(ClassName)		do{\
		if(m_dwMaxBuffLen == 0)	break;\
		DWORD dwLatestCachedSize = m_CachedSendData.GetCachedDataSize();\
		m_CachedSendData.RemoveData(dwACKSequenceuence);\
		if(dwLatestCachedSize >= m_dwMaxBuffLen && \
		   m_CachedSendData.GetCachedDataSize() < m_dwMaxBuffLen)\
		{\
			/*QT_INFO_TRACE_THIS(ClassName << "::OnACK latest size = " << dwLatestCachedSize << " now size = " << m_CachedSendData.GetCachedDataSize() << " Status = " << m_LinkStatus);*/\
			m_PersistTimer.Cancel();\
			m_LinkStatus = LINK_ACTIVE_STATUS;\
			m_pITransportSink->OnSend(this);\
		}\
	}while(0);


CCacheBuff::CCacheBuff()
	: m_pBeginNode(NULL)
	, m_pCursorNode(NULL)
	, m_pEndNode(NULL)
	, m_dwCachedDataSize(0)
	, m_dwLatestRemovePosition(0)
	, m_dwSequence(0)
{}

CCacheBuff::~CCacheBuff()
{
	if(m_dwCachedDataSize > 0) {
		QT_WARNING_TRACE_THIS("CCacheBuff::~CCacheBuff some data be cached");
	}
	Clear();
}

void CCacheBuff::Clear()
{
	Node_t *pNext, *pCursor = m_pBeginNode;
	DWORD dwLeftLength = 0;
	while(pCursor) {
		pNext = pCursor->m_pNext;
		dwLeftLength += pCursor->m_dwLeftDataLength;
		delete pCursor;
		pCursor = pNext;	
	}
	m_dwCachedDataSize = 0;
	m_dwLatestRemovePosition = 0;
	m_pBeginNode = m_pEndNode = m_pCursorNode = NULL;
	if(dwLeftLength > 0){
		QT_WARNING_TRACE_THIS("CCacheBuff::Clear remain some data that no acked!, length  = " << dwLeftLength);
	}
}

QtResult CCacheBuff::AddData(CQtMessageBlock &DataMsg)
{
//	QT_INFO_TRACE_THIS("CCacheBuff::AddData Sequenceuence = " << m_dwSequence << " org data length = " << dwDataLen);
//	Modify for compatible old version, add header length Victor 7/6
	DWORD dwDataLen = DataMsg.GetChainedLength() + CPacketPDU::GetFixLength(CS_PDU_TYPE_DATA, TRUE);
	Node_t *pNode = new Node_t(m_dwSequence, dwDataLen, DataMsg);
	if(!pNode) {
		QT_ERROR_TRACE_THIS("CCacheBuff::AddData out of memory!");
		return QT_ERROR_FAILURE;
	}
	if(!m_pBeginNode)   //the buff list is empty
		m_pBeginNode = m_pEndNode = m_pCursorNode = pNode;
	else {
		m_pEndNode->m_pNext = pNode;
		m_pEndNode = pNode;
		if(!m_pCursorNode)  //if the data in list already be sent over, reset it
			m_pCursorNode = pNode;
	}
	m_dwCachedDataSize += dwDataLen;
	m_dwSequence += dwDataLen;
	return QT_OK;
}

QtResult CCacheBuff::RemoveData(DWORD dwEndSequence)
{
	if(dwEndSequence == m_dwLatestRemovePosition)
		return QT_OK;
	Node_t *pNext, *pCursor = m_pBeginNode;
	while(pCursor) {

		if(dwEndSequence < m_dwLatestRemovePosition){
			//the Sequenceuence is rewind, so we must remove all data that's Sequenceuence big than dwEndSequence
			while(pCursor) {
				pNext = pCursor->m_pNext;
				if(dwEndSequence <= pCursor->m_dwStartSequence && 
					dwEndSequence <= (pCursor->m_dwStartSequence + pCursor->m_dwLeftDataLength) ) {
					///the data that's Sequenceuence less than need to remove
					m_dwCachedDataSize -= pCursor->m_dwLeftDataLength;
					delete pCursor;
					m_pBeginNode = pNext;
				}
				else {
					break;
				}				
				pCursor = pNext;
			}
		}
		if(!pCursor)//over!
			break;

		pNext = pCursor->m_pNext;
		if(dwEndSequence >= (pCursor->m_dwStartSequence + pCursor->m_dwLeftDataLength))
			///the data that's Sequenceuence less than need to remove
		{
			m_dwCachedDataSize -= pCursor->m_dwLeftDataLength;
			delete pCursor;
			m_pBeginNode = pNext;
		}
		else {
			if(dwEndSequence > pCursor->m_dwStartSequence && dwEndSequence < (pCursor->m_dwStartSequence + pCursor->m_dwLeftDataLength))
				///the part data that need to remove
			{
				DWORD dwPartLen = dwEndSequence - pCursor->m_dwStartSequence;
				m_dwCachedDataSize -= dwPartLen;
				pCursor->m_dwStartSequence += dwPartLen;
				pCursor->m_dwLeftDataLength -= dwPartLen;
				QT_ASSERTE(pCursor->m_dwLeftDataLength < pCursor->m_dwOrgDataLength);
				pCursor->m_pmbDataBlock->AdvanceChainedReadPtr(dwPartLen);
			}
			break;
		}
		pCursor = pNext;
	}

	if(!m_pBeginNode) {
		///all data be removed
		QT_ASSERTE(m_dwCachedDataSize == 0);
		m_pEndNode = m_pCursorNode = NULL;
	}
	
	//reset the latest Sequenceuence
	m_dwLatestRemovePosition = dwEndSequence;
	return QT_OK;
}

void CCacheBuff::Rewind()
{
	m_pCursorNode = m_pBeginNode;
}

QtResult CCacheBuff::Next()
{
	if(!m_pCursorNode)
		return QT_ERROR_FAILURE;
	
	m_pCursorNode = m_pCursorNode->m_pNext;
	return QT_OK;

}

CQtMessageBlock *CCacheBuff::GetData(BOOL &bIsPart)
{
	if(m_pCursorNode)
	{
		if(m_pCursorNode->m_dwOrgDataLength == m_pCursorNode->m_dwLeftDataLength)
			bIsPart = FALSE;
		else
			bIsPart = TRUE;
		return m_pCursorNode->m_pmbDataBlock;
	}
	return NULL;
}

DWORD CCacheBuff::GetCachedDataSize()
{
	return m_dwCachedDataSize;
}


void CCacheBuff::Disable()
{
	m_pCursorNode = NULL;
}

//////////////////////////////////////////////////////////////////////////


CRLBClient::CRLBClient(IConnectorT *pConnector)
: CReconnClient(pConnector)
{
	m_LinkType = RLB_LINK;
}

CRLBClient::~CRLBClient()
{
	QT_INFO_TRACE_THIS("CRLBClient::~CRLBClient()");
}

void CRLBClient::OnACK(DWORD dwACKSequenceuence)
{
	ACK("CRLBClient");
}

QtResult  CRLBClient::SendData(CQtMessageBlock &aData, CQtTransportParameter *pParameter)
{
	if(m_dwMaxBuffLen == 0) 
		return CReconnClient::SendData(aData, pParameter);

	RLB_SENDDATA("CRLBClient");
}

void CRLBClient::OnReceiveConnectResponse(CRespPDU &responsePDU)
{
	//Restore(m_RcvBuff, m_PDUType, m_dwLatestPacketSize);
	OnACK(responsePDU.GetAck());
	m_CachedSendData.Rewind();
	if(m_dwMaxBuffLen > 0)
	{
		LINK_TYPE latestLinkStatus = m_LinkStatus;
		m_LinkStatus = LINK_ACTIVE_STATUS;
		SEND_CACHED_DATA("CRLBClient", m_LinkStatus);
		m_LinkStatus = latestLinkStatus;
	}
	CReconnClient::OnReceiveConnectResponse(responsePDU);
}

void CRLBClient::OnSend(IQtTransport *pTransport, CQtTransportParameter *pParameter)
{
	m_LinkStatus = LINK_ACTIVE_STATUS;
	if(m_dwMaxBuffLen > 0)
		SEND_CACHED_DATA("CRLBClient", m_LinkStatus);
	PktConnectionWithKeepAlive_T::OnSend(pTransport, pParameter);
}

BOOL CRLBClient::NeedKeepaliveFlag()
{
	BOOL bIsPart;
	if(m_CachedSendData.GetData(bIsPart))
		return FALSE;
	return TRUE;
}

void CRLBClient::OnTimer(CQtTimerWrapperID *aId)
{
	if(aId == &m_PersistTimer) {
		if(m_dwMaxBuffLen > 0)
			SEND_CACHED_DATA("CRLBClient", LINK_ACTIVE_STATUS);
	}
	else 
		CReconnClient::OnTimer(aId);
}

void CRLBClient::OnDisconnect(QtResult aReason,	IQtTransport *pTransport)
{
	m_CachedSendData.Disable();
	CReconnClient::OnDisconnect(aReason, pTransport);
}

//////////////////////////////////////////////////////////////////////////

#if defined (USE_SOCKETSERVER)
CRLBServer::CRLBServer(IQtTransport *pTransport
					   , ConnectionList_T *pConnectionList
					   , IQtTransportSink* pITransportSink)
: CReconnServer(pTransport, pConnectionList, pITransportSink)
{
	m_LinkType = RLB_LINK;
}

CRLBServer::~CRLBServer()
{
	QT_INFO_TRACE_THIS("CRLBServer::~CRLBServer()");
}

QtResult CRLBServer::Attach( IQtTransport *pTransport, DWORD dwAck)
{
	QT_INFO_TRACE_THIS("CRLBServer::Attach ack = " << dwAck);
	QT_ASSERTE(SERVER_CONNECTION == m_ConnectionType);
	QT_ASSERTE(pTransport);
	QT_ASSERTE(RLB_LINK == m_LinkType); //only work for reconnect connection
	if(m_pmbSendData) {
		m_pmbSendData->DestroyChained();
		m_pmbSendData = NULL;
	}
	if(m_pITransport.Get())	{
		m_pITransport->Disconnect(QT_ERROR_NETWORK_CONNECTION_RECONNECT);
	}
	m_pITransport = pTransport;
	m_pITransport->OpenWithSink(this);
	m_AbateTimer.Cancel();
	LINK_STATUS latestLinkStatus = m_LinkStatus;
	OnACK(dwAck);
	m_CachedSendData.Rewind();
	m_LinkStatus = LINK_ACTIVE_STATUS;
	//Restore(m_RcvBuff, m_PDUType, m_dwLatestPacketSize);
	if(m_dwMaxBuffLen > 0)
		SEND_CACHED_DATA("CRLBServer", m_LinkStatus);
	if(LINK_BLOCK_STATUS == latestLinkStatus) {
		OnSend(m_pITransport.Get());
	}
	return QT_OK;
}

QtResult  CRLBServer::SendData(CQtMessageBlock &aData, CQtTransportParameter *pParameter)
{
	if(m_dwMaxBuffLen == 0) 
		return CReconnServer::SendData(aData, pParameter);
	RLB_SENDDATA("CRLBServer");
}

void CRLBServer::OnACK(DWORD dwACKSequenceuence)
{
	ACK("CRLBServer");
}

void CRLBServer::OnSend(IQtTransport *pTransport, CQtTransportParameter *pParameter)
{
	m_LinkStatus = LINK_ACTIVE_STATUS;
	if(m_dwMaxBuffLen > 0)
		SEND_CACHED_DATA("CRLBServer", m_LinkStatus);
	PktConnectionWithKeepAlive_T::OnSend(pTransport, pParameter);
}

BOOL CRLBServer::NeedKeepaliveFlag()
{
	BOOL bIsPart;
	if(m_CachedSendData.GetData(bIsPart))
		return FALSE;
	return TRUE;
}

void CRLBServer::OnTimer(CQtTimerWrapperID *aId)
{
	if(aId == &m_PersistTimer) {
		if(m_dwMaxBuffLen > 0)
			SEND_CACHED_DATA("CRLBServer", LINK_ACTIVE_STATUS);
	}
	else
		CReconnServer::OnTimer(aId);
}

void CRLBServer::OnDisconnect(QtResult aReason,	IQtTransport *pTransport)
{
	m_CachedSendData.Disable();
	CReconnServer::OnDisconnect(aReason, pTransport);
}

//////////////////////////////////////////////////////////////////////////
#endif //USE_SOCKETSERVER
#endif
