

#include "QtBase.h"
#include "QtConnectionInterface.h"
#include "QtConnectorWrapper.h"
#include "QtDetectionConnector.h"
#include "QtAcceptorTcp.h"
#include "QtAcceptorUdp.h"
#include "QtThreadInterface.h"
#include "QtThreadManager.h"
#include "QtThreadReactor.h"
#include "QtConnectorThreadProxy.h"
#include "QtAcceptorThreadProxy.h"
#include "QtsRlb.h"
#include "QtsPacketSender.h"
#include "QtsLenPacketConnection.h"
#include "QtsConnectionAdapter.h"
#include "QtsRlbTcp.h"
#include "QtsPkg.h"
#include "QtsPkgSender.h"
#include "QtsLenPkg.h"

#if defined (_NEW_PROTO_TP)
//#ifdef QT_WIN32
//  #include "QtReactorWin32Message.h"
//#else
//  #include "QtReactorRealTimeSignal.h"
//#endif // QT_WIN32

typedef CConnectorT<CRLBClient>		CRLBConnector;
typedef CConnectorT<CPacketClient>	CPKGConnector;
typedef CConnectorT<CReconnClient>	CRECConnector;
typedef CConnectorT<CLenPacketConnection> CLenPKGConnector;
typedef CConnectorT<CPacketSenderConnection> CPkgSenderConnector;

typedef CAcceptorSinkT<CConnectionAdaptor>	CAcceptorAdaptorSink;
typedef CAcceptorT<CConnectionAdaptor>		CAcceptorAdaptor;
typedef CAcceptorSinkT<CLenPacketConnection> CLenAcceptorSink;
typedef CAcceptorT<CLenPacketConnection>	CLenAcceptor;
typedef CAcceptorSinkT<CPacketSenderConnection> CSenderAcceptorSink;
typedef CAcceptorT<CPacketSenderConnection>	CSenderAcceptor;

CQtConnectionManager CQtConnectionManager::s_ConnectionManagerSingleton;

CQtConnectionManager::CQtConnectionManager()
	: m_pThreadNetwork(NULL)
{
}

CQtConnectionManager::~CQtConnectionManager()
{
}

CQtConnectionManager* CQtConnectionManager::Instance()
{
	if (!s_ConnectionManagerSingleton.m_pThreadNetwork) {
		QtResult rv = s_ConnectionManagerSingleton.SpawnNetworkThread_i();
		if (QT_FAILED(rv)) {
			QT_WARNING_TRACE("CQtConnectionManager::Instance, SpawnNetworkThread_i() failed!"
				" rv=" << rv);
			return NULL;
		}
	}
	return &s_ConnectionManagerSingleton;
}

void CQtConnectionManager::CleanupInstance()
{
	s_ConnectionManagerSingleton.m_pThreadNetwork = NULL;
}

