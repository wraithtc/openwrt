
#include "QtBase.h"
#include "QtThreadManager.h"
#include "QtThreadReactor.h"
#include "QtReactorInterface.h"
#include "QtConnectionInterface.h"
#include "QtThreadTask.h"
#include "QtAtomicOperationT.h"
#include "QtDnsManager.h"

#if defined (QT_WIN32)
  #include "QtReactorSelect.h"
  #include "QtReactorWin32Message.h"

  extern ATOM g_atomRegisterClass;
  extern HINSTANCE g_pReactorWin3Instance;
  extern BOOL	g_bRunTimeLoad;  
#elif defined (QT_MACOS)
  #include "QtThreadMacEventPatch.h"
#elif defined (QT_LINUX)
  #include <sys/utsname.h>
  #ifndef QT_PORT_CLIENT
    #include "QtReactorRealTimeSignal.h"
  #endif // QT_PORT_CLIENT
  #ifdef QT_ENABLE_EPOLL
    #include "QtReactorEpoll.h"
  #endif // QT_ENABLE_EPOLL
#endif // QT_WIN32

#ifdef QT_USE_REACTOR_SELECT
  #include "QtReactorSelect.h"
#endif // QT_USE_REACTOR_SELECT

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
  extern ATOM g_atomDnsRegisterClass;
#endif // QT_WIN32_ENABLE_AsyncGetHostByName

static CQtThreadManager *s_pThreadManagerOnlyOne = NULL;
static BOOL s_bThreadManagerOnlyOneByNew = FALSE;

void CQtThreadManager::CleanupOnlyOne()
{
	QT_INFO_TRACE("CQtThreadManager::CleanupOnlyOne,");
	//Victor 2006.3.24, if pull out net line on unix, the gethostbyname will be block for ~50s
	//so cancel the dns thread to interrupt it
	//Victor 2006.5.22, this modification will let nbrplay freezen on LINUX and SOLARIS platform

#if defined QT_LINUX || defined QT_SOLARIS
	QT_INFO_TRACE("CQtThreadManager::CleanupOnlyOne stop dns thread");
	CQtDnsManager::Instance()->Shutdown();
#endif
	if (s_bThreadManagerOnlyOneByNew)
		delete s_pThreadManagerOnlyOne;
	s_pThreadManagerOnlyOne = NULL;
	CQtConnectionManager::CleanupInstance();
#if defined QT_QTEC_UNIFIED_TRACE /*&& defined QT_WIN32 // the changes for all platform to avoid crash */ //for bug303444, need unload wbxtrace.dll when unload MMP
	QT_INFO_TRACE("CQtThreadManager::CleanupOnlyOne close tracer");
	CQtT120Trace::instance()->Close();
#endif
}

CQtThreadManager::CQtThreadManager()
	: m_TTpyeUserThread(TT_USER_DEFINE_BASE)
{
#if defined QT_WIN32 || defined QT_PORT_CLIENT
	QT_INFO_TRACE_THIS("CQtThreadManager::CQtThreadManager,");
#endif

	QT_ASSERTE(!s_pThreadManagerOnlyOne);
	s_pThreadManagerOnlyOne = this;
}

