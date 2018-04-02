/*------------------------------------------------------*/
/* the template class for package connection            */
/*                                                      */
/* CsPacketConnection_T.h                               */
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

#if !defined _PACKET_CONNECTION_T_H && defined _NEW_PROTO_TP
#define _PACKET_CONNECTION_T_H

#include "QtDefines.h"
#include "QtDebug.h"
#include "QtErrorNetwork.h"
#include "QtTimerWrapperID.h"
#include "QtTimeValue.h"
#include "QtInetAddr.h"
#include "QtMessageBlock.h"
#include "QtConnectionInterface.h"
#include "QtUtilTemplates.h"
#include "timer.h"
#include "QtsPacketPDU.h"
#include <vector>
#include <queue>

#ifdef QT_WIN32
#pragma message("************Now use the new protocol TP *************")
#endif

#define		LINK_ABATE_TIME				((LONG)150)				///abate time  is 150s
#define		LINK_RTT_DETECT_INTERVAL	((LONG)300)				///RTT detect interval 300s
#define		KEEPALIVE_INTERVAL			((LONG)15) 				///keep alive interval 15s
#define		MAX_ACK_DATA_PIECE			(4 * 1024)				///if receive MAX_ACK_DATA_PIECE data but not ack, ack it immediately
#define		MAX_KEEPALIVE_DETECT		6						///if not get keeplive when idle, disconnect it
#define		KEEPALIVE_REQUEST_TIMES		3
#define		MAX_BUFF_LENGTH				(DWORD)(128 * 1024)
#define		RTT_FLAT_RATIO				(FLOAT)0.9

template <typename SocketType> class CAcceptorT;
template <typename SocketType> class CAcceptorSinkT;
template <typename SocketType> class CConnectorT;

