/*------------------------------------------------------*/
/* Base class for observer pattern                      */
/*                                                      */
/* QtObserver.h                                         */
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

#ifndef QTOBSERVER_H
#define QTOBSERVER_H

#include "QtThreadInterface.h"

class QT_OS_EXPORT IQtObserver
{
public:
	virtual void OnObserve(LPCSTR aTopic, LPVOID aData = NULL) = 0;

protected:
	virtual ~IQtObserver() { }
};

template <class T>
class CQtObserverEventT : public IQtEvent
{
public:
	CQtObserverEventT(T *aT, IQtObserver *aObserver)
		: m_pT(aT)
		, m_pObserver(aObserver)
	{
	}

	// interface IQtEvent
	virtual QtResult OnEventFire()
	{
		QT_ASSERTE_RETURN(m_pT, QT_ERROR_NULL_POINTER);
		return m_pT->OnObserverEvent(m_pObserver);
	}
	
private:
	T *m_pT;
	IQtObserver *m_pObserver;
};

#endif // !QTOBSERVER_H
