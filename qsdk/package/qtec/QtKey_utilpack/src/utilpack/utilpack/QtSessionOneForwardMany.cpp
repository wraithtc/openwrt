
#include "QtBase.h"
#include "QtSessionOneForwardMany.h"
#include "QtThreadManager.h"
#include "QtMessageBlock.h"
#include "QtEventQueueBase.h"

CQtSessionOneForwardMany::CQtSessionOneForwardMany()
{
	m_pThreadNetwork = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_NETWORK);
	QT_ASSERTE(m_pThreadNetwork);
}

CQtSessionOneForwardMany::~CQtSessionOneForwardMany()
{
}

DWORD CQtSessionOneForwardMany::AddReference()
{
	return CQtReferenceControlMutilThread::AddReference();
}

DWORD CQtSessionOneForwardMany::ReleaseReference()
{
	return CQtReferenceControlMutilThread::ReleaseReference();
}

QtResult CQtSessionOneForwardMany::AddTransport(IQtTransport *aTrpt)
{
	QT_ASSERTE_RETURN(aTrpt, QT_ERROR_INVALID_ARG);

	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	m_Trpts.push_back(aTrpt);
	return QT_OK;
}

QtResult CQtSessionOneForwardMany::RemoveTransport(IQtTransport *aTrpt)
{
	QT_ASSERTE_RETURN(aTrpt, QT_ERROR_INVALID_ARG);

#if 0
	CQtConnectionManager::CType type = CQtConnectionManager::CTYPE_NONE;
	aTrpt->GetOption(QT_OPT_TRANSPORT_TRAN_TYPE, &type);
	QT_ASSERTE(QT_BIT_ENABLED(type, CQtConnectionManager::CTYPE_UDP));
#endif // 0

	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	TrptsType::iterator iter = std::find(
		m_Trpts.begin(), 
		m_Trpts.end(),
		aTrpt);
	if (iter != m_Trpts.end()) {
		m_Trpts.erase(iter);
		return QT_OK;
	}
	else
		return QT_ERROR_NOT_FOUND;
}

QtResult CQtSessionOneForwardMany::RemoveAllTransports()
{
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	m_Trpts.clear();
	return QT_OK;
}

QtResult CQtSessionOneForwardMany::
SendDataToAll(CQtMessageBlock &aData, CQtTransportParameter *aPara, 
			  IQtTransport *aSender)
{
	DWORD dwTotal = aData.GetChainedLength();
	if (aPara)
		aPara->m_dwHaveSent = 0;

	QtResult rv;
	if (CQtThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId())) {
		Send_i(aData, aPara, aSender);
		rv = QT_OK;
	}
	else {
		CEventSendDataToAll *pEvent = new CEventSendDataToAll(
			this, aData, aPara, aSender);
		rv = m_pThreadNetwork->GetEventQueue()->PostEvent(pEvent);
	}

	if (QT_SUCCEEDED(rv)) {
		rv = aData.AdvanceChainedReadPtr(dwTotal);
		QT_ASSERTE(QT_SUCCEEDED(rv));
		if (aPara)
			aPara->m_dwHaveSent = dwTotal;
	}
	else {
		QT_ERROR_TRACE_THIS("CQtSessionOneForwardMany::SendDataToAll,"
			" PostEvent() failed!");
		QT_ASSERTE(rv != QT_ERROR_PARTIAL_DATA);
	}
	return rv;
}

QtResult CQtSessionOneForwardMany::
Send_i(CQtMessageBlock &aData, CQtTransportParameter *aPara, IQtTransport *aSender)
{
	DWORD dwSendSuccessful = 0;
	DWORD dwSendFailed = 0;
	DWORD dwSendSkip = 0;
	
#ifndef QT_DISABLE_EVENT_REPORT
	CQtTimeValue tvCur = CQtTimeValue::GetTimeOfDay();
#endif // !QT_DISABLE_EVENT_REPORT
	
	{
		CQtMutexGuardT<CQtSessionOneForwardMany::MutexType> theGuard(m_Mutex);

		QT_ASSERTE(!aData.GetNext());
		LPCSTR pStart = aData.GetTopLevelReadPtr();
		DWORD dwLen = aData.GetTopLevelLength();

		CQtSessionOneForwardMany::TrptsType::iterator iter = m_Trpts.begin();
		for ( ; iter != m_Trpts.end(); ++iter) {
			IQtTransport *pTrans = (*iter).Get();
			if (aSender == pTrans) {
				dwSendSkip++;
				continue;
			}

			// TODO: implement backward in CQtMessageBlock.
			CQtMessageBlock mbCopy(
				dwLen, 
				const_cast<char*>(pStart), 
				CQtMessageBlock::DONT_DELETE,
				dwLen);
			QtResult rv = pTrans->SendData(
				mbCopy, 
				aPara);
			if (QT_FAILED(rv)) {
				// we do nothing if send failed.
				dwSendFailed++;
			}
			else {
				dwSendSuccessful++;
			}
		}
	}
	
#ifndef QT_DISABLE_EVENT_REPORT
	CQtTimeValue tvSub = CQtTimeValue::GetTimeOfDay() - tvCur;
	if (tvSub > CQtEventQueueBase::s_tvReportInterval) {
		QT_WARNING_TRACE_THIS("CEventSendDataToAll::OnEventFire, report,"
			" sec=" << tvSub.GetSec() << 
			" usec=" << tvSub.GetUsec() <<
			" dwSendSuccessful=" << dwSendSuccessful << 
			" dwSendFailed=" << dwSendFailed << 
			" dwSendSkip=" << dwSendSkip);
	}
#endif // !QT_DISABLE_EVENT_REPORT
	return QT_OK;
}

//////////////////////////////////////////////////////////////////////
// class CEventSendDataToAll
//////////////////////////////////////////////////////////////////////

CEventSendDataToAll::
CEventSendDataToAll(CQtSessionOneForwardMany *aSession, 
			   CQtMessageBlock &aData, 
			   CQtTransportParameter *aPara,
			   IQtTransport *aSender)
	: m_pOwnerSession(aSession)
	, m_pParaTransportParameter(NULL)
	, m_pTransportender(aSender)
{
	QT_ASSERTE(m_pOwnerSession);

	if (aPara) {
		m_TransportParameter = *aPara;
		m_pParaTransportParameter = &m_TransportParameter;
	}

	m_pMessageBlock = aData.DuplicateChained();
	QT_ASSERTE(m_pMessageBlock);
}

CEventSendDataToAll::~CEventSendDataToAll()
{
	if (m_pMessageBlock)
		m_pMessageBlock->DestroyChained();
}

QtResult CEventSendDataToAll::OnEventFire()
{
	QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
		m_pOwnerSession->m_pThreadNetwork->GetThreadId()));

	if (m_pMessageBlock) {
		m_pOwnerSession->Send_i(
			*m_pMessageBlock, 
			m_pParaTransportParameter, 
			m_pTransportender.ParaIn());
	}
	return QT_OK;
}
