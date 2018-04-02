/*------------------------------------------------------*/
/* Thread proxy class for IQtAcceptorConnectorSink      */
/*                                                      */
/* QtAcceptorConnectorSinkThreadProxy.h                 */
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

#ifndef QTACCEPTORCONNECTORSINKTHREADPROXY_H
#define QTACCEPTORCONNECTORSINKTHREADPROXY_H

#include "QtTransportThreadProxy.h"

// gdb5.2 don't support namespace debug due to limition of g++3.2,
// VC6 without SP complain friend calss embeded class.
// so we have to disable embeded class in order to debug on Redhat8 and make VC6 happy.
#define QT_DISABLE_EMBEDED_CLASS 1

	template <class ThreadProxyType>
	class CEventOnConnectIndication : public IQtEvent
	{
	public:
		CEventOnConnectIndication(
				ThreadProxyType *aConnectorThreadProxy,
				QtResult aReason,
				IQtTransport *aTrpt,
				IQtAcceptorConnectorId *aRequestId)
			: m_pOwnerThreadProxy(aConnectorThreadProxy)
			, m_Reason(aReason)
			, m_pTrpt(aTrpt)
			, m_pRequestId(aRequestId)
		{
			QT_ASSERTE(m_pOwnerThreadProxy);
		}

		virtual ~CEventOnConnectIndication() { }

		virtual QtResult OnEventFire()
		{
			QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
				m_pOwnerThreadProxy->m_pThreadUser->GetThreadId()));

			if (m_pOwnerThreadProxy->IsFlagStopped()) {
				QT_WARNING_TRACE_THIS("CQtAcceptorConnectorSinkThreadProxyT::"
					"CEventOnConnectIndication::OnEventFire, stopped."
					" m_pOwnerThreadProxy=" << m_pOwnerThreadProxy.Get());
				if (m_pTrpt)
					m_pTrpt->Disconnect(QT_ERROR_NOT_INITIALIZED);
				return QT_OK;
			}

			// SetStopFlag() will ResetSink to NULL, store it before.
			IQtAcceptorConnectorSink* pSink = m_pOwnerThreadProxy->m_pSinkActual;
			QT_ASSERTE(pSink);

			/// Stop connector to avoid it CannelConnect again.
			/// It should do nothing to the acceptor.
			m_pOwnerThreadProxy->SetStopFlag();

			if (pSink) {
				pSink->OnConnectIndication(
					m_Reason, 
					m_pTrpt.ParaIn(), 
					m_pRequestId);
				return QT_OK;
			}
			else
				return QT_ERROR_NULL_POINTER;
		}

	private:
		CQtComAutoPtr<ThreadProxyType> m_pOwnerThreadProxy;
		QtResult m_Reason;
		CQtComAutoPtr<IQtTransport> m_pTrpt;
		IQtAcceptorConnectorId *m_pRequestId;
	};

template <class ThreadProxyType>
class CQtAcceptorConnectorSinkThreadProxyT
	: public IQtAcceptorConnectorSink
{
public:
	CQtAcceptorConnectorSinkThreadProxyT(ThreadProxyType *pThreadProxy)
		: m_pSinkActual(NULL)
		, m_pThreadProxy(pThreadProxy)
	{
		QT_ASSERTE(m_pThreadProxy);
	}

	virtual ~CQtAcceptorConnectorSinkThreadProxyT() { }

	void ResetSink(IQtAcceptorConnectorSink *aSink)
	{
		QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
			m_pThreadProxy->m_pThreadUser->GetThreadId()));
		m_pSinkActual = aSink;
	}

	// interface IQtAcceptorConnectorSink
	virtual void OnConnectIndication(
		QtResult aReason,
		IQtTransport *aTrpt,
		IQtAcceptorConnectorId *aRequestId)
	{
		QT_ASSERTE(aRequestId == m_pThreadProxy->GetActualAcceptorConnectorId());
		QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(
			m_pThreadProxy->m_pThreadNetwork->GetThreadId()));
		
		CQtTransportThreadProxy *pTransportThreadProxy = NULL;
		if (QT_SUCCEEDED(aReason)) {
			pTransportThreadProxy = new CQtTransportThreadProxy(
				aTrpt, 
				m_pThreadProxy->m_pThreadNetwork, 
				m_pThreadProxy->m_pThreadUser,
				m_pThreadProxy->m_Type);
			if (!pTransportThreadProxy)
				aReason = QT_ERROR_OUT_OF_MEMORY;
			else 
				aReason = aTrpt->OpenWithSink(pTransportThreadProxy);

			if (QT_FAILED(aReason)) {
				delete pTransportThreadProxy;
				pTransportThreadProxy = NULL;

				if (!m_pThreadProxy->IsConnector()) {
					QT_WARNING_TRACE_THIS("CQtAcceptorConnectorSinkThreadProxyT::OnConnectIndication,"
						" It's acceptor, don't callback.");
					return;
				}
			}
		}
		
#ifdef QT_DEBUG
		if (!pTransportThreadProxy) {
			QT_ASSERTE(!aTrpt);
			QT_ASSERTE(QT_FAILED(aReason));
			QT_ASSERTE(m_pThreadProxy->IsConnector());
		}
#endif // QT_DEBUG

		QT_INFO_TRACE_THIS("CQtAcceptorConnectorSinkThreadProxyT::OnConnectIndication"
			" aReason=" << aReason << 
			" aTrpt=" << aTrpt << 
			" aRequestId=" << aRequestId << 
			" m_pThreadProxy=" << m_pThreadProxy);

		CEventOnConnectIndication<ThreadProxyType> *pEvent = 
			new CEventOnConnectIndication<ThreadProxyType>(
				m_pThreadProxy, 
				aReason, 
				pTransportThreadProxy, 
				m_pThreadProxy);
		m_pThreadProxy->m_pThreadUser->GetEventQueue()->PostEvent(pEvent);
	}

protected:
	IQtAcceptorConnectorSink *m_pSinkActual;
	ThreadProxyType *m_pThreadProxy;
};

#endif // !QTACCEPTORCONNECTORSINKTHREADPROXY_H
