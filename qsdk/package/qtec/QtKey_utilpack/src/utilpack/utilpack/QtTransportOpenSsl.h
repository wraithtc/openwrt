/*------------------------------------------------------*/
/* SSL transport using OpenSSL library                  */
/*                                                      */
/* QtTransportOpenSsl.h                                 */
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

#ifndef QTTRANSPORTOPENSSL_H
#define QTTRANSPORTOPENSSL_H

#include "QtTransportTcp.h"
#include "QtMessageBlock.h"
#include <openssl/ssl.h>

class QT_OS_EXPORT CQtTransportOpenSsl : public CQtTransportTcp
{
public:
	CQtTransportOpenSsl(IQtReactor *pReactor);
	virtual ~CQtTransportOpenSsl();

	int InitSsl(CQtConnectionManager::CType aType);

	// interface AQtEventHandler
	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE);

	// interface IQtTransport
	virtual QtResult SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara = NULL);
	virtual QtResult GetOption(DWORD aCommand, LPVOID aArg);
	
	int RecvFromSocket();

	int DoBioRecv(LPSTR aBuf, DWORD aLen, int &aErr);
	int DoBioSend(LPCSTR aBuf, DWORD aLen, int &aErr);

	SSL* GetSslPtr();

	static void TraceOpenSslError(LPCSTR aFuncName, LPVOID pThis);

protected:
	virtual QtResult Close_t(QtResult aReason);

	static int VerifyCallback(int aOk, X509_STORE_CTX *aCtx);

private:
	static SSL_CTX *s_pSslCtx;
	
	SSL *m_pSsl;
	CQtMessageBlock m_mbSslRecvBuffer;
	CQtConnectionManager::CType m_Type;
};

#endif // !QTTRANSPORTOPENSSL_H
