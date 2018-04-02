/*------------------------------------------------------*/
/* UDP socket for adapting Conector pattern             */
/*                                                      */
/* QtConnectorUdp.h                                     */
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

#ifndef QTCONNECTORUDPT_H
#define QTCONNECTORUDPT_H

#include "QtReactorInterface.h"
#include "QtReferenceControl.h"
#include "QtInetAddr.h"
#include "QtObserver.h"
#include "QtDnsManager.h"

template <class UpperType, class UpTrptType, class UpSockType>
class QT_OS_EXPORT CQtConnectorUdpT 
	: public AQtEventHandler
	, public AQtConnectorInternal
	, public IQtTimerHandler
	, public IQtObserver 
{
public:
	CQtConnectorUdpT(IQtReactor *aReactor, UpperType &aUpper)
		: m_pReactor(aReactor)
		, m_Upper(aUpper)
		, m_pTransport(NULL)
		, m_bResolving(FALSE)
	{
	}
	
	virtual ~CQtConnectorUdpT()
	{
		Close();
	}

	// interface AQtConnectorInternal
	virtual int Connect(const CQtInetAddr &aAddr, CQtInetAddr *aAddrLocal = NULL)
	{
		const CQtInetAddr *pAddrConnect = &aAddr;
		if (aAddrLocal && aAddrLocal != &m_addrLocal)
			m_addrLocal = *aAddrLocal;

#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		if (!aAddr.IsResolved()) {
			m_addrUnResolved = aAddr;
			pAddrConnect = &m_addrUnResolved;

			CQtComAutoPtr<CQtDnsRecord> pRecord;
			CQtString strHostName = m_addrUnResolved.GetHostName();
			QtResult rv = CQtDnsManager::Instance()->AsyncResolve(
				pRecord.ParaOut(),
				strHostName,
				this);
			if (QT_SUCCEEDED(rv)) {
				DWORD dwIp = *(pRecord->begin());
				rv = m_addrUnResolved.SetIpAddrBy4Bytes(dwIp);
				if (QT_FAILED(rv)) {
					QT_WARNING_TRACE_THIS("CQtConnectorUdpT::Connect, wrong ip addr from DSN,"
						" dwIp=" << dwIp << " hostname=" << strHostName);
					return -1;
				}
				if (m_bResolving) {
					CQtDnsManager::Instance()->CancelResolve(this);
					m_bResolving = FALSE;
				}
			}
			else if (rv == QT_ERROR_WOULD_BLOCK) {
				m_bResolving = TRUE;
				return 0;
			}
			else
				return -1;
		}
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME

		int nRet = 0;
		QT_ASSERTE_RETURN(!m_pTransport, -1);
		m_pTransport = new UpTrptType(m_pReactor, *pAddrConnect);
		if (!m_pTransport) 
			return -1;

		UpSockType &sockPeer = m_pTransport->GetPeer();
		QT_ASSERTE(sockPeer.GetHandle() == QT_INVALID_HANDLE);
		if (sockPeer.Open(m_addrLocal) == -1) {
			QT_WARNING_TRACE_THIS("CQtConnectorUdpT::Connect, m_Socket.Open() failed!"
				" addr=" << m_addrLocal.GetIpDisplayName() <<
				" port=" << m_addrLocal.GetPort() <<
				"err=" << errno);
			return -1;
		}

		//that we have an issue the network thread will be blocked on a receive on UDP failover if we have no set the socket with NON_BLOCK during failover
		if (sockPeer.Enable(CQtIPCBase::NON_BLOCK) == -1) {
			QT_ERROR_TRACE_THIS("CQtConnectorUdpT::Connect_i, Enable(NON_BLOCK) failed! err=" << errno);
			return -1;
		}

		nRet = ::connect(
			(QT_SOCKET)sockPeer.GetHandle(), 
			reinterpret_cast<const struct sockaddr *>(pAddrConnect->GetPtr()), 
			pAddrConnect->GetSize());
		if (nRet == -1) {
			QT_WARNING_TRACE_THIS("CQtConnectorUdpT::Connect, connect() failed!"
				" addr=" << pAddrConnect->GetIpDisplayName() <<
				" port=" << pAddrConnect->GetPort() <<
				" err=" << errno);
			return -1;
		}
		else {
			QT_INFO_TRACE_THIS("CQtConnectorUdpT::Connect, connect() successful."
				" addr=" << pAddrConnect->GetIpDisplayName() <<
				" port=" << pAddrConnect->GetPort() <<
				" fd=" << sockPeer.GetHandle());

#ifdef QT_SUPPORT_QOS
			CQtTransportBase::SetQos2Socket(sockPeer.GetHandle());
#endif // QT_SUPPORT_QOS

			// can't use NotifyHandler(WRITE_MASK) due to not RegiesterHandler.
#ifdef QT_DEBUG
			QtResult rv = 
#endif // QT_DEBUG
				m_pReactor->ScheduleTimer(this, NULL, CQtTimeValue::s_tvZero, 1);
#ifdef QT_DEBUG
			QT_ASSERTE(QT_SUCCEEDED(rv));
#endif
			return 0;
		}
	}

	virtual int Close(QtResult aReason = QT_OK)
	{
		if (m_pReactor)
			m_pReactor->CancelTimer(this);
		if (m_pTransport) {
			delete m_pTransport;
			m_pTransport = NULL;
		}
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		if (m_bResolving) {
			CQtDnsManager::Instance()->CancelResolve(this);
			m_bResolving = FALSE;
		}
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		return 0;
	}

	/// interface AQtEventHandler
	virtual QT_HANDLE GetHandle() const 
	{
		QT_ASSERTE_RETURN(m_pTransport, QT_INVALID_HANDLE);
		return m_pTransport->GetHandle();
	}

	virtual int OnClose(QT_HANDLE aFd, MASK aMask)
	{
		QT_WARNING_TRACE_THIS("CQtConnectorUdpT::OnClose, it's impossible!"
			" aFd=" << aFd <<
			" aMask=" << aMask);
		return 0;
	}

	void OnTimeout(const CQtTimeValue &aCurTime, LPVOID aArg)
	{
		UpTrptType* pTrans = m_pTransport;
		m_pTransport = NULL;
		m_Upper.OnConnectIndication(QT_OK, pTrans, this);
	}

	void OnObserve(LPCSTR aTopic, LPVOID aData)
	{
		QT_ASSERTE(CQtThreadManager::IsEqualCurrentThread(CQtThreadManager::TT_NETWORK));
		QT_ASSERTE(!strcmp(aTopic, "DnsManager"));

		int nErr = *static_cast<int*>(aData);
		if (nErr || Connect(m_addrUnResolved, &m_addrLocal) == -1) {
			Close();
			m_Upper.OnConnectIndication(
				QT_ERROR_NETWORK_DNS_FAILURE,
				NULL, 
				this);
		}
	}
	
private:
	IQtReactor *m_pReactor;
	UpperType &m_Upper;
	UpTrptType *m_pTransport;
	CQtInetAddr m_addrUnResolved;
	CQtInetAddr m_addrLocal;
	BOOL m_bResolving;
};

#endif // !QTCONNECTORUDPT_H
