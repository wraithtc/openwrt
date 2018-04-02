/*------------------------------------------------------*/
/* Templates classes                                    */
/*                                                      */
/* QtUtilTemplates.h                                    */
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

#ifndef QTUTILTEMPLATES_H
#define QTUTILTEMPLATES_H

#include "QtThreadManager.h"
#include "QtReferenceControl.h"
#include "QtTimerWrapperID.h"
#include "QtTimeValue.h"

/**
 * @class CQtSingletonT
 *
 * @brief A Singleton Adapter uses the Adapter pattern to turn ordinary
 * classes into Singletons optimized with the Double-Checked
 * Locking optimization pattern.
 */

//Victor 10/16 2006, That static var not be init on MAC if unload dll, why? 
template <class Type> 
class CQtSingletonT : public CQtCleanUpBase
{
public:
	static Type* Instance() 
	{
		if (!m_psInstance) {
			MutexType *pMutex = NULL;
			CQtThreadManager::Instance()->GetSingletonMutex(pMutex);
			if (pMutex) {
				CQtMutexGuardT<MutexType> theGuard(*pMutex);
				if (!m_psInstance) {
					m_psInstance = new CQtSingletonT<Type>();
				}
			}
			QT_ASSERTE(m_psInstance);
		}
		return &m_psInstance->m_Instance;
	}
	
protected:
	CQtSingletonT(){};

	virtual ~CQtSingletonT() 
	{ 
		QT_INFO_TRACE_THIS("CQtSingletonT::~CQtSingletonT() instance = " << m_psInstance); 
		m_psInstance = NULL;
	}
	Type m_Instance;

private:
	typedef CQtMutexThreadRecursive MutexType;
	
	// = Prevent assignment and initialization.
	void operator = (const CQtSingletonT&);
	CQtSingletonT(const CQtSingletonT&);
	static CQtSingletonT *m_psInstance;
};

template< typename Type> 
CQtSingletonT<Type> * CQtSingletonT<Type>::m_psInstance = NULL;

template <class DeleteType>
class CQtEventDeleteT : public IQtEvent
{
public:
	CQtEventDeleteT(DeleteType *aDelete)
		: m_pDeleteType(aDelete)
		, m_bHaveDeleted(FALSE)
		, m_bHaveLaunched(FALSE)
	{
		QT_ASSERTE(m_pDeleteType);
		QT_ASSERTE(static_cast<void*>(aDelete) != static_cast<void*>(this));
	}

	virtual ~CQtEventDeleteT() 
	{
		if (!m_bHaveDeleted) {
			m_bHaveDeleted = TRUE;
			delete m_pDeleteType;
		}
	}

	QtResult Launch(AQtThread* aThread)
	{
		QT_ASSERTE_RETURN(aThread, QT_ERROR_INVALID_ARG);
		QT_ASSERTE_RETURN(!m_bHaveLaunched, QT_ERROR_ALREADY_INITIALIZED);
		m_bHaveLaunched = TRUE;

		QtResult rv = QT_ERROR_NULL_POINTER;
		IQtEventQueue *pEq = aThread->GetEventQueue();
		if (pEq) {
			rv = pEq->PostEvent(this);
		}
		
		if (QT_FAILED(rv)) {
			QT_ERROR_TRACE_THIS("CQtEventDeleteT::Launch, PostEvent() failed! rv=" << rv);
		}
		return rv;
	}

	virtual QtResult OnEventFire()
	{
//		QT_ASSERTE(0 == m_pDeleteType->GetReference());
		QT_ASSERTE(m_bHaveLaunched);

		QT_ASSERTE(!m_bHaveDeleted);
		m_bHaveDeleted = TRUE;
		delete m_pDeleteType;
//		m_pDeleteType = NULL;
		return QT_OK;
	}

private:
	DeleteType *m_pDeleteType;
	BOOL m_bHaveDeleted;
	BOOL m_bHaveLaunched;
};

template <class DeleteType>
class CQtEventDeleteRefT : public IQtEvent
{
public:
	CQtEventDeleteRefT(DeleteType *aDelete)
		: m_pDeleteType(aDelete)
		, m_bHaveDeleted(FALSE)
		, m_bHaveLaunched(FALSE)
	{
		QT_ASSERTE(m_pDeleteType);
		QT_ASSERTE(static_cast<void*>(aDelete) != static_cast<void*>(this));
	}

	virtual ~CQtEventDeleteRefT() 
	{
		if (!m_bHaveDeleted && !m_pDeleteType->GetReference()) {
			m_bHaveDeleted = TRUE;
			delete m_pDeleteType;
		}
	}

