/*------------------------------------------------------*/
/* Forwart session (1 to many)                          */
/*                                                      */
/* QtSessionOneForwardMany.h                            */
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

#ifndef QTSESSIONONEFORWARDMANY_H
#define QTSESSIONONEFORWARDMANY_H

#include "QtSessionInterface.h"
#include "QtConnectionInterface.h"
#include "QtMutex.h"
#include <list>

class QT_OS_EXPORT CQtSessionOneForwardMany 
	: public IQtSessionOneForwardMany
	, public CQtReferenceControlMutilThread
{
public:
	CQtSessionOneForwardMany();
	virtual ~CQtSessionOneForwardMany();

	// interface IQtReferenceControl
	virtual DWORD AddReference();
	virtual DWORD ReleaseReference();

	// interface IQtSessionOneForwardMany
	virtual QtResult AddTransport(IQtTransport *aTrpt);

	virtual QtResult RemoveTransport(IQtTransport *aTrpt);

	virtual QtResult RemoveAllTransports();
	
	virtual QtResult SendDataToAll(
		CQtMessageBlock &aData, 
		CQtTransportParameter *aPara = NULL,
		IQtTransport *aSender = NULL);

private:
	QtResult Send_i(
		CQtMessageBlock &aData, 
		CQtTransportParameter *aPara, 
		IQtTransport *aSender);

private:
	typedef CQtMutexThread MutexType;
	typedef std::list<CQtComAutoPtr<IQtTransport> > TrptsType;

	AQtThread *m_pThreadNetwork;
	MutexType m_Mutex;
	TrptsType m_Trpts;

	friend class CEventSendDataToAll;
};

	class CEventSendDataToAll : public IQtEvent
	{
	public:
		CEventSendDataToAll(
			CQtSessionOneForwardMany *aSession,
			CQtMessageBlock &aData, 
			CQtTransportParameter *aPara,
			IQtTransport *aSender);

		virtual ~CEventSendDataToAll();

		virtual QtResult OnEventFire();

	private:
		CQtComAutoPtr<CQtSessionOneForwardMany> m_pOwnerSession;
		CQtMessageBlock *m_pMessageBlock;
		CQtTransportParameter m_TransportParameter;
		CQtTransportParameter *m_pParaTransportParameter;
		CQtComAutoPtr<IQtTransport> m_pTransportender;
	};

#endif // !QTSESSIONONEFORWARDMANY_H
