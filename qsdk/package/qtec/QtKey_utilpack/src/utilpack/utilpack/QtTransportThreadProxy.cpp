
#include "QtBase.h"
#include "QtTransportThreadProxy.h"
#include "QtMessageBlock.h"

//////////////////////////////////////////////////////////////////////
// class CQtTransportThreadProxy
//////////////////////////////////////////////////////////////////////

CQtTransportThreadProxy::
CQtTransportThreadProxy(IQtTransport *aActual, 
						AQtThread *aThreadNetwork, 
						AQtThread *aThreadUser,
						CQtConnectionManager::CType aType)
	: m_pTransportActual(aActual)
	, m_pSinkActual(NULL)
	, m_pThreadUser(aThreadUser)
	, m_pThreadNetwork(aThreadNetwork)
	, m_Type(aType)
	, m_bIsBufferFull(FALSE)
	, m_bNeedOnSend(FALSE)
{
	QT_ASSERTE(m_pTransportActual);
	QT_ASSERTE(m_pThreadUser);
	QT_ASSERTE(m_pThreadNetwork);
	QT_ASSERTE(m_pThreadNetwork != m_pThreadUser);

	// we set start flag here to allow upper layer not invoke OpenWithSink.
	CQtStopFlag::SetStartFlag();
	CQtStopFlag::m_Est.Reset2ThreadId(m_pThreadUser->GetThreadId());
}

CQtTransportThreadProxy::~CQtTransportThreadProxy()
{
	// the current thread is network thread due to <m_pTransportActual>.
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
	if (m_pTransportActual) {
		m_pTransportActual->Disconnect(QT_OK);
		m_pTransportActual = NULL;
	}
}

DWORD CQtTransportThreadProxy::AddReference()
{
	return CQtReferenceControlMutilThread::AddReference();
}

DWORD CQtTransportThreadProxy::ReleaseReference()
{
	return CQtReferenceControlMutilThread::ReleaseReference();
}

void CQtTransportThreadProxy::OnReferenceDestory()
{
	// this assert helps to debug that the upper layer 
	// didn't call Disconnect() before.
	QT_ASSERTE(CQtStopFlag::m_bStoppedFlag);

	m_TimerReference0.Schedule(this, CQtTimeValue::s_tvZero, 1);
}

void CQtTransportThreadProxy::OnTimer(CQtTimerWrapperID* aId)
{
	QtResult rv;
	QT_ASSERTE(aId == &m_TimerReference0);
	
	// we must Cancel() timer in same thread as the Schedule().
	rv = m_TimerReference0.Cancel();
	QT_ASSERTE(rv == QT_ERROR_NOT_FOUND);
	
	// only the network thread can delete this due to <m_pTransportActual>,
	// so we have to post delete event to the network thread.
	if (CQtThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId())) {
//		QT_INFO_TRACE_THIS("CQtTransportThreadProxy::OnTimer,"
//			" post delete event to the network thread");
		CQtEventDeleteRefT<CQtTransportThreadProxy> *pEventDelete;
		pEventDelete = new CQtEventDeleteRefT<CQtTransportThreadProxy>(this);
		if (pEventDelete) {
			rv = pEventDelete->Launch(m_pThreadNetwork);
//			QT_ASSERTE(QT_SUCCEEDED(rv));
		}
	}
	else {
		QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
		delete this;
	}
}

QtResult CQtTransportThreadProxy::OpenWithSink(IQtTransportSink *aSink)
{
	QT_ASSERTE_RETURN(aSink, QT_ERROR_INVALID_ARG);
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));

	// we allow the upper layer invokes this function many times.
	QT_ASSERTE(m_pSinkActual != aSink);
	m_pSinkActual = aSink;
	return QT_OK;
}

IQtTransportSink* CQtTransportThreadProxy::GetSink()
{
	return m_pSinkActual;
}

