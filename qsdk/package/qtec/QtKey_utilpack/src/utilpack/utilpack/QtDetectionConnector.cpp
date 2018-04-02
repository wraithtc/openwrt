#include "QtBase.h"
#include "QtDetectionConnector.h"

///////////////////////////////////////////
//class CQtDetectionConnector
///////////////////////////////////////////
CQtDetectionConnector::CQtDetectionConnector()
{
	m_wPriority = TOP_PRIORITY;
	m_bGetHighestPriority = FALSE;
	m_aSink = NULL;
	
	m_dwConnFailedCnt = 0;
	m_dwConnAddCnt = 0;
}

CQtDetectionConnector::~CQtDetectionConnector()
{
	m_aSink = NULL;

	CancelConnect(QT_OK);
//	m_conn_list.clear();
}

BOOL CQtDetectionConnector::IsConnector()
{
	return TRUE;
}

QtResult CQtDetectionConnector::AddConnection(
											  CQtConnectionManager::CType Type, 
											  const CQtInetAddr &aAddrPeer,
											  CQtTimeValue *aTimeDelay)
{
	QT_INFO_TRACE_THIS("CQtDetectionConnector::AddConnection type = " << Type << " addr = " << aAddrPeer.GetIpDisplayName() << " port = " << aAddrPeer.GetPort());
	CQtComAutoPtr<IQtConnector> pTPConnector, pBackTPConnector;	
	CQtConnectionManager::CType bType = Type & (~CQtConnectionManager::CTYPE_HTTP_NOPROXY_TRY_DIRECT), DirectType = CQtConnectionManager::CTYPE_NONE;
	QtResult rv = QT_ERROR_FAILURE;
	CQtConnectionManager *pManager = CQtConnectionManager::Instance();
	if(!pManager) 
	{
		QT_ERROR_TRACE_THIS("CQtDetectionConnector::AddConnection Connection manager cannot got");
		return rv;
	}
	rv = pManager->CreateConnectionClient(
		bType, 
		pTPConnector.ParaOut());
	
	if( QT_SUCCEEDED(rv) && Type & CQtConnectionManager::CTYPE_HTTP_NOPROXY_TRY_DIRECT)
	{
		if(Type & CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY)  //add a directly connection
		{
			DirectType = CQtConnectionManager::CTYPE_SSL_DIRECT | bType & (~CQtConnectionManager::CTYPE_SSL_WITH_BROWER_PROXY);
		}
		else if(Type & CQtConnectionManager::CTYPE_HTTP_WITH_BROWER_PROXY)
		{
			DirectType = CQtConnectionManager::CTYPE_HTTP_DIRECT| bType & (~CQtConnectionManager::CTYPE_HTTP_WITH_BROWER_PROXY);
		}
		else if(Type & CQtConnectionManager::CTYPE_HTTPS_WITH_BROWER_PROXY)
		{
			DirectType = CQtConnectionManager::CTYPE_HTTPS_DIRECT| bType & (~CQtConnectionManager::CTYPE_HTTPS_WITH_BROWER_PROXY);
		}
		else if(Type & CQtConnectionManager::CTYPE_TCP_WITH_BROWER_PROXY)
		{
			DirectType = CQtConnectionManager::CTYPE_TCP| bType & (~CQtConnectionManager::CTYPE_TCP_WITH_BROWER_PROXY);
		}
		if(DirectType != CQtConnectionManager::CTYPE_NONE )
		{
			QtResult ret = QT_ERROR_FAILURE;
			CQtConnectionManager* pConnectManager = CQtConnectionManager::Instance();
			if(pConnectManager)
			{
				ret = pConnectManager->CreateConnectionClient(
				DirectType, 
				pBackTPConnector.ParaOut());
			if(QT_FAILED(ret))
			{
				QT_ERROR_TRACE_THIS("CQtDetectionConnector::AddConnection back connection failed, rv = " << ret);
			}
		}
			else
			{
				QT_ERROR_TRACE_THIS("CQtDetectionConnector::AddConnection connection manager is null");
				return ret;
			}
		}
	}
	else if(QT_FAILED(rv))
	{
		QT_ERROR_TRACE_THIS("CQtDetectionConnector::AddConnection create connection failed, rv = " << rv);
		return QT_ERROR_FAILURE;
	}

	if(QT_SUCCEEDED(rv))
	{
		CQtComAutoPtr<CConnectorItem> pConnItem(new CConnectorItem(
			pTPConnector.ParaIn(),
			pBackTPConnector.ParaIn(),
			Type, 
			m_wPriority++, 
			aAddrPeer,
			this,
			aTimeDelay));
		
		m_conn_list.push_back(pConnItem);
	}
	
	return rv;		
}

