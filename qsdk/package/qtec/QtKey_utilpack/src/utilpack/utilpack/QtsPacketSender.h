/*------------------------------------------------------*/
/* the package sender class definition				    */
/*                                                      */
/* CsPacketSender.h						                */
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

#if !defined _PACKET_SENDER_H && defined _NEW_PROTO_TP
#define _PACKET_SENDER_H

#include "QtsPacketConnection_T.h"

class PkgSenderPDU
{
public:
	PkgSenderPDU(){}
	
	static int GetFixLength(PACKET_TYPE PacketType, BOOL bNeedAck = FALSE)
	{return (DWORD)0;}

	static QtResult PeekType(LPVOID msg, PACKET_TYPE &type){
		type = CS_PDU_TYPE_DATA;
		return QT_OK;
	}
	
	QtResult Encode(CQtMessageBlock& mb, CQtMessageBlock &mbData, BOOL bNeedAck, DWORD dwAck)
	{ return QT_OK;	}
	
	QtResult DecodeWithoutData(LPVOID msg, BOOL bNeedAck)
	{ return QT_OK;	}
	QtResult DecodeWithoutData(CQtMessageBlock& mb, BOOL bNeedAck)
	{ return QT_OK;	}

	QtResult DecodeData(CQtMessageBlock& mb, BOOL bNeedAck)
	{ return QT_OK; }	
	
	CQtMessageBlock *GetData()
	{ return NULL; }
	
	DWORD GetDataLen()
	{ return (DWORD)0; }
	
	DWORD GetAck()
	{ return (DWORD)0; }
};

typedef CPacketConnection_T<PkgSenderPDU, false>   CPacketSenderConnection_t;

class CPacketSenderConnection: public CPacketSenderConnection_t
{
public:
	CPacketSenderConnection(IConnectorT *pConnector)
		: CPacketSenderConnection_t(CLIENT_CONNECTION, SENDER_PKG_LINK)
		, m_pConnector(pConnector)
	{}
	
	CPacketSenderConnection(CAcceptorT<CPacketSenderConnection> *pAcceptor, LINK_TYPE type)
		: CPacketSenderConnection_t(SERVER_CONNECTION, SENDER_PKG_LINK)
		, m_pAcceptor(pAcceptor)
	{}
	
	
	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *pITransport,
		IQtAcceptorConnectorId *aRequestId)
	{
		QT_STATE_TRACE_THIS("CPacketSenderConnection::OnConnectIndication, intance type = " << m_ConnectionType);
		CPacketSenderConnection_t::OnConnectIndication(aReason, pITransport, aRequestId);
		if(QT_SUCCEEDED(aReason))
		{
			m_LinkStatus = LINK_ACTIVE_STATUS;
			CQtConnectionManager::CType lTransport;
			QtResult result = pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, &lTransport);
			QT_ASSERTE(QT_SUCCEEDED(result));
			m_TransportType = lTransport;
		}
		QT_ASSERTE(INVALID_CONNECTION != m_ConnectionType);
		if(m_ConnectionType == CLIENT_CONNECTION)
				m_pConnector->OnConnectIndication(aReason, this, m_pConnector);
		else
			m_pAcceptor->OnConnectIndication(aReason, this, m_pAcceptor.Get());
	}
	
	
	QtResult Disconnect(QtResult aReason = 0)
	{
		if(m_pITransport.Get())
			m_pITransport->Disconnect(0);
		m_LinkStatus = LINK_INACTIVE_STATUS;
		m_pITransport = NULL;
		m_pITransportSink = NULL;
		Clear();
		return QT_OK;
	}
		
	void OnReceive(CQtMessageBlock &aData, IQtTransport *pTransport,
		CQtTransportParameter *pParameter = NULL)
	{
		//		QT_INFO_TRACE_THIS("CPacketConnection_T::OnReceive transport = " << pTransport 
		//			<< " Parameter = " << pParameter << " Status = " << m_LinkStatus << " length = " << aData.GetChainedLength());
		QT_ASSERTE_RETURN_VOID(pTransport == m_pITransport.Get());
		QT_ASSERTE(LINK_INACTIVE_STATUS != m_LinkStatus);
		if(m_pITransportSink)
		{
			m_pITransportSink->OnReceive(aData, this, pParameter);
		}
		else
		{
			QT_ERROR_TRACE_THIS("CPacketSenderConnection::OnReceive sink is null");
		}
	}

	/// If success, fill <pParameter->m_dwHaveSent> if <pParameter> is not NULL:
	///    if <aData> has sent completely, return QT_OK;
	///    else return QT_ERROR_PARTIAL_DATA;
	/// Note: <aData> has been advanced <pParameter->m_dwHaveSent> bytes in this function.
	virtual QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *pParameter = NULL)
	{
		if(LINK_ACTIVE_STATUS != m_LinkStatus)
		{
			QT_WARNING_TRACE_THIS("CPacketConnection_T::SendData, invalid status, status = " << m_LinkStatus);
			return QT_ERROR_FAILURE;
		}
		
		QtResult result = CPacketSenderConnection_t::SendData();
		
		if(QT_SUCCEEDED(result))
		{
			DWORD dwDataLen = aData.GetChainedLength();
			m_dwTotalSendBytes += dwDataLen;
			
			if(m_pmbSendData)
				m_pmbSendData->Append(aData.DuplicateChained());
			else
				m_pmbSendData = aData.DuplicateChained();
			
			CPacketSenderConnection_t::SendData();
			aData.AdvanceChainedReadPtr(dwDataLen);//Advance upper layer data read ptr
			if(pParameter)
				pParameter->m_dwHaveSent = dwDataLen;
		}
		return result;	
	}


private:
	IConnectorT			*m_pConnector;
	CQtComAutoPtr< CAcceptorT<CPacketSenderConnection> > m_pAcceptor;
};

#endif