QtResult CQtTransportThreadProxy::
SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara)
{
	DWORD dwTotal = aData.GetChainedLength();
	if (aPara)
		aPara->m_dwHaveSent = 0;

	if (m_bIsBufferFull) {
		m_bNeedOnSend = TRUE;
		return QT_ERROR_PARTIAL_DATA;
	}

	QtResult rv;
	if (CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId())) {
		rv = Send_i(&aData, aPara, FALSE);
	}
	else {
		CEventSendData *pEvent = new CEventSendData(this, aData, aPara);
		rv = m_pThreadNetwork->GetEventQueue()->PostEvent(pEvent);

		if (QT_SUCCEEDED(rv)) {
			rv = aData.AdvanceChainedReadPtr(dwTotal);
			QT_ASSERTE(QT_SUCCEEDED(rv));
			if (aPara)
				aPara->m_dwHaveSent = dwTotal;
		}
		else {
			QT_ERROR_TRACE_THIS("CQtTransportThreadProxy::SendData,"
				" PostEvent() failed!");
			QT_ASSERTE(rv != QT_ERROR_PARTIAL_DATA);
		}
	}
	return rv;
}

QtResult CQtTransportThreadProxy::SetOption(DWORD aCommand, LPVOID aArg)
{
	// this function should be invoked in the different threads.
	if (m_pTransportActual)
		return m_pTransportActual->SetOption(aCommand, aArg);
	else
		return QT_ERROR_NULL_POINTER;
}

QtResult CQtTransportThreadProxy::GetOption(DWORD aCommand, LPVOID aArg)
{
	// this function should be invoked in the different threads.
	if (!m_pTransportActual)
		return QT_ERROR_NULL_POINTER;

	switch(aCommand)
	{
	case QT_OPT_TRANSPORT_TRAN_TYPE: {
		DWORD dwTransType;
		QtResult rv = m_pTransportActual->GetOption(aCommand, &dwTransType);
		if (QT_SUCCEEDED(rv)) {
			CQtConnectionManager::CType *pType = static_cast<CQtConnectionManager::CType*>(aArg);
			QT_ASSERTE_RETURN(pType, QT_ERROR_INVALID_ARG);

			*pType = dwTransType;
			if (QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_INVOKE_MULTITHREAD))
				QT_SET_BITS(*pType, CQtConnectionManager::CTYPE_INVOKE_MULTITHREAD);
			else if (QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_INVOKE_NETWORK_THREAD))
				QT_SET_BITS(*pType, CQtConnectionManager::CTYPE_INVOKE_NETWORK_THREAD);
		}
		return rv; 
									 }
	
	case QT_OPT_LOWER_TRANSPORT: 
		*(static_cast<IQtTransport**>(aArg)) = m_pTransportActual.Get();
		return QT_OK;
	
	default:
		return m_pTransportActual->GetOption(aCommand, aArg);
	}
}

QtResult CQtTransportThreadProxy::Disconnect(QtResult aReason)
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));
	// allow upper layer not invoke OpenWithSink before.

	QT_INFO_TRACE_THIS("CQtTransportThreadProxy::Disconnect,"
		" aReason=" << aReason << 
		" tran=" << m_pTransportActual.Get() <<
		" stop flag = " << CQtStopFlag::IsFlagStopped());
	if (CQtStopFlag::IsFlagStopped())
		return QT_OK;
	CQtStopFlag::SetStopFlag();
	m_pSinkActual = NULL;
	
	CEventDisconnect *pEvent = new CEventDisconnect(this, aReason);
	return m_pThreadNetwork->GetEventQueue()->PostEvent(pEvent);
}

void CQtTransportThreadProxy::
OnReceive(CQtMessageBlock &aData, IQtTransport *aTrptId, 
		  CQtTransportParameter *aPara)
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
	QT_ASSERTE(aTrptId == m_pTransportActual.Get());
	
	CEventOnReceive *pEvent = new CEventOnReceive(this, aData, this, aPara);
	m_pThreadUser->GetEventQueue()->PostEvent(pEvent);
}

