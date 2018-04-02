
#include "QtBase.h"
#include "QtConnectorThreadProxy.h"
#include "QtTransportThreadProxy.h"

//////////////////////////////////////////////////////////////////////
// class CQtConnectorThreadProxy
//////////////////////////////////////////////////////////////////////

CQtConnectorThreadProxy::
CQtConnectorThreadProxy(CQtConnectionManager::CType aType, 
						AQtThread *aThreadNetwork,
						AQtThread *aThreadUser)
	: CQtAcceptorConnectorSinkThreadProxyT<CQtConnectorThreadProxy>(this)
	, m_pThreadUser(aThreadUser)
	, m_pThreadNetwork(aThreadNetwork)
	, m_Type(aType)
{
	QT_INFO_TRACE_THIS("CQtConnectorThreadProxy::CQtConnectorThreadProxy");
	if (!m_pThreadNetwork) {
		m_pThreadNetwork = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_NETWORK);
		QT_ASSERTE(m_pThreadNetwork);
	}
	if (!m_pThreadUser) {
		m_pThreadUser = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_CURRENT);
		QT_ASSERTE(m_pThreadUser);
	}
	QT_ASSERTE(m_pThreadUser != m_pThreadNetwork);
}

CQtConnectorThreadProxy::~CQtConnectorThreadProxy()
{
	QT_INFO_TRACE_THIS("CQtConnectorThreadProxy::~CQtConnectorThreadProxy");
	// the current thread is network thread due to <m_pConActual>.
//	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
}

DWORD CQtConnectorThreadProxy::AddReference()
{
	return CQtReferenceControlMutilThread::AddReference();
}

DWORD CQtConnectorThreadProxy::ReleaseReference()
{
	return CQtReferenceControlMutilThread::ReleaseReference();
}

void CQtConnectorThreadProxy::OnReferenceDestory()
{
	// this assert helps to debug that the upper layer 
	// didn't call CancelConnect() before.
	QT_ASSERTE(CQtStopFlag::m_bStoppedFlag);

	// only the user thread can delete this due to <m_pConActual>,
	// so we have to post delete event to the network thread.
	if (CQtThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId())) {
//		QT_INFO_TRACE_THIS("CQtTransportThreadProxy::OnReferenceDestory,"
//			" post delete event to the network thread");
		CQtEventDeleteRefT<CQtConnectorThreadProxy> *pEventDelete;
		pEventDelete = new CQtEventDeleteRefT<CQtConnectorThreadProxy>(this);
		if (pEventDelete)
			pEventDelete->Launch(m_pThreadNetwork);
	}
	else {
		QT_INFO_TRACE_THIS("CQtConnectorThreadProxy::OnReferenceDestory");
		delete this;
	}
}

BOOL CQtConnectorThreadProxy::IsConnector()
{
	return TRUE;
}

void CQtConnectorThreadProxy::SetStopFlag()
{
	CQtAcceptorConnectorSinkThreadProxyT<CQtConnectorThreadProxy>::ResetSink(NULL);
	CQtStopFlag::SetStopFlag();
}

void CQtConnectorThreadProxy::
AsycConnect(IQtAcceptorConnectorSink *aSink,  
			const CQtInetAddr &aAddrPeer, 
			CQtTimeValue *aTimeout,
			CQtInetAddr *aAddrLocal)
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));
	QT_ASSERTE(CQtStopFlag::IsFlagStopped());

	// Don't check m_pConActual due to its operations are all in network thread.
//	QT_ASSERTE(!m_pConActual);

	QT_INFO_TRACE_THIS("CQtConnectorThreadProxy::AsycConnect,"
		" aSink=" << aSink <<
		" addr=" << aAddrPeer.GetIpDisplayName() << 
		" port=" << aAddrPeer.GetPort() << 
		" sec=" << (aTimeout ? aTimeout->GetSec() : -1) << 
		" usec=" << (aTimeout ? aTimeout->GetUsec() : -1));

	QT_ASSERTE(!CQtAcceptorConnectorSinkThreadProxyT<CQtConnectorThreadProxy>::m_pSinkActual);
	QT_ASSERTE(aSink);
	CQtAcceptorConnectorSinkThreadProxyT<CQtConnectorThreadProxy>::ResetSink(aSink);

	QtResult rv = QT_ERROR_OUT_OF_MEMORY;
	CEventAsycConnect *pEvent = new CEventAsycConnect(
		this, this, aAddrPeer, aTimeout, aAddrLocal);
	if (pEvent) {
		rv = m_pThreadNetwork->GetEventQueue()->PostEvent(pEvent);
	}

	if (QT_SUCCEEDED(rv))
		CQtStopFlag::SetStartFlag();

	// TODO: OnConnecteIndication(FAILED) if QT_FAILED(rv)
	QT_ASSERTE(QT_SUCCEEDED(rv));
}

