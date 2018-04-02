/*------------------------------------------------------*/
/* Condition variable classes                           */
/*                                                      */
/* QtConditionVariable.h                                */
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

#ifndef QTCONDITIONVARIABLE_H
#define QTCONDITIONVARIABLE_H

#include "QtMutex.h"

/**
 * Mainly copied from <ACE_Semaphore>
 * Wrapper for Dijkstra style general semaphores.
 */
class QT_OS_EXPORT CQtSemaphore
{
public:
	CQtSemaphore(LONG aInitialCount = 0, 
		LPCSTR aName = NULL, 
		LONG aMaximumCount = 0x7fffffff);

	~CQtSemaphore();

	/// Block the thread until the semaphore count becomes
	/// greater than 0, then decrement it.
	QtResult Lock();

	/// Increment the semaphore by 1, potentially unblocking a waiting thread.
	QtResult UnLock();

	/// Not supported yet.
//	QtResult TryLock();

	// No time wait function due to pthread restriction.
//	QtResult Wait(CQtTimeValue *aTimeout = NULL);

	/// Increment the semaphore by <aCount>, potentially
	/// unblocking waiting threads.
	QtResult PostN(LONG aCount);

	QT_SEMAPHORE_T& GetSemaphoreType() { return m_Semaphore;}

private:
	QT_SEMAPHORE_T m_Semaphore;
};

class CQtTimeValue;

/**
 * Mainly copyed from <ACE_Condition>.
 * <CQtConditionVariableThread> allows threads to block until shared 
 * data changes state.
 */
class QT_OS_EXPORT CQtConditionVariableThread  
{
public:
	CQtConditionVariableThread(CQtMutexThread &aMutex);
	~CQtConditionVariableThread();

	/// Block on condition.
	/// <aTimeout> is relative time.
	QtResult Wait(CQtTimeValue *aTimeout = NULL);

	/// Signal one waiting hread.
	QtResult Signal();

	/// Signal all waiting thread.
	QtResult Broadcast();

	/// Return the underlying mutex.
	CQtMutexThread& GetUnderlyingMutex() { return m_MutexExternal; }

private:
	CQtMutexThread &m_MutexExternal;

#ifdef QT_WIN32
	/// Number of waiting threads.
	long waiters_;
	/// Serialize access to the waiters count.
	CQtMutexThread waiters_lock_;
	/// Queue up threads waiting for the condition to become signaled.
	CQtSemaphore sema_;
	/**
	 * An auto reset event used by the broadcast/signal thread to wait
	 * for the waiting thread(s) to wake up and get a chance at the
	 * semaphore.
	 */
	HANDLE waiters_done_;
	/// Keeps track of whether we were broadcasting or just signaling.
	size_t was_broadcast_;
#else
	pthread_cond_t m_Condition;
#endif // QT_WIN32
};

/**
 * Mainly copied from <ACE_Event>
 *
 * @brief A wrapper around the Win32 event locking mechanism.
 *
 * Portable implementation of an Event mechanism, which is
 * native to Win32, but must be emulated on UNIX.  Note that
 * this only provides global naming support on Win32.  
 */
class QT_OS_EXPORT CQtEventThread
{
public:
	CQtEventThread(BOOL aManualReset = FALSE,
             BOOL aInitialState = FALSE,
             LPCSTR aName = NULL);

	~CQtEventThread();

	/**
	 * if MANUAL reset
	 *    sleep till the event becomes signaled
	 *    event remains signaled after wait() completes.
	 * else AUTO reset
	 *    sleep till the event becomes signaled
	 *    event resets wait() completes.
	 * <aTimeout> is relative time.
	 */
	QtResult Wait(CQtTimeValue *aTimeout = NULL);

	/**
	 * if MANUAL reset
	 *    wake up all waiting threads
	 *    set to signaled state
	 * else AUTO reset
	 *    if no thread is waiting, set to signaled state
	 *    if thread(s) are waiting, wake up one waiting thread and
	 *    reset event
	 */
	QtResult Signal();

	/// Set to nonsignaled state.
	QtResult Reset();

	/**
	 * if MANUAL reset
	 *    wakeup all waiting threads and
	 *    reset event
	 * else AUTO reset
	 *    wakeup one waiting thread (if present) and
	 *    reset event
	 */
	QtResult Pulse();

private:
#ifdef QT_WIN32
	HANDLE handle_;
#else
	/// Protect critical section.
	CQtMutexThread lock_;

	/// Keeps track of waiters.
	CQtConditionVariableThread condition_;

	/// Specifies if this is an auto- or manual-reset event.
	int manual_reset_;

	/// "True" if signaled.
	int is_signaled_;

	/// Number of waiting threads.
	u_long waiting_threads_;
#endif
};

#endif // !QTCONDITIONVARIABLE_H
