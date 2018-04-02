/*------------------------------------------------------*/
/* Connection interfaces                                */
/*                                                      */
/* QtConnectionInterface.h                              */
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

#ifndef QTCONNECTIONINTERFACE_H
#define QTCONNECTIONINTERFACE_H

#include "QtDefines.h"
#include "QtReferenceControl.h"
#include "QtUtilTemplates.h"
#include "QtMutex.h"

class CQtInetAddr;
class CQtTimeValue;
class CQtMessageBlock;

class IQtAcceptorConnectorSink;
class IQtTransportSink;
class IQtReferenceControl;
  class IQtTransport;
  class IQtAcceptorConnectorId;
    class IQtConnector;
      class IQtDetectionConnector;
	class IQtAcceptor;

class QT_OS_EXPORT CQtConnectionManager
{
public:
	static CQtConnectionManager* Instance();
	static void CleanupInstance();

	typedef DWORD CType;
	enum { 
		// connection type
		CTYPE_NONE = 0,
		CTYPE_TCP = (1 << 0),
		CTYPE_UDP = (1 << 1),
		CTYPE_SSL_DIRECT = (1 << 2),
		CTYPE_SSL_WITH_BROWER_PROXY = (1 << 3),
		CTYPE_SSL = CTYPE_SSL_DIRECT | CTYPE_SSL_WITH_BROWER_PROXY,
		CTYPE_HTTP_DIRECT = (1 << 4),
		CTYPE_HTTP_WITH_BROWER_PROXY = (1 << 5),
		CTYPE_HTTP = CTYPE_HTTP_DIRECT | CTYPE_HTTP_WITH_BROWER_PROXY,
		CTYPE_HTTPS_DIRECT = (1 << 6),
		CTYPE_HTTPS_WITH_BROWER_PROXY = (1 << 7),
		CTYPE_HTTPS = CTYPE_HTTPS_DIRECT | CTYPE_HTTPS_WITH_BROWER_PROXY,
		CTYPE_WEBEX_GATEWAY = (1 << 8),
		CTYPE_WEBEX_GATEWAY_SSL_WITH_BROWER_PROXY = CTYPE_SSL_WITH_BROWER_PROXY | CTYPE_WEBEX_GATEWAY,
		CTYPE_WEBEX_GATEWAY_SSL_DIRECT = CTYPE_SSL_DIRECT | CTYPE_WEBEX_GATEWAY,
		CTYPE_WEBEX_GATEWAY_TCP_DIRECT = CTYPE_WEBEX_GATEWAY | CTYPE_TCP,
		
		
		// connect though browse proxy, it builds a HTTP or SOCKS tunnel though proxy.
		CTYPE_TCP_WITH_BROWER_PROXY = (1 << 15),

		CTYPE_WEBEX_GATEWAY_TCP_WITH_BROWSE_PROXY = CTYPE_WEBEX_GATEWAY | CTYPE_TCP_WITH_BROWER_PROXY,
		// the range of connection type is from (1 << 0) to (1 << 15)
		CTYPE_TYPE_MASK = 0xFFFF,

		// thread module of connection
		// the upper layer prefers invoking and callbacked in network thread. 
		CTYPE_INVOKE_NETWORK_THREAD = (1 << 16),
		
		// the upper layer will invoke the functions of Connection in multi-threads.
		CTYPE_INVOKE_MULTITHREAD = (1 << 17), 

		CTYPE_HTTP_NOPROXY_TRY_DIRECT = (1 << 18),
		// the range of connection type is from (1 << 19) to (1 << 16)
		CTYPE_INVOKE_MASK = 0xF0000,

		// property of sending.
		CTYPE_SEND_NO_PARTIAL_DATA = (1 << 20),
		CTYPE_SEND_PRIORITY = (1 << 21),

		// the range of sending is from (1 << 23) to (1 << 20)
		CTYPE_SEND_MASK = 0xF00000,

		CTYPE_PDU_FIT = (1 << 25),
		CTYPE_PDU_RLB = (1 << 26),
		// will Add length ahead to upper layer data
		CTYPE_PDU_LENGTH = (1 << 27),

