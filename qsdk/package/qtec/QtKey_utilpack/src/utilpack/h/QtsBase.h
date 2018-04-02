/*------------------------------------------------------*/
/* Connection Service Base classes                      */
/*                                                      */
/* CsRlbTcp.h                                           */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*      zhubin (zhubin@qtec.cn)                         */
/*                                                      */
/* History                                              */
/*    2017/02/15  Create                                */
/*                                                      */
/*------------------------------------------------------*/

//$Id: CsBase.h,v 1.83.4.2 2010/05/26 08:56:39 jerryh Exp $

#if !defined CS_BASE_H  && !defined (_NEW_PROTO_TP)
#define CS_BASE_H

#ifdef QT_WIN32
#pragma message("************Now use the old protocol TP *************")
#endif

#include <map>
#include "QtDefines.h"
#include "QtDebug.h"
#include "QtErrorNetwork.h"
#include "QtTimerWrapperID.h"
#include "QtTimeValue.h"
#include "QtInetAddr.h"
#include "QtMessageBlock.h"
#include "QtConnectionInterface.h"
#include "QtUtilTemplates.h"
#include "QtTraceFromT120.h"
#include "QtsSendBuf.h"
#include "timer.h"

using namespace std;

#define STATUS_UNCONNECTED		0x0001
#define STATUS_CONNECTED		0x0002
#define STATUS_DATA_CAN_SEND	0x0003
#define STATUS_NEED_RECONNECT	0x0004

#define SERVER_INSTANCE			0x01
#define CLIENT_INSTANCE			0x02

#define BYTES_TO_ACK_BACK				10240		//ACK back every 10K bytes received
#define SERVER_UNAVAIL_TIMEOUT			(long)120	//2 minutes
#define SERVER_NORMAL_DISCONN_TIMEOUT	(long)90	//120-90=30 seconds	
#define SERVER_CHECK_INTERVAL			(long)5	//5 seconds	
#define ALIVE_DETECT_TIMEOUT			(long)30 //30 seconds
#define INTERVAL1						(long)60	//For Conn-Resp Checking

// budingc modify at 10/21/2004, keep alive every 15 second, 8 times, 2 minutes.
//#define INTERVAL2						(long)20	//For Keep Alive
#define INTERVAL2						(long)10	//For Keep Alive, change from 15 to 10, 10/13 2006 Victor
#define KEEPALIVE_DETECT_TIMES			3
#define MAX_TIMES_NO_RECV_KA		8		//For Keep Alive or Data recv Checking(MAX_TIMES_NO_RECV_KA*INTERVAL2)
#define UDP_HAND_VAL				50
template <class ServerType> class ServerListT;
template <class ServerType> class CConnAcceptorT;
template <class ServerType> class CConnAcceptorSinkT;
template <class ClientType> class CConnConnectorT;
///////////////////////////////////////////
//class CConnBase
///////////////////////////////////////////
class QT_OS_EXPORT CConnBase : 
	public IQtTransport,				//As a Transport Handle to Upper layer
	public IQtTransportSink,			//As a Sink to TP
	public IQtAcceptorConnectorSink,	//As a ConnectorSink to TP
	public CQtTimerWrapperIDSink,
	public CQtReferenceControlSingleThreadTimerDelete	//Reference tool
{
protected:
	low_ticker	m_Ticker;
	low_ticker	m_LatestRcvTicker;
	low_ticker	m_LatestSndTicker;
        BOOL m_bHandshake;
public:
	// interface IQtTransport
	//Accept ITransportSink from Upper layer
	virtual QtResult OpenWithSink(IQtTransportSink* aSink)
	{
		m_pITransportSink = aSink; 

		return QT_OK;
	};

	virtual IQtTransportSink* GetSink()
	{
		return m_pITransportSink;
	};

	virtual QtResult SendData(
		CQtMessageBlock &aData, 
		CQtTransportParameter *aPara = NULL) = 0;
	
	virtual QtResult SetOption(
		DWORD aCommand, 
		LPVOID aArg) = 0;

	virtual QtResult GetOption(
		DWORD aCommand, 
		LPVOID aArg) = 0;
	
	virtual QtResult Disconnect(
		QtResult aReason) = 0;

	// interface IQtTransportSink
	virtual void OnReceive(
		CQtMessageBlock &aData,
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL) = 0;

	virtual void OnSend(
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL) = 0;

	virtual void OnDisconnect(
		QtResult aReason,
		IQtTransport *aTrptId) = 0;

	// interface IQtAcceptorConnectorSink
	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *aTrpt,
		IQtAcceptorConnectorId *aRequestId) = 0;
	
	// interface CQtTimerWrapperIDSink
	virtual void OnTimer(CQtTimerWrapperID* aId) = 0;

	// interface IQtReferenceControl
	virtual DWORD AddReference()
	{
		return CQtReferenceControlSingleThreadTimerDelete::AddReference();
	};

	virtual DWORD ReleaseReference()
	{
		return CQtReferenceControlSingleThreadTimerDelete::ReleaseReference();
	};
