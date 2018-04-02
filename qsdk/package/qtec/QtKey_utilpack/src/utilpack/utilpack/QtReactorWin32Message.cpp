
#include "QtBase.h"
#include "QtReactorWin32Message.h"

//////////////////////////////////////////////////////////////////////
// class CQtReactorWin32Message
//////////////////////////////////////////////////////////////////////

#define WM_WIN32_SOCKET_SELECT    WM_USER+33
#define WM_WIN32_SOCKET_NOTIFY    WM_USER+34
#define WIN32_SOCKET_CLASS_NAME	  "QtWin32SocketNotification"

HINSTANCE g_pReactorWin3Instance;
ATOM g_atomRegisterClass;
char g_szClassName[MAX_PATH] = {0};

LRESULT CALLBACK CQtReactorWin32Message::
Win32SocketWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	switch(uMsg)
	{
	case WM_WIN32_SOCKET_SELECT: 
	{
		CQtReactorWin32Message *pReactor = (CQtReactorWin32Message *)::GetWindowLong(hwnd, 0);
		AQtEventHandler::MASK maskEvent = AQtEventHandler::NULL_MASK;
		QT_HANDLE sockHandle = (QT_HANDLE)wParam;
		QT_ASSERTE(pReactor);
		QT_ASSERTE(sockHandle != QT_INVALID_HANDLE);

		QtResult rvError = QT_OK;
		int nErrorCode = WSAGETSELECTERROR(lParam);
		if (nErrorCode != 0)
			rvError = QT_ERROR_NETWORK_SOCKET_ERROR;

		BOOL bHandle = TRUE;
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			maskEvent |= AQtEventHandler::ACCEPT_MASK;
			break;
		case FD_READ:
			maskEvent |= AQtEventHandler::READ_MASK;
			break;
		case FD_WRITE:
			maskEvent |= AQtEventHandler::WRITE_MASK;
			break;
		case FD_CONNECT:
			maskEvent |= AQtEventHandler::CONNECT_MASK;
			break;
		case FD_CLOSE:
		//	QT_ASSERTE(nErrorCode);
			rvError = QT_ERROR_NETWORK_SOCKET_CLOSE;
			break;
		default:
			bHandle = FALSE;
			break;
		}

		if (WSAGETSELECTEVENT(lParam) == FD_CLOSE && nErrorCode == 0) {
			// this is a graceful close, 
			// we must check the remain data in the socket.
			for ( ; ; ) {
				unsigned long dwRemain = 0;
				int nRet = ::ioctlsocket(
					(QT_SOCKET)sockHandle, 
					FIONREAD, 
					&dwRemain);
				if (nRet == 0 && dwRemain > 0) {
					QT_WARNING_TRACE("CQtReactorWin32Message::Win32SocketWndProc,"
						" data remained in the handle wehn closing, recv it."
						" dwRemain=" << dwRemain);
					pReactor->ProcessHandleEvent(
						sockHandle, 
						AQtEventHandler::READ_MASK, 
						QT_OK, 
						FALSE);
				}
				else
					break;
			}
		}
		
		if (nErrorCode != 0 || WSAGETSELECTEVENT(lParam) == FD_CLOSE) {
			QT_INFO_TRACE("CQtReactorWin32Message::Win32SocketWndProc, handle is closed."
				" fd=" << sockHandle <<
				" mask=" << maskEvent <<
				" nErrorCode=" << nErrorCode <<
				" lParam=" << lParam <<
				" rvError=" << rvError);
			QT_SET_BITS(maskEvent, AQtEventHandler::CLOSE_MASK);
		}

		if (bHandle)
			pReactor->ProcessHandleEvent(sockHandle, maskEvent, rvError, FALSE);
		else {
			QT_WARNING_TRACE("CQtReactorWin32Message::Win32SocketWndProc, unknown SELECTEVENT"
				" wParam=" << wParam << " lParam=" << lParam);
		}
		
#ifndef QT_ENABLE_CALENDAR_TIMER
		pReactor->ProcessTimerTick();
#endif // !QT_ENABLE_CALENDAR_TIMER
		return 0;
	}

	case WM_WIN32_SOCKET_NOTIFY:
	{
		CQtReactorWin32Message *pReactor = (CQtReactorWin32Message *)::GetWindowLong(hwnd, 0);
		QT_HANDLE fdOn = (QT_HANDLE)wParam;
		AQtEventHandler::MASK maskEh = (AQtEventHandler::MASK)lParam;
		QT_ASSERTE(pReactor);

		pReactor->ProcessHandleEvent(fdOn, maskEh, QT_OK, TRUE);
#ifndef QT_ENABLE_CALENDAR_TIMER
		pReactor->ProcessTimerTick();
#endif // !QT_ENABLE_CALENDAR_TIMER
		return 0;
	}

	case WM_TIMER:
	{
		CQtReactorWin32Message *pReactor = (CQtReactorWin32Message *)::GetWindowLong(hwnd, 0);
		QT_ASSERTE(pReactor);

#ifdef QT_ENABLE_CALENDAR_TIMER
		pReactor->m_CalendarTimer.TimerTick();
#else
		pReactor->ProcessTimerTick();
#endif // QT_ENABLE_CALENDAR_TIMER
		
		break;
	}
	
	default :
		break;
	}

	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

