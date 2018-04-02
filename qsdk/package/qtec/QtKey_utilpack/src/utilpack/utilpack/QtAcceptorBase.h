/*------------------------------------------------------*/
/* Base class for acceptor pattern                      */
/*                                                      */
/* QtAcceptorBase.h                                     */
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

#ifndef QTACCEPTORBASE_H
#define QTACCEPTORBASE_H

#include "QtReferenceControl.h"
#include "QtReactorInterface.h"
#include "QtConnectionInterface.h"
#if defined (USE_SOCKETSERVER) || (!defined QT_WIN32 && !defined QT_PORT_CLIENT)

class QT_OS_EXPORT CQtAcceptorBase 
	: public IQtAcceptor
	, public CQtReferenceControlSingleThread 
{
public:
	CQtAcceptorBase();
	virtual ~CQtAcceptorBase();

	// interface IQtReferenceControl
	virtual DWORD AddReference();
	virtual DWORD ReleaseReference();

	// interface IQtAcceptorConnectorId
	virtual BOOL IsConnector();

	/// the <aCommand>s are all listed in file QtErrorNetwork.h
	virtual QtResult SetOption(DWORD aCommand, LPVOID aArg);
	virtual QtResult GetOption(DWORD aCommand, LPVOID aArg);
	
protected:
	AQtThread *m_pThreadNetwork;
	IQtReactor *m_pReactor;
	IQtReactor *m_pReactorNetwork;
	IQtAcceptorConnectorSink *m_pSink;
	int m_nRcvBuffLen;
	int m_nSndBuffLen;
	int m_Interval;
	int m_AcceptCount;
};
#endif
#endif // !QTACCEPTORBASE_H
