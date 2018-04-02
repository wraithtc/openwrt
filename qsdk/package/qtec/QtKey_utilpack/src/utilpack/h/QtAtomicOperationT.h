/*------------------------------------------------------*/
/* Template class for atomic operations                 */
/*                                                      */
/* QtAtomicOperationT.h                                 */
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

#ifndef QTATOMICOPERATIONT_H
#define QTATOMICOPERATIONT_H

#include "QtDefines.h"

// mainly copied from ace/Atomic_Oh.h
#if defined (QT_WIN32) // for windows
  #if (_M_IX86 > 400)
    #define QT_HAS_PENTIUM
  #endif // _M_IX86
#elif defined (__GNUC__) // for g++
  #if !defined (__MINGW32__) && (defined (i386) || defined (__i386__))
  // If running an Intel, assume that it's a Pentium so that
  // ACE_OS::gethrtime () can use the RDTSC instruction.  If running a
  // 486 or lower, be sure to comment this out.  (If not running an
  // Intel CPU, this #define will not be seen because of the i386
  // protection, so it can be ignored.)
    # define QT_HAS_PENTIUM
  #endif /* i386 */
#endif // QT_WIN32

#include "QtThreadManager.h"

template <class MutexType> 
class QT_OS_EXPORT CQtAtomicOperationT
{
public:
	CQtAtomicOperationT(long aValue = 0) 
		: m_pMutex(NULL)
		, m_lValue(aValue)
	{
		CQtThreadManager::Instance()->GetReferenceControlMutex(m_pMutex);
	}
	
	long operator++ ()
	{
		CQtMutexGuardT<MutexType> theGuard(*m_pMutex);
		return ++m_lValue;
	}
	
	long operator++ (int)
	{
		return ++*this - 1;
	}
	
	long operator-- (void)
	{
		CQtMutexGuardT<MutexType> theGuard(*m_pMutex);
		return --m_lValue;
	}
	
	long operator-- (int)
	{
		return --*this + 1;
	}
	
	int operator== (long aRight) const
	{
		return (this->m_lValue == aRight);
	}

	int operator!= (long aRight) const
	{
		return (this->m_lValue != aRight);
	}

	int operator>= (long aRight) const
	{
		return (this->m_lValue >= aRight);
	}

	int operator> (long aRight) const
	{
		return (this->m_lValue > aRight);
	}

	int operator<= (long aRight) const
	{
		return (this->m_lValue <= aRight);
	}
	
	int operator< (long aRight) const
	{
		return (this->m_lValue < aRight);
	}

	void operator= (long aRight)
	{
		CQtMutexGuardT<MutexType> theGuard(*m_pMutex);
		m_lValue = aRight;
	}

	void operator= (const CQtAtomicOperationT &aRight)
	{
		CQtMutexGuardT<MutexType> theGuard(*m_pMutex);
		m_lValue = aRight.m_lValue;
	}

	long GetValue() const
	{
		return this->m_lValue;
	}
	
private:
	MutexType *m_pMutex;
	long m_lValue;
};


/// Specialization of <CQtAtomicOperationT> for platforms that 
/// support atomic integer operations.
#if defined (QT_WIN32)
  #define QT_HAS_BUILTIN_ATOMIC_OP
#elif defined (__GNUC__) && defined (QT_HAS_PENTIUM)
  #define QT_HAS_BUILTIN_ATOMIC_OP
#endif // QT_WIN32