QtResult CQtConnectionManager::
CreateCsConnectionClient(CType &aType, IQtConnector *&aConClient)
{
	QtResult rv = QT_ERROR_FAILURE;

	//Decide Base Connection Type
	CType typeBaseConnection = (aType & CQtConnectionManager::CTYPE_TYPE_MASK);
	
	if( (aType & CQtConnectionManager::CTYPE_PDU_MASK) == CTYPE_PDU_RELIABLE)
	{
		//for compatible old version tp, that send udp data with rlb>! Victor 7/7
	//	QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);

		CQtComAutoPtr<IQtConnector> pTPConnector;
		rv = CreateConnectionClient_i(
			typeBaseConnection, 
			pTPConnector.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CRLBConnector> pConnConnector(
			new CRLBConnector(pTPConnector.ParaIn())
			);
		
		aType = CTYPE_PDU_RELIABLE;
		
		if(pConnConnector.Get()) {
			rv = QT_OK;
			aConClient = pConnConnector.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}

		if (aConClient)
			aConClient->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionClient(), CTYPE_PDU_RELIABLE, BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}
	else if(aType & CTYPE_PDU_RECONNECT)
	{
		//typeBaseConnection |= CTYPE_PDU_PACKAGE;
		CQtComAutoPtr<IQtConnector> pTPConnector;
		rv = CreateConnectionClient_i(
			typeBaseConnection, 
			pTPConnector.ParaOut());
		
		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CRECConnector> pConnConnector(
			new CRECConnector(pTPConnector.ParaIn())
			);
		
		aType = CTYPE_PDU_RECONNECT;
		
		if(pConnConnector.Get()) {
			rv = QT_OK;
			aConClient = pConnConnector.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}
		
		if (aConClient)
			aConClient->AddReference();
		
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionClient(), CTYPE_PDU_RECONNECT, BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}
	else if(QT_BIT_ENABLED(aType, CTYPE_SEND_NO_PARTIAL_DATA)) {
		QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);

		CQtComAutoPtr<IQtConnector> pTPConnector;
		rv = CreateConnectionClient_i(
			typeBaseConnection, 
			pTPConnector.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CPkgSenderConnector> pConnConnector(
			new CPkgSenderConnector(pTPConnector.ParaIn())
			);
		
		aType = CTYPE_SEND_NO_PARTIAL_DATA;
		
		if(pConnConnector.Get()) {
			rv = QT_OK;
			aConClient = pConnConnector.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}

		if (aConClient)
			aConClient->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionClient(), CTYPE_SEND_NO_PARTIAL_DATA, BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}
	else if(QT_BIT_ENABLED(aType, CTYPE_PDU_PACKAGE)) {
		//Decide whether need KeepAlive Mechanism
		BOOL bNeedKeepAlive = TRUE;
		
		if(QT_BIT_DISABLED(aType, CTYPE_PDU_KEEPALIVE))
			bNeedKeepAlive = FALSE;
		
		CQtComAutoPtr<IQtConnector> pTPConnector;
		rv = CreateConnectionClient_i(
			typeBaseConnection, 
			pTPConnector.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CPKGConnector> pConnConnector(
			new CPKGConnector(pTPConnector.ParaIn())
			);
		
		aType = CTYPE_PDU_PACKAGE;
		
		if(pConnConnector.Get()) {
			rv = QT_OK;
			aConClient = pConnConnector.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}

		if (aConClient)
			aConClient->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionClient(), CTYPE_PDU_PACKAGE, bNeedKeepAlive = " << bNeedKeepAlive << ", BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}
	else if(QT_BIT_ENABLED(aType, CTYPE_PDU_LENGTH)) {

		QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);

		CQtComAutoPtr<IQtConnector> pTPConnector;
		rv = CreateConnectionClient_i(
			typeBaseConnection, 
			pTPConnector.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CLenPKGConnector> pConnConnector(
			new CLenPKGConnector(pTPConnector.ParaIn())
			);
		
		aType = CTYPE_PDU_LENGTH;
		
		if(pConnConnector.Get()) {
			rv = QT_OK;
			aConClient = pConnConnector.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}

		if (aConClient)
			aConClient->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionClient(), CTYPE_PDU_LENGTH, BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}
	
	return rv;
}

QtResult CQtConnectionManager::
CreateConnectionClient_i(CType aType, IQtConnector *&aConClient)
{
	QtResult rv = QT_ERROR_FAILURE;
	QT_ASSERTE(!aConClient);
	aConClient = NULL;

	CType typeConnection = aType;
	QT_CLR_BITS(typeConnection, CTYPE_INVOKE_NETWORK_THREAD);
	QT_CLR_BITS(typeConnection, CTYPE_INVOKE_MULTITHREAD);

	/////////////////////Connection Service Add Begin/////////////////////
	CQtComAutoPtr<IQtConnector> pConConnector;
	if(QT_BIT_ENABLED(typeConnection, CTYPE_PDU_MASK) 
		|| QT_BIT_ENABLED(typeConnection, CTYPE_SEND_MASK)) {
		rv = CreateCsConnectionClient(typeConnection, pConConnector.ParaOut());
		if (QT_FAILED(rv)) {
			aConClient = NULL;
			return rv;
		}
	}
	/////////////////////Connection Service Add End//////////////////////
	
	switch(typeConnection) {
	case CTYPE_TCP: 
	case CTYPE_UDP:
	case CTYPE_SSL_DIRECT:
	case CTYPE_SSL_WITH_BROWER_PROXY:
	case CTYPE_SSL:
	case CTYPE_TCP_WITH_BROWER_PROXY:
	{
		std::auto_ptr<CQtConnectorWrapper> pCw(new CQtConnectorWrapper());
		if (pCw.get()) {
			rv = pCw->Init(typeConnection);
			if (QT_SUCCEEDED(rv))
				aConClient = pCw.release();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}
		break;
	}

	case CTYPE_TCP_WITH_BROWER_PROXY | CTYPE_TCP:
	{
		std::auto_ptr<CQtDetectionConnector> pConClient(new CQtDetectionConnector());
		if (!pConClient.get()) {
			rv = QT_ERROR_OUT_OF_MEMORY;
			break;
		}

		//pConClient->SetTimeDelay(CQtTimeValue(10L));

		CQtInetAddr addrNull;
		CQtTimeValue tv(0L);
		rv = pConClient->AddConnection(CTYPE_TCP, addrNull,&tv);
		if (QT_SUCCEEDED(rv)) {
			rv = pConClient->AddConnection(CTYPE_TCP_WITH_BROWER_PROXY, addrNull,&tv);
			if (QT_SUCCEEDED(rv))
				aConClient = pConClient.release();
		}
		break;
	}

	/////////////////////Connection Service Add Begin/////////////////////
	case CTYPE_PDU_RELIABLE:
	case CTYPE_PDU_PACKAGE:
	case CTYPE_SEND_NO_PARTIAL_DATA:
	case CTYPE_PDU_LENGTH:
	case CTYPE_PDU_RECONNECT:
	case CTYPE_PDU_RLB:
	{
		aConClient = pConConnector.Get();
		break;
	}
	/////////////////////Connection Service Add End//////////////////////
	default:
		QT_WARNING_TRACE_THIS("CQtConnectionManager::CreateConnectionClient_i, wrong type=" << aType);
		rv = QT_ERROR_INVALID_ARG;
		aConClient= NULL;
		break;
	}

#ifdef QT_DEBUG
	if (QT_SUCCEEDED(rv))
		QT_ASSERTE(aConClient);
	else
		QT_ASSERTE(!aConClient);
#endif // QT_DEBUG

	if (aConClient)
		aConClient->AddReference();
	return rv;
}

#if defined (USE_SOCKETSERVER)
QtResult CQtConnectionManager::
CreateCsConnectionServer(CType &aType, IQtAcceptor *&aAcceptor)
{
	QtResult rv = QT_ERROR_FAILURE;

	//Decide Base Connection Type
	CType typeBaseConnection = (aType & CQtConnectionManager::CTYPE_TYPE_MASK);
	LINK_TYPE  acceptType = INVALID_LINK;
	if((aType & CTYPE_PDU_MASK) == CTYPE_PDU_ANY) 
	{
		acceptType = ANY_LINK;
		aType = CTYPE_PDU_ANY;
	}
	else
	{
		if(QT_BIT_ENABLED(aType, CTYPE_PDU_RELIABLE) && QT_BIT_ENABLED(aType, CTYPE_PDU_RLB))
		{
//			QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP); //remove for compatible old version, Victor 7/12
			acceptType |= RLB_LINK;
			aType = CTYPE_PDU_RELIABLE;
		}
		else if(QT_BIT_ENABLED(aType,CTYPE_PDU_RECONNECT))
		{
			acceptType |= REC_LINK;
			aType = CTYPE_PDU_RECONNECT;
		}
		else if(QT_BIT_ENABLED(aType, CTYPE_PDU_PACKAGE))
		{
			acceptType |= PKG_LINK;
			aType = CTYPE_PDU_PACKAGE;
	}
		else if(QT_BIT_ENABLED(aType, CTYPE_SEND_NO_PARTIAL_DATA))
		{
		QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);
			acceptType |= SENDER_PKG_LINK;
			aType = CTYPE_SEND_NO_PARTIAL_DATA;
		}
		else if(QT_BIT_ENABLED(aType, CTYPE_PDU_LENGTH))
		{
			acceptType |= LEN_PKG_LINK;
			aType = CTYPE_PDU_LENGTH;
		}
	}
	QT_ASSERTE_RETURN(acceptType != INVALID_LINK, QT_ERROR_FAILURE);	
		
		CQtComAutoPtr<IQtAcceptor> pTPAcceptor;
		rv = CreateConnectionServer(
			typeBaseConnection, 
			pTPAcceptor.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		
	if(aType == CTYPE_PDU_LENGTH)
	{
		CQtComAutoPtr<CLenAcceptorSink> pAcceptorSink(new CLenAcceptorSink);
		CQtComAutoPtr<CLenAcceptor> pAcceptor = new CLenAcceptor(pTPAcceptor.ParaIn(), pAcceptorSink.ParaIn());
		if(pAcceptor.Get()) {
			rv = QT_OK;
			aAcceptor = pAcceptor.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aAcceptor= NULL;
		}

		if (aAcceptor)
			aAcceptor->AddReference();
	}
	else if(aType == CTYPE_SEND_NO_PARTIAL_DATA)
	{
		CQtComAutoPtr<CSenderAcceptorSink> pAcceptorSink(new CSenderAcceptorSink);
		CQtComAutoPtr<CSenderAcceptor> pAcceptor = new CSenderAcceptor(pTPAcceptor.ParaIn(), pAcceptorSink.ParaIn());
		if(pAcceptor.Get()) {
			rv = QT_OK;
			aAcceptor = pAcceptor.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aAcceptor= NULL;
		}

		if (aAcceptor)
			aAcceptor->AddReference();
	}
	else
	{
		CQtComAutoPtr<CAcceptorAdaptorSink> pAcceptorSink(new CAcceptorAdaptorSink(acceptType));
		CQtComAutoPtr<CAcceptorAdaptor> pAcceptor = new CAcceptorAdaptor(pTPAcceptor.ParaIn(), pAcceptorSink.ParaIn());
		if(pAcceptor.Get()) {
			rv = QT_OK;
			aAcceptor = pAcceptor.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aAcceptor= NULL;
		}

		if (aAcceptor)
			aAcceptor->AddReference();
	}

	QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionServer()");
	return rv;
}

QtResult CQtConnectionManager::
CreateConnectionServer_i(CType aType, IQtAcceptor *&aAcceptor)
{
	QtResult rv = QT_ERROR_FAILURE;
	QT_ASSERTE(!aAcceptor);
	aAcceptor = NULL;

	CType typeAcception = aType;
	QT_CLR_BITS(typeAcception, CTYPE_INVOKE_NETWORK_THREAD);
	QT_CLR_BITS(typeAcception, CTYPE_INVOKE_MULTITHREAD);
	
	/////////////////////Connection Service Add Begin/////////////////////
	CQtComAutoPtr<IQtAcceptor> pAcceptor = NULL;
	if(QT_BIT_ENABLED(typeAcception, CTYPE_PDU_MASK) 
		|| QT_BIT_ENABLED(typeAcception, CTYPE_SEND_MASK)) {
		rv = CreateCsConnectionServer(typeAcception, pAcceptor.ParaOut());
		if (QT_FAILED(rv)) {
			aAcceptor = NULL;
			return rv;
		}
	}
	/////////////////////Connection Service Add End//////////////////////
	
	switch(typeAcception) {
	case CTYPE_TCP: 
		aAcceptor = new CQtAcceptorTcp();
		if (aAcceptor)
			rv = QT_OK;
		break;

	case CTYPE_UDP:
		aAcceptor = new CQtAcceptorUdp();
		if (aAcceptor)
			rv = QT_OK;
		break;
	/////////////////////Connection Service Add Begin/////////////////////
	case CTYPE_PDU_RECONNECT:
	case CTYPE_PDU_RELIABLE:
	case CTYPE_SEND_NO_PARTIAL_DATA:
	case CTYPE_PDU_PACKAGE:
	case CTYPE_PDU_LENGTH:
	case CTYPE_PDU_ANY:
	{
		aAcceptor = pAcceptor.Get();
		break;
	}
	/////////////////////Connection Service Add End//////////////////////
	default:
		QT_WARNING_TRACE_THIS("CQtConnectionManager::CreateConnectionServer, wrong type=" << aType);
		rv = QT_ERROR_INVALID_ARG;
		break;
	}

#ifdef QT_DEBUG
	if (QT_SUCCEEDED(rv))
		QT_ASSERTE(aAcceptor);
	else
		QT_ASSERTE(!aAcceptor);
#endif // QT_DEBUG

	if (aAcceptor)
		aAcceptor->AddReference();
	return rv;
}
#endif //USE_SOCKETSERVER
QtResult CQtConnectionManager::
CreateDetectionConnectionClient(IQtDetectionConnector *&aConClient)
{
	QT_ASSERTE(!aConClient);
	QtResult rv = QT_OK;
	
	std::auto_ptr<CQtDetectionConnector> pCw(new CQtDetectionConnector());
	if (pCw.get()) {
			aConClient = pCw.release();
	}
	else {
		rv = QT_ERROR_OUT_OF_MEMORY;
		aConClient= NULL;
	}

	if(aConClient)
		aConClient->AddReference();

	return rv;
}

QtResult CQtConnectionManager::SpawnNetworkThread_i()
{
	// we have to instance CQtThreadManager because it will invoke SpawnNetworkThread_i().
	CQtThreadManager *pInst = CQtThreadManager::Instance();
	if (m_pThreadNetwork)
		return QT_OK;

	CQtThreadManager::TModule tmodule = CQtThreadManager::GetNetworkThreadModule();
	if (tmodule == CQtThreadManager::TM_SINGLE_MAIN) {
		std::auto_ptr<CQtThreadDummy> pThreadReactor(new CQtThreadDummy());
		if (!pThreadReactor.get())
			return QT_ERROR_OUT_OF_MEMORY;

		QtResult rv = pThreadReactor->Init(
			pInst->GetThread(CQtThreadManager::TT_MAIN), CQtThreadManager::TT_NETWORK);
		if (QT_FAILED(rv))
			return rv;

		m_pThreadNetwork = pThreadReactor.release();
	}
	else {
		IQtReactor *pReactorNetwork = CQtThreadManager::CreateNetworkReactor();
		QtResult rv = pInst->CreateReactorThread(
			CQtThreadManager::TT_NETWORK, pReactorNetwork, m_pThreadNetwork);
		if (QT_FAILED(rv))
			return rv;
	}

	return QT_OK;
}

QtResult CQtConnectionManager::
CreateConnectionClient(CType aType, IQtConnector *&aConClient)
{
	QT_ASSERTE(!aConClient);
#ifdef QT_DEBUG
	if (QT_BIT_ENABLED(aType, CTYPE_INVOKE_MULTITHREAD)) {
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_NETWORK_THREAD));
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionClient, CTYPE_INVOKE_MULTITHREAD.");
	}
	if (QT_BIT_ENABLED(aType, CTYPE_INVOKE_NETWORK_THREAD)) {
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_MULTITHREAD));
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionClient, CTYPE_INVOKE_NETWORK_THREAD.");
	}
