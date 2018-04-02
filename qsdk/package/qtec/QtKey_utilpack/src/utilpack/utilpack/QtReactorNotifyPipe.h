/*------------------------------------------------------*/
/* Nofily reactor implemented by CPipe                  */
/*                                                      */
/* QtReactorNotifyPipe.h                                */
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

#ifndef QTREACTORNOTIFYPIPE_H
#define QTREACTORNOTIFYPIPE_H

//#ifndef QT_LINUX
//  #error ERROR: ReactorNotifyPipe only supports LINUX now!
//#endif // QT_LINUX

#include "QtPipe.h"
#include "QtReactorInterface.h"

class CQtReactorBase;

class CQtReactorNotifyPipe : public AQtEventHandler
{
public:
	CQtReactorNotifyPipe();
	virtual ~CQtReactorNotifyPipe();

	QtResult Open(CQtReactorBase *aReactor);
	QtResult Close();

	// interface AQtEventHandler
	virtual QT_HANDLE GetHandle() const ;
	virtual int OnInput(QT_HANDLE aFd = QT_INVALID_HANDLE);

	QtResult Notify(AQtEventHandler *aEh, AQtEventHandler::MASK aMask);
	
private:
	struct CBuffer
	{
		CBuffer(QT_HANDLE aFd = QT_INVALID_HANDLE,
			   AQtEventHandler::MASK aMask = AQtEventHandler::NULL_MASK)
			: m_Fd(aFd), m_Mask(aMask)
		{
		}

		QT_HANDLE m_Fd;
		AQtEventHandler::MASK m_Mask;
	};
	
	CQtPipe m_PipeNotify;
	CQtReactorBase *m_pReactor;
};

#endif // !QTREACTORNOTIFYPIPE_H