CQtThreadManager::~CQtThreadManager()
{
#if defined QT_WIN32 || defined QT_PORT_CLIENT
	QT_INFO_TRACE_THIS("CQtThreadManager::~CQtThreadManager");
#endif
	
	StopAllThreads(NULL);
	JoinAllThreads();

	// cleaup instance before delete threads because some instances will use threads.
	CQtCleanUpBase::CleanupAll();

	ThreadsType tmpThreads;
	{
		CQtMutexGuardT<MutexType> theGuard(m_Mutex);
		m_Threads.swap(tmpThreads);
	}
	ThreadsType::iterator iter = tmpThreads.begin();
	for ( ; iter != tmpThreads.end(); ++iter)
		(*iter)->Destory(QT_OK);

#ifdef QT_WIN32
	if (g_atomRegisterClass) {
		if (!::UnregisterClass(reinterpret_cast<LPCSTR>(g_atomRegisterClass), g_pReactorWin3Instance)) {
			QT_INFO_TRACE_THIS("CQtThreadManager::~CQtThreadManager, UnregisterClass() failed!"
				" g_atomRegisterClass=" << g_atomRegisterClass << 
				" g_pReactorWin3Instance=" << g_pReactorWin3Instance << 
				" err=" << ::GetLastError());
		}
		else {
			QT_INFO_TRACE_THIS("CQtThreadManager::~CQtThreadManager,"
				" g_atomRegisterClass=" << g_atomRegisterClass << 
				" g_pReactorWin3Instance=" << g_pReactorWin3Instance);
		}
		g_atomRegisterClass = 0;
	}
#endif // QT_WIN32

#ifdef QT_WIN32_ENABLE_AsyncGetHostByName
	if (g_atomDnsRegisterClass) {
		if (!::UnregisterClass(reinterpret_cast<LPCSTR>(g_atomDnsRegisterClass), NULL)) {
			QT_INFO_TRACE_THIS("CQtThreadManager::~CQtThreadManager, UnregisterClass() failed!"
				" g_atomDnsRegisterClass=" << g_atomDnsRegisterClass << 
				" err=" << ::GetLastError());
		}
		else {
			QT_INFO_TRACE_THIS("CQtThreadManager::~CQtThreadManager,"
				" g_atomDnsRegisterClass=" << g_atomDnsRegisterClass);
		}
		g_atomDnsRegisterClass = 0;
	}
#endif // QT_WIN32_ENABLE_AsyncGetHostByName

	SocketCleanup();
	s_pThreadManagerOnlyOne = NULL;
	CQtConnectionManager::CleanupInstance();
}

CQtThreadManager* CQtThreadManager::Instance()
{
//	QT_ASSERTE(s_pThreadManagerOnlyOne);
	if (!s_pThreadManagerOnlyOne) {
//		QT_WARNING_TRACE_THIS("CQtThreadManager::Instance, s_pThreadManagerOnlyOne is NULL!"
//			" You should alloc it in the stack or heap first!");

#ifdef QT_PORT_CLIENT
		//ignore SIG_PIPE signal
		::signal(SIGPIPE, SIG_IGN);
#endif
		// We have to new <CQtThreadManager> becauase Connect1.1 UI doesn't
		// alloc it in the main function.
		// we have to assume it's in main thread.
		QT_INFO_TRACE("CQtThreadManager::Instance, new CQtThreadManager.");
		new CQtThreadManager();
		s_bThreadManagerOnlyOneByNew = TRUE;

		QtResult rv = s_pThreadManagerOnlyOne ? s_pThreadManagerOnlyOne->InitMainThread(0, NULL) : QT_ERROR_FAILURE;
		if (QT_FAILED(rv)) {
			delete s_pThreadManagerOnlyOne;
			s_pThreadManagerOnlyOne = NULL;
			return NULL;
		}
		else {
#if (defined QT_LINUX || defined QT_UNIX ) && defined QT_PORT_CLIENT
			//Now LINUX and SOLARIS client all used load model, so can need not register the function,
			//otherwise if unload lib before exit, will cause crash 2005 11/17
#else
#ifdef QT_WIN32
			QT_INFO_TRACE("CQtThreadManager::Instance now dynamic load flag = " << g_bRunTimeLoad);
			if(!g_bRunTimeLoad)
#endif
#ifndef QT_DYNAMIC_LOAD
			{
				int nRet = ::atexit(CleanupOnlyOne);
				if (nRet != 0) {
					QT_WARNING_TRACE("CQtThreadManager::Instance, atexit() failed. err=" << errno);
				}
			}
#endif
#endif
		}
	}
	return s_pThreadManagerOnlyOne;
}