template <typename PacketType, bool NeedKeepAlive>
class CPacketConnection_T :
	public IQtTransport,				//As a Transport Handle to Upper layer
	public IQtTransportSink,			//As a Sink to TP
	public IQtAcceptorConnectorSink,	//As a ConnectorSink to TP
	public CQtTimerWrapperIDSink,
	public CQtReferenceControlSingleThreadTimerDelete	//Reference tool
{
public:

	typedef BYTE			LINK_STATUS;
	///connection status define
	enum
	{
		LINK_INACTIVE_STATUS = 0,			///the connection not be built
		LINK_PREP_STATUS,					///the connection is be built but handshake is not complete
		LINK_ACTIVE_STATUS,					///the connection can be used now
		LINK_PENDING_STATUS,				///the connection is active, but send data failed
		LINK_BLOCK_STATUS					///the connection is active, but max buff size limit
	};
	
	typedef	BYTE			CONNECTION_TYPE;	///to distinguish the connection is socket client or socket server 
	enum{
		INVALID_CONNECTION,
		CLIENT_CONNECTION,
		SERVER_CONNECTION
	};
	
	virtual DWORD AddReference()
	{
		return CQtReferenceControlSingleThreadTimerDelete::AddReference();
	}

	virtual DWORD ReleaseReference()
	{
		return CQtReferenceControlSingleThreadTimerDelete::ReleaseReference();
	}

public:
    CPacketConnection_T(CONNECTION_TYPE ConnectionType, LINK_TYPE LinkType)
		: m_ConnectionType(ConnectionType)
		, m_LinkType(LinkType)
	{	
		m_dwTotalSendBytes = 0;
		m_dwTotalRcvBytes = 0;
		m_pmbSendData = NULL;
		m_pITransportSink = NULL;
		m_dwAbateTime = LINK_ABATE_TIME;
		m_dwKeepaliveInterval = KEEPALIVE_INTERVAL;
		m_dwRTT = 0;
		m_wRTTCount = 0;
		m_wConnectionID = 0;
		m_wDetectKeepAliveTimes = 0;
		m_bNeedKeepAlive = NeedKeepAlive;
		m_bNeedRTTDetect = FALSE;
		m_bNeedPkgCache = FALSE;
		m_dwMaxBuffLen = MAX_BUFF_LENGTH;
		m_LinkStatus = LINK_INACTIVE_STATUS;
		
		m_PDUType = CS_PDU_TYPE_INVALID;
		m_dwLatestPacketSize = 0;
		m_dwLatestRcvDataLength = 0;
		m_dwAckedSequence = 0;
		m_TransportType = CQtConnectionManager::CTYPE_NONE;
		m_bGetDisconnectPDU = FALSE;
		if(m_bNeedKeepAlive)
		{
			m_KeepaliveTimer.Schedule(this, (LONG)m_dwKeepaliveInterval);
		}
	}

    virtual ~CPacketConnection_T()
	{
		QT_STATE_TRACE_THIS("CPacketConnection_T::~CPacketConnection_T(), ID = " << m_wConnectionID 
			<< " Type = " << m_LinkType << " Status = " << m_LinkStatus << " Total Sent = " << m_dwTotalSendBytes << " Total Received = " << m_dwTotalRcvBytes);
		if(m_pITransport.Get())
		{
			Disconnect();
		}
	}

	void Clear()
	{
		if(m_pmbSendData)
		{
			m_pmbSendData->DestroyChained();
			m_pmbSendData = NULL;
		}

		m_PDUType = CS_PDU_TYPE_INVALID;
		m_dwLatestPacketSize = 0;
	}

	void OnReceiveRTTDetectReq(CKDRPDU  &RTTRequestPDU)
	{
		CKDRPDU RttResponsePDU(CS_PDU_TYPE_RTT_RESP, RTTRequestPDU.GetValue());
		CQtMessageBlock mb(RttResponsePDU.GetFixLength(CS_PDU_TYPE_RTT_RESP));
		RttResponsePDU.Encode(mb);
		if(!m_pmbSendData)
			m_pmbSendData = mb.DuplicateChained();
		else
			m_pmbSendData->Append(mb.DuplicateChained());
		SendData();
	}

	void OnReceiveRTTDetectResp(CKDRPDU  &RTTResponsePDU)
	{
		if(RTTResponsePDU.GetValue() == m_wRTTCount)
		{
			if(0 == m_dwRTT)
				m_dwRTT = (DWORD)m_RTTDetectTick.elapsed();
			else
				m_dwRTT = (DWORD)(m_dwRTT * RTT_FLAT_RATIO + m_RTTDetectTick.elapsed() * (1 - RTT_FLAT_RATIO));
			QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceiveRTTDetectResp my RTT =  " << m_dwRTT << " and new rtt test value = " << m_RTTDetectTick.elapsed());
		}
		else
		{
			QT_WARNING_TRACE_THIS("CPacketConnection_T::OnReceiveRTTDetectResp can not match RTT count, my count = " << 
				m_wRTTCount << " response id in PDU is " << RTTResponsePDU.GetValue());
			m_RTTDetectTick.reset();
		}
	}
	
	BOOL NeedKeepaliveFlag() { 	return TRUE; }

public:				// for timer 

	virtual void OnTimer(CQtTimerWrapperID* aId)
	{
		if(aId == &m_KeepaliveTimer && LINK_ACTIVE_STATUS == m_LinkStatus)
		{
			++m_wDetectKeepAliveTimes;
			if(m_LatestSndTick.overtime_sec(m_dwKeepaliveInterval) 
				|| m_wDetectKeepAliveTimes >= KEEPALIVE_REQUEST_TIMES)
				//need send keepalive 
			{
				KeepAlive();
			}
		}
		else if(aId == &m_RTTDetectTimer && 
			LINK_ACTIVE_STATUS == m_LinkStatus) // to detect the connection's RTT
		{
			CKDRPDU RTTRequestPDU(CS_PDU_TYPE_RTT_REQ, ++m_wRTTCount);
			CQtMessageBlock mb(RTTRequestPDU.GetFixLength(CS_PDU_TYPE_RTT_REQ));
			RTTRequestPDU.Encode(mb);
			if(!m_pmbSendData)
				m_pmbSendData = mb.DuplicateChained();
			else
				m_pmbSendData->Append(mb.DuplicateChained());
			SendData();
			m_RTTDetectTick.reset();
		}
		if(aId == &m_AbateTimer ||	//abate timer overflow
			(m_wDetectKeepAliveTimes >= MAX_KEEPALIVE_DETECT && LINK_ACTIVE_STATUS <= m_LinkStatus)) 	///the connection is overtime, may be already break, destroy my self
		{
			QT_WARNING_TRACE_THIS("CPacketConnection_T::OnTimer, get no packet in abate time");
			CQtComAutoPtr<CPacketConnection_T<PacketType, NeedKeepAlive> > self(this);
			if(m_pITransport.Get())
			{
				m_pITransport->Disconnect(QT_ERROR_NETWORK_ABATE_CONNECTION);
				m_pITransport = NULL;
			}
			if(m_pITransportSink)
			{
				m_pITransportSink->OnDisconnect(QT_ERROR_NETWORK_ABATE_CONNECTION, this);
				m_pITransportSink = NULL;
			}
			Clear();
			m_LinkStatus = LINK_INACTIVE_STATUS;
		}
	}
	
public:
	QtResult OpenWithSink(IQtTransportSink *aSink)
	{
		QT_STATE_TRACE_THIS("CPacketConnection_T::OpenWithSink aSink = " << aSink); 
		m_pITransportSink = aSink;
		return QT_OK;
	}
	
	IQtTransportSink* GetSink(){ return m_pITransportSink;	}
	void SetID(WORD ID)	{	m_wConnectionID = ID;	}
	WORD GetID(){ return m_wConnectionID; }
	
	DWORD GetACKSequence()
	{
		if(m_dwAckedSequence != m_dwTotalRcvBytes)
			m_dwAckedSequence = m_dwTotalRcvBytes;
		return m_dwAckedSequence;
	}
	
	virtual QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *pParameter = NULL)
	{
		if(LINK_ACTIVE_STATUS != m_LinkStatus )
		{
			if(!(RLB_LINK == m_LinkType && LINK_BLOCK_STATUS == m_LinkStatus))
			{
//				QT_WARNING_TRACE_THIS("CPacketConnection_T::SendData, invalid status, status = " << m_LinkStatus);
				if(LINK_PENDING_STATUS == m_LinkStatus)
					m_LinkStatus = LINK_BLOCK_STATUS;
				return QT_ERROR_FAILURE;
			}
		}
		QT_ASSERTE_RETURN(aData.GetChainedLength() > 0, QT_OK);
			
		QtResult ret = SendData();
		
		if(QT_SUCCEEDED(ret) || 
			(m_bNeedPkgCache && m_pmbSendData->GetChainedLength() < m_dwMaxBuffLen))//work for package cache
		{
			DWORD dwDataLen = aData.GetChainedLength();
			m_dwTotalSendBytes += dwDataLen;

			CQtMessageBlock mb(PacketType::GetFixLength(CS_PDU_TYPE_DATA, RLB_LINK == m_LinkType));
			ret = m_DataPDU.Encode(mb, aData, RLB_LINK == m_LinkType, GetACKSequence());
			if(!QT_SUCCEEDED(ret))
			{
				QT_ERROR_TRACE_THIS("CPacketConnection_T::SendData ecode failed!");
				return QT_ERROR_FAILURE;
			}
			if(RLB_LINK == m_LinkType)
			{
				m_pmbSendData = mb.DuplicateChained();
				SendData();
				if(m_dwMaxBuffLen == 0)
					aData.AdvanceChainedReadPtr(aData.GetChainedLength());//Advance upper layer data read ptr

			}
			else
			{
				if(!m_pmbSendData)
				{
					if(QT_FAILED(m_pITransport->SendData(mb)))
					{
						m_pmbSendData = mb.DuplicateChained();
						if(!m_pmbSendData)
						{
							QT_ERROR_TRACE_THIS("CPacketConnection_T::SendData out of memory");
							OnDisconnect(QT_ERROR_NOT_AVAILABLE, m_pITransport.Get());
							return QT_ERROR_FAILURE;
						}
					}
				}
				else // for need buff package
				{
					m_pmbSendData->Append(mb.DuplicateChained());
					SendData();
				}
				aData.AdvanceChainedReadPtr(aData.GetChainedLength());//Advance upper layer data read ptr
			}
			if(pParameter)
				pParameter->m_dwHaveSent = dwDataLen;
		}
		return ret;	
	}
	
	/// the <aCommand>s are all listed in file QtErrorNetwork.h
	QtResult SetOption(DWORD aCommand, LPVOID aArg)
	{
		QT_STATE_TRACE_THIS("CPacketConnection_T::SetOption command = " << aCommand 
			<< " arg = " << aArg);

		switch(aCommand) {
		case CS_OPT_NEED_KEEPALIVE:
			if(*static_cast< BOOL * >(aArg) == TRUE) {
				QT_ASSERTE_RETURN(LEN_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
				QT_ASSERTE_RETURN(SENDER_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
			}
			if(m_bNeedKeepAlive != *static_cast< BOOL * >(aArg) )
			{
				if(m_bNeedKeepAlive) {
					m_KeepaliveTimer.Cancel();
				}
				else {
					m_KeepaliveTimer.Schedule(this, (LONG)m_dwKeepaliveInterval);
				}
				m_bNeedKeepAlive = *static_cast< BOOL * >(aArg);
			}
			break;
		case CS_OPT_KEEPALIVE_INTERVAL:
			if(m_bNeedKeepAlive) {
				QT_ASSERTE_RETURN(LEN_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
				QT_ASSERTE_RETURN(SENDER_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
			}
			QT_ASSERTE_RETURN(*static_cast< DWORD * >(aArg) > 0, QT_ERROR_FAILURE);
			m_dwKeepaliveInterval = *static_cast< DWORD * >(aArg);
			break;
		case CS_OPT_ABATE_TIME:
			QT_ASSERTE_RETURN(*static_cast< DWORD * >(aArg) > 0, QT_ERROR_FAILURE);
			m_dwAbateTime = *static_cast< DWORD * >(aArg);
			break;
		case CS_OPT_DETECT_RTT:
			QT_ASSERTE_RETURN(SENDER_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
			QT_ASSERTE_RETURN(LEN_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
			if(m_bNeedRTTDetect !=  *static_cast< BOOL * >(aArg)) {
				if(!m_bNeedRTTDetect) {
					m_RTTDetectTimer.Schedule(this, (LONG)LINK_RTT_DETECT_INTERVAL);
				}
				else {
					m_RTTDetectTimer.Cancel();
				}
			}
			m_bNeedRTTDetect = *static_cast< BOOL * >(aArg);
			
			break;
		case CS_OPT_PKG_NEED_BUF:
			QT_ASSERTE_RETURN(PKG_LINK == m_LinkType, QT_ERROR_FAILURE);
			m_bNeedPkgCache = *static_cast< BOOL * >(aArg);
			break;
		case CS_OPT_MAX_SENDBUF_LEN:
			QT_ASSERTE_RETURN(m_dwMaxBuffLen > 0, QT_ERROR_FAILURE);
			m_dwMaxBuffLen = *static_cast< DWORD * >(aArg);
			break;
		default:
			if(m_pITransport.Get()) {
				m_pITransport->SetOption(aCommand, aArg);
			}
			else {
				QT_WARNING_TRACE_THIS("CPacketConnection_T::SetOption transport is NULL");
			}
		}
		return QT_OK;
	}

	QtResult GetOption(DWORD aCommand, LPVOID aArg)
	{
		QT_STATE_TRACE_THIS("CPacketConnection_T::GetOption command = " << aCommand 
			<< " arg = " << aArg);
		switch(aCommand) {
		case CS_OPT_NEED_KEEPALIVE:
			QT_ASSERTE_RETURN(LEN_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
			QT_ASSERTE_RETURN(SENDER_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
			*static_cast< BOOL * >(aArg) = m_bNeedKeepAlive;
			break;
		case CS_OPT_KEEPALIVE_INTERVAL:
			QT_ASSERTE_RETURN(LEN_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
			*static_cast< DWORD * >(aArg) = m_dwKeepaliveInterval;
			break;
		case CS_OPT_ABATE_TIME:
			*static_cast< DWORD * >(aArg) = m_dwAbateTime;
			break;

		case CS_OPT_INQUIRE_RTT:
			QT_ASSERTE_RETURN(LEN_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
			*static_cast< WORD * >(aArg) = m_dwRTT;
			break;

		case CS_OPT_DETECT_RTT:
			QT_ASSERTE_RETURN(LEN_PKG_LINK != m_LinkType, QT_ERROR_FAILURE);
			 *static_cast< BOOL * >(aArg) = m_bNeedRTTDetect;
			 break;
			
		case QT_OPT_TRANSPORT_TRAN_TYPE:
			if(m_pITransport.Get())	{
				if(m_pITransport.Get()) {
					CQtConnectionManager::CType TransportType = m_TransportType;
					if(m_bNeedKeepAlive)
						TransportType |= CQtConnectionManager::CTYPE_PDU_KEEPALIVE;
					switch(m_LinkType) {
					case REC_LINK:
						TransportType |= CQtConnectionManager::CTYPE_PDU_RECONNECT;
						break;
					case RLB_LINK:
						TransportType |= CQtConnectionManager::CTYPE_PDU_RELIABLE;
						break;
					case PKG_LINK:
						TransportType |= CQtConnectionManager::CTYPE_PDU_PACKAGE;
						break;
					case LEN_PKG_LINK:
						TransportType |= CQtConnectionManager::CTYPE_PDU_LENGTH;
						break;
					case SENDER_PKG_LINK:
						TransportType |= CQtConnectionManager::CTYPE_SEND_NO_PARTIAL_DATA;
						break;
					}
					*(static_cast<CQtConnectionManager::CType*>(aArg)) =  TransportType;
					
					return QT_OK;
				}
			}
			
			return QT_ERROR_FAILURE;
		case CS_OPT_PKG_NEED_BUF:
			QT_ASSERTE_RETURN(PKG_LINK == m_LinkType, QT_ERROR_FAILURE);
			 *static_cast< BOOL * >(aArg) = m_bNeedPkgCache;
			break;
		case CS_OPT_MAX_SENDBUF_LEN:
			QT_ASSERTE_RETURN(*static_cast< DWORD * >(aArg) > 0, QT_ERROR_FAILURE);
			*static_cast< DWORD * >(aArg) = m_dwMaxBuffLen;
			break;
		default:
			if(m_pITransport.Get()) {
				m_pITransport->GetOption(aCommand, aArg);
			}
			else {
				QT_WARNING_TRACE_THIS("CPacketConnection_T::GetOption transport is NULL");
				return QT_ERROR_FAILURE;
			}
		}
		return QT_OK;	
	}
	
	/// Disconnect the connection, and will not callback <IQtTransportSink> longer.
	QtResult Disconnect(QtResult aReason = 0)
	{
		QT_STATE_TRACE_THIS("CPacketConnection_T::Disconnect reason = " << aReason << " transport = " << m_pITransport.Get() << " ID = " << m_wConnectionID);
		m_AbateTimer.Cancel();
		QtResult ret;
		if(m_pITransport.Get())
		{
			//send disconnect PDU first, then disconnect the connection
			CDisconnectPDU DisconnectPDU(/*CS_PDU_TYPE_DISCONN, */aReason);
			CQtMessageBlock mb(DisconnectPDU.GetFixLength(CS_PDU_TYPE_DISCONN));
			if(QT_FAILED(DisconnectPDU.Encode(mb)))	{
				ret =  QT_ERROR_FAILURE;
			}
			else {
				if(!m_pmbSendData)
					m_pmbSendData = mb.DuplicateChained();
				else
					m_pmbSendData->Append(mb.DuplicateChained());
				ret = SendData();
			}
			m_pITransport->Disconnect(0);
		}
		m_LinkStatus = LINK_INACTIVE_STATUS;
		m_pITransport = NULL;
		m_pITransportSink = NULL;
		Clear();
		return ret;
	}
	
	virtual QtResult Attach(IQtTransport *pTransport, DWORD dwAck)
	{
		QT_ASSERTE(SERVER_CONNECTION == m_ConnectionType);
		QT_ASSERTE(pTransport);
		QT_ASSERTE(REC_LINK == m_LinkType); //only work for reconnect connection
		if(m_pmbSendData) {
			m_pmbSendData->DestroyChained();
			m_pmbSendData = NULL;
		}
		
		if(m_pITransport.Get()) {
			m_pITransport->Disconnect(QT_ERROR_NETWORK_CONNECTION_RECONNECT);
		}
		m_pITransport = pTransport;
		m_pITransport->OpenWithSink(this);
		m_AbateTimer.Cancel();
		LINK_STATUS latestLinkStatus = m_LinkStatus;
		OnACK(dwAck);
		m_LinkStatus = LINK_ACTIVE_STATUS;
		if(LINK_PENDING_STATUS == latestLinkStatus)	{
			OnSend(m_pITransport.Get());
		}
		return QT_OK;
	}
	
	///only work for client
	void CancelConnect(){QT_ASSERTE(CLIENT_CONNECTION == m_ConnectionType);}


	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *pITransport,
		IQtAcceptorConnectorId *aRequestId)
	{

		QT_STATE_TRACE_THIS("CPacketConnection_T::OnConnectIndication aReason = " << aReason << 
			" transport = " << pITransport );
		
		if(QT_SUCCEEDED(aReason))	{
			m_RTTDetectTick.reset();
			CQtConnectionManager::CType lTransport;
			QtResult ret = pITransport->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, &lTransport);
			QT_ASSERTE(QT_SUCCEEDED(ret));
			m_TransportType = lTransport;
			if(LINK_INACTIVE_STATUS == m_LinkStatus)
				m_LinkStatus = LINK_PREP_STATUS;
			QT_ASSERTE_RETURN_VOID(pITransport);
			m_pITransport = pITransport;
			m_pITransport->OpenWithSink(this);
		}
		//otherwise, the instance will destroy when abate timer overflow
	}
		
	void EliminateIntactPacket(CQtMessageBlock &aData,
		CQtTransportParameter *pParameter = NULL)  //Eliminate intact packet first
	{
		QtResult ret = PacketType::PeekType(static_cast<void *>(const_cast<char *>(aData.GetTopLevelReadPtr())), m_PDUType);
		if(QT_SUCCEEDED(ret))
			ret = ((DWORD)PacketType::GetFixLength(m_PDUType, RLB_LINK == m_LinkType) == aData.GetChainedLength() ? QT_OK : QT_ERROR_FAILURE);
		if(QT_SUCCEEDED(ret))
		{
			switch(m_PDUType)
			{
			case CS_PDU_TYPE_DATA_OLD_VERSION:
			case CS_PDU_TYPE_DATA:
				ret = m_DataPDU.DecodeWithoutData(aData, RLB_LINK == m_LinkType);
				if(QT_OK == ret)	{
					if(m_pITransportSink) {
						///separate the packet's data from receive data
						m_pITransportSink->OnReceive(aData, this, pParameter);
					}
					else {
						QT_ERROR_TRACE_THIS("CCsConn::OnReceive, can't get the sink,discard data, length =" << m_DataPDU.GetDataLen());
					}
				}
				aData.AdvanceChainedReadPtr(aData.GetChainedLength());
				//add for compatible to old version tp, Victor 7/6
				m_dwTotalRcvBytes +=  (m_dwLatestPacketSize + CPacketPDU::GetFixLength(CS_PDU_TYPE_DATA, RLB_LINK == m_LinkType)) ;
				OnACK(m_DataPDU.GetAck());
				//add for compatible to old version tp, Victor 7/6
				/// to check need ACK or not
				if(m_dwMaxBuffLen > 0 && RLB_LINK == m_LinkType && MAX_ACK_DATA_PIECE < (m_dwTotalRcvBytes - m_dwAckedSequence)){
					KeepAlive();//ack it with keep alive PDU
				} 
				break;
			case CS_PDU_TYPE_CONN_REQ:
				{
					CReqPDU reqPDU(m_LinkType, RLB_LINK & m_LinkType);
					ret = reqPDU.Decode(aData);
					if(QT_SUCCEEDED(ret)) {
						QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive connect request PDU, id = " << reqPDU.GetTag() 
							<< " type = " << reqPDU.GetType());
						OnReceiveConnectRequest(reqPDU);
					}
					else {
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive connect request PDU, but decode failed!" );
					}
				}
				break;
			case CS_PDU_TYPE_CONN_RESP:
				{
					CRespPDU respPDU(RLB_LINK == m_LinkType);
					ret = respPDU.Decode(aData);
					if(QT_SUCCEEDED(ret)){
						m_dwRTT = m_RTTDetectTick.elapsed();
						QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive connect response PDU, id = " << respPDU.GetTag());
						this->OnReceiveConnectResponse(respPDU);
					}
					else{
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive connect response PDU, but decode failed!" );
					}
				}
				break;
			case CS_PDU_TYPE_DISCONN:
				{
					CQtComAutoPtr<CPacketConnection_T<PacketType, NeedKeepAlive> > self(this);
					CDisconnectPDU DisconnecPDU/*(CS_PDU_TYPE_DISCONN)*/;
					DisconnecPDU.Decode(aData);
					m_bGetDisconnectPDU = TRUE;
					QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive disconnect PDU, reason = " << DisconnecPDU.GetValue());
					OnDisconnect(DisconnecPDU.GetValue());
					return;
				}
			case CS_PDU_TYPE_KEEPALIVE:
				{
					CKDRPDU KeepAlivePDU(CS_PDU_TYPE_KEEPALIVE);
					ret = KeepAlivePDU.Decode(m_RcvBuff);
					if(QT_FAILED(ret)){
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive RTT Request PDU, decode failed!");
						break;
					}
					OnKeepAlive(KeepAlivePDU);
				}
				break;
			case CS_PDU_TYPE_RTT_REQ:
				{
					CKDRPDU RttPDU(CS_PDU_TYPE_RTT_REQ);
					ret = RttPDU.Decode(m_RcvBuff);
					if(QT_FAILED(ret))	{
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive RTT Request PDU, decode failed!");
						break;
					}
					QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive RTT Request PDU, times = " << RttPDU.GetValue());
					OnReceiveRTTDetectReq(RttPDU);
				}
				break;
				
			case CS_PDU_TYPE_RTT_RESP:
				{
					CKDRPDU RttPDU(CS_PDU_TYPE_RTT_RESP);
					ret = RttPDU.Decode(m_RcvBuff);
					if(QT_FAILED(ret))	{
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive RTT Response PDU decode failed!");
						break;
					}
					QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive RTT Response PDU, response ID = " << RttPDU.GetValue());
					OnReceiveRTTDetectResp(RttPDU);
				}
				break;
			default:
				break;
			}
		}
		m_PDUType = CS_PDU_TYPE_INVALID;
		if(	QT_FAILED(ret) )	{
			QT_ERROR_TRACE_THIS("CPacketConnection_T::DealPDUPacket failed!");
			CQtComAutoPtr<CPacketConnection_T<PacketType, NeedKeepAlive> > self(this);
			OnDisconnect(QT_ERROR_NOT_AVAILABLE, m_pITransport.Get());
		}
	}
    /**
     * @param aData     the data be received from raw connection
     * @param pTransport   the raw connection who received the data
     * @param pParameter     the parameter
     */
	void OnReceive(CQtMessageBlock &aData, IQtTransport *pTransport,
		CQtTransportParameter *pParameter = NULL)
	{
//		QT_INFO_TRACE_THIS("CPacketConnection_T::OnReceive transport = " << pTransport 
//			<< " Parameter = " << pParameter << " Status = " << m_LinkStatus << " length = " << aData.GetChainedLength());
		QT_ASSERTE_RETURN_VOID(pTransport == m_pITransport.Get());
		QT_ASSERTE_RETURN_VOID(LINK_INACTIVE_STATUS != m_LinkStatus);
		/*if(CQtConnectionManager::CTYPE_UDP == m_TransportType) ///already is packet
		{
			EliminateIntactPacket(aData);
			return;
		}*/


		DWORD dwReceiveDataLen = aData.GetChainedLength();
		if(QT_FAILED(m_RcvBuff.resize(m_RcvBuff.m_dwCursor + dwReceiveDataLen))) {
			QT_ERROR_TRACE_THIS("_T::OnReceive out of memory");
			aData.AdvanceChainedReadPtr(dwReceiveDataLen);
			return;
		}
		
		aData.Read((LPVOID)((char *)m_RcvBuff.m_pBuff + m_RcvBuff.m_dwCursor), dwReceiveDataLen);
		m_RcvBuff.m_dwCursor += dwReceiveDataLen;

		m_LatestRcvTick.reset();
		m_wDetectKeepAliveTimes = 0;
		
		QtResult ret = QT_OK;
		int nFixPDULength;
		while(m_RcvBuff.m_dwCursor > 0)	{
			///if type is be reset, peek type from PDU
			if(CS_PDU_TYPE_INVALID == m_PDUType){
				PacketType::PeekType(m_RcvBuff.m_pBuff, m_PDUType);
			}
			if(CS_PDU_TYPE_INVALID == m_PDUType){
				QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive invalid PDU type = " << m_PDUType << " Status = " << m_LinkStatus << " data len = " << dwReceiveDataLen);
				ret = QT_ERROR_FAILURE;
			}
			else{
				nFixPDULength = PacketType::GetFixLength(m_PDUType, RLB_LINK == m_LinkType);
				if(nFixPDULength < 0) {
					QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive invalid PDU type = " << m_PDUType << " Status = " << m_LinkStatus << " data len = " << dwReceiveDataLen);
					ret = QT_ERROR_FAILURE;
				}
				if(nFixPDULength > (int)m_RcvBuff.m_dwCursor) {
					//the data length is less than the PDU need, waiting for the latter data arrived
					break;
				}
			}

			BOOL bPartData = FALSE;
			switch(m_PDUType) {
			case CS_PDU_TYPE_DATA_OLD_VERSION:
			case CS_PDU_TYPE_DATA:
				if(m_dwLatestPacketSize == 0) {
					m_DataPDU.DecodeWithoutData(m_RcvBuff.m_pBuff, RLB_LINK == m_LinkType);
				 	m_dwLatestPacketSize = m_DataPDU.GetDataLen();
				}
					
				if(m_RcvBuff.m_dwCursor >= (m_dwLatestPacketSize + nFixPDULength))
				{
					if(m_pITransportSink) {
						///separate the packet's data from receive data
						if(m_dwLatestPacketSize > 0) {
							CQtMessageBlock bk(m_dwLatestPacketSize, 
								(char *)m_RcvBuff.m_pBuff + nFixPDULength, 
								CQtMessageBlock::DONT_DELETE, m_dwLatestPacketSize);
							m_pITransportSink->OnReceive(bk, this, pParameter);
						}
						else {
							QT_WARNING_TRACE_THIS("CPacketConnection_T::::OnReceive get the data length is 0");
						}
					}
					else {
						QT_ERROR_TRACE_THIS("CCsConn::OnReceive, can't get the sink,discard data, length =" << m_DataPDU.GetDataLen());
					}
					//add for compatible to old version tp, Victor 7/6
					m_dwTotalRcvBytes +=  (m_dwLatestPacketSize + CPacketPDU::GetFixLength(CS_PDU_TYPE_DATA, RLB_LINK == m_LinkType)) ;
					// if RLB_LINK == m_LinkType
					// to remove data from buff if the connection is rlb connection
					OnACK(m_DataPDU.GetAck());

					DWORD dwNeedRemoveLen = nFixPDULength + m_dwLatestPacketSize;
					int nRemained = m_RcvBuff.m_dwCursor - dwNeedRemoveLen;
					if(nRemained > 0)
						memmove(m_RcvBuff.m_pBuff, (void *)((char *)m_RcvBuff.m_pBuff + dwNeedRemoveLen), nRemained);					
					m_RcvBuff.m_dwCursor -= dwNeedRemoveLen;

//					int nLastReceive = m_dwLatestPacketSize - m_dwLatestRcvDataLength;
//					m_dwTotalRcvBytes +=  nLastReceive ; // for compatible to old version victor 7/6
					m_dwLatestPacketSize = 0;					//reset the PDU length
					m_dwLatestRcvDataLength = 0;				//reset the latest rcv data length

				}
				else {/// receive part of the packet
					DWORD dwLatestLength = m_RcvBuff.m_dwCursor - nFixPDULength;
					m_dwTotalRcvBytes += (dwLatestLength - m_dwLatestRcvDataLength);
					m_dwLatestRcvDataLength = dwLatestLength;
					bPartData = TRUE;
				}
				

				/// to check need ACK or not
				if(m_dwMaxBuffLen > 0 && RLB_LINK == m_LinkType && MAX_ACK_DATA_PIECE < (m_dwTotalRcvBytes - m_dwAckedSequence)){
					KeepAlive();//ack it with keep alive PDU
				} 
				break;

			case CS_PDU_TYPE_CONN_REQ:
				{
					CReqPDU reqPDU(m_LinkType, RLB_LINK & m_LinkType);
					ret = reqPDU.Decode(m_RcvBuff);
					if(QT_SUCCEEDED(ret)) {
						QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive connect request PDU, id = " << reqPDU.GetTag() 
							<< " type = " << reqPDU.GetType());
						OnReceiveConnectRequest(reqPDU);
					}
					else {
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive connect request PDU, but decode failed!" );
					}
				}
				break;
				
			case CS_PDU_TYPE_CONN_RESP:
				{
					CRespPDU respPDU(RLB_LINK == m_LinkType);
					ret = respPDU.Decode(m_RcvBuff);
					if(QT_SUCCEEDED(ret)){
						m_dwRTT = m_RTTDetectTick.elapsed();
						QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive connect response PDU, id = " << respPDU.GetTag());
						this->OnReceiveConnectResponse(respPDU);
					}
					else{
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive connect response PDU, but decode failed!" );
					}
				}
				break;

			case CS_PDU_TYPE_DISCONN:
				{
					CQtComAutoPtr<CPacketConnection_T<PacketType, NeedKeepAlive> > self(this);
					CDisconnectPDU DisconnecPDU/*(CS_PDU_TYPE_DISCONN)*/;
					DisconnecPDU.Decode(m_RcvBuff);
					m_bGetDisconnectPDU = TRUE;
					QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive disconnect PDU, reason = " << DisconnecPDU.GetValue());
					OnDisconnect(DisconnecPDU.GetValue());
					return;
				}
				break;

			case CS_PDU_TYPE_KEEPALIVE:
				{
//					QT_INFO_TRACE_THIS("CPacketConnection_T::OnReceive before keepalive PDU left length = " << m_pmbRcvData->GetChainedLength());
					CKDRPDU KeepAlivePDU(CS_PDU_TYPE_KEEPALIVE);
					ret = KeepAlivePDU.Decode(m_RcvBuff);
					if(QT_FAILED(ret)){
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive RTT Request PDU, decode failed!");
						break;
					}
//					QT_INFO_TRACE_THIS("CPacketConnection_T::OnReceive keepalive PDU, values = " << KeepAlivePDU.GetValue() << " left length = " << m_pmbRcvData->GetChainedLength());
					OnKeepAlive(KeepAlivePDU);
				}
				break;

			case CS_PDU_TYPE_RTT_REQ:
				{
					CKDRPDU RttPDU(CS_PDU_TYPE_RTT_REQ);
					ret = RttPDU.Decode(m_RcvBuff);
					if(QT_FAILED(ret))	{
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive RTT Request PDU, decode failed!");
						break;
					}
					QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive RTT Request PDU, times = " << RttPDU.GetValue());
					OnReceiveRTTDetectReq(RttPDU);
				}
				break;

			case CS_PDU_TYPE_RTT_RESP:
				{
					CKDRPDU RttPDU(CS_PDU_TYPE_RTT_RESP);
					ret = RttPDU.Decode(m_RcvBuff);
					if(QT_FAILED(ret))	{
						QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceive RTT Response PDU decode failed!");
						break;
					}
					QT_STATE_TRACE_THIS("CPacketConnection_T::OnReceive RTT Response PDU, response ID = " << RttPDU.GetValue());
					OnReceiveRTTDetectResp(RttPDU);
				}
				break;
				
			default:
				break;
			}
			if(bPartData)
				break;
			
			m_PDUType = CS_PDU_TYPE_INVALID;
			if(	QT_FAILED(ret) )	{
				CQtComAutoPtr<CPacketConnection_T<PacketType, NeedKeepAlive> > self(this);
				OnDisconnect(QT_ERROR_NOT_AVAILABLE, m_pITransport.Get());
				return;
			}
		}
	}
	
	virtual void OnSend(IQtTransport *pTransport, CQtTransportParameter *pParameter = NULL)
	{
		QT_STATE_TRACE_THIS("CPacketConnection_T::OnSend transport = " << pTransport 
			<< " Parameter = " << pParameter << " Status = " << m_LinkStatus);
		
		QT_ASSERTE(LINK_INACTIVE_STATUS != m_LinkStatus);
		QT_ASSERTE(LINK_PREP_STATUS != m_LinkStatus);
		m_LinkStatus = LINK_ACTIVE_STATUS;
		SendData();
		m_pITransportSink->OnSend(this, pParameter);
	}

	virtual void OnDisconnect(QtResult aReason)
	{
		QT_STATE_TRACE_THIS("CPacketConnection_T::OnDisconnect Reason = " << aReason );
		this->OnDisconnect(aReason, m_pITransport.Get());
	}
    /**
     * @param aReason   the raw connection disconnect reason
     * @param pTransport   the raw connection be broken
     */
	virtual void OnDisconnect(QtResult aReason, IQtTransport *pTransport)
	{

		QT_STATE_TRACE_THIS("CPacketConnection_T::OnDisconnect transport = " <<	pTransport 
			<< " Reason = " << aReason << " Status = " << m_LinkStatus);

		QT_ASSERTE(m_pITransport == pTransport);
		if(m_LinkStatus = LINK_INACTIVE_STATUS)
			return;
		CQtComAutoPtr<CPacketConnection_T<PacketType, NeedKeepAlive> > self(this);
		if(m_pITransport.Get())	{
			m_pITransport->Disconnect((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
		}
		if(m_pITransportSink){
			m_pITransportSink->OnDisconnect(aReason, this);//Network error, notify upper layer
		}
		m_pITransport = NULL;
		m_pITransportSink = NULL;
		Clear();
		m_LinkStatus = LINK_INACTIVE_STATUS;
		m_AbateTimer.Cancel();
		m_KeepaliveTimer.Cancel();
	}

protected:

	void SetRTT(DWORD dwRTTValue) /// for server connection first built
	{ m_dwRTT = dwRTTValue;	}

	QtResult KeepAlive()
	{
		if(LINK_ACTIVE_STATUS != m_LinkStatus   ///connection is invalid
			|| !NeedKeepaliveFlag()				/// need not keep alive flag be set
			|| m_pmbSendData)					/// some data be cached
		{
			//QT_WARNING_TRACE_THIS("CPacketConnection_T::KeepAlive status is invalid, status = " << m_LinkStatus);
			return QT_ERROR_FAILURE;
		}
		
		CKDRPDU keepAlivePDU(CS_PDU_TYPE_KEEPALIVE, GetACKSequence());
		// QT_INFO_TRACE_THIS(" send keep alive value = " << m_dwTotalRcvBytes);
		CQtMessageBlock mb(keepAlivePDU.GetFixLength(CS_PDU_TYPE_KEEPALIVE));
		if(!QT_SUCCEEDED(keepAlivePDU.Encode(mb)))
			return QT_ERROR_FAILURE;
		
		if(!m_pmbSendData)
			m_pmbSendData = mb.DuplicateChained();
		else
			m_pmbSendData->Append(mb.DuplicateChained());
		SendData();
		return QT_OK;
	}
	
	virtual void OnKeepAlive(CKDRPDU &keepAlivePDU)
	{
		QT_STATE_TRACE_THIS("::OnKeepAlive receive keep alive, value = " << keepAlivePDU.GetValue());
		if(!m_bNeedKeepAlive) {
			//if I am not supply keepalive, I must echo it
			if(m_LatestSndTick.overtime_sec(KEEPALIVE_INTERVAL)){
				KeepAlive();
			}
		}
		OnACK(keepAlivePDU.GetValue());
	}
	
	virtual void OnACK(DWORD dwACKSequence)
	{}
	
	virtual void OnReceiveConnectRequest(CReqPDU &requestPDU)
	{
		QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceiveConnectRequest");
		QT_ASSERTE(FALSE);
	}
	
	virtual void OnReceiveConnectResponse(CRespPDU &responsePDU)
	{
		QT_ERROR_TRACE_THIS("CPacketConnection_T::OnReceiveConnectResponse");
		QT_ASSERTE(FALSE);
	}	
	///send data directly without building PDU
	QtResult SendData()
	{
		QtResult ret = QT_OK;
		if(!m_pmbSendData)
			return ret;
		m_LatestSndTick.reset();
		if(!m_pITransport.Get())
		{
			QT_WARNING_TRACE_THIS("_T::SendData() transport already be closed!");
			return QT_ERROR_FAILURE;
		}
		ret = m_pITransport->SendData(*m_pmbSendData, &m_TransportParameter);
		if(QT_SUCCEEDED(ret))
		{
			m_pmbSendData->DestroyChained();
			m_pmbSendData = NULL;
		}
		else if( CQtConnectionManager::CTYPE_UDP != (m_TransportType & 0x0000FFFF) &&
			QT_ERROR_PARTIAL_DATA == ret && LINK_ACTIVE_STATUS == m_LinkStatus)
		{
			///if the connection is base TCP or others that base on TCP, block the connection
			///and wait for onsend event to active the connections 
			QT_WARNING_TRACE_THIS("CPacketConnection_T::SendData link status is covert from active to block");
			m_LinkStatus = LINK_BLOCK_STATUS;
		}
		return ret;		
	}

protected:
	
	friend class CConnectionAdaptor;
	const CONNECTION_TYPE		m_ConnectionType;

	DWORD				m_dwTotalSendBytes;		//for stat all sent data
	DWORD				m_dwTotalRcvBytes;		//for stat all received data
	
	CQtMessageBlock		*m_pmbSendData;			//to cache the send block
	IQtTransportSink	*m_pITransportSink;		//ITransportSink obj from Upper layer
	CQtComAutoPtr<IQtTransport> m_pITransport;	//ITransport obj from TP layer
	CQtTransportParameter	m_TransportParameter;

	//////////////////////////////////////////////////////////////////////////
	ticker				m_LatestRcvTick;		//when receive any data, reset it
	ticker				m_LatestSndTick;		//when send any data, reset it
	ticker				m_RTTDetectTick;		//when start a rtt request, reset it
	
	CQtTimerWrapperID m_AbateTimer;				//abate timer
	CQtTimerWrapperID m_KeepaliveTimer;			//keepalive timer
	CQtTimerWrapperID m_RTTDetectTimer;			//to detect RTT timer;
	
	DWORD				m_dwAbateTime;			//abate time
	DWORD				m_dwKeepaliveInterval;
	DWORD				m_dwRTT;		
	WORD				m_wRTTCount;			//to identifier RTT, used to match request with response
	WORD				m_wConnectionID;
	WORD				m_wDetectKeepAliveTimes;
	BOOL				m_bNeedKeepAlive;
	BOOL				m_bNeedRTTDetect;
	BOOL				m_bNeedPkgCache;
	DWORD				m_dwMaxBuffLen;
	//////////////////////////////////////////////////////////////////////////
	LINK_TYPE			m_LinkType;
	LINK_STATUS			m_LinkStatus;
	
	//////////////////////////////////////////////////////////////////////////
	PacketType			m_DataPDU;

	PACKET_TYPE			m_PDUType;
	DWORD				m_dwLatestPacketSize;				///to stat the lastest packet size
	DWORD				m_dwLatestRcvDataLength;
	DWORD				m_dwAckedSequence;

	//////////////////////////////////////////////////////////////////////////
	CQtConnectionManager::CType		m_TransportType;			//the raw link type, such as tcp, udp and so on
	BOOL				m_bGetDisconnectPDU;

	TBuff				m_RcvBuff;

};

//////////////////////////////////////////////////////////////////////////
//class CConnectionList, only can hold 65535 connections on server side
//////////////////////////////////////////////////////////////////////////
#define MAX_CONNECTION_CAN_HOLD		65535
#define INVALID_CONNECTION_ID		0
#define INIT_CONNECTION_ID			1

typedef CPacketConnection_T<CDataPktPDU, true> PktConnectionWithKeepAlive_T;

template <typename ConnectionType>
class QT_OS_EXPORT CConnectionList
{
public:
	CConnectionList()
		:m_ConnectionList(MAX_CONNECTION_CAN_HOLD), m_wID(INVALID_CONNECTION_ID)
	{
		QT_ASSERTE(m_ConnectionList.size() == MAX_CONNECTION_CAN_HOLD);
		m_bFirstRound = TRUE;
	}
	
	virtual ~CConnectionList()
	{
		for(unsigned i = 0; i < m_ConnectionList.size(); ++i) {
			if(m_ConnectionList[i].Get()) {
				m_ConnectionList[i]->OnDisconnect(QT_ERROR_NETWORK_SOCKET_CLOSE);
				m_ConnectionList[i] = NULL;
			}
		}
	};
	
	//Add Server & get a new Channel# for it, Return the Channel#
	WORD AddConnection(ConnectionType* pConnection)
	{
		QT_ASSERTE_RETURN(pConnection, INVALID_CONNECTION_ID);
		QT_ASSERTE(pConnection->GetID() == INVALID_CONNECTION_ID);
		CQtComAutoPtr<ConnectionType> lConnection(pConnection);
		if(m_bFirstRound)	{
			if((++m_wID) > MAX_CONNECTION_CAN_HOLD)	{
				m_bFirstRound = FALSE;
			}
			else {
				QT_ASSERTE(!m_ConnectionList[m_wID - 1].Get());
				QT_STATE_TRACE_THIS("CConnectionList::AddConnection channel id = " << m_wID);
				pConnection->SetID(m_wID);
				m_ConnectionList[m_wID - 1] = lConnection;
				return pConnection->GetID();
			}
		}
		if(!m_bFirstRound)	{
			if(m_FreeList.empty()) {
				QT_WARNING_TRACE_THIS("CConnectionList::AddConnection connection list is full!");
				return INVALID_CONNECTION_ID;
			}
			QT_ASSERTE(!m_ConnectionList[m_FreeList.front() - 1].Get());
			QT_STATE_TRACE_THIS("CConnectionList::AddConnection channel id = " << m_FreeList.front());
			pConnection->SetID(m_FreeList.front());
			m_ConnectionList[m_FreeList.front() - 1] = lConnection;
			m_FreeList.pop();
			return pConnection->GetID();
		}
		QT_ASSERTE_RETURN(FALSE, INVALID_CONNECTION_ID);
	};
	
	CQtComAutoPtr<ConnectionType> GetConnection(WORD wID)
	{
		CQtComAutoPtr<ConnectionType> pConnection = m_ConnectionList[wID - 1];
		if(pConnection.Get())	{
			QT_STATE_TRACE_THIS("CConnectionList::GetConnection(), Find the connection, ID = " << wID << " connection = " << m_ConnectionList[wID - 1].Get());
		}
		else {
			QT_STATE_TRACE_THIS("CConnectionList::GetConnection(), can not get the connection, ID = " << wID);
		}
		return pConnection;
	};
	void RemoveConnection(WORD wID)
	{
		if(wID > INVALID_CONNECTION_ID && m_ConnectionList[wID - 1].Get()) {
			QT_STATE_TRACE_THIS("CConnectionList::RemoveConnection ID = " << wID << " Transport = " << m_ConnectionList[wID - 1].Get());
			m_ConnectionList[wID - 1] = NULL;
			m_FreeList.push(wID);
		}
	}

	void RemoveConnection(PktConnectionWithKeepAlive_T *pConnection)
	{
		QT_ASSERTE_RETURN_VOID(pConnection);
		RemoveConnection(pConnection->GetID());
	}
	
	WORD ChannelCalculator()
	{
		m_wID++;
		if(m_wID > MAX_CONNECTION_CAN_HOLD || 
			m_wID == INVALID_CONNECTION_ID ||
			m_wID < INIT_CONNECTION_ID)
			m_wID = INIT_CONNECTION_ID;
		return m_wID;
	}
	
protected:
	std::queue<WORD>	m_FreeList;	///hold the free list
	std::vector< CQtComAutoPtr<ConnectionType> > m_ConnectionList; //hold the connection on server side that need reconnect
	WORD m_wID;			///to store connection ID
	BOOL m_bFirstRound;
};

typedef CConnectionList<PktConnectionWithKeepAlive_T> ConnectionList_T;

///////////////////////////////////////////
//class CConnectorT
///////////////////////////////////////////

/// interface first
class QT_OS_EXPORT IConnectorT : public IQtConnector
{
public:
	virtual ~IConnectorT(){}
	virtual void AsycConnect(
		IQtAcceptorConnectorSink *aSink,	//IConnectorSink From Upper layer
		const CQtInetAddr &aAddrPeer, 
		CQtTimeValue *aTimeout = NULL,
		CQtInetAddr *aAddrLocal = NULL) = 0;

	virtual void AsycReconnect(IQtAcceptorConnectorSink *aSink, CQtTimeValue *aTimeout = NULL) = 0;
	virtual void CancelConnect() = 0;
	virtual BOOL IsConnector() = 0;

	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *pITransport,
		IQtAcceptorConnectorId *aRequestId) = 0;
		
};

template <typename ConnectorType>
class QT_OS_EXPORT CConnectorT :
			public IConnectorT, 
			public CQtReferenceControlSingleThread	//Reference tool
{
public:
	virtual void AsycConnect(
		IQtAcceptorConnectorSink *aSink,	//IConnectorSink From Upper layer
		const CQtInetAddr &aAddrPeer, 
		CQtTimeValue *aTimeout = NULL,
		CQtInetAddr *aAddrLocal = NULL)
	{
		m_pConnectorSink = aSink;
		m_aAddrPeer = aAddrPeer;

		m_pSocket = (new ConnectorType(this));
		QT_ASSERTE_RETURN_VOID(m_pSocket);		///out of memory
		m_pSocket->SetOption(CS_OPT_NEED_KEEPALIVE, (LPVOID)&m_bNeedKeepAlive);
		
		//AsycConnect to server
		m_pConnector->AsycConnect(m_pSocket.ParaIn(), m_aAddrPeer, aTimeout, aAddrLocal);
	};

	//For reconnection
	void AsycReconnect(IQtAcceptorConnectorSink *aSink, CQtTimeValue *aTimeout = NULL)
	{
		QT_STATE_TRACE_THIS("CConnectorT::AsycReconnect reconnect to " << 
			m_aAddrPeer.GetIpDisplayName() << " port = " << m_aAddrPeer.GetPort());
		m_pConnector->AsycConnect(aSink, m_aAddrPeer, aTimeout);
	};

	virtual void CancelConnect()
	{
		if(m_pSocket.Get())
		{
			m_pSocket->CancelConnect();
			m_pSocket = NULL;
		}
		m_pConnector->CancelConnect();
	}

	void OnConnectIndication(
		QtResult aReason,
		IQtTransport *pITransport,
		IQtAcceptorConnectorId *aRequestId)
	{
		m_pConnectorSink->OnConnectIndication(aReason, pITransport, aRequestId);
	}
	
	BOOL IsConnector()
	{
		return TRUE;
	}

	virtual DWORD AddReference()
	{
		return CQtReferenceControlSingleThread::AddReference();
	}
	
	virtual DWORD ReleaseReference()
	{
		return CQtReferenceControlSingleThread::ReleaseReference();
	}
public:
	CConnectorT(IQtConnector* pConnector, BOOL bNeedKeepAlive = FALSE)
		:m_pConnectorSink(NULL), m_pConnector(pConnector), m_bNeedKeepAlive(bNeedKeepAlive)
	{
		QT_ASSERTE(pConnector);
	}

	virtual ~CConnectorT() 	{}

protected:
	IQtAcceptorConnectorSink* m_pConnectorSink;	//IConnectorSink From Upper layer
	CQtComAutoPtr<ConnectorType> m_pSocket;
	CQtComAutoPtr<IQtConnector> m_pConnector;	//IConnector obj from TP layer	
	CQtInetAddr m_aAddrPeer;					//For saving the IP & Port
	BOOL		m_bNeedKeepAlive;
};

///////////////////////////////////////////
//class CAcceptorSinkT
///////////////////////////////////////////
template <typename AcceptType>
class QT_OS_EXPORT CAcceptorSinkT : 
		public IQtAcceptorConnectorSink, 
		public CQtReferenceControlSingleThread	//Reference tool
{
public:
	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *pITransport,
		IQtAcceptorConnectorId *aRequestId)
	{
		if(QT_SUCCEEDED(aReason)) {
			AcceptType* pSvrSocket = new AcceptType(m_pAcceptor.ParaIn(), m_AcceptorType);
			QT_ASSERTE_RETURN_VOID(pSvrSocket);			//out of memory
			pSvrSocket->SetOption(CS_OPT_NEED_KEEPALIVE, (LPVOID)&m_bNeedKeepAlive);
			
			pSvrSocket->OnConnectIndication(aReason, pITransport, aRequestId);
		}
		else { //Something wrong
			QT_ERROR_TRACE_THIS("CAcceptorSinkT::OnConnectIndication aReson = " << aReason <<
				" request acceptor = " << aRequestId);
			m_pAcceptor->OnConnectIndication(aReason, NULL, aRequestId);
		}
	};

public:

	void SetConnAcceptor(CAcceptorT<AcceptType>* pConnAcceptor)	{
		m_pAcceptor = pConnAcceptor;
	}

public:
	CAcceptorSinkT(LINK_TYPE acceptType = INVALID_LINK, BOOL bNeedKeepAlive = FALSE)
		: m_pAcceptor(NULL), m_AcceptorType(acceptType), m_bNeedKeepAlive(bNeedKeepAlive)
	{}

	virtual ~CAcceptorSinkT()	{}

protected:
	CConnectionList<AcceptType> m_ConnectionList;
	CQtComAutoPtr<CAcceptorT<AcceptType> > m_pAcceptor;
	LINK_TYPE	m_AcceptorType;
	BOOL		m_bNeedKeepAlive;
};
		
///////////////////////////////////////////
//class CAcceptorT
///////////////////////////////////////////
template <typename SocketType>
class QT_OS_EXPORT CAcceptorT : 
		public IQtAcceptor,
		public CQtReferenceControlSingleThread	//Reference tool
{
public:
	virtual QtResult StartListen(
		IQtAcceptorConnectorSink *aSink,	//IAcceptorSink From Upper layer
		const CQtInetAddr &aAddrListen) { 
		QT_INFO_TRACE_THIS("CConnAcceptor::StartListen(), sink = " << aSink << 
			" Address = " << aAddrListen.GetIpDisplayName() << " Port = " << aAddrListen.GetPort());
		QT_ASSERTE_RETURN(aSink, QT_ERROR_FAILURE);
		m_pAcceptorConnectorSink = aSink;

		m_pAcceptorSink->SetConnAcceptor(this);
		return m_pAcceptor->StartListen(m_pAcceptorSink.ParaIn(), aAddrListen);
	};

	virtual QtResult StopListen(QtResult aReason) {
		m_pAcceptorSink = NULL;
		return m_pAcceptor->StopListen(aReason);
	};

public:
	CAcceptorT(
		IQtAcceptor* pAcceptor, //TP layer Acceptor
		CAcceptorSinkT<SocketType> *pAcceptorSink)
		: m_pAcceptor(pAcceptor)
		, m_pAcceptorConnectorSink(NULL)
		, m_pAcceptorSink(pAcceptorSink)
	{ }

	virtual ~CAcceptorT() {}
	
	virtual DWORD AddReference() {
		return CQtReferenceControlSingleThread::AddReference();
	}
	
	virtual DWORD ReleaseReference() {
		return CQtReferenceControlSingleThread::ReleaseReference();
	}
public:

	void OnConnectIndication(QtResult aReason
		, IQtTransport *pITransport
		, IQtAcceptorConnectorId *aRequestId) {
		m_pAcceptorConnectorSink->OnConnectIndication(aReason, pITransport, aRequestId);
	}
		
	virtual BOOL IsConnector(){
		return FALSE;
	}
	
protected:
	CQtComAutoPtr<IQtAcceptor> m_pAcceptor;				//IAcceptor obj from TP layer		
	IQtAcceptorConnectorSink* m_pAcceptorConnectorSink;			//IAcceptorSink From Upper layer
	CQtComAutoPtr<CAcceptorSinkT<SocketType> > m_pAcceptorSink;
};

#endif






















