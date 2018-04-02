/*------------------------------------------------------*/
/* Wrap TCP/UDP/HTTP/SSL/Proxy connector                */
/*                                                      */
/* CQtConnectorWrapper.h                                 */
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

#ifndef QTCONNECTORWRAPPER_H
#define QTCONNECTORWRAPPER_H

#if defined (QT_WIN32) || defined (QT_PORT_CLIENT) //Be not server.
  #define QT_SUPPORT_OPENSSL 1
#endif // QT_WIN32

#include "QtConnectionInterface.h"
#include "QtReactorInterface.h"
#include "QtReferenceControl.h"
#include "QtInetAddr.h"

class QT_OS_EXPORT CQtConnectorWrapper 
	: public IQtConnector
	, public IQtTimerHandler
	, public CQtReferenceControlSingleThread
{
	CQtConnectorWrapper(CQtConnectorWrapper &);
	CQtConnectorWrapper& operator=(CQtConnectorWrapper&);

public:
	CQtConnectorWrapper();
	virtual ~CQtConnectorWrapper();

	// interface IQtReferenceControl
	virtual DWORD AddReference();
	virtual DWORD ReleaseReference();

	QtResult Init(CQtConnectionManager::CType aType);

	int OnConnectIndication(
		QtResult aReason, 
		IQtTransport *aTrpt,
		AQtConnectorInternal *aId);

	// interface IQtAcceptorConnectorId
	virtual BOOL IsConnector();

	// interface IQtConnector
	virtual void AsycConnect(
		IQtAcceptorConnectorSink* aSink,
		const CQtInetAddr& aAddrPeer, 
		CQtTimeValue* aTimeout = NULL,
		CQtInetAddr *aAddrLocal = NULL);

	virtual void CancelConnect(QtResult aReason);

protected:
	// interface IQtTimerHandler
	virtual void OnTimeout(const CQtTimeValue &aCurTime, LPVOID aArg);

	void Close_i(QtResult aReason = QT_OK);

private:
	IQtReactor *m_pReactor;
	IQtAcceptorConnectorSink *m_pSink;
	AQtConnectorInternal *m_pConnector;
	CQtConnectionManager::CType m_Type;
	CQtInetAddr	m_AddrPeer;
	BOOL m_bClosed;
};

#endif // !QTCONNECTORWRAPPER_H