QtResult CQtThreadManager::InitMainThread(int aArgc, char** aArgv)
{
	QT_INFO_TRACE_THIS("CQtThreadManager::InitMainThread, argc  = " << aArgc);
	{
		CQtMutexGuardT<MutexType> theGuard(m_Mutex);
		QT_ASSERTE(m_Threads.empty());
		if (!m_Threads.empty()) {
			QT_WARNING_TRACE_THIS("CQtThreadManager::InitMainThread, You should InitMainThread before creating other thread!");
			return QT_ERROR_ALREADY_INITIALIZED;
		}
	}
#ifdef QT_HAS_BUILTIN_ATOMIC_OP
	CQtAtomicOperationT<CQtMutexThread>::init_functions();
#endif // QT_HAS_BUILTIN_ATOMIC_OP

	QtResult rv = SocketStartup();
	if (QT_FAILED(rv))
		return rv;
	IQtReactor *pReactorMain = NULL;
#ifdef QT_WIN32
	if (GetNetworkThreadModule() == TM_SINGLE_MAIN)
		pReactorMain = CreateNetworkReactor();
	else
		pReactorMain = new CQtReactorWin32Message();
#elif !defined (QT_BIND_WITH_EUREKA)
	if (GetNetworkThreadModule() == TM_SINGLE_MAIN)
		pReactorMain = CreateNetworkReactor();
	else {
		// the main thread on Linux is task thread.
		pReactorMain = NULL;
	}
#endif // QT_WIN32
	AQtThread *pThread = NULL;
	if (pReactorMain)
		rv = CreateReactorThread(CQtThreadManager::TT_MAIN, pReactorMain, pThread);
	else {
#if defined QT_MACOS && !defined (QT_HEARTBEAT_SUPPORT)
		pThread = new CQtThreadMacEventPatch();
#elif defined (QT_PORT_CLIENT) || defined (QT_HEARTBEAT_SUPPORT)
		pThread = new CQtThreadHeartBeat();
#else
		pThread = new CQtThreadTask();
#endif // QT_MACOS
		if (pThread)
			rv = pThread->Create(CQtThreadManager::TT_MAIN);
		else
			rv = QT_ERROR_OUT_OF_MEMORY;
	}
	if (QT_FAILED(rv)) {
		if (pThread)
		pThread->Destory(rv);
		return rv;
	}

#if	!defined (QT_BIND_WITH_EUREKA)
	// create network thread when program startuping.
	if (CQtConnectionManager::Instance() == NULL) {
		if (pThread)
		pThread->Destory(QT_OK);
		return QT_ERROR_UNEXPECTED;
	}
#endif
	pThread->OnThreadInit();
	return QT_OK;
}

IQtReactor* CQtThreadManager::CreateNetworkReactor()
{
	IQtReactor *pReactorRet = NULL;
#ifdef QT_USE_REACTOR_SELECT
	pReactorRet = new CQtReactorSelect();
#elif defined (QT_WIN32)
#ifdef QT_ENABLE_SELECT_WINOS
	pReactorRet = new CQtReactorSelect(); //enable select model on winos 4/29 2009 Victor Cui
#else
	pReactorRet = new CQtReactorWin32AsyncSelect();
#endif //!QT_ENABLE_SELECT_WINOS
#else // !QT_WIN32
	struct utsname utName;
	if (::uname(&utName) < 0) {
		QT_WARNING_TRACE("CQtThreadManager::CreateNetworkReactor, uname() failed!"
			"err=" << errno);
	}
	else {
		if (!strncmp(utName.release, "2.4.", 4)) {
			int nMinVer = ::atoi(utName.release + 4);
			if (nMinVer >= 18)
				pReactorRet = new CQtReactorRealTimeSignal();
			else {
				QT_WARNING_TRACE("CQtThreadManager::CreateNetworkReactor,"
					" Only 2.4.18 or above supports realtime signal."
					" release=" << utName.release << 
					" sysname=" << utName.sysname);
			}
		}

#ifdef QT_ENABLE_EPOLL
		else if (!strncmp(utName.release, "2.6.", 4)) {
			pReactorRet = new CQtReactorEpoll();
		}
#endif // QT_ENABLE_EPOLL
		else {
			QT_WARNING_TRACE("CQtThreadManager::CreateNetworkReactor,"
				" don't support this release of Linux."
				" release=" << utName.release << 
				" sysname=" << utName.sysname);
		}

	}
#endif // QT_USE_REACTOR_SELECT
	
	return pReactorRet;
}

