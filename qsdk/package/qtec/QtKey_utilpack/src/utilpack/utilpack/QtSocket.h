/*------------------------------------------------------*/
/* Wrapper class for socket(TCP, UDP)                   */
/*                                                      */
/* QtSocket.h                                           */
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

#ifndef QTSOCKET_H
#define QTSOCKET_H

#include "QtDefines.h"
#include "QtError.h"
#include "QtInetAddr.h"

class QT_OS_EXPORT CQtIPCBase
{
public:
	enum { NON_BLOCK = 0 };

	CQtIPCBase() : m_Handle(QT_INVALID_HANDLE) { }

	QT_HANDLE GetHandle() const;
	void SetHandle(QT_HANDLE aNew);

	int Enable(int aValue) const ;
	int Disable(int aValue) const ;
	int Control(int aQtd, void *aArg) const;
	
protected:
	QT_HANDLE m_Handle;
};


class QT_OS_EXPORT CQtSocketBase : public CQtIPCBase
{
protected:
	CQtSocketBase();
	~CQtSocketBase();

public:
	/// Wrapper around the BSD-style <socket> system call (no QoS).
	int Open(int aFamily, int aType, int aProtocol, BOOL aReuseAddr);

	/// Close down the socket handle.
	int Close();

	/// Wrapper around the <setsockopt> system call.
	int SetOption(int aLevel, int aOption, const void *aOptval, int aOptlen) const ;

	/// Wrapper around the <getsockopt> system call.
	int GetOption(int aLevel, int aOption, void *aOptval, int *aOptlen) const ;

	/// Return the address of the remotely connected peer (if there is
	/// one), in the referenced <aAddr>.
	int GetRemoteAddr(CQtInetAddr &aAddr) const;

	/// Return the local endpoint address in the referenced <aAddr>.
	int GetLocalAddr(CQtInetAddr &aAddr) const;

	/// Recv an <aLen> byte buffer from the connected socket.
	int Recv(char *aBuf, DWORD aLen, int aFlag = 0) const ;

	/// Recv an <aIov> of size <aCount> from the connected socket.
	int RecvV(iovec aIov[], DWORD aCount) const ;

	/// Send an <aLen> byte buffer to the connected socket.
	int Send(const char *aBuf, DWORD aLen, int aFlag = 0) const ;

	/// Send an <aIov> of size <aCount> from the connected socket.
	int SendV(const iovec aIov[], DWORD aCount) const ;
};


class QT_OS_EXPORT CQtSocketTcp : public CQtSocketBase
{
public:
	CQtSocketTcp();
	~CQtSocketTcp();

	int Open(BOOL aReuseAddr = FALSE);
	int Open(BOOL aReuseAddr, const CQtInetAddr &aLocal);
	int Close(QtResult aReason = QT_OK);
	int CloseWriter();
	int CloseReader();
};

class QT_OS_EXPORT CQtSocketUdp : public CQtSocketBase
{
public:
	CQtSocketUdp();
	~CQtSocketUdp();

	int Open(const CQtInetAddr &aLocal);

	int RecvFrom(char *aBuf, 
				 DWORD aLen, 
				 CQtInetAddr &aAddr, 
				 int aFlag = 0) const ;

	int SendTo(const char *aBuf, 
			   DWORD aLen, 
			   const CQtInetAddr &aAddr, 
			   int aFlag = 0) const ;

	int SendVTo(const iovec aIov[], 
				DWORD aCount,
				const CQtInetAddr &aAddr) const ;
};


// inline functions
inline QT_HANDLE CQtIPCBase::GetHandle() const 
{
	return m_Handle;
}

inline void CQtIPCBase::SetHandle(QT_HANDLE aNew)
{
	QT_ASSERTE(m_Handle == QT_INVALID_HANDLE || aNew == QT_INVALID_HANDLE);
	m_Handle = aNew;
}

inline CQtSocketBase::CQtSocketBase()
{
}

inline CQtSocketBase::~CQtSocketBase()
{
	Close();
}

inline int CQtSocketBase::SetOption(int aLevel, int aOption, const void *aOptval, int aOptlen) const 
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	int nRet = ::setsockopt((QT_SOCKET)m_Handle, aLevel, aOption, 
#ifdef QT_WIN32
		static_cast<const char*>(aOptval), 
#else // !QT_WIN32
		aOptval,
#endif // QT_WIN32
		aOptlen);

