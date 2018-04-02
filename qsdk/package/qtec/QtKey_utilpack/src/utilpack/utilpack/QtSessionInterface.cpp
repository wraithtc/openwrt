
#include "QtBase.h"
#include "QtSessionInterface.h"
#include "QtSessionOneForwardMany.h"

CQtSessionManager CQtSessionManager::s_QtSessionManagerSingleton;

CQtSessionManager::CQtSessionManager()
{
}

CQtSessionManager::~CQtSessionManager()
{
}

CQtSessionManager* CQtSessionManager::Instance()
{
	return &s_QtSessionManagerSingleton;
}

QtResult CQtSessionManager::
CreateSessionForward(IQtSessionOneForwardMany *&aForward)
{
	aForward = new CQtSessionOneForwardMany();
	if (aForward) {
		aForward->AddReference();
		return QT_OK;
	}
	else
		return QT_ERROR_OUT_OF_MEMORY;
}
