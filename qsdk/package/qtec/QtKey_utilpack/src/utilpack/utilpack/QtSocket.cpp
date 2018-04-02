
#include "QtBase.h"
#include "QtSocket.h"
#include "QtUtilClasses.h"

#if defined (QT_SUPPORT_QOS) && defined (QT_WIN32)
  #include <Qos.h>
#endif // QT_SUPPORT_QOS && QT_WIN32

//////////////////////////////////////////////////////////////////////
// class CQtIPCBase
//////////////////////////////////////////////////////////////////////

int CQtIPCBase::Enable(int aValue) const 
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	switch(aValue) {
	case NON_BLOCK: 
		{
#ifdef QT_WIN32
		u_long nonblock = 1;
		int nRet = ::ioctlsocket((QT_SOCKET)m_Handle, FIONBIO, &nonblock);
		if (nRet == SOCKET_ERROR) {
			errno = ::WSAGetLastError();
			nRet = -1;
		}
		return nRet;

#else // !QT_WIN32
		int nVal = ::fcntl(m_Handle, F_GETFL, 0);
		if (nVal == -1)
			return -1;
		nVal |= O_NONBLOCK;
		if (::fcntl(m_Handle, F_SETFL, nVal) == -1)
			return -1;
		return 0;
#endif // QT_WIN32
		}

	default:
		QT_ERROR_TRACE("CQtIPCBase::Enable, aValue=" << aValue);
		return -1;
	}
}

int CQtIPCBase::Disable(int aValue) const 
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);
	switch(aValue) {
	case NON_BLOCK:
		{
#ifdef QT_WIN32
		u_long nonblock = 0;
		int nRet = ::ioctlsocket((QT_SOCKET)m_Handle, FIONBIO, &nonblock);
		if (nRet == SOCKET_ERROR) {
			errno = ::WSAGetLastError();
			nRet = -1;
		}
		return nRet;

#else // !QT_WIN32
		int nVal = ::fcntl(m_Handle, F_GETFL, 0);
		if (nVal == -1)
			return -1;
		nVal &= ~O_NONBLOCK;
		if (::fcntl(m_Handle, F_SETFL, nVal) == -1)
			return -1;
		return 0;
#endif // QT_WIN32
		}

	default:
		QT_ERROR_TRACE("CQtIPCBase::Disable, aValue=" << aValue);
		return -1;
	}
}

int CQtIPCBase::Control(int aQtd, void *aArg) const
{
	int nRet;
#ifdef QT_WIN32
	nRet = ::ioctlsocket((QT_SOCKET)m_Handle, aQtd, static_cast<unsigned long *>(aArg));
	if (nRet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nRet = -1;
	}
#elif defined (QT_MACOS)
	nRet = ::ioctl(m_Handle, aQtd, (char *)aArg);
#else
	nRet = ::ioctl(m_Handle, aQtd, aArg);
#endif // QT_WIN32
	return nRet;
}


//////////////////////////////////////////////////////////////////////
// class CQtSocketBase
//////////////////////////////////////////////////////////////////////

int CQtSocketBase::Open(int aFamily, int aType, int aProtocol, BOOL aReuseAddr)
{
	int nRet = -1;
	Close();
	
#if defined (QT_SUPPORT_QOS) && defined (QT_WIN32)
	DWORD bufferSize = 0;
    DWORD numProtocols, i;
    LPWSAPROTOCOL_INFO installedProtocols, qosProtocol; 
    
    //
    // Call WSAEnumProtocols to determine buffer size required
    //
    numProtocols = WSAEnumProtocols(NULL, NULL, &bufferSize);
    if((numProtocols != SOCKET_ERROR) && (WSAGetLastError() != WSAENOBUFS)){
        QT_ERROR_TRACE_THIS("CQtSocketBase::Open, WSAEnumProtocols(1) failed!"
			" err=" << WSAGetLastError()); 
        return -1;
    } else {
        //
        // Allocate a buffer to hold the list of protocol info structures
        //
        installedProtocols = (LPWSAPROTOCOL_INFO)malloc(bufferSize);
   
        //
        // Enumerate the protocols, find the QoS enabled one
        //
        numProtocols = WSAEnumProtocols(NULL,
                                        installedProtocols,
                                        &bufferSize);
        if(numProtocols == SOCKET_ERROR){
            QT_ERROR_TRACE_THIS("CQtSocketBase::Open, WSAEnumProtocols(2) failed!"
				" err=" << WSAGetLastError()); 
	        return -1;
        } 
        else 
        {
            qosProtocol = installedProtocols;

            for( i=0 ; i < numProtocols ; qosProtocol++, i++)
            {
                if  ((qosProtocol->dwServiceFlags1 & XP1_QOS_SUPPORTED) &&
                    (qosProtocol->iSocketType    == aType) &&
                    (qosProtocol->iAddressFamily == AF_INET))
                {
                    break;
                }
                     
            }
        }
    
        //
        // Now open the socket.
        //
        m_Handle = (QT_HANDLE)WSASocket(0, 
                       aType, 
                       0, 
                       qosProtocol,          // Use the QoS SP we found
                       0, 
                       WSA_FLAG_OVERLAPPED); // *MUST* be overlapped!
    
        //
        // De-allocate protocol info buffer
        //
        free(installedProtocols);

		if (m_Handle != QT_INVALID_HANDLE)
			nRet = 0;
	}

#else
	m_Handle = (QT_HANDLE)::socket(aFamily, aType, aProtocol);
	if (m_Handle != QT_INVALID_HANDLE) {
		nRet = 0;
		if (aFamily != PF_UNIX && aReuseAddr) {
			int nReuse = 1;
			nRet = SetOption(SOL_SOCKET, SO_REUSEADDR, &nReuse, sizeof(nReuse));
		}
	}
#endif // QT_SUPPORT_QOS && QT_WIN32

	if (nRet == -1) {
		CQtErrnoGuard theGuard;
		Close();
	}
	return nRet;
}