		// property of private connection PDU.
		// the following propertys contain CTYPE_SEND_NO_PARTIAL_DATA,
		// connect request and response for detecting connect
		CTYPE_PDU_PACKAGE = (1 << 28),
		CTYPE_PDU_KEEPALIVE = (1 << 29),
		CTYPE_PDU_RECONNECT = (1 << 30),
		// the reliable connection implements packet, keep-alive and reconnection functions.
		CTYPE_PDU_RELIABLE = CTYPE_PDU_PACKAGE | CTYPE_PDU_KEEPALIVE | CTYPE_PDU_RECONNECT | CTYPE_PDU_RLB,

		CTYPE_PDU_ANY = CTYPE_PDU_RELIABLE | CTYPE_PDU_FIT,
		// the range of PDU is from (1 << 31) to (1 << 24)
		CTYPE_PDU_MASK = 0xFF000000
	};

	enum CPriority
	{
		CPRIORITY_HIGH,
		CPRIORITY_ABOVE_NORMAL,
		CPRIORITY_NORMAL,
		CPRIORITY_BELOW_NORMAL,
		CPRIORITY_LOW
	};

	enum 
	{
		UDP_SEND_MAX_LEN = 16 * 1024,
		UDP_ONE_PACKET_MAX_LEN = 1514-14-20-8,
		DETECTING_HTTP_PROXY_TIME = 15 // proxy timeout 15s
	};

	/// Create <IQtConnector>.
	QtResult CreateConnectionClient(
		CType aType, 
		IQtConnector *&aConClient);

	/// Create <IQtDetectionConnector>.
	QtResult CreateDetectionConnectionClient(
		IQtDetectionConnector *&aConClient);

	/// Create <IQtAcceptor>.
	QtResult CreateConnectionServer(
		CType aType, 
		IQtAcceptor *&aAcceptor);

public:
	~CQtConnectionManager();

private:
	CQtConnectionManager();

	QtResult SpawnNetworkThread_i();

	QtResult CreateConnectionClient_i(
		CType aType, 
		IQtConnector *&aConClient);

	QtResult CreateConnectionServer_i(
		CType aType, 
		IQtAcceptor *&aAcceptor);

	//For Creating Connection Service Connector
	QtResult CreateCsConnectionClient(
		CType &aType, 
		IQtConnector *&aConClient);

	//For Creating Connection Service Acceptor
	QtResult CreateCsConnectionServer(
		CType &aType, 
		IQtAcceptor *&aAcceptor);

	static CQtConnectionManager s_ConnectionManagerSingleton;
	AQtThread *m_pThreadNetwork;
};

class QT_OS_EXPORT CQtTransportParameter
{
public:
	CQtTransportParameter(CQtConnectionManager::CPriority aPriority = CQtConnectionManager::CPRIORITY_NORMAL)
		: m_dwHaveSent(0)
		, m_Priority(aPriority)
	{
	}

	DWORD m_dwHaveSent;
	CQtConnectionManager::CPriority m_Priority;
};

/// the sink classes don't need ReferenceControl
class QT_OS_EXPORT IQtAcceptorConnectorSink 
{
public:
	/**
	 * <aReason> indicates Connect successful or failed, 
	 * if successful, <aTrpt> is not NULL, otherwise is NULL.
	 * <aRequestId> indicates which Connector calls it.
	 */
	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *aTrpt,
		IQtAcceptorConnectorId *aRequestId) = 0;

protected:
	virtual ~IQtAcceptorConnectorSink() {}
};

class QT_OS_EXPORT IQtTransportSink 
{
public:
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

protected:
	virtual ~IQtTransportSink() {}
};

class QT_OS_EXPORT IQtTransport : public IQtReferenceControl
{
public:
	virtual QtResult OpenWithSink(IQtTransportSink *aSink) = 0;

	virtual IQtTransportSink* GetSink() = 0;

