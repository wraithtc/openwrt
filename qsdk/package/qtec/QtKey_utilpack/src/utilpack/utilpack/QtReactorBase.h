/*------------------------------------------------------*/
/* Reactor common base class                            */
/*                                                      */
/* QtReactorBase.h                                      */
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

#ifndef QTREACTORBASE_H
#define QTREACTORBASE_H

#include "QtDebug.h"
#include "QtReactorInterface.h"
#include "QtUtilClasses.h"
#include "QtEventQueueBase.h"
#include "QtObserver.h"

#ifdef QT_WIN32
#include <map>
#endif // QT_WIN32

static const DWORD s_dwDefaultTimerTickInterval = 30; // 30ms

class CQtReactorBase;
class CQtTimerQueueBase;

class CQtEventHandlerRepository
{
public:
	CQtEventHandlerRepository();
	~CQtEventHandlerRepository();

	QtResult Open();
	QtResult Close();

	struct CElement
	{
		AQtEventHandler *m_pEh;
		AQtEventHandler::MASK m_Mask;

		CElement(AQtEventHandler *aEh = NULL,
				 AQtEventHandler::MASK aMask = AQtEventHandler::NULL_MASK)
			: m_pEh(aEh), m_Mask(aMask)
		{
		}

		void Clear()
		{
			m_pEh = NULL; 
			m_Mask = AQtEventHandler::NULL_MASK;
		}

		BOOL IsCleared() const { return m_pEh == NULL; }
	};

	/**
	 * If success:
	 *    if <aFd> is found, return QT_OK;
	 *    else return QT_ERROR_NOT_FOUND;
	 */
	QtResult Find(QT_HANDLE aFd, CElement &aEle);

	/**
	 * If success:
	 *    if <aFd> is found, return QT_ERROR_FOUND;
	 *    else return QT_OK;
	 */
	QtResult Bind(QT_HANDLE aFd, const CElement &aEle);
	
	/**
	 * If success:
	 *    return QT_OK;
	 */
	QtResult UnBind(QT_HANDLE aFd);

	BOOL IsVaildHandle(QT_HANDLE aFd)
	{
#ifdef QT_WIN32
		if (aFd != QT_INVALID_HANDLE)
#else
		if (aFd >= 0 && aFd < m_nMaxHandler)
#endif // QT_WIN32
			return TRUE;
		else
			return FALSE;
	}

#ifndef QT_WIN32
	int GetMaxHandlers()
	{
		return m_nMaxHandler;
	}

	CElement* GetElement()
	{
		return m_pHandlers;
	}
#endif // !QT_WIN32

#ifndef QT_WIN32
	static QtResult SetRlimit(int aResource, int aMaxNum, int &aActualNum);
#endif // !QT_WIN32

	int FillFdSets(fd_set &aFsRead, fd_set &aFsWrite, fd_set &aFsException);

private:
#ifdef QT_WIN32
	typedef std::map<QT_HANDLE, CElement> HandlersType;
	HandlersType m_Handlers;
#else
	CElement *m_pHandlers;
	int m_nMaxHandler;
#endif // QT_WIN32
};


