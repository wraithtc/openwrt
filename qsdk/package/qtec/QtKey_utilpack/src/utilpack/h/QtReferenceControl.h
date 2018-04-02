/*------------------------------------------------------*/
/* ReferenceControl interface and util classes          */
/*                                                      */
/* QtReferenceControl.h                                 */
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

#ifndef QTREFERENCECONTROL_H
#define QTREFERENCECONTROL_H

#include "QtDefines.h"
#include "QtAtomicOperationT.h"

class QT_OS_EXPORT IQtReferenceControl
{
public:
	virtual DWORD AddReference() = 0;
	virtual DWORD ReleaseReference() = 0;

protected:
	virtual ~IQtReferenceControl() {}
};

/**
 *	ReferenceControl basic classes, for mutil-thread.
 *	TODO: use AtomicIncrement instead MutexThread!
 */
template <class MutexType> class CQtReferenceControlT  
{
public:
	CQtReferenceControlT()
	{
	}

	virtual ~CQtReferenceControlT()
	{
	}

	virtual DWORD AddReference()
	{
		return ++m_Atomic;
	}

	virtual DWORD ReleaseReference()
	{
		DWORD dwRef = --m_Atomic;
		if (dwRef == 0) 
			OnReferenceDestory();
		return dwRef;
	}

	virtual DWORD GetReference()
	{
		return m_Atomic.GetValue();
	}

protected:
	virtual void OnReferenceDestory()
	{
		delete this;
	}

	CQtAtomicOperationT<MutexType> m_Atomic;
};


#if 0
template <class MutexType> 
class CQtReferenceControlT  
{
public:
	CQtReferenceControlT()
		: m_dwReference(0)
	{
	}

	virtual ~CQtReferenceControlT()
	{
	}

	DWORD AddReference()
	{
		MutexType *pMutex = NULL;
		CQtThreadManager::Instance()->GetReferenceControlMutex(pMutex);
		QT_ASSERTE(pMutex);
		
		CQtMutexGuardT<MutexType> theGuard(*pMutex);
		DWORD dwRef = ++m_dwReference;
		return dwRef;
	}

	DWORD ReleaseReference()
	{
		QT_ASSERTE(m_dwReference > 0);
		MutexType *pMutex = NULL;
		CQtThreadManager::Instance()->GetReferenceControlMutex(pMutex);
		QT_ASSERTE(pMutex);

		DWORD dwRef;
		{
			CQtMutexGuardT<MutexType> theGuard(*pMutex);
			dwRef = --m_dwReference;
		}
		if (dwRef == 0) 
			OnReferenceDestory();
		return dwRef;
	}

	DWORD GetReference()
	{
		return m_dwReference;
	}

protected:
	virtual void OnReferenceDestory()
	{
		delete this;
	}

	DWORD m_dwReference;
};

/**
 * ReferenceControl basic classes, for single-thread.
 * Can't share the <CQtMutexNullSingleThread> in the CQtThreadManager::Instance().
 * Every <CQtReferenceControlSingleThread> has its own <CQtMutexNullSingleThread>.
 */
class CQtReferenceControlT<CQtMutexNullSingleThread>  
{
public:
	CQtReferenceControlT()
		: m_dwReference(0)
	{
	}

	virtual ~CQtReferenceControlT()
	{
	}

	DWORD AddReference()
	{
		CQtMutexGuardT<MutexType> theGuard(m_Mutex);
		DWORD dwRef = ++m_dwReference;
		return dwRef;
	}

	DWORD ReleaseReference()
	{
		QT_ASSERTE(m_dwReference > 0);
		DWORD dwRef;
		{
			CQtMutexGuardT<MutexType> theGuard(m_Mutex);
			dwRef = --m_dwReference;
		}
		if (dwRef == 0) 
			OnReferenceDestory();
		return dwRef;
	}

	DWORD GetReference()
	{
		return m_dwReference;
	}

protected:
	virtual void OnReferenceDestory()
	{
		delete this;
	}

	DWORD m_dwReference;
	typedef CQtMutexNullSingleThread MutexType;
	MutexType m_Mutex;
};
#endif

typedef CQtReferenceControlT<CQtMutexNullSingleThread> CQtReferenceControlSingleThread;
typedef CQtReferenceControlT<CQtMutexThread> CQtReferenceControlMutilThread;


/**
 *	Auto pointer for ReferenceControl
 */
template <class T> class CQtComAutoPtr
{
public:
	CQtComAutoPtr(T *aPtr = NULL) 
		: m_pRawPtr(aPtr)
	{
		if (m_pRawPtr)
			m_pRawPtr->AddReference();
	}

	CQtComAutoPtr(const CQtComAutoPtr& aAutoPtr) 
		: m_pRawPtr(aAutoPtr.m_pRawPtr)
	{
		if (m_pRawPtr)
			m_pRawPtr->AddReference();
	}

	~CQtComAutoPtr() 
	{
		if (m_pRawPtr)
			m_pRawPtr->ReleaseReference();
	}

	CQtComAutoPtr& operator = (const CQtComAutoPtr& aAutoPtr) 
	{
		return (*this = aAutoPtr.m_pRawPtr);
	}

	CQtComAutoPtr& operator = (T* aPtr) 
	{
		if (m_pRawPtr == aPtr)
			return *this;

		if (aPtr)
			aPtr->AddReference();
		if (m_pRawPtr)
			m_pRawPtr->ReleaseReference();
		m_pRawPtr = aPtr;
		return *this;
	}

	operator void* () const 
	{
		return m_pRawPtr;
	}

	T* operator -> () const 
	{
		QT_ASSERTE(m_pRawPtr);
		return m_pRawPtr;
	}

	T* Get() const 
	{
		return m_pRawPtr;
	}

	T* ParaIn() const 
	{
		return m_pRawPtr;
	}

	T*& ParaOut() 
	{
		if (m_pRawPtr) {
			m_pRawPtr->ReleaseReference();
			m_pRawPtr = NULL;
		}
		return static_cast<T*&>(m_pRawPtr);
	}

	T*& ParaInOut() 
	{
		return static_cast<T*&>(m_pRawPtr);
	}

	T& operator * () const 
	{
		QT_ASSERTE(m_pRawPtr);
		return *m_pRawPtr;
	}

private:
	T *m_pRawPtr;
};

#endif // !QTREFERENCECONTROL_H
