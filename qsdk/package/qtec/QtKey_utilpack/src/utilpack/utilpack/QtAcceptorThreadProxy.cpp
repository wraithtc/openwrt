
#include "QtBase.h"
#include "QtAcceptorThreadProxy.h"
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

CQtAcceptorThreadProxy::
CQtAcceptorThreadProxy(CQtConnectionManager::CType aType,
					   AQtThread *aThreadNetwork,
					   AQtThread *aThreadUser)
	: CQtAcceptorConnectorSinkThreadProxyT<CQtAcceptorThreadProxy>(this)
	, m_pThreadUser(aThreadUser)
	, m_pThreadNetwork(aThreadNetwork)
	, m_Type(aType)
	, m_nRcvBuffLen(DEFAULT_RCVBUFF_SIZE)
	, m_nSndBuffLen(DEFAULT_SNDBUFF_SIZE)
{
	if (!m_pThreadNetwork) {
		m_pThreadNetwork = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_NETWORK);
		QT_ASSERTE(m_pThreadNetwork);
	}
	if (!m_pThreadUser) {
		m_pThreadUser = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_CURRENT);
		QT_ASSERTE(m_pThreadUser);
	}
	QT_ASSERTE(m_pThreadNetwork != m_pThreadUser);
}

CQtAcceptorThreadProxy::~CQtAcceptorThreadProxy()
{
	// the current thread is network thread due to <m_pAcceptorActual>.
//	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
}

DWORD CQtAcceptorThreadProxy::AddReference()
{
	return CQtReferenceControlMutilThread::AddReference();
}

DWORD CQtAcceptorThreadProxy::ReleaseReference()
{
	return CQtReferenceControlMutilThread::ReleaseReference();
}

void CQtAcceptorThreadProxy::OnReferenceDestory()
{
	// this assert helps to debug that the upper layer 
	// didn't call StopListen() before.
	QT_ASSERTE(CQtStopFlag::m_bStoppedFlag);

	// only the user thread can delete this due to <m_pAcceptorActual>,
	// so we have to post delete event to the network thread.
	if (CQtThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId())) {
//		QT_INFO_TRACE_THIS("CQtAcceptorThreadProxy::OnReferenceDestory,"
//			" post delete event to the network thread");
		CQtEventDeleteRefT<CQtAcceptorThreadProxy> *pEventDelete;
		pEventDelete = new CQtEventDeleteRefT<CQtAcceptorThreadProxy>(this);
		if (pEventDelete)
			pEventDelete->Launch(m_pThreadNetwork);
	}
	else {
		delete this;
	}
}

BOOL CQtAcceptorThreadProxy::IsConnector()
{
	return FALSE;
}

QtResult CQtAcceptorThreadProxy::
StartListen(IQtAcceptorConnectorSink *aSink, const CQtInetAddr &aAddrListen, int nTraceInterval)
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));
	QT_ASSERTE_RETURN(CQtStopFlag::IsFlagStopped(), QT_ERROR_ALREADY_INITIALIZED);

	// Don't check m_pAcceptorActual due to its operations are all in network thread.
//	QT_ASSERTE(!m_pAcceptorActual);
	
	QT_INFO_TRACE_THIS("CQtAcceptorThreadProxy::StartListen,"
		" aSink=" << aSink <<
		" addr=" << aAddrListen.GetIpDisplayName() << 
		" port=" << aAddrListen.GetPort());

	QT_ASSERTE(!CQtAcceptorConnectorSinkThreadProxyT<CQtAcceptorThreadProxy>::m_pSinkActual);
	QT_ASSERTE_RETURN(aSink, QT_ERROR_INVALID_ARG);
	CQtAcceptorConnectorSinkThreadProxyT<CQtAcceptorThreadProxy>::ResetSink(aSink);
	
	QtResult rv = QT_ERROR_OUT_OF_MEMORY;
	CEventStartListen *pEvent = new CEventStartListen(this, this, aAddrListen, nTraceInterval);
	if (pEvent) {
		// we must use SendEvent() to get the listen result.
		rv = m_pThreadNetwork->GetEventQueue()->SendEvent(pEvent);
	}

	if (QT_FAILED(rv)) {
		QT_WARNING_TRACE_THIS("CQtAcceptorThreadProxy::StartListen, SendEvent() failed.");
		CQtAcceptorConnectorSinkThreadProxyT<CQtAcceptorThreadProxy>::ResetSink(NULL);
	}
	else 
		CQtStopFlag::SetStartFlag();
	return rv;
}