#if 0
QtResult CQtThreadManager::CreateUserReactorThread(AQtThread *&aThread)
{
	QT_ASSERTE(!aThread);

	IQtReactor *pReactor = NULL;
#ifdef QT_WIN32
	pReactor = new CQtReactorWin32Message();
#else
	pReactor = NULL;
#endif // QT_LINUX
	QT_ASSERTE_RETURN(pReactor, QT_ERROR_OUT_OF_MEMORY);

	QtResult rv = CreateReactorThread(GetAndIncreamUserType(), pReactor, aThread);
	return rv;
}
#endif

QtResult CQtThreadManager::
CreateUserTaskThread(AQtThread *&aThread, TFlag aFlag, BOOL bWithTimerQueue, TType aType)
{
	QT_ASSERTE(!aThread);

	AQtThread *pThread = NULL;
	if (bWithTimerQueue)
		pThread = new CQtThreadTask();
	else
		pThread = new CQtThreadTaskWithEventQueueOnly();
	QT_ASSERTE_RETURN(pThread, QT_ERROR_OUT_OF_MEMORY);

	QtResult rv = pThread->Create(aType == TT_UNKNOWN ? GetAndIncreamUserType() : aType, aFlag);
	if (QT_FAILED(rv)) {
		pThread->Destory(rv);
		return rv;
	}

	aThread = pThread;
	return QT_OK;
}

CQtThreadManager::TType CQtThreadManager::GetAndIncreamUserType()
{
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	return ++m_TTpyeUserThread;
}

QtResult CQtThreadManager::
CreateReactorThread(TType aType, IQtReactor *aReactor, AQtThread *&aThread)
{
	// Note: aReactor will be delete if failed.
	QtResult rv;
	QT_ASSERTE_RETURN(aReactor, QT_ERROR_INVALID_ARG);
	QT_ASSERTE(!aThread);

	std::auto_ptr<CQtThreadReactor> pThreadReactor(new CQtThreadReactor());
	if (!pThreadReactor.get()) {
		delete aReactor;
		return QT_ERROR_OUT_OF_MEMORY;
	}
	
	rv = pThreadReactor->Init(aReactor);
	if (QT_FAILED(rv))
		return rv;

	rv = pThreadReactor->Create(aType);
	if (QT_FAILED(rv))
		return rv;

	aThread = pThreadReactor.release();
	return QT_OK;
}

QtResult CQtThreadManager::RegisterThread(AQtThread* aThread)
{
	QT_ASSERTE_RETURN(aThread, QT_ERROR_INVALID_ARG);
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);

	ThreadsType::iterator iter = m_Threads.begin();
	for ( ; iter != m_Threads.end(); ++iter) {
		// we don't check equal ThreadId because mutil AQtThreads may use the same id.
		if ((*iter) == aThread || 
			(*iter)->GetThreadType() == aThread->GetThreadType()) 
		{
			QT_WARNING_TRACE_THIS("CQtThreadManager::RegisterThread, have registered."
				" aThread=" << aThread <<
				" tid=" << aThread->GetThreadId() << 
				" type=" << aThread->GetThreadType());
			QT_ASSERTE(FALSE);
			return QT_ERROR_FOUND;
		}
	}

	m_Threads.push_back(aThread);
	return QT_OK;
}

