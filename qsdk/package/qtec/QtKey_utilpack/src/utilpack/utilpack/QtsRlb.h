/*------------------------------------------------------*/
/* the reliable connection class definition			    */
/*                                                      */
/* CsRlb.h								                */
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

#if !defined RELIABLE_CONNECTION_H && defined _NEW_PROTO_TP
#define RELIABLE_CONNECTION_H

#include "QtsReconn.h"

class CCacheBuff
{
	typedef struct Data_Node
	{
		DWORD	m_dwStartSequence;
		DWORD	m_dwLeftDataLength;
		const DWORD	m_dwOrgDataLength;
		CQtMessageBlock *m_pmbDataBlock;
		Data_Node	*m_pNext;
		Data_Node(DWORD dwSequence, DWORD dwDataLen, CQtMessageBlock &DataBk)
			:m_dwOrgDataLength(dwDataLen)
		{
			m_dwStartSequence = dwSequence;
			m_pmbDataBlock = DataBk.DuplicateChained();
			if(!m_pmbDataBlock)
			{
				QT_ERROR_TRACE_THIS("Data_Node::Data_Node out of memory");
				m_dwLeftDataLength = 0;
			}
			else
			{
				m_dwLeftDataLength = dwDataLen;
			}
			m_pNext = NULL;
		}
		~Data_Node()
		{
			if(m_pmbDataBlock)
			{
				m_pmbDataBlock->DestroyChained();
			}
		}
	}Node_t;

	Node_t	*m_pBeginNode;
	Node_t	*m_pCursorNode;
	Node_t	*m_pEndNode;
	DWORD	m_dwCachedDataSize;
	DWORD	m_dwLatestRemovePosition;
	DWORD	m_dwSequence;
public:
	CCacheBuff();
	~CCacheBuff();

	QtResult AddData(CQtMessageBlock &DataMsg);
	QtResult RemoveData(DWORD dwEndSeq);
	QtResult Next();
	void Rewind();
	CQtMessageBlock * GetData(BOOL &bIsPart);
	inline DWORD GetCachedDataSize();
	inline void Disable();
	void Clear();
	

};

//////////////////////////////////////////////////////////////////////////

class CRLBClient: public CReconnClient
{
public:
	CRLBClient(IConnectorT *pConnector);
	virtual ~CRLBClient();
	virtual QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *pParameter = NULL);
		
protected:
	virtual void OnReceiveConnectResponse(CRespPDU &responsePDU);
	virtual void OnACK(DWORD dwACKSequence);
	virtual void OnSend(IQtTransport *pTransport, CQtTransportParameter *pParameter = NULL);
	BOOL NeedKeepaliveFlag();
	void OnTimer(CQtTimerWrapperID *aId);
	virtual void OnDisconnect(QtResult aReason,	IQtTransport *pTransport);
protected:
	CCacheBuff			m_CachedSendData;
	CQtTimerWrapperID	m_PersistTimer;
};

//////////////////////////////////////////////////////////////////////////

#if defined (USE_SOCKETSERVER)
class CRLBServer: public CReconnServer
{
public:
	CRLBServer(IQtTransport *pTransport
		, ConnectionList_T *pConnectionList
		, IQtTransportSink* pITransportSink);

	virtual ~CRLBServer();

	virtual QtResult Attach(IQtTransport *pTransport, DWORD dwAck);
	QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *pParameter = NULL);
protected:
	virtual void OnACK(DWORD dwACKSequence);
	virtual void OnSend(IQtTransport *pTransport, CQtTransportParameter *pParameter = NULL);
	BOOL NeedKeepaliveFlag();

	virtual void OnDisconnect(QtResult aReason,	IQtTransport *pTransport);
	void OnTimer(CQtTimerWrapperID *aId);
	
protected:
	CCacheBuff	m_CachedSendData;	
	CQtTimerWrapperID		m_PersistTimer;
};
#endif //USE_SOCKETSERVER
#endif
