
#include "QtBase.h"
#include "QtUtilClasses.h"
#include "QtThreadManager.h"
///////////////////////////////////////////////////////////////
// class CQtEnsureSingleThread
///////////////////////////////////////////////////////////////

CQtEnsureSingleThread::CQtEnsureSingleThread()
	: m_ThreadIdOpen(CQtThreadManager::GetThreadSelfId())
{
}

void CQtEnsureSingleThread::EnsureSingleThread() const 
{
#ifdef QT_DEBUG
	QT_THREAD_ID tidCur = CQtThreadManager::GetThreadSelfId();
	QT_ASSERTE(CQtThreadManager::IsThreadEqual(m_ThreadIdOpen, tidCur));
#endif // QT_DEBUG
}

void CQtEnsureSingleThread::Reset2CurrentThreadId()
{
	m_ThreadIdOpen = CQtThreadManager::GetThreadSelfId();
}

void CQtEnsureSingleThread::Reset2ThreadId(QT_THREAD_ID aTid)
{
	m_ThreadIdOpen = aTid;
}


///////////////////////////////////////////////////////////////
// class CQtSignalStop
///////////////////////////////////////////////////////////////

CQtSignalStop CQtSignalStop::s_SignalStopSingleton;

CQtSignalStop::CQtSignalStop()
	: m_pThread(NULL)
{
}

CQtSignalStop::~CQtSignalStop()
{
}

CQtSignalStop* CQtSignalStop::Instance()
{
	return &s_SignalStopSingleton;
}

QtResult CQtSignalStop::Launch(int aSig)
{
	QtResult rv = QT_ERROR_FAILURE;

#ifdef QT_WIN32
	AQtThread *pMain = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_MAIN);
	if (pMain) {
		rv = pMain->Stop();
		QT_ASSERTE(QT_SUCCEEDED(rv));
	}
#else // !QT_WIN32
	if (m_pThread) {
		// Signal has posted, ignore it.
		return QT_OK;
	}

	AQtThread *pNetwork = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_NETWORK);
	// can't stop main thread directly in Linux 
	// due to dead lock of condition variable.
	if (!pNetwork) 
		return rv;
	
#if 0
	::printf("CQtSignalStop::Launch, thread id network=%d cur=%d, sig=%d.\n", 
		(int)pNetwork->GetThreadId(), 
		(int)CQtThreadManager::GetThreadSelfId(),
		aSig);
#endif
	
	// every thread will invoke this signal roution in Linux.
	// only the network thread allows post event int the signal roution.
	if (!CQtThreadManager::IsEqualCurrentThread(pNetwork->GetThreadId())) 
		return QT_OK;
	m_pThread = pNetwork;

	IQtEventQueue *pEq = m_pThread->GetEventQueue();
	if (pEq) {
		rv = pEq->PostEvent(this);
		::printf("CQtSignalStop::CQtSignalStop, PostEvent, rv=%d.\n", (int)rv);
	}
#endif // QT_WIN32
	return rv;
}

QtResult CQtSignalStop::OnEventFire()
{
	::printf("CQtSignalStop::OnEventFire. tid=%d.\n", (int)CQtThreadManager::GetThreadSelfId());
	AQtThread *pMain = CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_MAIN);
	if (pMain)
		return pMain->Stop();
	else
		return QT_ERROR_NULL_POINTER;
}

void CQtSignalStop::OnDestorySelf()
{
	// don't delete this
}


///////////////////////////////////////////////////////////////
// class CQtCleanUpBase
///////////////////////////////////////////////////////////////

CQtCleanUpBase* CQtCleanUpBase::s_pHeader = NULL;

CQtCleanUpBase::CQtCleanUpBase()
{
	MutexType *pMutex = NULL;
	CQtThreadManager::Instance()->GetSingletonMutex(pMutex);
	QT_ASSERTE(pMutex);

	CQtMutexGuardT<MutexType> theGuard(*pMutex);
	m_pNext = s_pHeader;
	s_pHeader = this;
}

CQtCleanUpBase::~CQtCleanUpBase()
{
}

void CQtCleanUpBase::CleanUp()
{
	delete this;
}

void CQtCleanUpBase::CleanupAll()
{
	MutexType *pMutex = NULL;
	CQtThreadManager::Instance()->GetSingletonMutex(pMutex);
	QT_ASSERTE(pMutex);

	CQtMutexGuardT<MutexType> theGuard(*pMutex);
	while (s_pHeader) {
		CQtCleanUpBase *pTmp = s_pHeader->m_pNext;
		s_pHeader->CleanUp();
		s_pHeader = pTmp;
	}
}


///////////////////////////////////////////////////////////////
// class CQtDataBlockNoMalloc
///////////////////////////////////////////////////////////////

CQtDataBlockNoMalloc::CQtDataBlockNoMalloc(LPCSTR aStr, DWORD aLen)
	: m_pBegin(aStr)
	, m_pEnd(aStr + aLen)
	, m_pCurrentRead(m_pBegin)
	, m_pCurrentWrite(const_cast<LPSTR>(m_pBegin))
{
	QT_ASSERTE(m_pBegin);
}

QtResult CQtDataBlockNoMalloc::
Read(LPVOID aDst, DWORD aCount, DWORD *aBytesRead)
{
	QT_ASSERTE_RETURN(aDst, QT_ERROR_INVALID_ARG);
	QT_ASSERTE_RETURN(m_pCurrentRead, QT_ERROR_NOT_INITIALIZED);
	QT_ASSERTE_RETURN(m_pCurrentRead <= m_pEnd, QT_ERROR_NOT_INITIALIZED);
	
	//2009 5.14 Victor we need check the data is enough or not for the read request
	//DWORD dwLen = QT_MIN(aCount, static_cast<DWORD>(m_pCurrentRead - m_pEnd));
	DWORD dwLen = QT_MIN(aCount, static_cast<DWORD>(m_pEnd - m_pCurrentRead));
	if (dwLen > 0) {
		::memcpy(aDst, m_pCurrentRead, dwLen);
		m_pCurrentRead += dwLen;
	}
	if (aBytesRead)
		*aBytesRead = dwLen;
	return dwLen == aCount ? QT_OK : QT_ERROR_PARTIAL_DATA;
}

QtResult CQtDataBlockNoMalloc::
Write(LPCVOID aSrc, DWORD aCount, DWORD *aBytesWritten)
{
	QT_ASSERTE_RETURN(aSrc, QT_ERROR_INVALID_ARG);
	QT_ASSERTE_RETURN(m_pCurrentWrite, QT_ERROR_NOT_INITIALIZED);
	QT_ASSERTE_RETURN(m_pCurrentWrite <= m_pEnd, QT_ERROR_NOT_INITIALIZED);

	DWORD dwLen = QT_MIN(aCount, static_cast<DWORD>(m_pEnd - m_pCurrentWrite));
	if (dwLen > 0) {
		::memcpy(m_pCurrentWrite, aSrc, dwLen);
		m_pCurrentWrite += dwLen;
	}
	if (aBytesWritten)
		*aBytesWritten = dwLen;
	return dwLen == aCount ? QT_OK : QT_ERROR_PARTIAL_DATA;
}


///////////////////////////////////////////////////////////////
// class CQtDataBlockNoMalloc
///////////////////////////////////////////////////////////////

void CQtStopFlag::SetStartFlag()
{
	m_Est.EnsureSingleThread();
	QT_ASSERTE(m_bStoppedFlag);
	m_bStoppedFlag = FALSE;
}

void CQtStopFlag::SetStopFlag()
{
	m_Est.EnsureSingleThread();
	m_bStoppedFlag = TRUE;
}

