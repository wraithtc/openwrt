/*------------------------------------------------------*/
/* Acceptor for UDP                                     */
/*                                                      */
/* QtAcceptorUdp.h                                      */
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

#ifndef QTACCEPTORUDP_H
#define QTACCEPTORUDP_H

#include "QtAcceptorBase.h"
#include "QtSocket.h"
#include "QtInetAddr.h"
#include "QtTransportUdp.h"
#include "QtUtilClasses.h"
#include <map>

#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

class QT_OS_EXPORT CQtAcceptorUdp 
	: public CQtAcceptorBase
	, public AQtEventHandler 
	, public CQtStopFlag
{
public:
	CQtAcceptorUdp();
	virtual ~CQtAcceptorUdp();
	
	// interface IQtAcceptor
	virtual QtResult StartListen(
		IQtAcceptorConnectorSink *aSink,
		const CQtInetAddr &aAddrListen, int nTraceInterval = 1);
	virtual QtResult StopListen(QtResult aReason);

	// iterface AQtEventHandler
	virtual QT_HANDLE GetHandle() const ;
	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE);
	virtual int OnClose(QT_HANDLE aFd, MASK aMask);

	// it will be invoked by CQtTransportUdp::Close_t().
	QtResult RemoveTransport(const CQtInetAddr &aAddr, CQtTransportUdp *aTrpt);

private:
	CQtSocketUdp m_Socket;
	CQtInetAddr m_AddrLocol;

	// use hash_map instead map.
//	typedef std::map<CQtInetAddr, CQtComAutoPtr<CQtTransportUdp> > TransportsType;
//	TransportsType m_Transports;
};
#endif
#endif // !QTACCEPTORUDP_H