#ifdef QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // QT_WIN32
	return nRet;
}

inline int CQtSocketBase::GetOption(int aLevel, int aOption, void *aOptval, int *aOptlen) const 
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	int nRet = ::getsockopt((QT_SOCKET)m_Handle, aLevel, aOption, 
#ifdef QT_WIN32
		static_cast<char*>(aOptval), 
		aOptlen
#else // !QT_WIN32
		aOptval,
#ifdef QT_MACOS
//	#ifdef __i386__
		reinterpret_cast<socklen_t*>(aOptlen)
//	#else
//		aOptlen
//	#endif		
#else		
		reinterpret_cast<socklen_t*>(aOptlen)
#endif		
#endif // QT_WIN32
		);

#ifdef QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // QT_WIN32
	return nRet;
}

inline int CQtSocketBase::Recv(char *aBuf, DWORD aLen, int aFlag) const
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	QT_ASSERTE(aBuf);
	
	int nRet = ::recv((QT_SOCKET)m_Handle, aBuf, aLen, aFlag);
#ifndef QT_WIN32
  #ifdef QT_MACOS
  #ifndef MachOSupport
	if (nRet == -1 && errno == 35)
		CFM_seterrno(35);
  #else
	if (nRet == -1 && errno == EAGAIN)
		errno = EWOULDBLOCK;
  #endif	//MachOSupport	
  #else
	if (nRet == -1 && errno == EAGAIN)
		errno = EWOULDBLOCK;
  #endif
#else // !QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // QT_WIN32

	return nRet;
}

inline int CQtSocketBase::RecvV(iovec aIov[], DWORD aCount) const 
{
	int nRet;
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	QT_ASSERTE(aIov);
	
#ifdef QT_WIN32
	DWORD dwBytesReceived = 0;
	DWORD dwFlags = 0;
	nRet = ::WSARecv((QT_SOCKET)m_Handle,
                      (WSABUF *)aIov,
                      aCount,
                      &dwBytesReceived,
                      &dwFlags,
                      0,
                      0);
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
	else {
		nRet = (int)dwBytesReceived;
	}
#else // !QT_WIN32
	nRet = ::readv(m_Handle, aIov, aCount);
#endif // QT_WIN32
	return nRet;
}

inline int CQtSocketBase::Send (const char *aBuf, DWORD aLen, int aFlag) const 
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	QT_ASSERTE(aBuf);

	int nRet = ::send((QT_SOCKET)m_Handle, aBuf, aLen, aFlag);
#ifndef QT_WIN32
  #ifdef QT_MACOS
  #ifndef MachOSupport
	if (nRet == -1 && errno == 35)
		CFM_seterrno(35);
  #else
	if (nRet == -1 && errno == EAGAIN)
		errno = EWOULDBLOCK;
  #endif	//MachOSupport
  #else
	if (nRet == -1 && errno == EAGAIN)
		errno = EWOULDBLOCK;
  #endif		
#else // !QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // QT_WIN32
	return nRet;
}

inline int CQtSocketBase::SendV(const iovec aIov[], DWORD aCount) const 
{
	int nRet;
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	QT_ASSERTE(aIov);
	
#ifdef QT_WIN32
	DWORD dwBytesSend = 0;
	nRet = ::WSASend((QT_SOCKET)m_Handle,
                      (WSABUF *)aIov,
                      aCount,
                      &dwBytesSend,
                      0,
                      0,
                      0);
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
	else {
		nRet = (int)dwBytesSend;
	}
#else // !QT_WIN32
	nRet = ::writev(m_Handle, aIov, aCount);
#endif // QT_WIN32
	return nRet;
}

inline CQtSocketTcp::CQtSocketTcp()
{
}

inline CQtSocketTcp::~CQtSocketTcp()
{
	Close();
}

inline int CQtSocketTcp::Open(BOOL aReuseAddr)
{
	return CQtSocketBase::Open(PF_INET, SOCK_STREAM, 0, aReuseAddr);
}