	QtResult Launch(AQtThread* aThread)
	{
		QT_ASSERTE_RETURN(aThread, QT_ERROR_INVALID_ARG);
		QT_ASSERTE_RETURN(!m_bHaveLaunched, QT_ERROR_ALREADY_INITIALIZED);
		m_bHaveLaunched = TRUE;

		QtResult rv = QT_ERROR_NULL_POINTER;
		IQtEventQueue *pEq = aThread->GetEventQueue();
		if (pEq) {
			rv = pEq->PostEvent(this);
		}
		
		if (QT_FAILED(rv)) {
			QT_ERROR_TRACE_THIS("CQtEventDeleteRefT::Launch, PostEvent() failed! rv=" << rv);
		}
		return rv;
	}

	virtual QtResult OnEventFire()
	{
//		QT_ASSERTE(0 == m_pDeleteType->GetReference());
		QT_ASSERTE(m_bHaveLaunched);

		QT_ASSERTE(!m_bHaveDeleted);
		if (!m_pDeleteType->GetReference()) {
			m_bHaveDeleted = TRUE;
			delete m_pDeleteType;
		}
		return QT_OK;
	}

private:
	DeleteType *m_pDeleteType;
	BOOL m_bHaveDeleted;
	BOOL m_bHaveLaunched;
};

template <class DeleteType>
class CQtTimerDeleteT : public IQtTimerHandler
{
public:
	CQtTimerDeleteT(DeleteType *aDelete)
		: m_pDeleteType(aDelete)
		, m_bHaveLaunched(FALSE)
	{
		QT_ASSERTE(m_pDeleteType);
		QT_ASSERTE(static_cast<void*>(aDelete) != static_cast<void*>(this));
	}
	
	virtual ~CQtTimerDeleteT() 
	{
	}

	QtResult Launch()
	{
		QT_ASSERTE_RETURN(!m_bHaveLaunched, QT_ERROR_ALREADY_INITIALIZED);
		m_bHaveLaunched = TRUE;
		
		IQtTimerQueue *pTimerQueue = NULL;
		AQtThread* pThread = 
			CQtThreadManager::Instance()->GetThread(CQtThreadManager::TT_CURRENT);
		if (pThread)
			pTimerQueue = pThread->GetTimerQueue();
		
		QtResult rv = QT_ERROR_NULL_POINTER;
		if (pTimerQueue) {
			// can't use CQtTimeValue::s_tvZero due to it is not exported. strange!
			rv = pTimerQueue->ScheduleTimer(this, NULL, CQtTimeValue(), 1);
			if (rv == QT_ERROR_FOUND)
				rv = QT_OK;
		} 

		if (QT_FAILED(rv)) {
			QT_ERROR_TRACE_THIS("CQtTimerDeleteT::Launch, ScheduleTimer() failde! rv=" << rv);
		}
		return rv;
	}
	
	virtual void OnTimeout(const CQtTimeValue &, LPVOID aArg)
	{
//		QT_ASSERTE(0 == m_pDeleteType->GetReference());
		QT_ASSERTE(m_bHaveLaunched);

		delete m_pDeleteType;
//		m_pDeleteType = NULL;
	}
	
private:
	DeleteType *m_pDeleteType;
	BOOL m_bHaveLaunched;
};

template <class MutexType>
class CQtReferenceControlTimerDeleteT 
	: public CQtReferenceControlT<MutexType>
{
public:
	CQtReferenceControlTimerDeleteT()
		: m_Delete(this)
	{
	}

	virtual ~CQtReferenceControlTimerDeleteT() { }

protected:
	virtual void OnReferenceDestory()
	{
#ifdef QT_DEBUG
		//QtResult rv = 
#endif // QT_DEBUG
			m_Delete.Launch();

		// the thread is stopped if rv == QT_ERROR_NOT_INITIALIZED.
		//QT_ASSERTE(QT_SUCCEEDED(rv) || rv == QT_ERROR_NOT_INITIALIZED);
		//disable up to fix bug 171368, because the thread already remove from thread manager(swap!)
	}

private:
	CQtTimerDeleteT<CQtReferenceControlTimerDeleteT> m_Delete;
	friend class CQtTimerDeleteT<CQtReferenceControlTimerDeleteT>;
};

typedef CQtReferenceControlTimerDeleteT<CQtMutexNullSingleThread> CQtReferenceControlSingleThreadTimerDelete;
typedef CQtReferenceControlTimerDeleteT<CQtMutexThread> CQtReferenceControlMutilThreadTimerDelete;

#endif // !QTUTILTEMPLATES_H
