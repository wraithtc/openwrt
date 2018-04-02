/*------------------------------------------------------*/
/* UDP transport                                        */
/*                                                      */
/* QtTransportUdp.h                                     */
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

#ifndef QTTRANSPORTUDP_H
#define QTTRANSPORTUDP_H

#include "QtTransportBase.h"
#include "QtSocket.h"
#include "QtInetAddr.h"
#include "QtMessageBlock.h"

#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
class CQtAcceptorUdp;
#endif
class CQtTransportUdp : public CQtTransportBase  
{
public:
	CQtTransportUdp(IQtReactor *pReactor, const CQtInetAddr &aAddrSend
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
		, CQtAcceptorUdp *pAcceptor = NULL
#endif
		);
	virtual ~CQtTransportUdp();

	// interface AQtEventHandler
	virtual QT_HANDLE GetHandle() const ;
	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE);

	// interface IQtTransport
	virtual QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara = NULL);
	virtual QtResult SetOption(DWORD aCommand, LPVOID aArg);
	virtual QtResult GetOption(DWORD aCommand, LPVOID aArg);

	CQtSocketUdp& GetPeer();
	
	void OnReceiveCallback(LPSTR aData, DWORD aLen);

protected:
	virtual QtResult Open_t();
	virtual QtResult Close_t(QtResult aReason);

private:
	CQtSocketUdp m_SocketUdp;
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)
	CQtComAutoPtr<CQtAcceptorUdp> m_pAcceptor;
#endif
	CQtInetAddr m_AddrSend;
};


// inline functions
inline void CQtTransportUdp::OnReceiveCallback(LPSTR aData, DWORD aLen)
{
	CQtMessageBlock mbOn(
		aLen, 
		aData, 
		CQtMessageBlock::DONT_DELETE | CQtMessageBlock::WRITE_LOCKED, 
		aLen);
	
	QT_ASSERTE(m_pSink);
	if (m_pSink)
		m_pSink->OnReceive(mbOn, this);
}

#endif // !QTTRANSPORTUDP_H
