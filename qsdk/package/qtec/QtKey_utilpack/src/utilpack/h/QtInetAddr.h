/*------------------------------------------------------*/
/* Internet address                                     */
/*                                                      */
/* QtInetAddr.h                                         */
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

#ifndef QTINETADDR_H
#define QTINETADDR_H

#include "QtDefines.h"
#include "QtStdCpp.h"

#ifndef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
  #define QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME 1
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME

/// The concept of <CQtInetAddr> is mainly copyed by <ACE_INET_Addr>
/// http://www.cs.wustl.edu/~schmidt/ACE.html
class QT_OS_EXPORT CQtInetAddr  
{
public:
	CQtInetAddr();
	
	/// Creates an <CQtInetAddr> from a <aPort> and the remote
	/// <aHostName>. The port number is assumed to be in host byte order.
	CQtInetAddr(LPCSTR aHostName, WORD aPort);

	CQtInetAddr(const CQtInetAddr &aRight)
	{
		if(this == &aRight) //if it is same, do nothing, otherwise the memory should be destroyed RT#HD0000002492809
			return;
		memcpy(&m_SockAddr, &aRight.m_SockAddr, sizeof(m_SockAddr));
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		m_strHostName = aRight.m_strHostName.c_str();
#endif		
		m_strUserData = aRight.m_strUserData;
		m_bLock = aRight.m_bLock;
		m_bIsResolved = aRight.m_bIsResolved;
	}

	CQtInetAddr & operator=(const CQtInetAddr &aRight)
	{
		if(this == &aRight) //if it is same, do nothing, otherwise the memory should be destroyed RT#HD0000002492809
			return *this;
		memcpy(&m_SockAddr, &aRight.m_SockAddr, sizeof(m_SockAddr));
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		m_strHostName = aRight.m_strHostName.c_str();
#endif		
		m_strUserData = aRight.m_strUserData;
		m_bLock = aRight.m_bLock;
		m_bIsResolved = aRight.m_bIsResolved;
		return *this;
	}
	/**
	* Initializes an <CQtInetAddr> from the <aIpAddrAndPort>, which can be
	* "ip-number:port-number" (e.g., "tango.cs.wustl.edu:1234" or
	* "128.252.166.57:1234").  If there is no ':' in the <address> it
	* is assumed to be a port number, with the IP address being
	* INADDR_ANY.
	*/
	CQtInetAddr(LPCSTR aIpAddrAndPort);

	QtResult Set(LPCSTR aHostName, WORD aPort);
	QtResult Set(LPCSTR aIpAddrAndPort);

	QtResult SetIpAddrByString(LPCSTR aIpAddr);
	QtResult SetIpAddrBy4Bytes(DWORD aIpAddr, BOOL aIsNetworkOrder = TRUE);
	QtResult SetPort(WORD aPort);

	/// Compare two addresses for equality.  The addresses are considered
	/// equal if they contain the same IP address and port number.
	bool operator == (const CQtInetAddr &aRight) const;

	/**
	 * Returns true if <this> is less than <aRight>.  In this context,
	 * "less than" is defined in terms of IP address and TCP port
	 * number.  This operator makes it possible to use <ACE_INET_Addr>s
	 * in STL maps.
	 */
	bool operator < (const CQtInetAddr &aRight) const;

	CQtString GetIpDisplayName() const;

	WORD GetPort() const { return ntohs(m_SockAddr.sin_port); }

	DWORD GetIpAddrIn4Bytes() const { return m_SockAddr.sin_addr.s_addr; }

	DWORD GetSize() const { return sizeof (sockaddr_in); }

	DWORD GetType() const { return m_SockAddr.sin_family; }

	const sockaddr_in* GetPtr() const 
	{ 
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		QT_ASSERTE(IsResolved());
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
		return &m_SockAddr; 
	}

	static BOOL IpAddrStringTo4Bytes(LPCSTR aIpStr, DWORD &aIpDword);
	static CQtString IpAddr4BytesToString(DWORD aIpDword);

#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
	BOOL IsResolved() const
	{
// 		return m_strHostName.empty() ? TRUE : FALSE;
		return m_bIsResolved;
	}

	CQtString GetHostName() const { return m_strHostName; }