inline int CQtSocketTcp::Close(QtResult aReason)
{
#ifdef QT_WIN32
	// We need the following call to make things work correctly on
	// Win32, which requires use to do a <CloseWriter> before doing the
	// close in order to avoid losing data.  Note that we don't need to
	// do this on UNIX since it doesn't have this "feature".  Moreover,
	// this will cause subtle problems on UNIX due to the way that
	// fork() works.
	if (m_Handle != QT_INVALID_HANDLE && QT_SUCCEEDED(aReason))
		CloseWriter();
#endif // QT_WIN32

	return CQtSocketBase::Close();
}

inline int CQtSocketTcp::CloseWriter()
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	int nRet = ::shutdown((QT_SOCKET)m_Handle, QT_SD_SEND);

#ifdef QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // QT_WIN32
	return nRet;
}

inline int CQtSocketTcp::CloseReader()
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	int nRet = ::shutdown((QT_SOCKET)m_Handle, QT_SD_RECEIVE);

#ifdef QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // QT_WIN32
	return nRet;
}

inline CQtSocketUdp::CQtSocketUdp()
{
}

inline CQtSocketUdp::~CQtSocketUdp()
{
	Close();
}

inline int CQtSocketUdp::
RecvFrom(char *aBuf, DWORD aLen, CQtInetAddr &aAddr, int aFlag) const 
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);

	int nSize = (int)aAddr.GetSize();
	int nRet = ::recvfrom((QT_SOCKET)m_Handle,
						  aBuf,
						  aLen,
						  aFlag,
						  reinterpret_cast<sockaddr *>(const_cast<sockaddr_in *>(aAddr.GetPtr())),
#ifdef QT_WIN32
						  &nSize
#else // !QT_WIN32
#ifdef QT_MACOS
			//		#ifdef __i386__
						reinterpret_cast<socklen_t*>(&nSize)
			//		#else
			//			&nSize
			//		#endif		
#else
						  reinterpret_cast<socklen_t*>(&nSize)
#endif						  
#endif // QT_WIN32
						   );

#ifdef QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // QT_WIN32

	return nRet;
}

inline int CQtSocketUdp::
SendTo(const char *aBuf, DWORD aLen, const CQtInetAddr &aAddr, int aFlag) const 
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);

	int nRet = ::sendto((QT_SOCKET)m_Handle,
						  aBuf,
						  aLen,
						  aFlag,
						  reinterpret_cast<const sockaddr *>(aAddr.GetPtr()),
#ifdef QT_WIN32
						  aAddr.GetSize()
#else // !QT_WIN32
#ifdef QT_MACOS
					#ifdef __i386__
						static_cast<socklen_t>(aAddr.GetSize())
					#else
						aAddr.GetSize()
					#endif		
#else
						  static_cast<socklen_t>(aAddr.GetSize())
#endif						  
#endif // QT_WIN32
						  );

#ifdef QT_WIN32
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#endif // QT_WIN32
	
	return nRet;
}

inline int CQtSocketUdp::
SendVTo(const iovec aIov[], DWORD aCount, const CQtInetAddr &aAddr) const 
{
	int nRet;
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	QT_ASSERTE(aIov);
	
#ifdef QT_WIN32
	DWORD dwBytesSend = 0;
	nRet = ::WSASendTo((QT_SOCKET)m_Handle,
                      (WSABUF *)aIov,
                      aCount,
                      &dwBytesSend,
                      0,
                      reinterpret_cast<const sockaddr *>(aAddr.GetPtr()),
                      aAddr.GetSize(),
					  NULL,
					  NULL);
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
	else {
		nRet = (int)dwBytesSend;
	}
#else // !QT_WIN32
	msghdr send_msg;
	send_msg.msg_iov = (iovec *)aIov;
	send_msg.msg_iovlen = aCount;
#ifdef QT_MACOS
	send_msg.msg_name = (char*)aAddr.GetPtr();
#else	
	send_msg.msg_name = (struct sockaddr *)aAddr.GetPtr();
#endif	
	send_msg.msg_namelen = aAddr.GetSize();
	send_msg.msg_control = 0;
	send_msg.msg_controllen = 0;
	send_msg.msg_flags = 0;
	nRet = ::sendmsg(m_Handle, &send_msg, 0);
#endif // QT_WIN32
	return nRet;
}


#endif // !QTSOCKET_H
