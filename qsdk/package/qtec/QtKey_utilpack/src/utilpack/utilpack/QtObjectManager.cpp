
#include "QtBase.h"
#include "QtObjectManager.h"


CQtObjectManager::CQtObjectManager()
{
}

CQtObjectManager::~CQtObjectManager()
{
}

//todo: Field, Buding
//CQtMutexThreadRecursive& CQtThreadManager::GetSingletonMutex()
CQtMutexThreadRecursive& CQtObjectManager::GetSingletonMutex()
{
	return m_SingletonMutex;
}
