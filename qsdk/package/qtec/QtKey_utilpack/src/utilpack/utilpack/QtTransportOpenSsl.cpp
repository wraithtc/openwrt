
#include "QtBase.h"
#include "QtTransportOpenSsl.h"
#include "QtTransportOpenSslBio.h"

#ifndef QT_MMP
  #include <openssl/err.h>
  #include <openssl/rand.h>
#else
  #include "cmssl.h"
#endif

SSL_CTX* CQtTransportOpenSsl::s_pSslCtx = NULL;

void CQtTransportOpenSsl::TraceOpenSslError(LPCSTR aFuncName, LPVOID pThis)
{
	QT_ASSERTE(aFuncName);
#ifdef QT_MMP
	unsigned long error_code = CQTSSL::ERR_get_error();
#else
	unsigned long error_code = ::ERR_get_error();
#endif
	char error_string[512];
	::memset(error_string, 0, sizeof(error_string));
	
#ifdef QT_MMP
	CQTSSL::ERR_error_string(error_code, error_string);
#else
	::ERR_error_string(error_code, error_string);
#endif
	QT_ERROR_TRACE(aFuncName << " err_str=" << error_string << " this=" << pThis);
}

CQtTransportOpenSsl::CQtTransportOpenSsl(IQtReactor *pReactor)
	: CQtTransportTcp(pReactor)
	, m_pSsl(NULL)
	, m_mbSslRecvBuffer(4096 * 4)
	, m_Type(CQtConnectionManager::CTYPE_NONE)
{
}

CQtTransportOpenSsl::~CQtTransportOpenSsl()
{
	Close_t(QT_OK);
}

int CQtTransportOpenSsl::InitSsl(CQtConnectionManager::CType aType)
{
	QT_ASSERTE(!m_pSsl);
	m_Type = aType;
	QT_ASSERTE(m_Type == CQtConnectionManager::CTYPE_SSL_DIRECT || 
		m_Type == CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY);

	if (!s_pSslCtx) {
#ifdef QT_MMP
		CQTSSL::SSL_library_init();
		CQTSSL::SSL_load_error_strings();
#else
		::SSL_library_init();
		::SSL_load_error_strings();
#endif
#ifdef QT_SOLARIS
		//
		// Solaris 8 lacks device /dev/urandom and /dev/random  
		// unless installed patch 112438 (Sparc) or 112439 (x86),
		// So we have to simulate the function of pseudo-random number generator.
		// Please refer to http://www.openssl.org/support/faq.html#PROG9
		// 
		srand48(time(NULL));
		const int pseudo_random_number = 256;
		long alNum[pseudo_random_number];
		for (int i = 0; i < pseudo_random_number; i++) {
			alNum[i] = lrand48();
		}
		RAND_add(alNum, pseudo_random_number*sizeof(long), pseudo_random_number*sizeof(long));
#endif // QT_SOLARIS
		
#ifdef QT_MMP
		SSL_METHOD *pMeth = CQTSSL::SSLv23_method();
#else
		SSL_METHOD *pMeth = ::SSLv23_method();
#endif
		if (!pMeth) {
			TraceOpenSslError("CQtTransportOpenSsl::InitSsl, SSLv23_method() failed!", this);
			return -1;
		}

#ifdef QT_MMP
		s_pSslCtx = CQTSSL::SSL_CTX_new(pMeth);
#else
		s_pSslCtx = ::SSL_CTX_new(pMeth);
#endif
		if(!s_pSslCtx) {
			TraceOpenSslError("CQtTransportOpenSsl::InitSsl, SSL_CTX_new() failed!", this);
			return -1;
		}
		
//		::SSL_CTX_set_cipher_list(s_pSslCtx,
//			"ALL:!DH:!ADH:!AES:!IDEA:+EXPORT:+3DES:");

#ifdef QT_SUPPORT_OPENSSL_VERIFY
		// set to verify peer with CA certificate
#ifdef QT_DEBUG
#	ifdef QT_MMP
		CQTSSL::SSL_CTX_set_verify(s_pSslCtx, SSL_VERIFY_PEER, VerifyCallback);
#	else
		::SSL_CTX_set_verify(s_pSslCtx, SSL_VERIFY_PEER, VerifyCallback);
#   endif
#else // !QT_DEBUG
#   ifdef QT_MMP
		CQTSSL::SSL_CTX_set_verify(s_pSslCtx, SSL_VERIFY_PEER, NULL);
#   else
		::SSL_CTX_set_verify(s_pSslCtx, SSL_VERIFY_PEER, NULL);
#   endif
#endif // QT_DEBUG

		CQtString strRoot(".");
#ifdef QT_WIN32
		char szTempPath[_MAX_PATH];
		if (::GetModuleFileNameA(NULL, szTempPath, _MAX_PATH)) {
			int ch = '\\';
			char* pEnd = ::strrchr(szTempPath, ch);
			if (pEnd)
				strRoot.assign(szTempPath, pEnd - szTempPath);
		}
#endif // WIN32
		strRoot += QT_OS_SEPARATE;
		strRoot += "root_ca_list.pem";
		
		if (::SSL_CTX_load_verify_locations(s_pSslCtx, strRoot.c_str(), NULL) <= 0) {
			TraceOpenSslError("CQtTransportOpenSsl::InitSsl, SSL_CTX_load_verify_locations() failed!", this);
			return -1;
		}
#endif // QT_SUPPORT_OPENSSL_VERIFY

#ifdef QT_MMP
		CQTSSL::SSL_CTX_ctrl(s_pSslCtx, SSL_CTRL_MODE, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);
#else
		::SSL_CTX_ctrl(s_pSslCtx, SSL_CTRL_MODE, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);
#endif // QT_MMP
	}

#ifdef QT_MMP
	m_pSsl = CQTSSL::SSL_new(s_pSslCtx);
#else
	m_pSsl = ::SSL_new(s_pSslCtx);
#endif

	if (!m_pSsl) {
		TraceOpenSslError("CQtTransportOpenSsl::InitSsl, SSL_new() failed!", this);
		return -1;
	}

	BIO *pBio = CQtTransportOpenSslBio::CreateOne(this);
	if (!pBio) {
		TraceOpenSslError("CQtTransportOpenSsl::InitSsl, BIO_new() failed!", this);
		return -1;
	}

#ifdef QT_MMP
	CQTSSL::SSL_set_bio(m_pSsl, pBio, pBio);
#else
	::SSL_set_bio(m_pSsl, pBio, pBio);
#endif
	return 0;
}