public:
	CConnBase() 
	{ 
		m_pITransportSink = NULL; 
		m_pITransport = NULL; 

		m_wLastStatus = STATUS_UNCONNECTED;
		m_wStatus = STATUS_UNCONNECTED;

		m_wChannel = 0;

		m_byConnType = CS_CONN_TYPE_NONE;

		m_cType = CQtConnectionManager::CTYPE_NONE;
		m_cBaseType = CQtConnectionManager::CTYPE_NONE;
		m_bPDUNeedACK = FALSE;
	};

	virtual ~CConnBase() 
	{
		m_Timer.Cancel();//Kill timer

		Reset();
	};

	//Local functions
	
	void SetChannel(WORD wChannel)
	{
		m_wChannel = wChannel;
	};

	WORD GetChannel()
	{
		return m_wChannel;
	};

	void SetTPTransport(IQtTransport *pTrans)
	{
		m_pITransport = pTrans;
	};

	IQtTransport* GetTPTransport()
	{
		return m_pITransport.Get();
	};

	void SetCurrStatus(WORD wStatus)
	{
		m_wLastStatus = m_wStatus;
		m_wStatus = wStatus;
	};

	WORD GetCurrStatus()
	{
		return m_wStatus;
	};

	BYTE GetConnType()
	{
		return m_byConnType;
	}
protected:
	void Reset()
	{
		//m_pITransportSink = NULL; 
		m_pITransport = NULL; 

		m_wLastStatus = STATUS_UNCONNECTED;
		m_wStatus = STATUS_UNCONNECTED;

		m_wChannel = 0;
	};

protected:
	IQtTransportSink* m_pITransportSink;		//ITransportSink obj from Upper layer
	CQtComAutoPtr<IQtTransport> m_pITransport;	//ITransport obj from TP layer
	
	CQtTimerWrapperID m_Timer;					//Timer
	CQtTimerWrapperID m_HandleTimer;			// Conn Timer
	DWORD	m_nInterval;

	WORD m_wLastStatus;
	WORD m_wStatus;	
	WORD m_wChannel;
	BYTE m_byConnType;

	CQtConnectionManager::CType m_cType;		//Connection Type of this class
	CQtConnectionManager::CType m_cBaseType;	//Base connection type
	BOOL						m_bPDUNeedACK;
	BOOL m_bNeedKeepAlive;	
	DWORD m_dwKeepAliveInterval;
};

///////////////////////////////////////////
//class CCsConn
///////////////////////////////////////////
class QT_OS_EXPORT CCsConn : public CConnBase
{
public:
	virtual QtResult SendData(
		CQtMessageBlock &aData, 
		CQtTransportParameter *aPara = NULL);

	virtual QtResult SetOption(
		DWORD aCommand, 
		LPVOID aArg);

	virtual QtResult GetOption(
		DWORD aCommand, 
		LPVOID aArg);

	virtual QtResult Disconnect(
		QtResult aReason);
public:
	virtual void OnReceive(
		CQtMessageBlock &aData,
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL);

	virtual void OnSend(
		IQtTransport *aTrptId,
		CQtTransportParameter *aPara = NULL);

	virtual void OnDisconnect(
		QtResult aReason,
		IQtTransport *aTrptId) = 0;

	virtual void OnTimer(
		CQtTimerWrapperID* aId) = 0;

protected:
	//Internal functions	
	virtual QtResult SendKeepAlive();
	virtual QtResult SendConnReq();
	virtual QtResult SendConnResp();
	virtual QtResult SendDisconn(QtResult aReason);
	
	virtual QtResult SendDataFromSendBuf();
	virtual void DoACK(DWORD dwACK);

	virtual void OnRecvConnResp();
	virtual void OnRecvConnReq();
	virtual void OnRecvDisconn();
	virtual void OnRecvKeepAlive();

	int GetPDUType();