// base class for rector, 
// we have to inherit from <CQtEventQueueUsingMutex> because we 
// will over write PostEvent() to do NotifyHandler().
class QT_OS_EXPORT CQtReactorBase 
	: public IQtReactor
	, public CQtStopFlag
	, public CQtEventQueueUsingMutex 
{
public:
	CQtReactorBase(PROPERTY aProperty = NULL_PROPERTY);
	virtual ~CQtReactorBase();

	// interface IQtReactor
	virtual QtResult Open();

	virtual QtResult RegisterHandler(
		AQtEventHandler *aEh, 
		AQtEventHandler::MASK aMask);

	virtual QtResult RemoveHandler(
		AQtEventHandler *aEh, 
		AQtEventHandler::MASK aMask = AQtEventHandler::ALL_EVENTS_MASK);

	virtual QtResult Close();

	// interface IQtTimerQueue
	virtual QtResult ScheduleTimer(IQtTimerHandler *aTh, 
					  LPVOID aArg,
					  const CQtTimeValue &aInterval,
					  DWORD aCount);

	virtual QtResult CancelTimer(IQtTimerHandler *aTh);

	// interface IQtEventQueue
	virtual QtResult SendEvent(IQtEvent *aEvent);
	virtual QtResult PostEvent(
		IQtEvent *aEvent, 
		EPriority aPri = IQtReactor::EPRIORITY_NORMAL);
	virtual DWORD GetPendingEventsCount();

	QtResult ProcessHandleEvent(
		QT_HANDLE aFd, 
		AQtEventHandler::MASK aMask, 
		QtResult aReason,
		BOOL aIsNotify,
		BOOL aDropConnect = FALSE);

protected:
	QtResult ProcessTimerTick();

	virtual void OnHandleRemoved(QT_HANDLE aFd) = 0;
	virtual QtResult OnHandleRegister(
		QT_HANDLE aFd, 
		AQtEventHandler::MASK aMask, 
		AQtEventHandler *aEh) = 0;

	CQtEnsureSingleThread m_Est;
	CQtTimerQueueBase *m_pTimerQueue;
	BOOL m_bNotifyFailed;
	
private:
	QtResult RemoveHandleWithoutFinding_i(
		QT_HANDLE aFd, 
		const CQtEventHandlerRepository::CElement &aHe, 
		AQtEventHandler::MASK aMask);

protected:
	CQtEventHandlerRepository m_EhRepository;
};


// inline functions
inline QtResult CQtEventHandlerRepository::Find(QT_HANDLE aFd, CElement &aEle)
{
#ifdef QT_WIN32
	QT_ASSERTE_RETURN(IsVaildHandle(aFd), QT_ERROR_INVALID_ARG);
	HandlersType::iterator iter = m_Handlers.find(aFd);
	if (iter == m_Handlers.end())
		return QT_ERROR_NOT_FOUND;
	else {
		aEle = (*iter).second;
		QT_ASSERTE(!aEle.IsCleared());
		return QT_OK;
	}
#else
	// CAcceptor maybe find fd after closed when program shutting down.
	if (!m_pHandlers)
		return QT_ERROR_NOT_INITIALIZED;
	QT_ASSERTE_RETURN(IsVaildHandle(aFd), QT_ERROR_INVALID_ARG);

	CElement &eleFind = m_pHandlers[aFd];
	if (eleFind.IsCleared()) 
		return QT_ERROR_NOT_FOUND;
	else {
		aEle = eleFind;
		return QT_OK;
	}
#endif // QT_WIN32
}

inline QtResult CQtEventHandlerRepository::Bind(QT_HANDLE aFd, const CElement &aEle)
{
	QT_ASSERTE_RETURN(IsVaildHandle(aFd), QT_ERROR_INVALID_ARG);
	QT_ASSERTE_RETURN(!aEle.IsCleared(), QT_ERROR_INVALID_ARG);

#ifdef QT_WIN32
	CElement &eleBind = m_Handlers[aFd];
#else
	QT_ASSERTE_RETURN(m_pHandlers, QT_ERROR_NOT_INITIALIZED);
	CElement &eleBind = m_pHandlers[aFd];
#endif // QT_WIN32
	
	BOOL bNotBound = eleBind.IsCleared();
	eleBind = aEle;
	return bNotBound ? QT_OK : QT_ERROR_FOUND;
}

inline QtResult CQtEventHandlerRepository::UnBind(QT_HANDLE aFd)
{
	QT_ASSERTE_RETURN(IsVaildHandle(aFd), QT_ERROR_INVALID_ARG);

#ifdef QT_WIN32
	m_Handlers.erase(aFd);
#else
	QT_ASSERTE_RETURN(m_pHandlers, QT_ERROR_NOT_INITIALIZED);
	m_pHandlers[aFd].Clear();
#endif // QT_WIN32
	
	return QT_OK;
}

#endif // !QTREACTORBASE_H
