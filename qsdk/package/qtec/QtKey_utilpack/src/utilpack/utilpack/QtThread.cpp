
#include "QtBase.h"
#include "QtThread.h"
#include "QtThreadInterface.h"
#include "QtConditionVariable.h"

///////////////////////////////////////////////////////////////
// class AQtThread
///////////////////////////////////////////////////////////////

AQtThread::AQtThread()
	: m_Tid(0)
#ifdef QT_WIN32
	, m_Handle(QT_INVALID_HANDLE)
#else
	, m_Handle((QT_THREAD_HANDLE)QT_INVALID_HANDLE)
#endif // QT_WIN32
	, m_Type(CQtThreadManager::TT_UNKNOWN)
	, m_Flag(CQtThreadManager::TF_NONE)
	, m_pEvent4Start(NULL)
	, m_bRegistered(FALSE)
	, m_bStopFlag(FALSE)
{
}

AQtThread::~AQtThread()
{
	if (m_bRegistered) {
		CQtThreadManager *pMananger = CQtThreadManager::Instance();
		if(pMananger)
			pMananger->UnregisterThread(this);
		m_bRegistered = FALSE;
	}
}

void AQtThread::Terminate()
{
#ifdef QT_WIN32
	CloseHandle(this->GetThreadHandle());
#else
	pthread_cancel(this->GetThreadId());
#endif
	m_bStopFlag = TRUE;
}
QtResult AQtThread::
Create(CQtThreadManager::TType aType, CQtThreadManager::TFlag aFlag)
{
	QT_INFO_TRACE_THIS("AQtThread::Create,"
		" aType=" << aType << 
		" aFlag=" << aFlag);

	QT_ASSERTE_RETURN(m_Type == CQtThreadManager::TT_UNKNOWN, QT_ERROR_ALREADY_INITIALIZED);
	QT_ASSERTE_RETURN(aType != CQtThreadManager::TT_UNKNOWN, QT_ERROR_INVALID_ARG);
	m_Type = aType;
	m_Flag = aFlag;
	if (m_Flag == CQtThreadManager::TF_NONE)
		m_Flag = CQtThreadManager::TF_JOINABLE;
	
	if (m_Type == CQtThreadManager::TT_MAIN) {
		// We have to assume the current thread is main thread.
		m_Tid = CQtThreadManager::GetThreadSelfId();
	}
	else {
		QT_ASSERTE(!m_pEvent4Start);
		m_pEvent4Start = new CQtEventThread();
		if (!m_pEvent4Start)
			return QT_ERROR_OUT_OF_MEMORY;

#ifdef QT_WIN32
		m_Handle = (HANDLE)::_beginthreadex(
			NULL,
			0,
			ThreadProc,
			this,
			0,
			(unsigned int *)(&m_Tid));
		if (m_Handle == 0) {
			QT_ERROR_TRACE_THIS("AQtThread::Create, _beginthreadex() failed! err=" << errno);
			return QT_ERROR_UNEXPECTED;
		}
#else // !QT_WIN32
		pthread_attr_t attr;
		int nRet;
		if ((nRet = ::pthread_attr_init(&attr)) != 0) {
			QT_ERROR_TRACE_THIS("AQtThread::Create, pthread_attr_init() failed! err=" << nRet);
			return QT_ERROR_UNEXPECTED;
		}

		int dstate = PTHREAD_CREATE_JOINABLE;
		if (QT_BIT_ENABLED(m_Flag, CQtThreadManager::TF_JOINABLE))
			dstate = PTHREAD_CREATE_JOINABLE;
		else if (QT_BIT_ENABLED(m_Flag, CQtThreadManager::TF_DETACHED))
			dstate = PTHREAD_CREATE_DETACHED;
		if ((nRet = ::pthread_attr_setdetachstate(&attr, dstate)) != 0) {
			QT_ERROR_TRACE_THIS("AQtThread::Create, pthread_attr_setdetachstate() failed! err=" << nRet);
			::pthread_attr_destroy(&attr);
			return QT_ERROR_UNEXPECTED;
		}

		if ((nRet = ::pthread_create(&m_Tid, &attr, ThreadProc, this)) != 0) {
			QT_ERROR_TRACE_THIS("AQtThread::Create, pthread_create() failed! err=" << nRet);
			::pthread_attr_destroy(&attr);
			return QT_ERROR_UNEXPECTED;
		}
		::pthread_attr_destroy(&attr);
		m_Handle = m_Tid;
#endif // QT_WIN32

		m_pEvent4Start->Wait();
		delete m_pEvent4Start;
		m_pEvent4Start = NULL;
	}

	CQtThreadManager *pManager = CQtThreadManager::Instance();
	QtResult rv = QT_ERROR_FAILURE;
	if(pManager)
		rv = pManager->RegisterThread(this);
	if (QT_SUCCEEDED(rv))
		m_bRegistered = TRUE;
	else {
		Stop();
		Join();
	}
	return rv;
}