CQtReactorWin32Message::CQtReactorWin32Message()
	: m_hwndNotify(NULL)
	, m_dwTimerId(0)
#ifdef QT_ENABLE_CALENDAR_TIMER
	, m_CalendarTimer(50, 1000*60*60*2, static_cast<CQtEventQueueUsingMutex*>(this))
#endif // QT_ENABLE_CALENDAR_TIMER
{
}

CQtReactorWin32Message::~CQtReactorWin32Message()
{
	Close();
}

QtResult CQtReactorWin32Message::Open()
{
	QtResult rv = QT_ERROR_UNEXPECTED;
	g_pReactorWin3Instance = (HMODULE)GetTPDllHandle(); //::GetModuleHandle(NULL); 
	QT_INFO_TRACE_THIS("CQtReactorWin32Message::Open() Handle = " << g_pReactorWin3Instance);
	QT_ASSERTE_RETURN(!m_hwndNotify, QT_ERROR_ALREADY_INITIALIZED);
	
#ifdef QT_ENABLE_CALENDAR_TIMER
	m_CalendarTimer.m_Est.Reset2CurrentThreadId();
#endif // QT_ENABLE_CALENDAR_TIMER
	rv = CQtReactorBase::Open();
	if (QT_FAILED(rv))
		goto fail;

	if (g_atomRegisterClass == 0) {
		DWORD dwCount = GetTickCount();
		snprintf(g_szClassName,sizeof(g_szClassName),"%s_%d",WIN32_SOCKET_CLASS_NAME,dwCount);

		WNDCLASS wc;
		wc.style = 0;
		wc.lpfnWndProc = Win32SocketWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(void*);
		wc.hInstance = g_pReactorWin3Instance;
		wc.hIcon = 0;
		wc.hCursor = 0;
		wc.hbrBackground = 0;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = g_szClassName;
		
		if ((g_atomRegisterClass = ::RegisterClass(&wc)) == 0) {
			QT_ERROR_TRACE_THIS("CQtReactorWin32Message::Open, RegisterClass() failed!"
				" err=" << ::GetLastError());
			rv = QT_ERROR_FAILURE;
			goto fail;
		}
	}

	m_hwndNotify = ::CreateWindow(g_szClassName, NULL, WS_OVERLAPPED, 0, 
		0, 0, 0, NULL, NULL, g_pReactorWin3Instance, 0);
	if (!m_hwndNotify) {
		QT_ERROR_TRACE_THIS("CQtReactorWin32Message::Open, CreateWindow() failed!"
			" err=" << ::GetLastError());
		rv = QT_ERROR_FAILURE;
		goto fail;
	}

	::SetLastError(0);
	if (::SetWindowLong(m_hwndNotify, 0, (LONG)this) == 0  && ::GetLastError() != 0) {
		QT_ERROR_TRACE_THIS("CQtReactorWin32Message::Open, SetWindowLong() failed!"
			" err=" << ::GetLastError());
		rv = QT_ERROR_FAILURE;
		goto fail;
	}

	m_dwTimerId = ::SetTimer(m_hwndNotify, m_dwTimerId, 30, NULL);
	if (m_dwTimerId == 0) {
		QT_ERROR_TRACE_THIS("CQtReactorWin32Message::Open, SetTimer() failed!"
			" err=" << ::GetLastError());
		rv = QT_ERROR_FAILURE;
		goto fail;
	}

	CQtStopFlag::m_bStoppedFlag = FALSE;
	return QT_OK;

fail:
	Close();
	QT_ASSERTE(QT_FAILED(rv));
	return rv;
}

QtResult CQtReactorWin32Message::
NotifyHandler(AQtEventHandler *aEh, AQtEventHandler::MASK aMask)
{
	// Different threads may call this function due to EventQueue.
	QT_HANDLE fdNew = QT_INVALID_HANDLE;
	if (aEh) {
		m_Est.EnsureSingleThread();
		fdNew = aEh->GetHandle();
		QT_ASSERTE(fdNew != QT_INVALID_HANDLE);
	}
	
	BOOL bRet = ::PostMessage(m_hwndNotify, WM_WIN32_SOCKET_NOTIFY, 
							  (WPARAM)fdNew, (LPARAM)aMask);
	if (!bRet) {
		QT_ERROR_TRACE_THIS("CQtReactorWin32Message::NotifyHandler, PostMessage() failed!"
			" err=" << ::GetLastError());
		return QT_ERROR_UNEXPECTED;
	}
	else
	{
		return QT_OK;
	}
}

