/*------------------------------------------------------*/
/* Session interfaces                                   */
/*                                                      */
/* QtSessionInterface.h                                 */
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

#ifndef QTSESSIONINTERFACE_H
#define QTSESSIONINTERFACE_H

#include "QtReferenceControl.h"

class IQtTransport;
class CQtMessageBlock;
class CQtTransportParameter;
class IQtSessionOneForwardMany;

class QT_OS_EXPORT CQtSessionManager
{
public:
	static CQtSessionManager* Instance();

	/// Create <IQtSessionOneForwardMany>.
	QtResult CreateSessionForward(IQtSessionOneForwardMany *&aForward);

	~CQtSessionManager();

private:
	CQtSessionManager();

	static CQtSessionManager s_QtSessionManagerSingleton;
};

class QT_OS_EXPORT IQtSessionOneForwardMany : public IQtReferenceControl
{
public:
	virtual QtResult AddTransport(IQtTransport *aTrpt) = 0;

	virtual QtResult RemoveTransport(IQtTransport *aTrpt) = 0;

	virtual QtResult RemoveAllTransports() = 0;

	// send <aData> to all added tranports except <aSender>.
	virtual QtResult SendDataToAll(
		CQtMessageBlock &aData, 
		CQtTransportParameter *aPara = NULL,
		IQtTransport *aSender = NULL) = 0;

protected:
	virtual ~IQtSessionOneForwardMany() { }
};

#endif // !QTSESSIONINTERFACE_H