	//Amount number of PDUs have been received, we may do ACK back to peer
	virtual void ACK2PeerIfPossiable(CQtMessageBlock &aData);
protected:
	void Reset();
	void Reset4Recv();
public:
	CCsConn(DWORD dwMaxSendBufLen);
	virtual ~CCsConn();

	void NeedKeepAlive(BOOL bNeedKeepAlive);
	BYTE GetInstanceType()const
	{
		return m_byInstanceType;
	}
protected:
	BYTE	m_byInstanceType;
	CCsSendBuf m_SendBuf;
	CQtMessageBlock *m_pmbLocData;	//local Data block for Recv
//	CQtMessageBlock *m_pmbLastGet;	//Keep the blocks last received
	CQtMessageBlock *m_pmbRecData;	
	CQtMessageBlock *m_pConnReqPDU;	//Keep ConnReq PDU for re-sending

	DWORD m_dwSeq4ACK;				//Seq# to ACK back
	DWORD m_dwCnt4JudgeACK;			//Counter for judging whether need to ACK back

	DWORD m_dwPDULen;				//Whole PDU length should be received this time
	BYTE m_byType;					//Type of PDU which is recving
	DWORD m_dwDataLen;				//When recving Data PDU, this indicate content len of the real data

	BOOL m_bNormalDisconn;			//Is normal disconnect
	BOOL m_bNoRoom;					//Does SendBuf haven't No eough rooms for new coming data

	BOOL m_bNeedKeepAlive;

	CQtTimeValue m_disconn_timestamp;	//Only server use

	DWORD m_dwKeepAliveInterval;

	BOOL m_bDisableKeepaliveFlagOrNot;

	DWORD m_dwMaxBuffLen;

	DWORD m_dwRcvBytesLatestKeepalive;
};

///////////////////////////////////////////
//class CConnConnectorT
///////////////////////////////////////////
template <class ClientType>
class QT_OS_EXPORT CConnConnectorT : 
			public IQtConnector, 
			public CQtReferenceControlMutilThread	//Reference tool
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

		m_pCli = new ClientType;//As a ITransport obj to Upper layer when callback pConnectorSink->OnConnectIndication(...)
		m_pCli->NeedKeepAlive(m_bNeedKeepAlive);
		m_pCli->SetConnConnector(this);
		
		//Call TP layer function
		m_pConnector->AsycConnect(
			m_pCli.ParaIn(), //CS layer CConnBase as IQtAcceptorConnectorSink to TP
			m_aAddrPeer, 
			aTimeout,
			aAddrLocal);
	};

	//For reconnection
	void AsycReconnect(CQtTimeValue *aTimeout = NULL)
	{
		AsycConnect(
			m_pConnectorSink,
			m_aAddrPeer, 
			aTimeout);
	};

	virtual void CancelConnect(QtResult aReason)
	{
		if(m_pCli.Get())
		{
			m_pCli->CancelHandShake();
			m_pCli = NULL;
		}
		//Call TP layer function
		m_pConnector->CancelConnect(aReason);
	};
public:
	// interface IQtReferenceControl
	virtual DWORD AddReference()
	{
		return CQtReferenceControlMutilThread::AddReference();
	};

	virtual DWORD ReleaseReference()
	{
		return CQtReferenceControlMutilThread::ReleaseReference();
	};

	virtual BOOL IsConnector()
	{
		return TRUE;
	};
public:
	IQtAcceptorConnectorSink* GetConnectorSink()
	{
		return m_pConnectorSink;
	};

	IQtConnector* GetTPConnector()
	{
		return m_pConnector.Get();
	};

	CQtInetAddr GetPeerAddr()
	{
		return m_aAddrPeer;
	};
public:
	CConnConnectorT(
		IQtConnector* pConnector, 
		BOOL bNeedKeepAlive = FALSE)
	{
		m_pConnectorSink = NULL;
		QT_ASSERTE(pConnector);
		m_pConnector = pConnector;
		m_bNeedKeepAlive = bNeedKeepAlive;
		QT_INFO_TRACE_THIS("CConnConnectorT::CConnConnectorT() connector = " << pConnector);
	};

	virtual ~CConnConnectorT() 
	{
		QT_INFO_TRACE_THIS("CConnConnectorT::~CConnConnectorT()");
	};

protected:
	IQtAcceptorConnectorSink* m_pConnectorSink;	//IConnectorSink From Upper layer
	CQtComAutoPtr<ClientType> m_pCli;
	CQtComAutoPtr<IQtConnector> m_pConnector;	//IConnector obj from TP layer	
	CQtInetAddr m_aAddrPeer;					//For saving the IP & Port
	BOOL m_bNeedKeepAlive;
};

