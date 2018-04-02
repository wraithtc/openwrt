
#include "QtBase.h"
#include "QtPipe.h"
#include "QtSocket.h"

CQtPipe::CQtPipe()
{
	m_Handles[0] = QT_INVALID_HANDLE;
	m_Handles[1] = QT_INVALID_HANDLE;
}

CQtPipe::~CQtPipe()
{
	Close();
}

QtResult CQtPipe::Open(DWORD aSize)
{
	QT_ASSERTE(m_Handles[0] == QT_INVALID_HANDLE && m_Handles[1] == QT_INVALID_HANDLE);

	int nRet = 0;
#if defined (QT_UNIX) || defined (QT_MACOS)
	nRet = ::socketpair(AF_UNIX, SOCK_STREAM, 0, m_Handles);
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtPipe::Open, socketpair() failed! err=" << errno);
		return nRet;
	}
#else //QT_WIN32
	CQtSocketTcp socketTcp;
	CQtInetAddr addrListen("127.0.0.1:0");
	CQtInetAddr addrConnect;
	CQtInetAddr addrPeer;
	int nAddrLen = addrPeer.GetSize();

	if (socketTcp.Open() == -1) {
		QT_ERROR_TRACE_THIS("CQtPipe::Open, open() failed! err=" << ::WSAGetLastError());
		goto fail;
	}

	nRet = ::bind(
		(QT_SOCKET)socketTcp.GetHandle(), 
		reinterpret_cast<const struct sockaddr *>(addrListen.GetPtr()), 
		addrListen.GetSize());
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtPipe::Open, bind() failed! err=" << ::WSAGetLastError());
		goto fail;
	}

	if (::listen((QT_SOCKET)socketTcp.GetHandle(), 5) == -1) {
		QT_ERROR_TRACE_THIS("CQtPipe::Open, listen() failed! err=" << ::WSAGetLastError());
		goto fail;
	}

	nRet = socketTcp.GetLocalAddr(addrConnect);
	QT_ASSERTE(nRet == 0);

	m_Handles[1] = (QT_HANDLE)::socket(AF_INET, SOCK_STREAM, 0);
	QT_ASSERTE(m_Handles[1] != QT_INVALID_HANDLE);

	nRet = ::connect(
		(QT_SOCKET)m_Handles[1], 
		reinterpret_cast<const struct sockaddr *>(addrConnect.GetPtr()), 
		addrConnect.GetSize());
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtPipe::Open, connect() failed! err=" << ::WSAGetLastError());
		goto fail;
	}
	
	m_Handles[0] = (QT_HANDLE)::accept(
		(QT_SOCKET)socketTcp.GetHandle(), 
		reinterpret_cast<struct sockaddr *>(const_cast<struct sockaddr_in *>(addrPeer.GetPtr())), 
		&nAddrLen);
	if (m_Handles[0] == QT_INVALID_HANDLE) {
		QT_ERROR_TRACE_THIS("CQtPipe::Open, accept() failed! err=" << ::WSAGetLastError());
		goto fail;
	}
#endif // defined (QT_UNIX) || defined (QT_MACOS)

	if (aSize > QT_DEFAULT_MAX_SOCKET_BUFSIZ)
		aSize = QT_DEFAULT_MAX_SOCKET_BUFSIZ;
	nRet = ::setsockopt((QT_SOCKET)m_Handles[0], SOL_SOCKET, SO_RCVBUF, 
#ifdef QT_WIN32
		reinterpret_cast<const char*>(&aSize), 
#else // !QT_WIN32
		&aSize,
#endif // QT_WIN32
		sizeof(aSize));
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtPipe::Open, setsockopt(0) failde! err=" << errno);
		goto fail;
	}
	nRet = ::setsockopt((QT_SOCKET)m_Handles[1], SOL_SOCKET, SO_SNDBUF, 
#ifdef QT_WIN32
		reinterpret_cast<const char*>(&aSize), 
#else // !QT_WIN32
		&aSize,
#endif // QT_WIN32
		sizeof(aSize));
	if (nRet == -1) {
		QT_ERROR_TRACE_THIS("CQtPipe::Open, setsockopt(1) failde! err=" << errno);
		goto fail;
	}
	return QT_OK;

fail:
	Close();
	return QT_ERROR_NOT_AVAILABLE;
}

QtResult CQtPipe::Close()
{
	int nRet = 0;
	if (m_Handles[0] != QT_INVALID_HANDLE) {
#ifdef QT_UNIX
		nRet = ::close(m_Handles[0]);
#else
	#ifdef QT_MACOS
	#ifndef MachOSupport
		nRet = ::closesocket((QT_SOCKET)m_Handles[0],0);
	#else	
		nRet = ::close(m_Handles[0]);
	#endif	//MachOSupport	
	#else	
		nRet = ::closesocket((QT_SOCKET)m_Handles[0]);
	#endif	
#endif // QT_UNIX
		m_Handles[0] = QT_INVALID_HANDLE;
	}
	if (m_Handles[1] != QT_INVALID_HANDLE) {
#ifdef QT_UNIX
		nRet |= ::close(m_Handles[1]);
#else
	#ifdef QT_MACOS
	#ifndef MachOSupport
		nRet |= ::closesocket((QT_SOCKET)m_Handles[1],0);
	#else	
		nRet |= ::close(m_Handles[1]);
	#endif	//MachOSupport	
	#else	
		nRet |= ::closesocket((QT_SOCKET)m_Handles[1]);
	#endif	
#endif // QT_UNIX
		m_Handles[1] = QT_INVALID_HANDLE;
	}
	return nRet == 0 ? QT_OK : QT_ERROR_NETWORK_SOCKET_ERROR;
}

QT_HANDLE CQtPipe::GetReadHandle() const
{
	return m_Handles[0];
}

QT_HANDLE CQtPipe::GetWriteHandle() const
{
	return m_Handles[1];
}