void CQtConnectorThreadProxy::CancelConnect(QtResult aReason)
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));

	if (CQtStopFlag::IsFlagStopped())
		return;

	// invoke CQtConnectorThreadProxy::SetStopFlag() in order to ResetSink(NULL).
//	CQtStopFlag::SetStopFlag();
	CQtConnectorThreadProxy::SetStopFlag();

	QT_INFO_TRACE_THIS("CQtConnectorThreadProxy::CancelConnect,ref = " << CQtReferenceControlMutilThread::GetReference() << ", Reason = " << aReason);

	CEventCancelConnect *pEvent = new CEventCancelConnect(this, aReason);
	m_pThreadNetwork->GetEventQueue()->PostEvent(pEvent);
}


//////////////////////////////////////////////////////////////////////
// class CEventAsycConnect
//////////////////////////////////////////////////////////////////////

CEventAsycConnect::
CEventAsycConnect(CQtConnectorThreadProxy *aConnectorThreadProxy, 
				  IQtAcceptorConnectorSink *aSink, 
				  const CQtInetAddr &aAddrPeer, 
				  CQtTimeValue *aTimeout,
				  CQtInetAddr *aAddrLocal)
	: m_pOwnerThreadProxy(aConnectorThreadProxy)
	, m_pSink(aSink)
	, m_addrPeer(aAddrPeer)
	, m_pParaTimeout(NULL)
	, m_pParaAddrLocal(NULL)
{
	QT_ASSERTE(m_pOwnerThreadProxy);
	if (aTimeout) {
		m_tvTimeout = *aTimeout;
		m_pParaTimeout = &m_tvTimeout;
	}
	if (aAddrLocal) {
		m_addrLocal = *aAddrLocal;
		m_pParaAddrLocal = &m_addrLocal;
	}
}

CEventAsycConnect::~CEventAsycConnect()
{
}

QtResult CEventAsycConnect::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerThreadProxy->m_pThreadNetwork->GetThreadId()));

	// we must new actual connector in the network thread.
	// <m_pConActual> may not be NULL due to OnConnectIndicatoin don't assign it to NULL.
//	QT_ASSERTE(!m_pOwnerThreadProxy->m_pConActual);

	CQtConnectionManager::CType type = m_pOwnerThreadProxy->m_Type;
	QT_CLR_BITS(type, CQtConnectionManager::CTYPE_INVOKE_MULTITHREAD);

	QtResult rv = CQtConnectionManager::Instance()->CreateConnectionClient(
		type, 
		m_pOwnerThreadProxy->m_pConActual.ParaOut());
	if (QT_FAILED(rv)) {
		QT_ERROR_TRACE_THIS("CEventAsycConnect::OnEventFire,"
			" can't create connector in the network thread. rv=" << rv);
		QT_ASSERTE(FALSE);
		return rv;
	}

	if (m_pOwnerThreadProxy->m_pConActual) {
		m_pOwnerThreadProxy->m_pConActual->
			AsycConnect(m_pSink, m_addrPeer, m_pParaTimeout, m_pParaAddrLocal);
		return QT_OK;
	}
	else
		return QT_ERROR_NULL_POINTER;
}


//////////////////////////////////////////////////////////////////////
// class CEventCancelConnect
//////////////////////////////////////////////////////////////////////

CEventCancelConnect::
CEventCancelConnect(CQtConnectorThreadProxy *aConnectorThreadProxy, QtResult aReason)
	: m_pOwnerThreadProxy(aConnectorThreadProxy)
    , m_Reason(aReason)
{
	QT_ASSERTE(m_pOwnerThreadProxy);
}

CEventCancelConnect::~CEventCancelConnect()
{
}

QtResult CEventCancelConnect::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerThreadProxy->m_pThreadNetwork->GetThreadId()));

	QT_ASSERTE(m_pOwnerThreadProxy->m_pConActual);
	if (m_pOwnerThreadProxy->m_pConActual) {
		m_pOwnerThreadProxy->m_pConActual->CancelConnect(m_Reason);
		m_pOwnerThreadProxy->m_pConActual = NULL;
		return QT_OK;
	}
	else
		return QT_ERROR_NULL_POINTER;
}