#endif // QT_DEBUG
	
	AQtThread *pNetwork = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_NETWORK);
	if (CQtThreadManager::IsEqualCurrentThread(pNetwork->GetThreadId())
		&& QT_BIT_DISABLED(aType, CTYPE_INVOKE_MULTITHREAD)) 
	{
		// the current thread is the network thread,
		// and will not invoke functions in the mutil-thread.
		return CreateConnectionClient_i(aType, aConClient);
	}
	else {
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionClient, create CQtConnectorThreadProxy.");
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_NETWORK_THREAD));

		// we must new actual connector in the network thread.
//		CQtComAutoPtr<IQtConnector> pConActual;
//		QtResult rv = CreateConnectionClient_i(aType, pConActual.ParaOut());
//		if (QT_FAILED(rv))
//			return rv;

		CQtConnectorThreadProxy *pConProxy = 
			new CQtConnectorThreadProxy(aType, pNetwork);
		if (!pConProxy)
			return QT_ERROR_OUT_OF_MEMORY;

		aConClient = pConProxy;
		aConClient->AddReference();
		return QT_OK;
	}
}

#if defined (USE_SOCKETSERVER)
QtResult CQtConnectionManager::
CreateConnectionServer(CType aType, IQtAcceptor *&aAcceptor)
{
	QT_ASSERTE(!aAcceptor);
#ifdef QT_DEBUG
	if (QT_BIT_ENABLED(aType, CTYPE_INVOKE_MULTITHREAD)) {
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_NETWORK_THREAD));
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionServer, CTYPE_INVOKE_MULTITHREAD.");
	}
	if (QT_BIT_ENABLED(aType, CTYPE_INVOKE_NETWORK_THREAD)) {
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_MULTITHREAD));
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionServer, CTYPE_INVOKE_NETWORK_THREAD.");
	}