#ifdef QT_HAS_BUILTIN_ATOMIC_OP
template<> class QT_OS_EXPORT CQtAtomicOperationT<CQtMutexThread>
{
public:
	CQtAtomicOperationT(long aValue = 0) 
		: m_lValue(aValue)
	{
	}

	/// Atomically pre-increment <m_lValue>.
	long operator++ ()
	{
#ifdef QT_WIN32
	return ::InterlockedIncrement(const_cast<long*>(&this->m_lValue));
#else
	return (*increment_fn_)(&this->m_lValue);
#endif // QT_WIN32
	}

	/// Atomically post-increment <m_lValue>.
	long operator++ (int)
	{
		return ++*this - 1;
	}

	/// Atomically pre-decrement <m_lValue>.
	long operator-- (void)
	{
#ifdef QT_WIN32
		return ::InterlockedDecrement(const_cast<long*>(&this->m_lValue));
#else
		return (*decrement_fn_)(&this->m_lValue);
#endif // QT_WIN32
	}

	/// Atomically post-decrement <m_lValue>.
	long operator-- (int)
	{
		return --*this + 1;
	}

	/// Atomically increment <m_lValue> by aRight.
	long operator+= (long aRight)
	{
#ifdef QT_WIN32
		return ::InterlockedExchangeAdd(const_cast<long*>(&this->m_lValue), aRight) + aRight;
#else
		return (*exchange_add_fn_)(&this->m_lValue, aRight) + aRight;
#endif // QT_WIN32
	}

	/// Atomically decrement <m_lValue> by aRight.
	long operator-= (long aRight)
	{
#ifdef QT_WIN32
		return ::InterlockedExchangeAdd(const_cast<long*>(&this->m_lValue), -aRight) - aRight;
#else
		return (*exchange_add_fn_)(&this->m_lValue, -aRight) - aRight;
#endif // QT_WIN32
	}

	/// Atomically compare <m_lValue> with aRight.
	int operator== (long aRight) const
	{
		return (this->m_lValue == aRight);
	}

	/// Atomically compare <m_lValue> with aRight.
	int operator!= (long aRight) const
	{
		return (this->m_lValue != aRight);
	}

	/// Atomically check if <m_lValue> greater than or equal to aRight.
	int operator>= (long aRight) const
	{
		return (this->m_lValue >= aRight);
	}

	/// Atomically check if <m_lValue> greater than aRight.
	int operator> (long aRight) const
	{
		return (this->m_lValue > aRight);
	}

	/// Atomically check if <m_lValue> less than or equal to aRight.
	int operator<= (long aRight) const
	{
		return (this->m_lValue <= aRight);
	}

	/// Atomically check if <m_lValue> less than aRight.
	int operator< (long aRight) const
	{
		return (this->m_lValue < aRight);
	}

	/// Atomically assign aRight to <m_lValue>.
	void operator= (long aRight)
	{
#ifdef QT_WIN32
		::InterlockedExchange(const_cast<long*>(&this->m_lValue), aRight);
#else
		(*exchange_fn_)(&this->m_lValue, aRight);
#endif // QT_WIN32
	}

	/// Atomically assign <aRight> to <m_lValue>.
	void operator= (const CQtAtomicOperationT &aRight)
	{
#ifdef QT_WIN32
		::InterlockedExchange(const_cast<long*>(&this->m_lValue), aRight.m_lValue);
#else
		(*exchange_fn_)(&this->m_lValue, aRight.m_lValue);
#endif // QT_WIN32
	}

	/// Explicitly return <m_lValue>.
	long GetValue() const
	{
		return this->m_lValue;
	}

	/// Used during <CQtThreadManager> initialization to optimize the fast
	/// atomic op implementation according to the number of CPUs.
	static void init_functions (void);

private:
	// Single-cpu atomic op implementations.
	static long single_cpu_increment (volatile long *value);
	static long single_cpu_decrement (volatile long *value);
	static long single_cpu_exchange (volatile long *value, long aRight);
	static long single_cpu_exchange_add (volatile long *value, long aRight);

	// Multi-cpu atomic op implementations.
	static long multi_cpu_increment (volatile long *value);
	static long multi_cpu_decrement (volatile long *value);
	static long multi_cpu_exchange (volatile long *value, long aRight);
	static long multi_cpu_exchange_add (volatile long *value, long aRight);

	// Pointers to selected atomic op implementations.
	static long (*increment_fn_) (volatile long *);
	static long (*decrement_fn_) (volatile long *);
	static long (*exchange_fn_) (volatile long *, long);
	static long (*exchange_add_fn_) (volatile long *, long);

private:
	/// Current object decorated by the atomic opearation.
	volatile long m_lValue;
};

#endif // QT_HAS_BUILTIN_ATOMIC_OP

#include "QtUtilClasses.h"

/// Specialization of <CQtAtomicOperationT> for single-thread.
/// Every <CQtAtomicOperationT> has its own <CQtMutexNullSingleThread>.
template<> class QT_OS_EXPORT CQtAtomicOperationT<CQtMutexNullSingleThread>
{
public:
	CQtAtomicOperationT(long aValue = 0) 
		: m_lValue(aValue)
	{
	}

	/// Atomically pre-increment <m_lValue>.
	long operator++ ()
	{
		m_Est.EnsureSingleThread();
		return ++this->m_lValue;
	}

	/// Atomically post-increment <m_lValue>.
	long operator++ (int)
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue++;
	}

	/// Atomically increment <m_lValue> by aRight.
	long operator+= (long aRight)
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue += aRight;
	}

	/// Atomically pre-decrement <m_lValue>.
	long operator-- (void)
	{
		m_Est.EnsureSingleThread();
		return --this->m_lValue;
	}

	/// Atomically post-decrement <m_lValue>.
	long operator-- (int)
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue--;
	}

	/// Atomically decrement <m_lValue> by aRight.
	long operator-= (long aRight)
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue -= aRight;
	}

	/// Atomically compare <m_lValue> with aRight.
	int operator== (long aRight) const
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue == aRight;
	}

	/// Atomically compare <m_lValue> with aRight.
	int operator!= (long aRight) const
	{
		m_Est.EnsureSingleThread();
		return !(*this == aRight);
	}

	/// Atomically check if <m_lValue> greater than or equal to aRight.
	int operator>= (long aRight) const
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue >= aRight;
	}

	/// Atomically check if <m_lValue> greater than aRight.
	int operator> (long aRight) const
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue > aRight;
	}

	/// Atomically check if <m_lValue> less than or equal to aRight.
	int operator<= (long aRight) const
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue <= aRight;
	}

	/// Atomically check if <m_lValue> less than aRight.
	int operator< (long aRight) const
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue < aRight;
	}

	/// Atomically assign aRight to <m_lValue>.
	void operator= (long aRight)
	{
		m_Est.EnsureSingleThread();
		this->m_lValue = aRight;
	}

	/// Atomically assign <aRight> to <m_lValue>.
	void operator= (const CQtAtomicOperationT &aRight)
	{
		if (&aRight == this)
			return; // Avoid deadlock...

		m_Est.EnsureSingleThread();
		this->m_lValue = aRight.GetValue();
	}

	/// Explicitly return <m_lValue>.
	long GetValue() const
	{
		m_Est.EnsureSingleThread();
		return this->m_lValue;
	}

private:
	CQtEnsureSingleThread m_Est;
	/// Current object decorated by the atomic opearation.
	volatile long m_lValue;
};

#endif // QTATOMICOPERATIONT_H
