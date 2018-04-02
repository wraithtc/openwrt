
#include "QtBase.h"
#include "QtTransportOpenSslBio.h"
#include "QtTransportOpenSsl.h"
#ifdef QT_MMP
#include "cmssl.h"
#endif

static BIO_METHOD s_Methods =
{
	CQtTransportOpenSslBio::BIO_TYPE, 
	"QtTransport_BIO",
	CQtTransportOpenSslBio::BIO_write,
	CQtTransportOpenSslBio::BIO_read,
	CQtTransportOpenSslBio::BIO_puts,
	NULL, // BIO_gets
	CQtTransportOpenSslBio::BIO_ctrl,
	CQtTransportOpenSslBio::BIO_new,
	CQtTransportOpenSslBio::BIO_free,
	NULL
};

BIO* CQtTransportOpenSslBio::CreateOne(CQtTransportOpenSsl *aTrpt)
{
#ifdef QT_MMP
	BIO *pBIO = CQTSSL::BIO_new(&s_Methods);
#else
	BIO *pBIO = ::BIO_new(&s_Methods);
#endif
	if (pBIO) {
#ifdef QT_MMP
		CQTSSL::BIO_ctrl(pBIO, BIO_C_SET_FILE_PTR, 0, aTrpt);
#else
		::BIO_ctrl(pBIO, BIO_C_SET_FILE_PTR, 0, aTrpt);
#endif
	}
	return pBIO;
}

int CQtTransportOpenSslBio::BIO_new(BIO *aBIO)
{
	aBIO->init  = 0;    // not initialized
	aBIO->num   = 0;    // still zero ( we can use it )
	aBIO->ptr   = 0;    // will be pointer to CQtTransportOpenSsl
	aBIO->flags = 0;    //
	return 1;
}

int CQtTransportOpenSslBio::BIO_free (BIO *aBIO)
{
	if (aBIO == 0)
		return 0;
	
	if (aBIO->shutdown) {
		aBIO->ptr   = 0;
		aBIO->init  = 0;
		aBIO->num   = 0;
		aBIO->flags = 0;
	}
	return 1;
}

int CQtTransportOpenSslBio::BIO_read(BIO *aBIO, char *aBuf, int aLen)
{
	CQtTransportOpenSsl *pTrans = static_cast<CQtTransportOpenSsl *>(aBIO->ptr);
	QT_ASSERTE_RETURN(aBuf && aLen > 0, -1);

	int nErrVal = 0;
	BIO_clear_retry_flags(aBIO);
	int nRet = pTrans->DoBioRecv(aBuf, static_cast<DWORD>(aLen), nErrVal);

	if (nRet <= 0) {
		if(nErrVal == EWOULDBLOCK)
			BIO_set_retry_read(aBIO);
		return -1;
	}
	else
		return nRet;
}

int CQtTransportOpenSslBio::BIO_write(BIO *aBIO, const char *aBuf, int aLen)
{
	CQtTransportOpenSsl *pTrans = static_cast<CQtTransportOpenSsl *>(aBIO->ptr);
	QT_ASSERTE_RETURN(aBuf && aLen > 0, -1);

	int nErrVal = 0;
	BIO_clear_retry_flags(aBIO);
	int nRet = pTrans->DoBioSend(aBuf, static_cast<DWORD>(aLen), nErrVal);

	if (nRet <= 0) {
		if(nErrVal == EWOULDBLOCK)
			BIO_set_retry_write(aBIO);
		return -1;
	}
	else
		return nRet;
}

long CQtTransportOpenSslBio::BIO_ctrl(BIO *aBIO, int aQtd, long aNum, void *aPtr)
{
	long lRet = 0;
	switch (aQtd)
    {
	case BIO_C_SET_FILE_PTR:
		aBIO->shutdown = static_cast<int>(aNum);
		aBIO->ptr = aPtr;
		aBIO->init = 1;
		break;

	case BIO_CTRL_INFO:
		lRet = 0;
		break;

	case BIO_CTRL_GET_CLOSE:
		lRet = aBIO->shutdown;
		break;

	case BIO_CTRL_SET_CLOSE:
		aBIO->shutdown = static_cast<int>(aNum);
		break;

	case BIO_CTRL_PENDING:
	case BIO_CTRL_WPENDING:
		lRet = 0;
		break;

	case BIO_CTRL_DUP:
	case BIO_CTRL_FLUSH:
		lRet = 1;
		break;

	default:
		lRet = 0;
		break;
	}

	return lRet;
}

int CQtTransportOpenSslBio::BIO_puts(BIO *aBIO, const char *aStr)
{
	return BIO_write(aBIO, aStr, strlen(aStr));
}
