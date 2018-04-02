
#include "QtBase.h"
#include "QtConditionVariable.h"
#include "QtTimeValue.h"

//////////////////////////////////////////////////////////////////////
// class CQtSemaphore
//////////////////////////////////////////////////////////////////////

CQtSemaphore::CQtSemaphore(LONG aInitialCount, LPCSTR aName, LONG aMaximumCount)
{
#ifdef QT_WIN32
	m_Semaphore = ::CreateSemaphoreA(NULL, aInitialCount, aMaximumCount, aName);
	if (m_Semaphore == 0) {
		QT_WARNING_TRACE_THIS("CQtSemaphore::CQtSemaphore, CreateSemaphoreA() failed!"
			" err=" << ::GetLastError());
		QT_ASSERTE(FALSE);
	}
#else // !QT_WIN32
	if (::sem_init(&m_Semaphore, 0, aInitialCount) == -1) {
		QT_WARNING_TRACE_THIS("CQtSemaphore::CQtSemaphore, sem_init() failed!"
			" err=" << errno);
		QT_ASSERTE(FALSE);
	}
#endif
}

CQtSemaphore::~CQtSemaphore()
{
#ifdef QT_WIN32
	if (!::CloseHandle(m_Semaphore)) {
		QT_WARNING_TRACE_THIS("CQtSemaphore::~CQtSemaphore, CloseHandle() failed!"
			" err=" << ::GetLastError());
	}
#else // !QT_WIN32
	if (::sem_destroy(&m_Semaphore) == -1) {
		QT_WARNING_TRACE_THIS("CQtSemaphore::~CQtSemaphore, sem_destroy() failed!"
			" err=" << errno);
	}
#endif // QT_WIN32
}

