
#include "QtBase.h"
#include "QtThreadReactor.h"
#include "QtReactorInterface.h"
#include "QtConditionVariable.h"

//////////////////////////////////////////////////////////////////////
// class CQtThreadReactor
//////////////////////////////////////////////////////////////////////

CQtThreadReactor::CQtThreadReactor()
	: m_pReactor(NULL)
{
}

CQtThreadReactor::~CQtThreadReactor()
{
	delete m_pReactor;
}

QtResult CQtThreadReactor::Init(IQtReactor *aReactor)
{
	QT_ASSERTE_RETURN(!m_pReactor, QT_ERROR_ALREADY_INITIALIZED);
	QT_ASSERTE_RETURN(aReactor, QT_ERROR_INVALID_ARG);
	
	m_pReactor = aReactor;
	return QT_OK;
}

QtResult CQtThreadReactor::
Create(CQtThreadManager::TType aType, CQtThreadManager::TFlag aFlag)
{
	QtResult rv = AQtThread::Create(aType, aFlag);
	if (QT_SUCCEEDED(rv)) {
		// have to open reactor here because main function will do some initial stuffs.
		if (m_Type == CQtThreadManager::TT_MAIN) {
			rv = m_pReactor->Open();
			if (QT_FAILED(rv)) {
				QT_ERROR_TRACE_THIS("CQtThreadReactor::OnThreadRun, m_pReactor->Open() failed! rv=" << rv);
			}
		}
	}
	return rv;
}

void CQtThreadReactor::OnThreadInit()
{
	QT_ASSERTE_RETURN_VOID(m_pReactor);

	if (m_Type != CQtThreadManager::TT_MAIN) {
		QtResult rv = m_pReactor->Open();
		if (QT_FAILED(rv)) {
			QT_ERROR_TRACE_THIS("CQtThreadReactor::OnThreadInit, m_pReactor->Open() failed! rv=" << rv);
			QT_ASSERTE(FALSE);
		}
	}
}

void CQtThreadReactor::OnThreadRun()
{
	QT_ASSERTE_RETURN_VOID(m_pReactor);
	QT_INFO_TRACE_THIS("CQtThreadReactor::OnThreadRun, Begin. m_pReactor = " << m_pReactor);
	
	m_pReactor->RunEventLoop();

	// close the notify avoid Close in other thread .
	// because it will remove handler in the reactor.
	m_pReactor->Close();

	QT_INFO_TRACE_THIS("CQtThreadReactor::OnThreadRun, End.");
}

QtResult CQtThreadReactor::Stop(CQtTimeValue* aTimeout)
{
//	QT_ASSERTE_RETURN(!aTimeout, QT_ERROR_NOT_IMPLEMENTED);
	QT_ASSERTE_RETURN(m_pReactor, QT_ERROR_NOT_INITIALIZED);
	return m_pReactor->StopEventLoop();
}

IQtReactor* CQtThreadReactor::GetReactor()
{
	return m_pReactor;
}

IQtEventQueue* CQtThreadReactor::GetEventQueue()
{
	return m_pReactor;
}

IQtTimerQueue* CQtThreadReactor::GetTimerQueue()
{
	return m_pReactor;
}

//////////////////////////////////////////////////////////////////////
// class CQtThreadDummy
//////////////////////////////////////////////////////////////////////

CQtThreadDummy::CQtThreadDummy()
	: m_pActualThread(NULL)
{
}

CQtThreadDummy::~CQtThreadDummy()
{
}

QtResult CQtThreadDummy::Init(AQtThread *aThread, CQtThreadManager::TType aType)
{
	QT_ASSERTE_RETURN(!m_pActualThread, QT_ERROR_ALREADY_INITIALIZED);
	QT_ASSERTE_RETURN(aThread, QT_ERROR_INVALID_ARG);
	QT_ASSERTE(aThread->GetThreadType() != aType);
	
	m_Type = aType;
	m_Tid = aThread->GetThreadId();
	m_Handle = aThread->GetThreadHandle();
	m_pActualThread = aThread;

	return CQtThreadManager::Instance()->RegisterThread(this);
}

QtResult CQtThreadDummy::
Create(CQtThreadManager::TType aType, CQtThreadManager::TFlag aFlag)
{
	QT_ASSERTE(!"CQtThreadDummy::Create");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtThreadDummy::Stop(CQtTimeValue* aTimeout)
{
	if (m_pActualThread)
		return m_pActualThread->Stop(aTimeout);
	return
		QT_ERROR_NOT_INITIALIZED;
}

void CQtThreadDummy::OnThreadRun()
{
	QT_ASSERTE(!"CQtThreadDummy::OnThreadRun");
}

IQtReactor* CQtThreadDummy::GetReactor()
{
	if (m_pActualThread)
		return m_pActualThread->GetReactor();
	else
		return NULL;
}

IQtEventQueue* CQtThreadDummy::GetEventQueue()
{
	if (m_pActualThread)
		return m_pActualThread->GetEventQueue();
	else
		return NULL;
}

IQtTimerQueue* CQtThreadDummy::GetTimerQueue()
{
	if (m_pActualThread)
		return m_pActualThread->GetTimerQueue();
	else
		return NULL;
}