QtResult CQtAcceptorThreadProxy::StopListen(QtResult aReason)
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));

	if (CQtStopFlag::IsFlagStopped())
		return QT_OK;
	CQtStopFlag::SetStopFlag();

//	QT_INFO_TRACE_THIS("CQtAcceptorThreadProxy::StopListen, aReason=" << aReason);
	
	CQtAcceptorConnectorSinkThreadProxyT<CQtAcceptorThreadProxy>::ResetSink(NULL);

	CEventStopListen *pEvent = new CEventStopListen(this, aReason);
	if (pEvent) {
#if 0
		// TODO : SendEvent() or PostEvent() when network thread is main thread?
		// If use SendEvent(), StartListen() will be invoked 
		// successfully if it is invoked after this function at once.
		// But the UDP transport will not SendData() successfully if 
		// the UDP socket is closed in UDP acceptor.
		if (CQtThreadManager::IsThreadEqual(m_pThreadUser->GetThreadId(), m_pThreadNetwork->GetThreadId()))
			return m_pThreadNetwork->GetEventQueue()->PostEvent(pEvent);
		else
			return m_pThreadNetwork->GetEventQueue()->SendEvent(pEvent);
#else
		// use PostEvent() in spite of network thread is main thread or serparator thread.
		return m_pThreadNetwork->GetEventQueue()->PostEvent(pEvent);
#endif
	}
	else 
		return QT_ERROR_OUT_OF_MEMORY;
}


//////////////////////////////////////////////////////////////////////
// class CEventStartListen
//////////////////////////////////////////////////////////////////////

CEventStartListen::
CEventStartListen(CQtAcceptorThreadProxy *aThreadProxy, 
				  IQtAcceptorConnectorSink *aSink, 
				  const CQtInetAddr &aAddrListen, int nTraceInterval)
	: m_pOwnerThreadProxy(aThreadProxy)
	, m_pSink(aSink)
	, m_addrListen(aAddrListen)
	, m_nTraceInterval(nTraceInterval)
{
}

CEventStartListen::~CEventStartListen()
{
}

QtResult CEventStartListen::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerThreadProxy->m_pThreadNetwork->GetThreadId()));

	// we must new actual acceptor in the network thread.
	QT_ASSERTE(!m_pOwnerThreadProxy->m_pAcceptorActual);
	CQtConnectionManager::CType type = m_pOwnerThreadProxy->m_Type;
	QT_CLR_BITS(type, CQtConnectionManager::CTYPE_INVOKE_MULTITHREAD);

	QtResult rv = CQtConnectionManager::Instance()->CreateConnectionServer(
		type, 
		m_pOwnerThreadProxy->m_pAcceptorActual.ParaOut());
	if (QT_FAILED(rv)) {
		QT_ERROR_TRACE_THIS("CEventStartListen::OnEventFire,"
			" can't create acceptor in the network thread. rv=" << rv);
		return rv;
	}
	m_pOwnerThreadProxy->
		m_pAcceptorActual->SetOption(QT_OPT_TRANSPORT_RCV_BUF_LEN, &m_pOwnerThreadProxy->m_nRcvBuffLen);
	m_pOwnerThreadProxy->
		m_pAcceptorActual->SetOption(QT_OPT_TRANSPORT_SND_BUF_LEN, &m_pOwnerThreadProxy->m_nSndBuffLen);
	rv = m_pOwnerThreadProxy->
		m_pAcceptorActual->StartListen(m_pSink, m_addrListen, m_nTraceInterval);
	if (QT_FAILED(rv))
		m_pOwnerThreadProxy->m_pAcceptorActual = NULL;
	return rv;
}


//////////////////////////////////////////////////////////////////////
// class CEventStopListen
//////////////////////////////////////////////////////////////////////

CEventStopListen::
CEventStopListen(CQtAcceptorThreadProxy *aConnectorThreadProxy,
				 QtResult aReason)
	: m_pOwnerThreadProxy(aConnectorThreadProxy)
	, m_Reason(aReason)
{
	QT_ASSERTE(m_pOwnerThreadProxy);
}

CEventStopListen::~CEventStopListen()
{
}

QtResult CEventStopListen::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerThreadProxy->m_pThreadNetwork->GetThreadId()));

	QT_ASSERTE(m_pOwnerThreadProxy->m_pAcceptorActual);
	if (m_pOwnerThreadProxy->m_pAcceptorActual) {
		QtResult rv = m_pOwnerThreadProxy->m_pAcceptorActual->StopListen(m_Reason);
		m_pOwnerThreadProxy->m_pAcceptorActual = NULL;
		return rv;
	}
	else
		return QT_ERROR_NULL_POINTER;
}
#endif