int CQtSocketBase::GetRemoteAddr(CQtInetAddr &aAddr) const
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);

	int nSize = (int)aAddr.GetSize();
	int nGet = ::getpeername((QT_SOCKET)m_Handle,
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
	if (nGet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nGet = -1;
	}
#endif // QT_WIN32

	return nGet;
}

int CQtSocketBase::GetLocalAddr(CQtInetAddr &aAddr) const
{
//	QT_ASSERTE(m_Handle != QT_INVALID_HANDLE);

	int nSize = (int)aAddr.GetSize();
	int nGet = ::getsockname((QT_SOCKET)m_Handle,
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
	if (nGet == SOCKET_ERROR) {
		errno = ::WSAGetLastError();
		nGet = -1;
	}
#endif // QT_WIN32

	return nGet;
}

int CQtSocketBase::Close()
{
	int nRet = 0;
	if (m_Handle != QT_INVALID_HANDLE) {
#ifdef QT_WIN32
		nRet = ::closesocket((QT_SOCKET)m_Handle);
		if (nRet == SOCKET_ERROR) {
			errno = ::WSAGetLastError();
			nRet = -1;
		}
#else
		nRet = ::close((QT_SOCKET)m_Handle);
#endif
		m_Handle = QT_INVALID_HANDLE;
	}
	return nRet;
}

//////////////////////////////////////////////////////////////////////
// class CQtSocketTcp
//////////////////////////////////////////////////////////////////////

int CQtSocketTcp::Open(BOOL aReuseAddr, const CQtInetAddr &aLocal)
{
	if (CQtSocketBase::Open(PF_INET, SOCK_STREAM, 0, aReuseAddr) == -1)
		return -1;

	if (::bind((QT_SOCKET)m_Handle, 
						  reinterpret_cast<const sockaddr *>(aLocal.GetPtr()),
#ifdef QT_WIN32
						  aLocal.GetSize()
#else // !QT_WIN32
#ifdef QT_MACOS
					#ifdef __i386__
						static_cast<socklen_t>(aLocal.GetSize())
					#else
						aLocal.GetSize()
					#endif		
#else
						  static_cast<socklen_t>(aLocal.GetSize())
#endif						  
#endif // QT_WIN32
						  ) == -1)
	{
#ifdef QT_WIN32
		errno = ::WSAGetLastError();
#endif // QT_WIN32
		CQtErrnoGuard theGuard;
		Close();
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////
// class CQtSocketUdp
//////////////////////////////////////////////////////////////////////

int CQtSocketUdp::Open(const CQtInetAddr &aLocal)
{
	if (CQtSocketBase::Open(PF_INET, SOCK_DGRAM, 0, FALSE) == -1)
		return -1;

	if (::bind((QT_SOCKET)m_Handle, 
						  reinterpret_cast<const sockaddr *>(aLocal.GetPtr()),
#ifdef QT_WIN32
						  aLocal.GetSize()
#else // !QT_WIN32
#ifdef QT_MACOS
					#ifdef __i386__
						static_cast<socklen_t>(aLocal.GetSize())
					#else
						aLocal.GetSize()
					#endif		
#else
						  static_cast<socklen_t>(aLocal.GetSize())
#endif						  
#endif // QT_WIN32
						  ) == -1)
	{
#ifdef QT_WIN32
		errno = ::WSAGetLastError();
#endif // QT_WIN32
		CQtErrnoGuard theGuard;
		Close();
		return -1;
	}
	return 0;
}