#endif // QT_DEBUG

	AQtThread *pNetwork = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_NETWORK);
	if (CQtThreadManager::IsEqualCurrentThread(pNetwork->GetThreadId())
		&& QT_BIT_DISABLED(aType, CTYPE_INVOKE_MULTITHREAD)) 
	{
		return CreateConnectionServer_i(aType, aAcceptor);
	}
	else
	{
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionServer, create CQtAcceptorThreadProxy.");
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_NETWORK_THREAD));

		CQtAcceptorThreadProxy *pAcceptorProxy = 
			new CQtAcceptorThreadProxy(aType, pNetwork);
		if (!pAcceptorProxy)
			return QT_ERROR_OUT_OF_MEMORY;

		aAcceptor = pAcceptorProxy;
		aAcceptor->AddReference();
		return QT_OK;
	}
}
#endif //USE_SOCKETSERVER
#else

//#ifdef QT_WIN32
//  #include "QtReactorWin32Message.h"
//#else
//  #include "QtReactorRealTimeSignal.h"
//#endif // QT_WIN32

CQtConnectionManager CQtConnectionManager::s_ConnectionManagerSingleton;

CQtConnectionManager::CQtConnectionManager()
	: m_pThreadNetwork(NULL)
{
}

CQtConnectionManager::~CQtConnectionManager()
{
}