int CQtTransportOpenSsl::VerifyCallback(int aOk, X509_STORE_CTX *aCtx)
{
	QT_INFO_TRACE("CQtTransportOpenSsl::VerifyCallback, error=" << aCtx->error << " aOk=" << aOk);

	switch (aCtx->error) {
	case X509_V_ERR_CERT_NOT_YET_VALID:
	case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
	case X509_V_ERR_CERT_HAS_EXPIRED:
	case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
		// to make HZ's SSL accelerator happy
//		QT_WARNING_LOG(("CQtTransportOpenSsl::VerifyCallback, error=%d aOk=%d", aCtx->error, aOk));
		aOk = 1;
		break;
	}
	return aOk;
}

int CQtTransportOpenSsl::OnInput(QT_HANDLE )
{
	QT_ASSERTE(m_pSink);
	
	int nRet = RecvFromSocket();
	if (nRet <= 0)
		return nRet;

	for ( ; ; ) {
		char szBuf[8*1024];
#ifdef QT_MMP
		int nRead = CQTSSL::SSL_read(m_pSsl, szBuf, sizeof(szBuf));
		int nErr = CQTSSL::SSL_get_error(m_pSsl, nRead);
#else
		int nRead = ::SSL_read(m_pSsl, szBuf, sizeof(szBuf));
		int nErr = ::SSL_get_error(m_pSsl, nRead);
#endif
		switch (nErr) {
			case SSL_ERROR_WANT_READ:
				QT_ASSERTE(m_mbSslRecvBuffer.GetTopLevelLength() == 0);
				m_mbSslRecvBuffer.RewindChained();
				return nRet == 1 ? -2 : 0;

			case SSL_ERROR_NONE: {
				QT_ASSERTE(nRead > 0);
				CQtMessageBlock mbOn(
					nRead, 
					szBuf, 
					CQtMessageBlock::DONT_DELETE, 
					nRead);
				
				QT_ASSERTE(m_pSink);
				if (m_pSink)
					m_pSink->OnReceive(mbOn, this);
				
				if (!m_pSsl) {
					// this transport is closed. we have to return
					return 0;
				}
				break;
			}
				
			default:
				QT_ERROR_TRACE_THIS("CQtTransportOpenSsl::OnInput , SSL_read() nErr = " << nErr);
				TraceOpenSslError("CQtTransportOpenSsl::OnInput, SSL_read() failed!", this);
				return -1;
		}
	}

	QT_ASSERTE(!"CQtTransportOpenSsl::OnInput, can't reach here!");
	return -1;
}

int CQtTransportOpenSsl::RecvFromSocket()
{
	QT_ASSERTE(m_mbSslRecvBuffer.GetTopLevelSpace() > 1460);

	int nRecv = CQtTransportTcp::Recv_i(
		m_mbSslRecvBuffer.GetTopLevelWritePtr(),
		m_mbSslRecvBuffer.GetTopLevelSpace());
	if (nRecv <= 0)
		return nRecv;

	int nRet = static_cast<DWORD>(nRecv) < m_mbSslRecvBuffer.GetTopLevelSpace() ? 1 : 2;
	m_mbSslRecvBuffer.AdvanceTopLevelWritePtr(nRecv);
	return nRet;
}

