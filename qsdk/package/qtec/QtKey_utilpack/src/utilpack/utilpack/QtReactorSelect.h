/*------------------------------------------------------*/
/* Reactor using select                                 */
/*                                                      */
/* QtReactorRealTimeSignal.h                            */
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

#ifndef QTREACTORSELECT_H
#define QTREACTORSELECT_H

#include "QtReactorBase.h"
#include "QtReactorNotifyPipe.h"

class CQtReactorSelect : public CQtReactorBase  
{
public:
	CQtReactorSelect();
	virtual ~CQtReactorSelect();

	// interface IQtReactor
	virtual QtResult Open();

	virtual QtResult NotifyHandler(
		AQtEventHandler *aEh, 
		AQtEventHandler::MASK aMask);

	virtual QtResult RunEventLoop();

	virtual QtResult StopEventLoop();
	
	virtual QtResult Close();

protected:
	virtual QtResult OnHandleRegister(QT_HANDLE aFd, 
		AQtEventHandler::MASK aMask, AQtEventHandler *aEh);
	virtual void OnHandleRemoved(QT_HANDLE aFd);

	void ProcessFdSets_i(
		fd_set &aFdSet, 
		AQtEventHandler::MASK aMask, 
		int &aActiveNumber, 
		int aMaxFd);

private:
	CQtReactorNotifyPipe m_Notify;
};

#endif // !QTREACTORSELECT_H