QtResult CQtReactorWin32Message::RunEventLoop()
{
	QT_INFO_TRACE_THIS("CQtReactorWin32Message::RunEventLoop");
	m_Est.EnsureSingleThread();

	MSG msg;
	while (!CQtStopFlag::m_bStoppedFlag && ::GetMessage(&msg, NULL, 0, 0)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return QT_OK;
}

// this function can be invoked in the different thread.
QtResult CQtReactorWin32Message::StopEventLoop()
{
	QT_INFO_TRACE_THIS("CQtReactorWin32Message::StopEventLoop");

//	::PostQuitMessage(0);
	CQtStopFlag::m_bStoppedFlag = TRUE;
	
	CQtEventQueueUsingMutex::Stop();
	return QT_OK;
}

QtResult CQtReactorWin32Message::Close()
{
	if (m_hwndNotify) {
		::DestroyWindow(m_hwndNotify);
		m_hwndNotify = NULL;
	}
	return CQtReactorBase::Close();
}

QtResult CQtReactorWin32Message::
OnHandleRegister(QT_HANDLE aFd, AQtEventHandler::MASK aMask, AQtEventHandler *aEh)
{
	m_Est.EnsureSingleThread();
	QT_ERROR_TRACE_THIS("CQtReactorWin32Message::OnHandleRegister,"
		" aFd=" << aFd <<
		" aMask=" << aMask);
	return QT_ERROR_UNEXPECTED;
}

void CQtReactorWin32Message::OnHandleRemoved(QT_HANDLE aFd)
{
	m_Est.EnsureSingleThread();
	QT_ERROR_TRACE_THIS("CQtReactorWin32Message::OnHandleRemoved, aFd=" << aFd);
}

#ifdef QT_ENABLE_CALENDAR_TIMER
QtResult CQtReactorWin32Message::
ScheduleTimer(IQtTimerHandler *aTh, LPVOID aArg, 
			  const CQtTimeValue &aInterval, DWORD aCount)
{
	return m_CalendarTimer.ScheduleTimer(aTh, aArg, aInterval, aCount);
}

QtResult CQtReactorWin32Message::CancelTimer(IQtTimerHandler *aTh)
{
	return m_CalendarTimer.CancelTimer(aTh);
}
#endif // QT_ENABLE_CALENDAR_TIMER


//////////////////////////////////////////////////////////////////////
// class CQtReactorWin32AsyncSelect
//////////////////////////////////////////////////////////////////////

CQtReactorWin32AsyncSelect::CQtReactorWin32AsyncSelect()
{
}

CQtReactorWin32AsyncSelect::~CQtReactorWin32AsyncSelect()
{
}

QtResult CQtReactorWin32AsyncSelect::
OnHandleRegister(QT_HANDLE aFd, AQtEventHandler::MASK aMask, AQtEventHandler *aEh)
{
	return DoAsyncSelect_i(aFd, AQtEventHandler::ALL_EVENTS_MASK);
}

void CQtReactorWin32AsyncSelect::OnHandleRemoved(QT_HANDLE aFd)
{
	DoAsyncSelect_i(aFd, AQtEventHandler::NULL_MASK);
}

QtResult CQtReactorWin32AsyncSelect::
DoAsyncSelect_i(QT_HANDLE aFd, AQtEventHandler::MASK aMask)
{
	long lEvent = 0;
	if (aMask & AQtEventHandler::CONNECT_MASK)
		lEvent |= FD_CONNECT;
	if (aMask & AQtEventHandler::ACCEPT_MASK)
		lEvent |= FD_ACCEPT;
	if (aMask & AQtEventHandler::READ_MASK)
		lEvent |= FD_READ;
	if (aMask & AQtEventHandler::WRITE_MASK)
		lEvent |= FD_WRITE;

	if (lEvent != 0)
		lEvent |= FD_CLOSE;
	if (::WSAAsyncSelect((SOCKET)aFd, m_hwndNotify, WM_WIN32_SOCKET_SELECT, lEvent) != 0) {
		QT_ERROR_TRACE_THIS("CQtReactorWin32AsyncSelect::DoAsyncSelect_i, WSAAsyncSelect() failed!"
			" aFd=" << aFd <<
			" err=" << ::WSAGetLastError());
		return QT_ERROR_UNEXPECTED;
	}
	else
		return QT_OK;
}