QtResult CQtTransportThreadProxy::
Send_i(CQtMessageBlock *aData, CQtTransportParameter* aPara, BOOL aIsDuplicated)
{
	// this function will take owner of <aData> if it is <aIsDuplicated>
	QtResult rv = QT_OK;
	QtResult rvSend = QT_OK;
	QT_ASSERTE_RETURN(m_pTransportActual, QT_ERROR_NULL_POINTER);
	
	// need not aData->DuplicateChained() if empty.
	if (m_SendBuffer.empty() && aData) {
		rv = m_pTransportActual->SendData(
			*aData, 
			aPara);
		if (QT_SUCCEEDED(rv) ||
			QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_UDP)) 
		{
			// delete <aData> to avoid memery leak.
			if (aIsDuplicated)
				aData->DestroyChained();
			return rv;
		}
		QT_ASSERTE(aData->GetChainedLength() > 0);
		// if failed, fall to next to buffer into the list.
		rvSend = rv;
	}

	// UDP should not be buffered.
	QT_ASSERTE(QT_BIT_DISABLED(m_Type, CQtConnectionManager::CTYPE_UDP));

	if (aData) {
		CItem itNew(
			aIsDuplicated ? aData : aData->DuplicateChained(), 
			aPara);
		m_SendBuffer.push_back(itNew);
		// assign itNew.m_pMbSend to NULL to avoid being destroyed
		itNew.m_pMbSend = NULL;

		if (!aIsDuplicated) {
			// It is OK if SendData() a half when m_SendBuffer.empty().
			DWORD dwTotal = aData->GetChainedLength();
			rv = aData->AdvanceChainedReadPtr(dwTotal);
			QT_ASSERTE(QT_SUCCEEDED(rv));
			if (aPara)
				aPara->m_dwHaveSent = dwTotal;
		}

		// mainly check for SendData() failed when m_SendBuffer.empty()
		if (QT_FAILED(rvSend))
			goto fail;
	}

	while (!m_SendBuffer.empty()) {
		CQtTransportThreadProxy::CItem &itTop = m_SendBuffer.front();
		rvSend = m_pTransportActual->SendData(
			*(itTop.m_pMbSend), 
			itTop.m_pParaTransportParameter);

		if (QT_FAILED(rvSend)) {
			QT_ASSERTE(itTop.m_pMbSend->GetChainedLength() > 0);
			goto fail;
		}
		else {
			QT_ASSERTE(itTop.m_pMbSend->GetChainedLength() == 0);
			m_SendBuffer.pop_front();
		}
	}
	m_bIsBufferFull = FALSE;
	return QT_OK;

fail:
	QT_ASSERTE(QT_FAILED(rvSend));
	if (rvSend != QT_ERROR_PARTIAL_DATA) {
		QT_ERROR_TRACE_THIS("CQtTransportThreadProxy::Send_i,"
			" SendData() failed. rvSend=" << rvSend);
	}
	m_bIsBufferFull = TRUE;
	// return OK because we have buffered the data.
	return QT_OK;
}

void CQtTransportThreadProxy::
OnSend(IQtTransport *aTrptId, CQtTransportParameter *aPara)
{
//	QT_INFO_TRACE_THIS("CQtTransportThreadProxy::OnSend aTrptId = " << aTrptId);
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
	QT_ASSERTE(aTrptId == m_pTransportActual.Get());

	if (QT_BIT_ENABLED(m_Type, CQtConnectionManager::CTYPE_UDP))
		return;

	Send_i(NULL, NULL, FALSE);

	// Do not need OnSend to upper layer if buffer full.
	if (m_bIsBufferFull) {
		QT_ASSERTE(!m_SendBuffer.empty());
		return;
	}

	CEventOnSend *pEvent = new CEventOnSend(this, this, aPara);
	m_pThreadUser->GetEventQueue()->PostEvent(pEvent);
}

void CQtTransportThreadProxy::
OnDisconnect(QtResult aReason, IQtTransport *aTrptId)
{
	QT_INFO_TRACE_THIS("CQtTransportThreadProxy::OnDisconnect aReason = " << aReason <<
		"aTrptId = " << aTrptId);
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
	if(aTrptId != m_pTransportActual.Get())
	{
		QT_ERROR_TRACE_THIS("CQtTransportThreadProxy::OnDisconnect input = " << aTrptId << " holder = " << m_pTransportActual.Get());
	}
	QT_ASSERTE(aTrptId == m_pTransportActual.Get());

	if (m_pTransportActual) {
		m_pTransportActual->Disconnect(aReason);
		// upper layer may invoke GetOpetion(), etc.
//		m_pTransportActual = NULL;
	}

	CEventOnDisconnect *pEvent = new CEventOnDisconnect(this, aReason, this);
	m_pThreadUser->GetEventQueue()->PostEvent(pEvent);
}