QtResult CQtThreadManager::UnregisterThread(AQtThread* aThread)
{
	QT_ASSERTE_RETURN(aThread, QT_ERROR_INVALID_ARG);
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);

	ThreadsType::iterator iter = m_Threads.begin();
	for ( ; iter != m_Threads.end(); ++iter) {
		if ((*iter) == aThread || 
			(*iter)->GetThreadType() == aThread->GetThreadType()) 
		{
			m_Threads.erase(iter);
			return QT_OK;
		}
	}
	return QT_ERROR_NOT_FOUND;
}

QtResult CQtThreadManager::StopAllThreads(CQtTimeValue* aTimeout)
{
	// not support timeout now.
	QT_ASSERTE(!aTimeout);
	
	// only the main thread can stop all threads.
	AQtThread *pMain = GetThread(TT_MAIN);
	if (pMain) {
		QT_ASSERTE_RETURN(
			IsThreadEqual(pMain->GetThreadId(), GetThreadSelfId()), 
			QT_ERROR_FAILURE);
	}

	ThreadsType tmpThreads;
	{
		CQtMutexGuardT<MutexType> theGuard(m_Mutex);
		tmpThreads = m_Threads;
	}
	ThreadsType::iterator iter = tmpThreads.begin();
	for ( ; iter != tmpThreads.end(); ++iter) {
		QT_INFO_TRACE_THIS("CQtThreadManager::StopAllThreads thread type = " << (*iter)->GetThreadType());
#if defined QT_PORT_CLIENT && (defined QT_LINUX || defined QT_SOLARIS)
		if ((*iter)->GetThreadType() != TT_MAIN && (*iter)->GetThreadType() != TT_DNS)
			(*iter)->Stop(aTimeout);
#else
		if ((*iter)->GetThreadType() != TT_MAIN)
			(*iter)->Stop(aTimeout);
#endif
	}

	return QT_OK;
}

QtResult CQtThreadManager::JoinAllThreads()
{
	// only the main thread can Join all threads.
	AQtThread *pMain = GetThread(TT_MAIN);
	if (pMain) {
		QT_ASSERTE_RETURN(
			IsThreadEqual(pMain->GetThreadId(), GetThreadSelfId()), 
			QT_ERROR_FAILURE);
	}

	ThreadsType tmpThreads;
	{
		CQtMutexGuardT<MutexType> theGuard(m_Mutex);
		tmpThreads = m_Threads;
	}
	ThreadsType::iterator iter = tmpThreads.begin();
	for ( ; iter != tmpThreads.end(); ++iter) {
		if ((*iter)->GetThreadType() != TT_MAIN)
			(*iter)->Join();
	}

	return QT_OK;
}

AQtThread* CQtThreadManager::GetThread(TType aType)
{
	CQtMutexGuardT<MutexType> theGuard(m_Mutex);
	if (TT_CURRENT == aType) {
		QT_THREAD_ID tidSelf = GetThreadSelfId();
		ThreadsType::iterator iter = m_Threads.begin();
		for ( ; iter != m_Threads.end(); ++iter) {
			if ((*iter)->GetThreadId() == tidSelf)
				return (*iter);
		}
	}
	else {
		ThreadsType::iterator iter = m_Threads.begin();
		for ( ; iter != m_Threads.end(); ++iter) {
			if ((*iter)->GetThreadType() == aType)
				return (*iter);
		}
	}
	return NULL;
}

IQtReactor* CQtThreadManager::GetThreadReactor(TType aType)
{
	AQtThread *pThread = GetThread(aType);
	if (pThread)
		return pThread->GetReactor();
	else
		return NULL;
}

IQtEventQueue* CQtThreadManager::GetThreadEventQueue(TType aType)
{
	AQtThread *pThread = GetThread(aType);
	if (pThread)
		return pThread->GetEventQueue();
	else
		return NULL;
}