///////////////////////////////////////////
//class CConnAcceptorSinkT
///////////////////////////////////////////
template <class ServerType>
class QT_OS_EXPORT CConnAcceptorSinkT : 
		public IQtAcceptorConnectorSink, 
		public CQtReferenceControlSingleThread	//Reference tool
{
public:
	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *aTrpt,
		IQtAcceptorConnectorId *aRequestId)
	{
		if(QT_SUCCEEDED(aReason))
		{
			ServerType* pSvr = (new ServerType);
			QT_INFO_TRACE_THIS("CConnAcceptorSinkT::OnConnectIndication aReason = " << aReason << " low tran = " << aTrpt << " request = " << aRequestId <<
				" wrapper tran = " << pSvr);
			pSvr->NeedKeepAlive(m_bNeedKeepAlive);
			
			pSvr->SetConnAcceptor(m_pConnAcceptor.ParaIn());
			pSvr->OnConnectIndication(
				aReason,
				aTrpt,
				aRequestId);
			
			
			//a new Channel# will be set in pSvr, but it's no use, only for Tagging
			if(pSvr->GetConnType() == CS_CONN_TYPE_RLB)
			{
				//m_ServerList.AddServer(pSvr);
				pSvr->SetServerList(&m_ServerList);
			}
		}
		else	//Something wrong
		{
			m_pConnAcceptor->GetAcceptorSink()->OnConnectIndication(
				aReason,
				NULL,
				aRequestId);
		}
	};

public:
	void NeedKeepAlive(BOOL bNeedKeepAlive)
	{
		m_bNeedKeepAlive = bNeedKeepAlive;
	};

	void SetConnAcceptor(CConnAcceptorT<ServerType>* pConnAcceptor)
	{
		m_pConnAcceptor = pConnAcceptor;
	};

	CConnAcceptorT<ServerType>* GetConnAccepter()
	{
		return m_pConnAcceptor.Get();
	};
public:
	// interface IQtReferenceControl
	virtual DWORD AddReference()
	{
		return CQtReferenceControlSingleThread::AddReference();
	};

	virtual DWORD ReleaseReference()
	{
		return CQtReferenceControlSingleThread::ReleaseReference();
	};
public:
	CConnAcceptorSinkT()
	{
		m_pConnAcceptor = NULL;
	};

	virtual ~CConnAcceptorSinkT()
	{
		m_pConnAcceptor = NULL;
	};
protected:
	ServerListT<ServerType> m_ServerList;
	CQtComAutoPtr<CConnAcceptorT<ServerType> > m_pConnAcceptor;

	BOOL m_bNeedKeepAlive;
};
		
///////////////////////////////////////////
//class CConnAcceptorT
///////////////////////////////////////////
template <class ServerType>
class QT_OS_EXPORT CConnAcceptorT : 
		public IQtAcceptor,
		public CQtReferenceControlSingleThread	//Reference tool
{
public:
	virtual QtResult StartListen(
		IQtAcceptorConnectorSink *aSink,	//IAcceptorSink From Upper layer
		const CQtInetAddr &aAddrListen, int nTraceInterval = 1)
	{ 
		QT_INFO_TRACE_THIS("CConnAcceptor::StartListen()");

		m_pAcceptorSink = aSink;

		m_pConnAcceptorSink->SetConnAcceptor(this);
		return m_pAcceptor->StartListen(
			m_pConnAcceptorSink.ParaIn(), 
			aAddrListen, nTraceInterval);
	};

	QtResult SetOption(DWORD aCommand, LPVOID aArg)
	{
		QT_INFO_TRACE_THIS("CConnAcceptorT::SetOption aCommand = " << aCommand << " aArg = " << aArg);
		return m_pAcceptor->SetOption(aCommand, aArg);
	}
	
	QtResult GetOption(DWORD aCommand, LPVOID aArg)
	{
		QT_INFO_TRACE_THIS("CConnAcceptorT::GetOption aCommand = " << aCommand << " aArg = " << aArg);
		return m_pAcceptor->GetOption(aCommand, aArg);
	}
	
	virtual QtResult StopListen(QtResult aReason)
	{
		m_pConnAcceptorSink = NULL;
		return m_pAcceptor->StopListen(aReason);
	};
public:
	// interface IQtReferenceControl
	virtual DWORD AddReference()
	{
		return CQtReferenceControlSingleThread::AddReference();
	};

	virtual DWORD ReleaseReference()
	{
		return CQtReferenceControlSingleThread::ReleaseReference();
	};

	virtual BOOL IsConnector()
	{
		return FALSE;
	};
public:
	CConnAcceptorT(
		IQtAcceptor* pAcceptor, //TP layer Acceptor
		CConnAcceptorSinkT<ServerType> *pConnAcceptorSink, //As a AcceptorSink obj to TP layer 
		BOOL bNeedKeepAlive = TRUE)
	{ 
		m_pAcceptor = pAcceptor; 
		m_pAcceptorSink = NULL;
		m_pConnAcceptorSink = pConnAcceptorSink;
		m_pConnAcceptorSink->NeedKeepAlive(bNeedKeepAlive);
	};
	virtual ~CConnAcceptorT() 
	{
		m_pAcceptor = NULL; 
		m_pAcceptorSink = NULL;
		m_pConnAcceptorSink = NULL;
	};	
public:
	IQtAcceptorConnectorSink* GetAcceptorSink()
	{
		return m_pAcceptorSink;
	};

	IQtAcceptor* GetTPAcceptor()
	{
		return m_pAcceptor.Get();
	};
protected:
	IQtAcceptorConnectorSink* m_pAcceptorSink;			//IAcceptorSink From Upper layer
	CQtComAutoPtr<CConnAcceptorSinkT<ServerType> > m_pConnAcceptorSink;
	CQtComAutoPtr<IQtAcceptor> m_pAcceptor;				//IAcceptor obj from TP layer		
};