	/// If success, fill <aPara->m_dwHaveSent> if <aPara> is not NULL:
	///    if <aData> has sent completely, return QT_OK;
	///    else return QT_ERROR_PARTIAL_DATA;
	/// Note: <aData> has been advanced <aPara->m_dwHaveSent> bytes in this function.
	virtual QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara = NULL) = 0;

	/// the <aCommand>s are all listed in file QtErrorNetwork.h
	virtual QtResult SetOption(DWORD aCommand, LPVOID aArg) = 0;
	virtual QtResult GetOption(DWORD aCommand, LPVOID aArg) = 0;

	/// Disconnect the connection, and will not callback <IQtTransportSink> longer.
	virtual QtResult Disconnect(QtResult aReason) = 0;

protected:
	virtual ~IQtTransport() {}
};

class QT_OS_EXPORT IQtAcceptorConnectorId : public IQtReferenceControl
{
public:
	virtual BOOL IsConnector() = 0;
	
protected:
	virtual ~IQtAcceptorConnectorId() {}
};

class QT_OS_EXPORT IQtConnector : public IQtAcceptorConnectorId
{
public:
	/**
	 * If <aTimeout> equals NULL, there is no limit time for connecting.
	 * No return value, IQtAcceptorConnectorSink::OnConnectIndication() will 
	 * callback to indicate successful or failed.
	 * 
	 * <aAddrLocal> is bind() before connect() if it is not NULL.
	 *
	 * If you want to send some user data in the connect request,
	 * call SetOption(QT_OPT_CONNECTION_CONNCET_DATA) before call this funtion.
	 */
	virtual void AsycConnect(
		IQtAcceptorConnectorSink *aSink,
		const CQtInetAddr &aAddrPeer, 
		CQtTimeValue *aTimeout = NULL,
		CQtInetAddr *aAddrLocal = NULL) = 0;

	/**
	 * IQtAcceptorConnectorSink::OnConnectIndication() will never callback 
	 * after invoking it.
	 */
	virtual void CancelConnect(QtResult aReason = QT_OK) = 0;

protected:
	virtual ~IQtConnector() {}
};

class QT_OS_EXPORT IQtDetectionConnector : public IQtConnector
{
public:
	/// add connection one by one, and the prior has high priority.
	// if <aTimeDelay> less than zero, it will delay <aTimeDelay> to start connecting, 
	//    (new implementation by budingc 04/28/2006)
	// if <aTimeDelay> greater than zero, it will start connecting at once, and wait high priority 
	//   connection for <aTimeDelay> after OnConnect successful. (old implementation)
	virtual QtResult AddConnection(
		CQtConnectionManager::CType Type, 
		const CQtInetAddr &aAddrPeer,
		CQtTimeValue *aTimeDelay) = 0;

	/// Start connecting at the same time in <aTimeout>,
	/// If low priority connection connected, wait high priority connection in <aTimeDelay>.
	virtual void StartDetectionConnect(
		IQtAcceptorConnectorSink *aSink,
		CQtTimeValue *aTimeout = NULL ) =0;

protected:
	virtual ~IQtDetectionConnector() {}
};

class QT_OS_EXPORT IQtAcceptor : public IQtAcceptorConnectorId
{
public:
	enum{
		DEFAULT_RCVBUFF_SIZE = 64 * 1024,
		DEFAULT_SNDBUFF_SIZE = 64 * 1024,
	};
	
	virtual QtResult StartListen(
		IQtAcceptorConnectorSink *aSink,
		const CQtInetAddr &aAddrListen, int nTraceInterval = 1) = 0;

	virtual QtResult StopListen(QtResult aReason) = 0;

	/// the <aCommand>s are all listed in file QtErrorNetwork.h
	virtual QtResult SetOption(DWORD aCommand, LPVOID aArg) = 0;
	virtual QtResult GetOption(DWORD aCommand, LPVOID aArg) = 0;
	
protected:
	virtual ~IQtAcceptor() {}
};

BOOL QT_OS_EXPORT IsAuthDialogPopup();
QtResult QT_OS_EXPORT InitProxy(BOOL bReload);

#ifdef ENABLE_SPA_SPECAIL
void QT_OS_EXPORT SetSPAHostFlag(const CQtString &aHost, BOOL bSPAFlag);
#endif

#ifdef QT_WIN32
void QT_OS_EXPORT SetBrowserType( int aBrowserType);
#endif


#endif // QTCONNECTIONINTERFACE_H