void CQtDetectionConnector::StartDetectionConnect(
												  IQtAcceptorConnectorSink *aSink,
												  CQtTimeValue *aTimeout)
												  //CQtTimeValue *aTimeDelay)
{
	QT_ASSERTE(!m_conn_list.empty());
	
//	if(aTimeDelay)
//	{
//		m_TimeDelay.Set(
//			aTimeDelay->GetSec(), 
//			aTimeDelay->GetUsec()
//			);
//	}
	
	m_dwConnAddCnt = m_conn_list.size();
	m_bGetHighestPriority = FALSE;
	m_aSink = aSink;
	m_dwConnFailedCnt = 0;
	
	iter_type it_stop = m_conn_list.begin();
	iter_type it_end = m_conn_list.end();
	
	for(; it_stop != it_end; it_stop++)
		(*it_stop)->AsycConnect(aTimeout);
}

void CQtDetectionConnector::AsycConnect(
										IQtAcceptorConnectorSink *aSink,
										const CQtInetAddr &aAddrPeer, 
										CQtTimeValue *aTimeout,
										CQtInetAddr *aAddrLocal)
{
	//Cannot call this function here.
//	QT_ASSERTE(FALSE);
	// budingc turn on this function
	QT_ASSERTE(!aAddrLocal);

	iter_type it_stop = m_conn_list.begin();
	iter_type it_end = m_conn_list.end();
	for(; it_stop != it_end; it_stop++)
		(*it_stop)->SetAddrPeer(aAddrPeer);

	// TimeDelay should be set before calling this function.
	StartDetectionConnect(aSink, aTimeout );
}

void CQtDetectionConnector::CancelConnect(QtResult aReason)
{
	QT_INFO_TRACE_THIS("CQtDetectionConnector::CancelConnect cancel all");
	if(m_conn_list.empty())
		return;
	
	iter_type it_stop = m_conn_list.begin();
	iter_type it_end = m_conn_list.end();
	
	for(; it_stop != it_end; it_stop++)
	{
		if(NULL == (*it_stop).Get())
			continue;
			(*it_stop)->CancelConnect(aReason);
// 		if((*it_stop)->GetStatus() == CONNECTOR_STATUS_CONNECTED)
// 		{
// 			if((*it_stop)->m_pITransport)
// 				(*it_stop)->m_pITransport->Disconnect((QtResult)QT_OK);
// 		}
// 		if((*it_stop)->m_pConnector)
// 			(*it_stop)->m_pConnector->CancelConnect(aReason);
// 		if((*it_stop)->m_pBackConnector)
// 			(*it_stop)->m_pBackConnector->CancelConnect(aReason);
// 		(*it_stop)->SetStatus(CONNECTOR_STATUS_UNCONNECTED);
	}
}

void CQtDetectionConnector::CancelConnect(CConnectorItem* pExclude, QtResult aReason)
{
	QT_INFO_TRACE_THIS("CQtDetectionConnector::CancelConnect item = " << pExclude);
	if(m_conn_list.empty())
		return;
	
	iter_type it_stop = m_conn_list.begin();
	iter_type it_end = m_conn_list.end();
	
	for(; it_stop != it_end; it_stop++)
	{
		if((*it_stop).Get() == pExclude || NULL == (*it_stop).Get())
			continue;
		
		(*it_stop)->CancelConnect(aReason);
	}
}
/////////////////////////////////////////////////////////
//Inner class CQtDetectionConnector::CConnectorItem
/////////////////////////////////////////////////////////

void CQtDetectionConnector::CConnectorItem::CancelConnect(QtResult aReason)
{
	QT_INFO_TRACE_THIS("CQtDetectionConnector::CConnectorItem::CancelConnect aReaon = " << aReason);
	
	if(m_pITransport)
		m_pITransport->Disconnect((QtResult)QT_OK);
		
	if(m_pConnector)
		m_pConnector->CancelConnect(aReason);
	if(m_pBackConnector)
		m_pBackConnector->CancelConnect(aReason);
	SetStatus(CONNECTOR_STATUS_UNCONNECTED);
	m_StartDelayTimer.Cancel();
	m_Timer.Cancel();
}

CQtDetectionConnector::CConnectorItem::CConnectorItem(
													  IQtConnector *pIQtConnector,
													  IQtConnector *pBackIQtConnector,
													  CQtConnectionManager::CType aType, 
													  WORD wPriority, 
													  CQtInetAddr aAddrPeer, 
													  CQtDetectionConnector* pOuterClass,
													  CQtTimeValue* aTimeDelay )