///////////////////////////////////////////
//class ServerListT
///////////////////////////////////////////
#include "timer.h"
template <class ServerType>
class QT_OS_EXPORT ServerListT/* : public CQtTimerWrapperIDSink*/
{
public:
	ServerListT()
		:m_svr_list(65535), m_wChannel(0)
	{
		m_wChannel = (WORD)ticker::now();
		QT_ASSERTE(m_svr_list.size() == 65535);
	}

	virtual ~ServerListT()
	{
		for(unsigned i = 0; i < m_svr_list.size(); ++i)
		{
			if(m_svr_list[i].Get())
			{
				m_svr_list[i]->OnDisconnect(QT_ERROR_NETWORK_SOCKET_CLOSE, m_svr_list[i]->GetTPTransport());
				m_svr_list[i] = NULL;
			}
		}
	};

	//Add Server & get a new Channel# for it, Return the Channel#
	WORD AddServer(ServerType* pSvr)
	{
		//CQtMutexGuardT<CQtMutexThread> AutoLock(m_Mutex);
		QT_ASSERTE(pSvr->GetChannel() == 0);
		CQtComAutoPtr<ServerType> pAPSvr(pSvr);
		WORD oldchannel = m_wChannel;
		for(;;)
		{
			if(oldchannel == ChannelCalculator())
			{
				QT_WARNING_TRACE_THIS("ServerListT::AddServer server list is full!");
				return 0;
			}
			if(!m_svr_list[m_wChannel - 1].Get())
			{
//				QT_INFO_TRACE_THIS("ServerListT::AddServer channel id = " << m_wChannel);
				pSvr->SetChannel(m_wChannel);
				m_svr_list[m_wChannel - 1] = pAPSvr;
				return pSvr->GetChannel();
			}
		}
	};

	CQtComAutoPtr<ServerType> GetServer(WORD wChannel)
	{
//		QT_INFO_TRACE_THIS("ServerListT<...>::GetServer(), Find the server, wChannel = " << wChannel << " server = " << m_svr_list[wChannel - 1].Get());
		CQtComAutoPtr<ServerType> pServer = m_svr_list[wChannel - 1];
		return pServer;
	};
	void RemoveServer(WORD wChannel)
	{
		if(wChannel > 0 && m_svr_list[wChannel - 1].Get())
		{
//			QT_INFO_TRACE_THIS("ServerListT::RemoveServer channel id = " << wChannel << " Transport = " << m_svr_list[wChannel - 1].Get());
			m_svr_list[wChannel - 1] = NULL;
		}
	}

	WORD ChannelCalculator()
	{
		m_wChannel++;
		if(m_wChannel == 0)
			m_wChannel = 1;
		return m_wChannel;
	}

protected:
	vector< CQtComAutoPtr<ServerType> > m_svr_list;
	WORD m_wChannel;
};

#endif // CS_BASE_H