	QtResult TryResolve();
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME

	//add by Nick
	void SetUserData(const CQtString strUserData) { m_strUserData = strUserData; }
	CQtString GetUserData() const { return m_strUserData;}

	void Lock(){ m_bLock = TRUE; }
	void UnLock(){ m_bLock = FALSE; }
	BOOL IsLock(){ return m_bLock; }

public:
	static CQtInetAddr s_InetAddrAny;

private:
	sockaddr_in m_SockAddr;
	
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
	// m_strHostName is empty that indicates resovled successfully,
	// otherwise it needs resolving.
	CQtString m_strHostName;
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME

	//add by Nick
	CQtString	m_strUserData;
	BOOL		m_bLock;
	//add it to idendtifing the resolve status, then we can keep host name
	BOOL		m_bIsResolved;
};


// inline functions
inline CQtInetAddr::CQtInetAddr():m_bLock(FALSE), m_bIsResolved(FALSE)
{
	Set(NULL, 0);
}

inline CQtInetAddr::CQtInetAddr(LPCSTR aHostName, WORD aPort):m_bLock(FALSE), m_bIsResolved(FALSE)
{
	Set(aHostName, aPort);
}

inline CQtInetAddr::CQtInetAddr(LPCSTR aIpAddrAndPort):m_bLock(FALSE), m_bIsResolved(FALSE)
{
	Set(aIpAddrAndPort);
}

inline QtResult CQtInetAddr::SetPort(WORD aPort)
{
	m_SockAddr.sin_port = htons(aPort);
	return QT_OK;
}

inline CQtString CQtInetAddr::GetIpDisplayName() const
{
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
	if (!IsResolved())
		return m_strHostName;
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME

	return IpAddr4BytesToString(m_SockAddr.sin_addr.s_addr);
}

inline bool CQtInetAddr::operator == (const CQtInetAddr &aRight) const
{
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
	QT_ASSERTE(IsResolved());
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
	
	// don't compare m_SockAddr.sin_zero due to getpeername() or getsockname() 
	// will fill it with non-zero value.
	return (::memcmp(
		&m_SockAddr, 
		&aRight.m_SockAddr, 
		sizeof(m_SockAddr) - sizeof(m_SockAddr.sin_zero)) == 0);
}

inline bool CQtInetAddr::operator < (const CQtInetAddr &aRight) const
{
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
	QT_ASSERTE(IsResolved());
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME

	return m_SockAddr.sin_addr.s_addr < aRight.m_SockAddr.sin_addr.s_addr
		|| (m_SockAddr.sin_addr.s_addr == aRight.m_SockAddr.sin_addr.s_addr 
		&& m_SockAddr.sin_port < aRight.m_SockAddr.sin_port);
}

inline QtResult CQtInetAddr::SetIpAddrBy4Bytes(DWORD aIpAddr, BOOL aIsNetworkOrder)
{
#ifdef QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
	// empty m_strHostName to indicate resovled successfully.
	//we should keep host name, and use m_bIsResolved to identify the resolve status 6/11 2006 Victor Cui
	//m_strHostName.resize(0);
#endif // QT_SUPPORT_ASYNC_RESOLVE_HOSTNAME
	
	if (aIsNetworkOrder)
		m_SockAddr.sin_addr.s_addr = aIpAddr;
	else
		m_SockAddr.sin_addr.s_addr = htonl(aIpAddr);

	m_bIsResolved = TRUE;
	return QT_OK;
}

inline BOOL CQtInetAddr::IpAddrStringTo4Bytes(LPCSTR aIpStr, DWORD &aIpDword)
{
	aIpDword = INADDR_ANY;
	BOOL bAddrOk = TRUE;
	if (aIpStr && *aIpStr) {
#ifdef QT_WIN32
		aIpDword = ::inet_addr(aIpStr);
		if (aIpDword == INADDR_NONE && strcmp("255.255.255.255", aIpStr)) {
			bAddrOk = FALSE;
		}
#else
		bAddrOk = ::inet_pton(AF_INET, aIpStr, &aIpDword) > 0 ? TRUE : FALSE;
#endif // QT_WIN32
	}
	return bAddrOk;
}


#endif // !QTINETADDR_H