int CQtTransportOpenSsl::DoBioRecv(LPSTR aBuf, DWORD aLen, int &aErr)
{
	if (m_mbSslRecvBuffer.GetTopLevelLength() > 0) {
		DWORD dwCopy = 0;
		m_mbSslRecvBuffer.Read(aBuf, aLen, &dwCopy);
		
		QT_ASSERTE(dwCopy > 0 && dwCopy <= aLen);
		aErr = 0;
		return static_cast<int>(dwCopy);
	}
	else {
		aErr = EWOULDBLOCK;
		return -1;
	}
}

int CQtTransportOpenSsl::DoBioSend(LPCSTR aBuf, DWORD aLen, int &aErr)
{
	int nSend = CQtTransportTcp::Send_i(aBuf, aLen);
	if (nSend > 0) {
		aErr = 0;
		return nSend;
	}
	else if (nSend == 0) {
		aErr = EWOULDBLOCK;
		return nSend;
	}
	else {
		aErr = errno;
		QT_ASSERTE(aErr != EWOULDBLOCK);
		return nSend;
	}
}

SSL* CQtTransportOpenSsl::GetSslPtr()
{
	return m_pSsl;
}

QtResult CQtTransportOpenSsl::Close_t(QtResult aReason)
{
	if (m_pSsl) {
#ifdef QT_MMP
		CQTSSL::SSL_shutdown(m_pSsl);
		CQTSSL::SSL_free(m_pSsl);
#else
		::SSL_shutdown(m_pSsl);
		::SSL_free(m_pSsl);
#endif
		m_pSsl = NULL;
	}
	return CQtTransportTcp::Close_t(aReason);
}

QtResult CQtTransportOpenSsl::
SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara)
{
	if (aPara)
		aPara->m_dwHaveSent = 0;

	if (!m_pSsl) {
		QT_WARNING_TRACE_THIS("CQtTransportOpenSsl::SendData, m_pSsl is NULL.");
		return QT_ERROR_NOT_INITIALIZED;
	}

	CQtString strBuffer;
	char szBuffer[4096];
	LPCSTR pszSend;
	DWORD dwSend;
	if (aData.GetNext() == NULL) {
		pszSend = aData.GetTopLevelReadPtr();
		dwSend = aData.GetTopLevelLength();
	}
	else {
		LPSTR pszCopy;
		dwSend = aData.GetChainedLength();
		if (dwSend < sizeof(szBuffer)) {
			pszCopy = szBuffer;
		}
		else {
			strBuffer.reserve(dwSend + 1);
			pszCopy = const_cast<LPSTR>(strBuffer.c_str());
		}
		pszSend = pszCopy;

		CQtMessageBlock *pMbMove = &aData;
		while (pMbMove) {
			::memcpy(pszCopy, pMbMove->GetTopLevelReadPtr(), pMbMove->GetTopLevelLength());
			pszCopy += pMbMove->GetTopLevelLength();
			pMbMove = pMbMove->GetNext();
		}
		QT_ASSERTE(dwSend == (DWORD)(pszCopy - pszSend) );
	}

#ifdef QT_MMP
	int nSend = CQTSSL::SSL_write(m_pSsl, pszSend, dwSend);
	int nErr = CQTSSL::SSL_get_error(m_pSsl, nSend);
#else
	int nSend = ::SSL_write(m_pSsl, pszSend, dwSend);
	int nErr = ::SSL_get_error(m_pSsl, nSend);
#endif

//	QT_INFO_TRACE_THIS("CQtTransportOpenSsl::SendData,"
//		" len1=" << aData.GetTopLevelLength() << 
//		" nSend=" << nSend << 
//		" nErr=" << nErr);

	switch (nErr) {
	case SSL_ERROR_WANT_WRITE:
		QT_ASSERTE(nSend <= 0);
		if (m_pReactor->GetProperty() & IQtReactor::SEND_REGISTER_PROPERTY)
			m_pReactor->RegisterHandler(this, AQtEventHandler::READ_MASK | AQtEventHandler::WRITE_MASK);
		CQtTransportTcp::m_bNeedOnSend = TRUE;
		return QT_ERROR_PARTIAL_DATA;
		
	case SSL_ERROR_NONE:
		QT_ASSERTE(nSend > 0);
		aData.AdvanceChainedReadPtr(nSend);
		if (aPara)
			aPara->m_dwHaveSent = nSend;
		return QT_OK;
		
	default:
		TraceOpenSslError("CCQtTransportOpenSsl::SendData, SSL_write() failed!", this);

		// Don't NotifyHandler avoid blocking due to otify pipe overflow.
//		if (m_pReactor)
//			m_pReactor->NotifyHandler(this, AQtEventHandler::CLOSE_MASK);
		return QT_ERROR_NETWORK_SOCKET_ERROR;
	}
}

QtResult CQtTransportOpenSsl::GetOption(DWORD aCommand, LPVOID aArg)
{
	switch (aCommand) {
	case QT_OPT_TRANSPORT_TRAN_TYPE:
		*(static_cast<CQtConnectionManager::CType*>(aArg)) = m_Type;
		return QT_OK;

	default:
		return CQtTransportTcp::GetOption(aCommand, aArg);
	}
}