{
	m_pConnector = pIQtConnector;
	m_pBackConnector = pBackIQtConnector;
	m_aType = aType;
	m_wPriority = wPriority;
	m_aAddrPeer = aAddrPeer;
	
	m_pOuterClass = pOuterClass;
	
	m_wStatus = CONNECTOR_STATUS_UNCONNECTED;
	
	// budingc 04/28/2006, if <aTimeDelay> less than zero, it will delay <aTimeDelay> to start connecting.
//	QT_ASSERTE( aTimeDelay );
//	m_TimeDelay = *aTimeDelay;
	if (aTimeDelay && *aTimeDelay < CQtTimeValue::s_tvZero)
		m_StartDelay = CQtTimeValue::s_tvZero - *aTimeDelay;
	else if (aTimeDelay)
	m_TimeDelay = *aTimeDelay;
	m_bStartDelayConnecting = FALSE;
	m_ConnectTimer = CQtTimeValue::s_tvZero;
}

CQtDetectionConnector::CConnectorItem::~CConnectorItem()
{
	//Empty
}

void CQtDetectionConnector::CConnectorItem::AsycConnect(CQtTimeValue *aTimeout)
{
	// budingc 04/28/2006.
	if (m_StartDelay > CQtTimeValue::s_tvZero) {
		QT_INFO_TRACE_THIS("CQtDetectionConnector::CConnectorItem::AsycConnect,"
			" delay start connection, sec=" << m_StartDelay.GetSec() << 
			" usec=" << m_StartDelay.GetUsec());
		if (aTimeout)
			m_SavedTimeoutByStartDelay = *aTimeout;
		m_StartDelayTimer.Schedule(this, m_StartDelay, 1);
		return;
	}

	m_pConnector->AsycConnect(
		this,
		m_aAddrPeer, 
		aTimeout);
	if(aTimeout)
		m_ConnectTimer = *aTimeout;
	else
		m_ConnectTimer = CQtTimeValue::s_tvZero;
}

void CQtDetectionConnector::CConnectorItem::CheckAndBeginStartDelay()
{
	if (m_StartDelay > CQtTimeValue::s_tvZero && !m_bStartDelayConnecting && 
		!m_pOuterClass->m_bGetHighestPriority && 
		m_pOuterClass->m_dwConnFailedCnt < m_pOuterClass->m_dwConnAddCnt) 
	{
		m_bStartDelayConnecting = TRUE;
		m_StartDelayTimer.Cancel();

		CQtTimeValue *pTimeout = NULL;
		if (m_SavedTimeoutByStartDelay > CQtTimeValue::s_tvZero)
			pTimeout = &m_SavedTimeoutByStartDelay;
		m_pConnector->AsycConnect(
			this,
			m_aAddrPeer, 
			pTimeout);
	}
}

void CQtDetectionConnector::CConnectorItem::IsAllFailed(QtResult aReason)
{
	if(++(m_pOuterClass->m_dwConnFailedCnt) 
		== (m_pOuterClass->m_dwConnAddCnt))//All connections failed
	{
		if(m_pOuterClass->m_aSink)
		{
			m_pOuterClass->m_aSink->OnConnectIndication(
				aReason,
				NULL,
				m_pOuterClass);	//Callback to Upper layer
		}

		m_pOuterClass->CancelConnect(QT_OK);
	}
}

