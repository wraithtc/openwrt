/*------------------------------------------------------*/
/* Acceptor for TCP                                     */
/*                                                      */
/* QtAcceptorTcp.h                                      */
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

#ifndef QTACCEPTORTCP_H
#define QTACCEPTORTCP_H

#include "QtAcceptorBase.h"
#include "QtSocket.h"

#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

class QT_OS_EXPORT CQtAcceptorTcp 
	: public CQtAcceptorBase
	, public AQtEventHandler
{
public:
	CQtAcceptorTcp();
	virtual ~CQtAcceptorTcp();
	
	// interface IQtAcceptor
	virtual QtResult StartListen(
		IQtAcceptorConnectorSink *aSink,
		const CQtInetAddr &aAddrListen, int nTraceInterval = 1);
	virtual QtResult StopListen(QtResult aReason);

	// iterface AQtEventHandler
	virtual QT_HANDLE GetHandle() const ;
	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE);
	virtual int OnClose(QT_HANDLE aFd, MASK aMask);

private:
	CQtSocketTcp m_Socket;
};
#endif
#endif // !QTACCEPTORTCP_H