//////////////////////////////////////////////////////////////////////
// class CEventSendData
//////////////////////////////////////////////////////////////////////

CEventSendData::
CEventSendData(CQtTransportThreadProxy *aThreadProxy, 
			   CQtMessageBlock &aData, 
			   CQtTransportParameter *aPara)
	: m_pOwnerThreadProxy(aThreadProxy)
	, m_pParaTransportParameter(NULL)
{
	QT_ASSERTE(m_pOwnerThreadProxy);
	m_pMessageBlock = aData.DuplicateChained();
	QT_ASSERTE(m_pMessageBlock);
	if (aPara) {
		m_TransportParameter = *aPara;
		m_pParaTransportParameter = &m_TransportParameter;
	}
}

CEventSendData::~CEventSendData()
{
	if (m_pMessageBlock) {
		m_pMessageBlock->DestroyChained();
		m_pMessageBlock = NULL;
	}
}

QtResult CEventSendData::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerThreadProxy->m_pThreadNetwork->GetThreadId()));

	QtResult rv = m_pOwnerThreadProxy->Send_i(
		m_pMessageBlock, 
		m_pParaTransportParameter, 
		TRUE);
	m_pMessageBlock = NULL;
	return rv;
}


//////////////////////////////////////////////////////////////////////
// class CQtTransportThreadProxy::CItem
//////////////////////////////////////////////////////////////////////

CQtTransportThreadProxy::CItem::
CItem(CQtMessageBlock *aMb, CQtTransportParameter *aPara)
	: m_pMbSend(aMb)
	, m_pParaTransportParameter(NULL)
{
	QT_ASSERTE(m_pMbSend);
	if (aPara) {
		m_TransportParameter = *aPara;
		m_pParaTransportParameter = &m_TransportParameter;
	}
}

CQtTransportThreadProxy::CItem::~CItem()
{
	if (m_pMbSend)
		m_pMbSend->DestroyChained();
}

//////////////////////////////////////////////////////////////////////
// class CEventDisconnect
//////////////////////////////////////////////////////////////////////

CEventDisconnect::
CEventDisconnect(CQtTransportThreadProxy *aThreadProxy, 
				 QtResult aReason)
	: m_pOwnerThreadProxy(aThreadProxy)
	, m_Reason(aReason)
{
	QT_ASSERTE(m_pOwnerThreadProxy);
}

CEventDisconnect::~CEventDisconnect()
{
}

QtResult CEventDisconnect::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerThreadProxy->m_pThreadNetwork->GetThreadId()));

	QtResult rv = QT_ERROR_NULL_POINTER;
	if (m_pOwnerThreadProxy->m_pTransportActual) {
		rv = m_pOwnerThreadProxy->m_pTransportActual->Disconnect(m_Reason);
//		QT_ASSERTE(QT_SUCCEEDED(rv));
	}
	return rv;
}

//////////////////////////////////////////////////////////////////////
// class CEventOnReceive
//////////////////////////////////////////////////////////////////////

CEventOnReceive::
CEventOnReceive(CQtTransportThreadProxy *aThreadProxy, 
				CQtMessageBlock &aData, 
				IQtTransport *aTrptId, 
				CQtTransportParameter *aPara)
	: m_pOwnerThreadProxy(aThreadProxy)
	, m_pData(aData.DuplicateChained())
	, m_pTrptId(aTrptId)
	, m_pParaTransportParameter(NULL)
{
	QT_ASSERTE(m_pData);
	QT_ASSERTE(m_pOwnerThreadProxy);
	if (aPara) {
		m_TransportParameter = *aPara;
		m_pParaTransportParameter = &m_TransportParameter;
	}
}