void CQtDetectionConnector::CConnectorItem::OnConnectIndication(
																QtResult aReason,
																IQtTransport *aTrpt,
																IQtAcceptorConnectorId *aRequestId)
{
	QT_INFO_TRACE_THIS("CQtDetectionConnector::CConnectorItem::OnConnectIndication aReason = " << aReason <<
		" aTrpt = " << aTrpt << " aRequestId = " << aRequestId << " m_wPriority = " << m_wPriority);
	m_aReason = aReason;
	m_pITransport = aTrpt;

	QT_ASSERTE(m_pConnector.Get() == aRequestId || m_pBackConnector.Get() == aRequestId);
	if(aReason == QT_ERROR_NETWORK_NO_PROXY && m_pBackConnector)
	{
//		QT_INFO_TRACE_THIS("CQtDetectionConnector::CConnectorItem::OnConnectIndication have no proxy start direct now");
		m_pBackConnector->AsycConnect(this, m_aAddrPeer, m_ConnectTimer == CQtTimeValue::s_tvZero ? NULL : &m_ConnectTimer);
		return;
	}
	if(QT_FAILED(aReason))
	{
		IsAllFailed(aReason);

		// budingc 04/28/2006, check less priority, start connecting if not.
		CQtDetectionConnector::iter_type iter = m_pOuterClass->m_conn_list.begin();
		for( ; iter != m_pOuterClass->m_conn_list.end(); ++iter) {
			if ((*iter)->m_wPriority == m_wPriority + 1) {
				(*iter)->CheckAndBeginStartDelay();
				break;
			}
		}
		return;
	}
	
	if(m_wPriority == TOP_PRIORITY)
	{
		if(!m_pOuterClass->m_bGetHighestPriority)
		{
			BOOL bIsAlive = FALSE;
			m_pITransport->GetOption(QT_OPT_TRANSPORT_SOCK_ALIVE, &bIsAlive);
			
			if(bIsAlive)
			{
				QT_INFO_TRACE_THIS("CConnectorItem::OnConnectIndication(), Final connection Priority = TOP_PRIORITY");
				m_pOuterClass->m_bGetHighestPriority = TRUE;
				m_pOuterClass->m_pChampionConnector = m_pConnector;
				m_wStatus = CONNECTOR_STATUS_CONNECTED;
				
				if(m_pOuterClass->m_aSink)
				{
					m_pOuterClass->m_aSink->OnConnectIndication(
						m_aReason,
						m_pITransport.ParaIn(),
						m_pOuterClass);	//Callback to Upper layer
				}

				// Cancel all Connect except this
				m_pOuterClass->CancelConnect(this, QT_OK);
			}
			else
			{
				QT_WARNING_TRACE_THIS("CConnectorItem::OnConnectIndication(), Final connection Priority = TOP_PRIORITY is not alive");
				IsAllFailed((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
			}
		}
		else
		{
			QT_WARNING_TRACE_THIS("CConnectorItem::OnConnectIndication(), Priority = TOP_PRIORITY, have to Disconnect");
			m_pITransport->Disconnect((QtResult)QT_OK);	//Disconnect this connection
			m_wStatus = CONNECTOR_STATUS_UNCONNECTED;
		}
		return;
	}	
	else
	{
		if(!m_pOuterClass->m_bGetHighestPriority)	//Still have chance to become the champion
		{
			QT_INFO_TRACE_THIS("CConnectorItem::OnConnectIndication(), Priority != TOP_PRIORITY, have a chance, schedule timer");
			m_Timer.Schedule(
				this, 
				//m_pOuterClass->m_TimeDelay, 
				m_TimeDelay,
				1);
		}
		else	//No chance here
		{
			QT_INFO_TRACE_THIS("CConnectorItem::OnConnectIndication(), Priority != TOP_PRIORITY, no chance");
			m_pITransport->Disconnect((QtResult)QT_OK);	//Disconnect this connection
			m_wStatus = CONNECTOR_STATUS_UNCONNECTED;
		}
	}
}

void CQtDetectionConnector::CConnectorItem::OnTimer(CQtTimerWrapperID* aId)
{	
	if (&m_StartDelayTimer == aId) {
		QT_ASSERTE(!m_bStartDelayConnecting);
		CheckAndBeginStartDelay();
		return;
	}

	//After Delay, still cannot get higher priority connection
	if(!m_pOuterClass->m_bGetHighestPriority)
	{
		BOOL bIsAlive = FALSE;

		if(m_pITransport)
			m_pITransport->GetOption(QT_OPT_TRANSPORT_SOCK_ALIVE, &bIsAlive);
		
		if(bIsAlive)
		{
			QT_INFO_TRACE_THIS("CConnectorItem::OnTimer(), Final connection Priority = " << m_wPriority);
			m_pOuterClass->m_bGetHighestPriority = TRUE;
			m_pOuterClass->m_pChampionConnector = m_pConnector;
			m_wStatus = CONNECTOR_STATUS_CONNECTED;

			if(m_pOuterClass->m_aSink)
			{
				m_pOuterClass->m_aSink->OnConnectIndication(
					m_aReason,
					m_pITransport.ParaIn(),
					m_pOuterClass);	//Callback to Upper layer
			}

			// Cancel all Connect except this
			m_pOuterClass->CancelConnect(this, QT_ERROR_NETWORK_CONNECT_TIMEOUT);
		}
		else
		{
			QT_INFO_TRACE_THIS("CConnectorItem::OnTimer(), connection Priority = " << m_wPriority << "is not alive");
			IsAllFailed((QtResult)QT_ERROR_NETWORK_CONNECT_ERROR);
		}
	}
	else	//After Delay, some higher priority connection has made
	{
		QT_INFO_TRACE_THIS("CConnectorItem::OnTimer(), Disconnect Priority = " << m_wPriority);
		m_pITransport->Disconnect((QtResult)QT_OK);	//Disconnect this connection
		m_wStatus = CONNECTOR_STATUS_UNCONNECTED;
	}
}