CQtConnectionManager* CQtConnectionManager::Instance()
{
	if (!s_ConnectionManagerSingleton.m_pThreadNetwork) {
		QtResult rv = s_ConnectionManagerSingleton.SpawnNetworkThread_i();
		if (QT_FAILED(rv)) {
			QT_WARNING_TRACE("CQtConnectionManager::Instance, SpawnNetworkThread_i() failed!"
				" rv=" << rv);
			return NULL;
		}
	}
	return &s_ConnectionManagerSingleton;
}

void CQtConnectionManager::CleanupInstance()
{
	s_ConnectionManagerSingleton.m_pThreadNetwork = NULL;
}

QtResult CQtConnectionManager::
CreateCsConnectionClient(CType &aType, IQtConnector *&aConClient)
{
	QtResult rv = QT_ERROR_FAILURE;

	//Decide Base Connection Type
	CType typeBaseConnection = (aType & CQtConnectionManager::CTYPE_TYPE_MASK);
	
	if(QT_BIT_ENABLED(aType, CTYPE_PDU_PACKAGE) 
		&& QT_BIT_ENABLED(aType, CTYPE_PDU_KEEPALIVE) 
		&& QT_BIT_ENABLED(aType, CTYPE_PDU_RECONNECT)) 
	{
		//QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);
		CQtComAutoPtr<IQtConnector> pTPConnector;
		rv = CreateConnectionClient_i(
			typeBaseConnection, 
			pTPConnector.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CRlbConnTCPConnector> pConnConnector(
			new CRlbConnTCPConnector(pTPConnector.ParaIn())
			);
		
		aType = CTYPE_PDU_RELIABLE;
		
		if(pConnConnector.Get()) {
			rv = QT_OK;
			aConClient = pConnConnector.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}

		if (aConClient)
			aConClient->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionClient(), CTYPE_PDU_RELIABLE, BaseType = " << typeBaseConnection );
	}
	else if(QT_BIT_ENABLED(aType, CTYPE_SEND_NO_PARTIAL_DATA)) {
		QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);

		CQtComAutoPtr<IQtConnector> pTPConnector;
		rv = CreateConnectionClient_i(
			typeBaseConnection, 
			pTPConnector.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CPkgSenderConnector> pConnConnector(
			new CPkgSenderConnector(pTPConnector.ParaIn())
			);
		
		aType = CTYPE_SEND_NO_PARTIAL_DATA;
		
		if(pConnConnector.Get()) {
			rv = QT_OK;
			aConClient = pConnConnector.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}

		if (aConClient)
			aConClient->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionClient(), CTYPE_SEND_NO_PARTIAL_DATA, BaseType = " << typeBaseConnection);
	}
	else if(QT_BIT_ENABLED(aType, CTYPE_PDU_PACKAGE)) {
		//Decide whether need KeepAlive Mechanism
		BOOL bNeedKeepAlive = TRUE;
		
		if(QT_BIT_DISABLED(aType, CTYPE_PDU_KEEPALIVE))
			bNeedKeepAlive = FALSE;
		
		CQtComAutoPtr<IQtConnector> pTPConnector;
		rv = CreateConnectionClient_i(
			typeBaseConnection, 
			pTPConnector.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CPkgConnConnector> pConnConnector(
			new CPkgConnConnector(
			pTPConnector.ParaIn(), 
			bNeedKeepAlive)
			);
		
		aType = CTYPE_PDU_PACKAGE;
		
		if(pConnConnector.Get()) {
			rv = QT_OK;
			aConClient = pConnConnector.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}

		if (aConClient)
			aConClient->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionClient(), CTYPE_PDU_PACKAGE, bNeedKeepAlive = " << bNeedKeepAlive << ", BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}
	else if(QT_BIT_ENABLED(aType, CTYPE_PDU_LENGTH)) {

		QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);

		CQtComAutoPtr<IQtConnector> pTPConnector;
		rv = CreateConnectionClient_i(
			typeBaseConnection, 
			pTPConnector.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CLenPkgConnConnector> pConnConnector(
			new CLenPkgConnConnector(pTPConnector.ParaIn())
			);
		
		aType = CTYPE_PDU_LENGTH;
		
		if(pConnConnector.Get()) {
			rv = QT_OK;
			aConClient = pConnConnector.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}

		if (aConClient)
			aConClient->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionClient(), CTYPE_PDU_LENGTH, BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}
	
	return rv;
}

QtResult CQtConnectionManager::
CreateConnectionClient_i(CType aType, IQtConnector *&aConClient)
{
	QtResult rv = QT_ERROR_FAILURE;
	QT_ASSERTE(!aConClient);
	aConClient = NULL;

	CType typeConnection = aType;
	QT_CLR_BITS(typeConnection, CTYPE_INVOKE_NETWORK_THREAD);
	QT_CLR_BITS(typeConnection, CTYPE_INVOKE_MULTITHREAD);

	/////////////////////Connection Service Add Begin/////////////////////
	CQtComAutoPtr<IQtConnector> pConConnector;
	if(QT_BIT_ENABLED(typeConnection, CTYPE_PDU_MASK) 
		|| QT_BIT_ENABLED(typeConnection, CTYPE_SEND_MASK)) {
		rv = CreateCsConnectionClient(typeConnection, pConConnector.ParaOut());
		if (QT_FAILED(rv)) {
			aConClient = NULL;
			return rv;
		}
	}
	/////////////////////Connection Service Add End//////////////////////
	
	switch(typeConnection) {
	case (CType)CTYPE_TCP: 
	case (CType)CTYPE_UDP:
	case (CType)CTYPE_SSL_DIRECT:
	case (CType)CTYPE_SSL_WITH_BROWER_PROXY:
	case (CType)CTYPE_SSL:
	case (CType)CTYPE_TCP_WITH_BROWER_PROXY:
	case (CType)CTYPE_HTTP_WITH_BROWER_PROXY: //for HTTP proxy
	case (CType)CTYPE_WEBEX_GATEWAY_SSL_WITH_BROWER_PROXY:
	case (CType)CTYPE_WEBEX_GATEWAY_SSL_DIRECT:
	case (CType)CTYPE_WEBEX_GATEWAY_TCP_DIRECT:
	case (CType)CTYPE_WEBEX_GATEWAY_TCP_WITH_BROWSE_PROXY:
	{
		std::auto_ptr<CQtConnectorWrapper> pCw(new CQtConnectorWrapper());
		if (pCw.get()) {
			rv = pCw->Init(typeConnection);
			if (QT_SUCCEEDED(rv))
				aConClient = pCw.release();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aConClient= NULL;
		}
		break;
	}

	case (CType)(CTYPE_TCP_WITH_BROWER_PROXY | CTYPE_TCP):
	{
		std::auto_ptr<CQtDetectionConnector> pConClient(new CQtDetectionConnector());
		if (!pConClient.get()) {
			rv = QT_ERROR_OUT_OF_MEMORY;
			break;
		}

		//pConClient->SetTimeDelay(CQtTimeValue(10L));

		CQtInetAddr addrNull;
		CQtTimeValue tv(0L);
		rv = pConClient->AddConnection(CTYPE_TCP, addrNull,&tv);
		if (QT_SUCCEEDED(rv)) {
			rv = pConClient->AddConnection(CTYPE_TCP_WITH_BROWER_PROXY, addrNull,&tv);
			if (QT_SUCCEEDED(rv))
				aConClient = pConClient.release();
		}
		break;
	}

	/////////////////////Connection Service Add Begin/////////////////////
	case (CType)CTYPE_PDU_RELIABLE:
	case (CType)CTYPE_PDU_PACKAGE:
	case (CType)CTYPE_SEND_NO_PARTIAL_DATA:
	case (CType)CTYPE_PDU_LENGTH:
	{
		aConClient = pConConnector.Get();
		break;
	}
	/////////////////////Connection Service Add End//////////////////////
	default:
		QT_WARNING_TRACE_THIS("CQtConnectionManager::CreateConnectionClient_i, wrong type=" << aType);
		rv = QT_ERROR_INVALID_ARG;
		aConClient= NULL;
		break;
	}

#ifdef QT_DEBUG
	if (QT_SUCCEEDED(rv))
		QT_ASSERTE(aConClient);
	else
		QT_ASSERTE(!aConClient);
#endif // QT_DEBUG

	if (aConClient)
		aConClient->AddReference();
	return rv;
}
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

QtResult CQtConnectionManager::
CreateCsConnectionServer(CType &aType, IQtAcceptor *&aAcceptor)
{
	QtResult rv = QT_ERROR_FAILURE;

	//Decide Base Connection Type
	CType typeBaseConnection = (aType & CQtConnectionManager::CTYPE_TYPE_MASK);
	
	if(QT_BIT_ENABLED(aType, CTYPE_PDU_PACKAGE) 
		&& QT_BIT_ENABLED(aType, CTYPE_PDU_KEEPALIVE) 
		&& QT_BIT_ENABLED(aType, CTYPE_PDU_RECONNECT)) 
	{
		//QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);

		CQtComAutoPtr<IQtAcceptor> pTPAcceptor;
		rv = CreateConnectionServer(
			typeBaseConnection, 
			pTPAcceptor.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CRlbConnTCPAcceptorSink> pConnAcceptorSink(new CRlbConnTCPAcceptorSink);
		CQtComAutoPtr<CRlbConnTCPAcceptor> pConnAcceptor(
			new CRlbConnTCPAcceptor(
			pTPAcceptor.ParaIn(), 
			pConnAcceptorSink.ParaIn())
			);

		aType = CTYPE_PDU_RELIABLE;
		
		if(pConnAcceptor.Get()) {
			rv = QT_OK;
			aAcceptor = pConnAcceptor.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aAcceptor= NULL;
		}

		if (aAcceptor)
			aAcceptor->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionServer(), CTYPE_PDU_RELIABLE");
	}
	else if(QT_BIT_ENABLED(aType, CTYPE_SEND_NO_PARTIAL_DATA)) {
		QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);
		
		CQtComAutoPtr<IQtAcceptor> pTPAcceptor;
		rv = CreateConnectionServer(
			typeBaseConnection, 
			pTPAcceptor.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CPkgSenderAcceptorSink> pPkgSenderAcceptorSink(new CPkgSenderAcceptorSink);
		CQtComAutoPtr<CPkgSenderAcceptor> pConnAcceptor(
			new CPkgSenderAcceptor(
			pTPAcceptor.ParaIn(), 
			pPkgSenderAcceptorSink.ParaIn())
			);
		
		aType = CTYPE_SEND_NO_PARTIAL_DATA;
		
		if(pConnAcceptor.Get()) {
			rv = QT_OK;
			aAcceptor = pConnAcceptor.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aAcceptor= NULL;
		}

		if (aAcceptor)
			aAcceptor->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionServer(), CTYPE_SEND_NO_PARTIAL_DATA, BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}
	else if(QT_BIT_ENABLED(aType, CTYPE_PDU_PACKAGE)) {
		//Decide whether need KeepAlive Mechanism
		BOOL bNeedKeepAlive = TRUE;
		
		if(QT_BIT_DISABLED(aType, CTYPE_PDU_KEEPALIVE))
			bNeedKeepAlive = FALSE;
		
		CQtComAutoPtr<IQtAcceptor> pTPAccepctor;
		rv = CreateConnectionServer(
			typeBaseConnection, 
			pTPAccepctor.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CPkgConnAcceptorSink> pConnAcceptorSink(
			new CPkgConnAcceptorSink);

		CQtComAutoPtr<CPkgConnAcceptor> pConnAcceptor(
			new CPkgConnAcceptor(
			pTPAccepctor.ParaIn(), 
			pConnAcceptorSink.ParaIn(), 
			bNeedKeepAlive)
			);

		aType = CTYPE_PDU_PACKAGE;
		
		if(pConnAcceptor.Get()) {
			rv = QT_OK;
			aAcceptor = pConnAcceptor.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aAcceptor= NULL;
		}

		if (aAcceptor)
			aAcceptor->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionServer(), CTYPE_PDU_PACKAGE, bNeedKeepAlive = " << bNeedKeepAlive << ", BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}
	else if(QT_BIT_ENABLED(aType, CTYPE_PDU_LENGTH)) {

		QT_ASSERTE(typeBaseConnection != CQtConnectionManager::CTYPE_UDP);
		
		CQtComAutoPtr<IQtAcceptor> pTPAcceptor;
		rv = CreateConnectionServer(
			typeBaseConnection, 
			pTPAcceptor.ParaOut());

		if(QT_FAILED(rv))
			return rv;
		
		CQtComAutoPtr<CLenPkgConnAcceptorSink> pPkgSenderAcceptorSink(new CLenPkgConnAcceptorSink);
		CQtComAutoPtr<CLenPkgConnAcceptor> pConnAcceptor(
			new CLenPkgConnAcceptor(
			pTPAcceptor.ParaIn(), 
			pPkgSenderAcceptorSink.ParaIn())
			);
		
		aType = CTYPE_PDU_LENGTH;
		
		if(pConnAcceptor.Get()) {
			rv = QT_OK;
			aAcceptor = pConnAcceptor.Get();
		}
		else {
			rv = QT_ERROR_OUT_OF_MEMORY;
			aAcceptor= NULL;
		}

		if (aAcceptor)
			aAcceptor->AddReference();

		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateCsConnectionServer(), CTYPE_PDU_LENGTH, BaseType = " << ((typeBaseConnection == CTYPE_TCP) ? "TCP" : "UDP"));
	}

	return rv;
}

QtResult CQtConnectionManager::
CreateConnectionServer_i(CType aType, IQtAcceptor *&aAcceptor)
{
	QtResult rv = QT_ERROR_FAILURE;
	QT_ASSERTE(!aAcceptor);
	aAcceptor = NULL;

	CType typeAcception = aType;
	QT_CLR_BITS(typeAcception, CTYPE_INVOKE_NETWORK_THREAD);
	QT_CLR_BITS(typeAcception, CTYPE_INVOKE_MULTITHREAD);
	
	/////////////////////Connection Service Add Begin/////////////////////
	CQtComAutoPtr<IQtAcceptor> pConnAcceptor = NULL;
	if(QT_BIT_ENABLED(typeAcception, CTYPE_PDU_MASK) 
		|| QT_BIT_ENABLED(typeAcception, CTYPE_SEND_MASK)) {
		rv = CreateCsConnectionServer(typeAcception, pConnAcceptor.ParaOut());
		if (QT_FAILED(rv)) {
			aAcceptor = NULL;
			return rv;
		}
	}
	/////////////////////Connection Service Add End//////////////////////
	
	switch(typeAcception) {
	case CTYPE_TCP: 
		aAcceptor = new CQtAcceptorTcp();
		if (aAcceptor)
			rv = QT_OK;
		break;

	case CTYPE_UDP:
		aAcceptor = new CQtAcceptorUdp();
		if (aAcceptor)
			rv = QT_OK;
		break;
	/////////////////////Connection Service Add Begin/////////////////////
	case CTYPE_PDU_RELIABLE:
	case CTYPE_SEND_NO_PARTIAL_DATA:
	case CTYPE_PDU_PACKAGE:
	case CTYPE_PDU_LENGTH:
	{
		aAcceptor = pConnAcceptor.Get();
		break;
	}
	/////////////////////Connection Service Add End//////////////////////
	default:
		QT_WARNING_TRACE_THIS("CQtConnectionManager::CreateConnectionServer, wrong type=" << aType);
		rv = QT_ERROR_INVALID_ARG;
		break;
	}

#ifdef QT_DEBUG
	if (QT_SUCCEEDED(rv))
		QT_ASSERTE(aAcceptor);
	else
		QT_ASSERTE(!aAcceptor);
#endif // QT_DEBUG

	if (aAcceptor)
		aAcceptor->AddReference();
	return rv;
}
#endif
QtResult CQtConnectionManager::
CreateDetectionConnectionClient(IQtDetectionConnector *&aConClient)
{
	QT_ASSERTE(!aConClient);
	QtResult rv = QT_OK;
	
	std::auto_ptr<CQtDetectionConnector> pCw(new CQtDetectionConnector());
	if (pCw.get()) {
			aConClient = pCw.release();
	}
	else {
		rv = QT_ERROR_OUT_OF_MEMORY;
		aConClient= NULL;
	}

	if(aConClient)
		aConClient->AddReference();

	return rv;
}

QtResult CQtConnectionManager::SpawnNetworkThread_i()
{
	// we have to instance CQtThreadManager because it will invoke SpawnNetworkThread_i().
	CQtThreadManager *pInst = CQtThreadManager::Instance();
	QT_ASSERTE_RETURN(pInst, QT_ERROR_FAILURE);
	if (m_pThreadNetwork)
		return QT_OK;

	CQtThreadManager::TModule tmodule = CQtThreadManager::GetNetworkThreadModule();
	if (tmodule == CQtThreadManager::TM_SINGLE_MAIN) {
		std::auto_ptr<CQtThreadDummy> pThreadReactor(new CQtThreadDummy());
		if (!pThreadReactor.get())
			return QT_ERROR_OUT_OF_MEMORY;

		QtResult rv = pThreadReactor->Init(
			pInst->GetThread(CQtThreadManager::TT_MAIN), CQtThreadManager::TT_NETWORK);
		if (QT_FAILED(rv))
			return rv;

		m_pThreadNetwork = pThreadReactor.release();
	}
	else {
		IQtReactor *pReactorNetwork = CQtThreadManager::CreateNetworkReactor();
		QtResult rv = pInst->CreateReactorThread(
			CQtThreadManager::TT_NETWORK, pReactorNetwork, m_pThreadNetwork);
		if (QT_FAILED(rv))
			return rv;
	}

	return QT_OK;
}

QtResult CQtConnectionManager::
CreateConnectionClient(CType aType, IQtConnector *&aConClient)
{
	QT_ASSERTE(!aConClient);
#ifdef QT_DEBUG
	if (QT_BIT_ENABLED(aType, CTYPE_INVOKE_MULTITHREAD)) {
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_NETWORK_THREAD));
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionClient, CTYPE_INVOKE_MULTITHREAD.");
	}
	if (QT_BIT_ENABLED(aType, CTYPE_INVOKE_NETWORK_THREAD)) {
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_MULTITHREAD));
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionClient, CTYPE_INVOKE_NETWORK_THREAD.");
	}
#endif // QT_DEBUG
	
	AQtThread *pNetwork = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_NETWORK);
	QT_ASSERTE_RETURN(pNetwork, QT_ERROR_FAILURE);
	if (CQtThreadManager::IsEqualCurrentThread(pNetwork->GetThreadId())
		&& QT_BIT_DISABLED(aType, CTYPE_INVOKE_MULTITHREAD)) 
	{
		// the current thread is the network thread,
		// and will not invoke functions in the mutil-thread.
		return CreateConnectionClient_i(aType, aConClient);
	}
	else {
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionClient,"
			" create CQtConnectorThreadProxy. aType=" << aType);
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_NETWORK_THREAD));

		// we must new actual connector in the network thread.
