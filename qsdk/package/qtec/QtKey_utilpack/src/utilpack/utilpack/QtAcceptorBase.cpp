
#include "QtBase.h"
#include "QtAcceptorBase.h"
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

CQtAcceptorBase::CQtAcceptorBase()
	: m_pThreadNetwork(CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_NETWORK))
	, m_pReactor(NULL)
	, m_pReactorNetwork(m_pThreadNetwork? m_pThreadNetwork->GetReactor() : NULL)
//	, m_ReactorThreadProxy(m_pReactorNetwork, CQtThreadManager::TT_NETWORK)
	, m_pSink(NULL)
	, m_nRcvBuffLen(DEFAULT_RCVBUFF_SIZE)
	, m_nSndBuffLen(DEFAULT_SNDBUFF_SIZE)
	, m_Interval(1)
	, m_AcceptCount(0)
{
//	if (CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()))
//		m_pReactor = m_pReactorNetwork;
//	else
//		m_pReactor = &m_ReactorThreadProxy;
	
	QT_ASSERTE(m_pThreadNetwork);
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
	m_pReactor = m_pReactorNetwork;

	QT_ASSERTE(m_pReactor);
}

CQtAcceptorBase::~CQtAcceptorBase()
{
}

DWORD CQtAcceptorBase::AddReference()
{
	return CQtReferenceControlSingleThread::AddReference();
}

DWORD CQtAcceptorBase::ReleaseReference()
{
	return CQtReferenceControlSingleThread::ReleaseReference();
}

BOOL CQtAcceptorBase::IsConnector()
{
	return FALSE;
}
QtResult CQtAcceptorBase::SetOption(DWORD aCommand, LPVOID aArg)
{
	QT_INFO_TRACE_THIS("CQtAcceptorBase::SetOption aCommand = " << aCommand << " aArg = " << aArg);
	QT_ASSERTE_RETURN(aArg, QT_ERROR_FAILURE);
	switch(aCommand) {
	case QT_OPT_TRANSPORT_RCV_BUF_LEN:
		m_nRcvBuffLen = *(static_cast<int *>(aArg)) ;
		break;
	case QT_OPT_TRANSPORT_SND_BUF_LEN:
		m_nSndBuffLen = *(static_cast<int *>(aArg)) ;
		break;
	default:
		QT_ASSERTE_RETURN(0, QT_ERROR_FAILURE);
	}
	return QT_OK;
}

QtResult CQtAcceptorBase::GetOption(DWORD aCommand, LPVOID aArg)
{
	QT_INFO_TRACE_THIS("CQtAcceptorBase::GetOption aCommand = " << aCommand << " aArg = " << aArg);
	QT_ASSERTE_RETURN(aArg, QT_ERROR_FAILURE);
	switch(aCommand) {
	case QT_OPT_TRANSPORT_RCV_BUF_LEN:
		*(static_cast<int *>(aArg)) = m_nRcvBuffLen;
		break;
	case QT_OPT_TRANSPORT_SND_BUF_LEN:
		*(static_cast<int *>(aArg)) = m_nSndBuffLen;
		break;
	default:
		QT_ASSERTE_RETURN(0, QT_ERROR_FAILURE);
	}
	return QT_OK;
}

#endif