CEventOnReceive::~CEventOnReceive()
{
	QT_ASSERTE(m_pData);
	
	if (m_pData) {
		m_pData->DestroyChained();
		m_pData = NULL;
	}
}

QtResult CEventOnReceive::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerThreadProxy->m_pThreadUser->GetThreadId()));

	if (m_pOwnerThreadProxy->CQtStopFlag::IsFlagStopped()) {
//		QT_WARNING_TRACE_THIS("CEventOnReceive::OnEventFire, stopped."
//			" m_pOwnerThreadProxy=" << m_pOwnerThreadProxy.Get());
		return QT_OK;
	}
	if (m_pOwnerThreadProxy->m_pSinkActual && m_pData) {
		m_pOwnerThreadProxy->m_pSinkActual->OnReceive(
			*m_pData, 
			m_pTrptId, 
			m_pParaTransportParameter);
		return QT_OK;
	}
	return QT_ERROR_NULL_POINTER;
}

//////////////////////////////////////////////////////////////////////
// class CEventOnSend
//////////////////////////////////////////////////////////////////////

CEventOnSend::
CEventOnSend(CQtTransportThreadProxy *aThreadProxy, 
				IQtTransport *aTrptId, 
				CQtTransportParameter *aPara)
	: m_pOwnerThreadProxy(aThreadProxy)
	, m_pTrptId(aTrptId)
	, m_pParaTransportParameter(NULL)
{
	QT_ASSERTE(m_pOwnerThreadProxy);
	if (aPara) {
		m_TransportParameter = *aPara;
		m_pParaTransportParameter = &m_TransportParameter;
	}
}

CEventOnSend::~CEventOnSend()
{
}

QtResult CEventOnSend::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerThreadProxy->m_pThreadUser->GetThreadId()));

	if (m_pOwnerThreadProxy->CQtStopFlag::IsFlagStopped()) {
//		QT_WARNING_TRACE_THIS("CEventOnSend::OnEventFire, stopped."
//			" m_pOwnerThreadProxy=" << m_pOwnerThreadProxy.Get());
		return QT_OK;
	}
	
	if (m_pOwnerThreadProxy->m_pSinkActual) {
		if (!m_pOwnerThreadProxy->m_bNeedOnSend) {
//			QT_INFO_TRACE_THIS("CEventOnSend::OnEventFire, needn't OnSend.");
			return QT_OK;
		}
		m_pOwnerThreadProxy->m_bNeedOnSend = FALSE;

		m_pOwnerThreadProxy->m_pSinkActual->OnSend(
			m_pTrptId, 
			m_pParaTransportParameter);
		return QT_OK;
	}
	return QT_ERROR_NULL_POINTER;
}

//////////////////////////////////////////////////////////////////////
// class CEventOnDisconnect
//////////////////////////////////////////////////////////////////////

CEventOnDisconnect::
CEventOnDisconnect(CQtTransportThreadProxy *aThreadProxy, 
				   QtResult aReason,
				   IQtTransport *aTrptId)
	: m_pOwnerThreadProxy(aThreadProxy)
	, m_Reason(aReason)
	, m_pTrptId(aTrptId)
{
	QT_ASSERTE(m_pOwnerThreadProxy);
}

CEventOnDisconnect::~CEventOnDisconnect()
{
}

QtResult CEventOnDisconnect::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerThreadProxy->m_pThreadUser->GetThreadId()));

	if (m_pOwnerThreadProxy->CQtStopFlag::IsFlagStopped()) {
		QT_WARNING_TRACE_THIS("CEventOnDisconnect::OnEventFire, stopped."
			" m_pOwnerThreadProxy=" << m_pOwnerThreadProxy.Get());
		return QT_OK;
	}
	m_pOwnerThreadProxy->CQtStopFlag::SetStopFlag();
	
	if (m_pOwnerThreadProxy->m_pSinkActual) {
		m_pOwnerThreadProxy->m_pSinkActual->OnDisconnect(
			m_Reason,
			m_pTrptId);
		return QT_OK;
	}
	return QT_ERROR_NULL_POINTER;
}