QtResult AQtThread::Destory(QtResult aReason)
{
	QT_INFO_TRACE_THIS("AQtThread::Destory, aReason=" << aReason);
	CQtThreadManager *pThreadManager = CQtThreadManager::Instance();
	QT_ASSERTE_RETURN(pThreadManager, QT_ERROR_FAILURE);
	if (m_bRegistered) {
		pThreadManager->UnregisterThread(this);
		m_bRegistered = FALSE;
	}
	if (QT_BIT_DISABLED(m_Flag, CQtThreadManager::TF_JOINABLE) && 
		++m_NeedDelete < 2)
	{
		return QT_OK;
	}
	
	delete this;
	return QT_OK;
}

#ifdef QT_WIN32
unsigned WINAPI AQtThread::ThreadProc(void *aPara)
#else
void* AQtThread::ThreadProc(void *aPara)
#endif // QT_WIN32
{
	AQtThread *pThread = static_cast<AQtThread *>(aPara);
	QT_ASSERTE_RETURN(pThread, NULL);

	pThread->OnThreadInit();
	if (pThread->m_Type != CQtThreadManager::TT_MAIN) {
		QT_ASSERTE(pThread->m_pEvent4Start);
		if (pThread->m_pEvent4Start)
			pThread->m_pEvent4Start->Signal();
	}

	pThread->OnThreadRun();

	pThread->SetStop();
	if (QT_BIT_DISABLED(pThread->m_Flag, CQtThreadManager::TF_JOINABLE) && 
		++pThread->m_NeedDelete >= 2)
	{
		delete pThread;
	}

#ifdef QT_WIN32
	::_endthread();
#endif // QT_WIN32
	return NULL;
}

QtResult AQtThread::Stop(CQtTimeValue* aTimeout)
{
	QT_ASSERTE(!"AQtThread::Stop");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult AQtThread::Join()
{
//	QT_ASSERTE_RETURN(!CQtThreadManager::IsEqualCurrentThread(m_Tid), QT_ERROR_FAILURE);
	if (CQtThreadManager::IsEqualCurrentThread(m_Tid))
		return QT_ERROR_FAILURE;

#ifdef QT_WIN32
	if(GetStopFlag())
	{
		QT_INFO_TRACE_THIS("AQtThread::Join() thread has stopped normally");
		Sleep(10);
		return QT_OK;
	}
	// Wait 100ms to avoid blocking, budingc modifed at 01/14.
	DWORD dwRet = ::WaitForSingleObject(m_Handle, 100);
	if (dwRet == WAIT_OBJECT_0)
		return QT_OK;
	else if (dwRet == WAIT_TIMEOUT) {
		QT_WARNING_TRACE_THIS("AQtThread::Join, WaitForSingleObject() timeout!");
		TerminateThread(m_Handle, 123);
		return QT_ERROR_TIMEOUT;
	}
	else {
		QT_ERROR_TRACE_THIS("AQtThread::Join, WaitForSingleObject() failed! err=" << ::GetLastError());
		return QT_ERROR_FAILURE;
	}
#else
	void *pThreadReturn;
	int nRet = ::pthread_join(m_Tid, &pThreadReturn);
	if (nRet != 0) {
		QT_ERROR_TRACE_THIS("AQtThread::Join, pthread_join() failed! err=" << nRet);
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;
#endif // QT_WIN32
}

QT_THREAD_ID AQtThread::GetThreadId()
{
	return m_Tid;
}

CQtThreadManager::TType AQtThread::GetThreadType()
{
	return m_Type;
}

QT_THREAD_HANDLE AQtThread::GetThreadHandle()
{
	return m_Handle;
}

IQtReactor* AQtThread::GetReactor()
{
	return NULL;
}

IQtEventQueue* AQtThread::GetEventQueue()
{
	return NULL;
}

IQtTimerQueue* AQtThread::GetTimerQueue()
{
	return NULL;
}

void AQtThread::OnThreadInit()
{
}