QtResult CQtSemaphore::Lock()
{
#ifdef QT_WIN32
	DWORD dwRet = ::WaitForSingleObject(m_Semaphore, INFINITE);
	switch (dwRet) {
	case WAIT_OBJECT_0:
		return QT_OK;
	default:
		QT_WARNING_TRACE_THIS("CQtSemaphore::Lock, WaitForSingleObject() failed!"
			" dwRet=" << dwRet << 
			" err=" << ::GetLastError());
		return QT_ERROR_FAILURE;
	}
#else // !QT_WIN32
	if (::sem_wait(&m_Semaphore) == -1) {
		QT_WARNING_TRACE_THIS("CQtSemaphore::Lock, sem_wait() failed!"
			" err=" << errno);
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;
#endif // QT_WIN32
}

QtResult CQtSemaphore::UnLock()
{
	return PostN(1);
}

QtResult CQtSemaphore::PostN(LONG aCount)
{
	QT_ASSERTE(aCount >= 1);
#ifdef QT_WIN32
	if (!::ReleaseSemaphore(m_Semaphore, aCount, NULL)) {
		QT_WARNING_TRACE_THIS("CQtSemaphore::UnLock, ReleaseSemaphore() failed!"
			" err=" << ::GetLastError());
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;
#else // !QT_WIN32
	for (LONG i = 0; i < aCount; i++) {
		if (::sem_post(&m_Semaphore) == -1) {
			QT_WARNING_TRACE_THIS("CQtSemaphore::UnLock, sem_post() failed!"
				" err=" << errno);
			return QT_ERROR_FAILURE;
		}
	}
	return QT_OK;
#endif // QT_WIN32
}


//////////////////////////////////////////////////////////////////////
// class CQtConditionVariableThread
//////////////////////////////////////////////////////////////////////

CQtConditionVariableThread::CQtConditionVariableThread(CQtMutexThread &aMutex)
	: m_MutexExternal(aMutex)
#ifdef QT_WIN32
	, sema_(0)
#endif // QT_WIN32
{
#ifdef QT_WIN32
	waiters_ = 0;
	was_broadcast_ = 0;
	waiters_done_= ::CreateEventA(NULL, 0, 0, NULL);
	if (waiters_done_ == 0) {
		QT_WARNING_TRACE_THIS("CQtConditionVariableThread::CQtConditionVariableThread, CreateEventA() failed!"
			" err=" << ::GetLastError());
		QT_ASSERTE(FALSE);
	}
#else // !QT_WIN32
	int nRet = ::pthread_cond_init(&m_Condition, NULL);
	if (nRet != 0) {
		QT_WARNING_TRACE_THIS("CQtConditionVariableThread::CQtConditionVariableThread, pthread_cond_init() failed!"
			" err=" << nRet);
		QT_ASSERTE(FALSE);
	}
#endif // QT_WIN32
}

CQtConditionVariableThread::~CQtConditionVariableThread()
{
#ifdef QT_WIN32
	if (!::CloseHandle(waiters_done_)) {
		QT_WARNING_TRACE_THIS("CQtConditionVariableThread::~CQtConditionVariableThread, CloseHandle() failed!"
			" err=" << ::GetLastError());
	}
#else // !QT_WIN32
	int nRet = ::pthread_cond_destroy(&m_Condition);
	if (nRet != 0) {
		QT_WARNING_TRACE_THIS("CQtConditionVariableThread::~CQtConditionVariableThread, pthread_cond_destroy() failed!"
			" err=" << nRet);
	}
#endif // QT_WIN32
}

QtResult CQtConditionVariableThread::Signal()
{
#ifdef QT_WIN32
	waiters_lock_.Lock();
	int have_waiters = waiters_ > 0;
	waiters_lock_.UnLock();

	if (have_waiters != 0)
		return sema_.UnLock();
	else
		return QT_OK;
#else // !QT_WIN32
	int nRet = ::pthread_cond_signal(&m_Condition);
	if (nRet != 0) {
		QT_WARNING_TRACE_THIS("CQtConditionVariableThread::Signal, pthread_cond_signal() failed!"
			" err=" << nRet);
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;
#endif // QT_WIN32
}

QtResult CQtConditionVariableThread::Wait(CQtTimeValue *aTimeout)
{
#ifdef QT_WIN32
	// Prevent race conditions on the <waiters_> count.
	waiters_lock_.Lock();
	waiters_++;
	waiters_lock_.UnLock();

	int msec_timeout;
	if (!aTimeout)
		msec_timeout = INFINITE;
	else {
		msec_timeout = aTimeout->GetTotalInMsec();
		if (msec_timeout < 0)
			msec_timeout = 0;
	}

	// We keep the lock held just long enough to increment the count of
	// waiters by one.  Note that we can't keep it held across the call
	// to WaitForSingleObject since that will deadlock other calls to
	// ACE_OS::cond_signal().
	QtResult rv = m_MutexExternal.UnLock();
	QT_ASSERTE_RETURN(QT_SUCCEEDED(rv), rv);

	// <CQtSemaphore> has not time wait function due to pthread restriction,
	// so we have to use WaitForSingleObject() directly.
	DWORD result = ::WaitForSingleObject(sema_.GetSemaphoreType(), msec_timeout);

	waiters_lock_.Lock();
	waiters_--;
	int last_waiter = was_broadcast_ && waiters_ == 0;
	waiters_lock_.UnLock();

	switch (result) {
	case WAIT_OBJECT_0:
		rv = QT_OK;
		break;
	case WAIT_TIMEOUT:
		rv = QT_ERROR_TIMEOUT;
		break;
	default:
		QT_WARNING_TRACE_THIS("CQtConditionVariableThread::Wait, WaitForSingleObject() failed!"
			" result=" << result << 
			" err=" << ::GetLastError());
		rv = QT_ERROR_FAILURE;
		break;
	}

	if (last_waiter) {
		if (!::SetEvent(waiters_done_)) {
			QT_WARNING_TRACE_THIS("CQtConditionVariableThread::Wait, SetEvent() failed!"
				" err=" << ::GetLastError());
		}
	}

	// We must always regain the <external_mutex>, even when errors
	// occur because that's the guarantee that we give to our callers.
	m_MutexExternal.Lock();
	return rv;
#else // !QT_WIN32
	if (!aTimeout) {
		int nRet = ::pthread_cond_wait(&m_Condition, &(m_MutexExternal.GetMutexType()));
		if (nRet != 0) {
			QT_WARNING_TRACE_THIS("CQtConditionVariableThread::Wait, pthread_cond_wait() failed!"
				" err=" << nRet);
			return QT_ERROR_FAILURE;
		}
	}
	else {
		struct timespec tsBuf;
		CQtTimeValue tvAbs = CQtTimeValue::GetTimeOfDay() + *aTimeout;
		tsBuf.tv_sec = tvAbs.GetSec();
		tsBuf.tv_nsec = tvAbs.GetUsec() * 1000;
		int nRet = ::pthread_cond_timedwait(
			&m_Condition, 
			&(m_MutexExternal.GetMutexType()),
			&tsBuf);

		if (nRet != 0) {
			if (nRet == ETIMEDOUT)
				return QT_ERROR_TIMEOUT;
			// EINTR is OK.
			else if (nRet == EINTR)
				return QT_OK;
			else {
				QT_WARNING_TRACE_THIS("CQtConditionVariableThread::Wait, pthread_cond_timedwait() failed!"
					" err=" << nRet);
				return QT_ERROR_FAILURE;
			}
		}
	}
	return QT_OK;
#endif // QT_WIN32
}

QtResult CQtConditionVariableThread::Broadcast()
{
	// The <external_mutex> must be locked before this call is made.
#ifdef QT_WIN32
	// This is needed to ensure that <waiters_> and <was_broadcast_> are
	// consistent relative to each other.
	waiters_lock_.Lock();
	int have_waiters = 0;
	if (waiters_ > 0) {
		// We are broadcasting, even if there is just one waiter...
		// Record the fact that we are broadcasting.  This helps the
		// cond_wait() method know how to optimize itself.  Be sure to
		// set this with the <waiters_lock_> held.
		was_broadcast_ = 1;
		have_waiters = 1;
	}
	waiters_lock_.UnLock();

	QtResult rv = QT_OK;
	if (have_waiters) {
		QtResult rv1 = sema_.PostN(waiters_);
		if (QT_FAILED(rv1))
			rv = rv1;
		
		DWORD result = ::WaitForSingleObject(waiters_done_, INFINITE);
		if (result != WAIT_OBJECT_0) {
			QT_WARNING_TRACE_THIS("CQtConditionVariableThread::Broadcast, WaitForSingleObject() failed!"
			" result=" << result << 
			" err=" << ::GetLastError());
			rv = QT_ERROR_FAILURE;
		}
		was_broadcast_ = 0;
	}
	return rv;
#else // !QT_WIN32
	int nRet = ::pthread_cond_broadcast(&m_Condition);
	if (nRet != 0) {
		QT_WARNING_TRACE_THIS("CQtConditionVariableThread::Signal, pthread_cond_broadcast() failed!"
			" err=" << nRet);
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;
#endif // QT_WIN32
}


//////////////////////////////////////////////////////////////////////
// class CQtEventThread
//////////////////////////////////////////////////////////////////////

CQtEventThread::CQtEventThread(BOOL aManualReset, BOOL aInitialState, LPCSTR aName)
#ifndef QT_WIN32
	: condition_(lock_)
#endif // !QT_WIN32
{
#ifdef QT_WIN32
	handle_ = ::CreateEventA(NULL, aManualReset, aInitialState, aName);
	if (handle_ == 0) {
		QT_WARNING_TRACE_THIS("CQtEventThread::CQtEventThread, CreateEventA() failed!"
			" err=" << ::GetLastError());
		QT_ASSERTE(FALSE);
	}
#else // !QT_WIN32
	manual_reset_ = aManualReset;
	is_signaled_ = aInitialState;
	waiting_threads_ = 0;
#endif // QT_WIN32
}

CQtEventThread::~CQtEventThread()
{
#ifdef QT_WIN32
	if (!::CloseHandle(handle_)) {
		QT_WARNING_TRACE_THIS("CQtEventThread::~CQtEventThread, CloseHandle() failed!"
			" err=" << ::GetLastError());
	}
#else // !QT_WIN32
	// not need do cleanup.
#endif // QT_WIN32
}

QtResult CQtEventThread::Wait(CQtTimeValue *aTimeout)
{
	QtResult rv;
#ifdef QT_WIN32
	int msec_timeout;
	if (!aTimeout)
		msec_timeout = INFINITE;
	else {
		msec_timeout = aTimeout->GetTotalInMsec();
		if (msec_timeout < 0)
			msec_timeout = 0;
	}

	DWORD result = ::WaitForSingleObject(handle_, msec_timeout);
	switch (result) {
	case WAIT_OBJECT_0:
		rv = QT_OK;
		break;
	case WAIT_TIMEOUT:
		rv = QT_ERROR_TIMEOUT;
		break;
	default:
		QT_WARNING_TRACE_THIS("CQtEventThread::Wait, WaitForSingleObject() failed!"
			" result=" << result << 
			" err=" << ::GetLastError());
		rv = QT_ERROR_FAILURE;
		break;
	}
#else // !QT_WIN32
	rv = lock_.Lock();
	QT_ASSERTE_RETURN(QT_SUCCEEDED(rv), rv);

	if (is_signaled_ == 1) {
		// event is currently signaled
		if (manual_reset_ == 0)
			// AUTO: reset state
			is_signaled_ = 0;
	}
	else {
		// event is currently not signaled
		waiting_threads_++;
		rv = condition_.Wait(aTimeout);
		waiting_threads_--;
	}

	lock_.UnLock();
#endif // QT_WIN32
	return rv;
}

QtResult CQtEventThread::Signal()
{
#ifdef QT_WIN32
	if (!::SetEvent(handle_)) {
		QT_WARNING_TRACE_THIS("CQtEventThread::Signal, SetEvent failed!"
			" err=" << ::GetLastError());
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;
#else // !QT_WIN32
	QtResult rv;
	rv = lock_.Lock();
	QT_ASSERTE_RETURN(QT_SUCCEEDED(rv), rv);

	if (manual_reset_ == 1) {
		// Manual-reset event.
		is_signaled_ = 1;
		rv = condition_.Broadcast();
	}
	else {
		// Auto-reset event
		if (waiting_threads_ == 0)
			is_signaled_ = 1;
		else
			rv = condition_.Signal();
	}

	lock_.UnLock();
	return rv;
#endif // QT_WIN32
}

QtResult CQtEventThread::Pulse()
{
#ifdef QT_WIN32
	if (!::PulseEvent(handle_)) {
		QT_WARNING_TRACE_THIS("CQtEventThread::~CQtEventThread, PulseEvent() failed!"
			" err=" << ::GetLastError());
		return QT_ERROR_FAILURE;
	}
	else
		return QT_OK;
#else // !QT_WIN32
	QtResult rv;
	rv = lock_.Lock();
	QT_ASSERTE_RETURN(QT_SUCCEEDED(rv), rv);

	if (manual_reset_ == 1) {
		// Manual-reset event: Wakeup all waiters.
		rv = condition_.Broadcast();
	}
	else {
		// Auto-reset event: wakeup one waiter.
		rv = condition_.Signal();
	}
	
	is_signaled_ = 0;
	lock_.UnLock();
	return rv;
#endif // QT_WIN32
}