//		CQtComAutoPtr<IQtConnector> pConActual;
//		QtResult rv = CreateConnectionClient_i(aType, pConActual.ParaOut());
//		if (QT_FAILED(rv))
//			return rv;

		CQtConnectorThreadProxy *pConProxy = 
			new CQtConnectorThreadProxy(aType, pNetwork);
		if (!pConProxy)
			return QT_ERROR_OUT_OF_MEMORY;

		aConClient = pConProxy;
		aConClient->AddReference();
		return QT_OK;
	}
}
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

QtResult CQtConnectionManager::
CreateConnectionServer(CType aType, IQtAcceptor *&aAcceptor)
{
	QT_ASSERTE(!aAcceptor);
#ifdef QT_DEBUG
	if (QT_BIT_ENABLED(aType, CTYPE_INVOKE_MULTITHREAD)) {
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_NETWORK_THREAD));
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionServer, CTYPE_INVOKE_MULTITHREAD.");
	}
	if (QT_BIT_ENABLED(aType, CTYPE_INVOKE_NETWORK_THREAD)) {
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_MULTITHREAD));
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionServer, CTYPE_INVOKE_NETWORK_THREAD.");
	}
#endif // QT_DEBUG

	AQtThread *pNetwork = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_NETWORK);
	if (pNetwork && CQtThreadManager::IsEqualCurrentThread(pNetwork->GetThreadId())
		&& QT_BIT_DISABLED(aType, CTYPE_INVOKE_MULTITHREAD)) 
	{
		return CreateConnectionServer_i(aType, aAcceptor);
	}
	else
	{
		QT_INFO_TRACE_THIS("CQtConnectionManager::CreateConnectionServer,"
			" create CQtAcceptorThreadProxy. aType=" << aType);
		QT_ASSERTE(QT_BIT_DISABLED(aType, CTYPE_INVOKE_NETWORK_THREAD));

		CQtAcceptorThreadProxy *pAcceptorProxy = 
			new CQtAcceptorThreadProxy(aType, pNetwork);
		if (!pAcceptorProxy)
			return QT_ERROR_OUT_OF_MEMORY;

		aAcceptor = pAcceptorProxy;
		aAcceptor->AddReference();
		return QT_OK;
	}
}
#endif
#endif