IQtTimerQueue* CQtThreadManager::GetThreadTimerQueue(TType aType)
{
	AQtThread *pThread = GetThread(aType);
	if (pThread)
		return pThread->GetTimerQueue();
	else
		return NULL;
}

BOOL CQtThreadManager::IsEqualCurrentThread(TType aType)
{
	AQtThread *pThread = Instance()->GetThread(aType);
	if (pThread)
		return IsThreadEqual(pThread->GetThreadId(), GetThreadSelfId());
	else
		return FALSE;
}

void CQtThreadManager::SleepMs(DWORD aMsec)
{
#ifdef QT_WIN32
	::Sleep(aMsec);
#else
	struct timespec ts, rmts;
	ts.tv_sec = aMsec/1000;
	ts.tv_nsec = (aMsec%1000)*1000000;

	for ( ; ; ) {
		int nRet = ::nanosleep(&ts, &rmts);
		if (nRet == 0)
			break;
		if (errno == EINTR) {
			ts = rmts;
		}
		else {
			QT_WARNING_TRACE("CQtThreadManager::SleepMs,"
				"nanosleep() failed! err=" << errno);
			break;
		}
	}
#endif // QT_WIN32
}

#ifdef QT_WIN32
BOOL CQtThreadManager::s_bSocketInited = FALSE;
#endif // QT_WIN32

QtResult CQtThreadManager::SocketStartup()
{
#ifdef QT_WIN32
	if (s_bSocketInited)
		return QT_OK;
	s_bSocketInited = TRUE;

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (::WSAStartup(wVersionRequested, &wsaData) != 0) {
		QT_WARNING_TRACE("CQtThreadManager::SocketStartup, WSAStartup() failed!"
			" err=" << ::WSAGetLastError());
		s_bSocketInited = FALSE;
		return QT_ERROR_FAILURE;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		QT_WARNING_TRACE("CQtThreadManager::SocketStartup, version error!"
			" wsaData.wVersion=" << wsaData.wVersion <<
			" wsaData.wVersion=" << wsaData.wVersion <<
			" err=" << ::WSAGetLastError());
		SocketCleanup();
		return QT_ERROR_NOT_AVAILABLE;
	}
#endif // QT_WIN32
	return QT_OK;
}

QtResult CQtThreadManager::SocketCleanup()
{
#ifdef QT_WIN32
	if (!s_bSocketInited)
		return QT_OK;
	s_bSocketInited = FALSE;

	if (::WSACleanup() != 0) {
		QT_WARNING_TRACE("CQtThreadManager::SocketCleanup, WSACleanup() failed!"
			" err=" << ::WSAGetLastError());
		return QT_ERROR_UNEXPECTED;
	}
#endif // QT_WIN32
	return QT_OK;
}

//#define QT_NETWORK_THREAD_SINGLE_MAIN

CQtThreadManager::TModule CQtThreadManager::GetNetworkThreadModule()
{
#ifdef QT_NETWORK_THREAD_SINGLE_MAIN
	return TM_SINGLE_MAIN;
#else
	return TM_MULTI_ONE_DEDICATED;
#endif // QT_NETWORK_THREAD_SINGLE_MAIN
}

#if defined (QT_HEARTBEAT_SUPPORT) && defined (QT_BIND_WITH_EUREKA)
static CQtThreadManager gThreadManager;
extern "C"
{
	bool utiltp_init(int aArgc, char** aArgv)
	{
		return gThreadManager.InitMainThread(aArgc, aArgv) == QT_OK ? true : false;
	}
	
	bool utiltp_doheartbeat()
	{
		AQtThread *pThread = gThreadManager.GetThread(CQtThreadManager::TT_MAIN);
		if(!pThread) return false;
		pThread->OnThreadRun();
		return true;
	}
}
#endif
