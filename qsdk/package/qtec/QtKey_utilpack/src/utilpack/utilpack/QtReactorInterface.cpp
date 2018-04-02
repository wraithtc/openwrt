
#include "QtBase.h"
#include "QtReactorInterface.h"

//////////////////////////////////////////////////////////////////////
// class AQtEventHandler
//////////////////////////////////////////////////////////////////////

AQtEventHandler::~AQtEventHandler()
{
}

QT_HANDLE AQtEventHandler::GetHandle() const 
{
	QT_ASSERTE(!"AQtEventHandler::GetHandle()");
	return QT_INVALID_HANDLE;
}

int AQtEventHandler::OnInput(QT_HANDLE )
{
	QT_ASSERTE(!"AQtEventHandler::OnInput()");
	return -1;
}

int AQtEventHandler::OnOutput(QT_HANDLE )
{
	QT_ASSERTE(!"AQtEventHandler::OnOutput()");
	return -1;
}

int AQtEventHandler::OnException(QT_HANDLE )
{
	QT_ASSERTE(!"AQtEventHandler::OnException()");
	return -1;
}

int AQtEventHandler::OnClose(QT_HANDLE , MASK )
{
	QT_ASSERTE(!"AQtEventHandler::OnClose()");
	return -1;
}


//////////////////////////////////////////////////////////////////////
// class IQtReactor
//////////////////////////////////////////////////////////////////////

IQtReactor::~IQtReactor()
{
}
