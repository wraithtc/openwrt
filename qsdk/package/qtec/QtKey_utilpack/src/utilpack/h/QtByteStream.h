/*------------------------------------------------------*/
/* Byte stream template                                 */
/*                                                      */
/* QtByteStream.h                                       */
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
/*
Processor     Operator System     Bytes Order
Alpha					ALL			Little endian
HP-PA					NT			Little endian
HP-PA					UNIX	    Big endian
Intelx86				ALL			Little endian <-----x86 system
Motorola680x()			ALL			Big endian
MIPS					NT			Little endian
MIPS					UNIX		Big endian
PowerPC					NT			Little endian
PowerPC					NOT NT		Big endian   <-----PPC system
RS/6000					UNIX	    Big endian
SPARC					UNIX		Big endian
IXP1200 ARM				ALL			Little endian 
*/
#ifndef QTBYTESTREAM_H
#define QTBYTESTREAM_H

#include "QtDebug.h"

template <class BlockType, class ConvertorType>
class CQtByteStreamT
{
	enum { STRING_LEN_MAX = 64 * 1024 - 1 };
public:
	CQtByteStreamT(BlockType &aBlock)
		: m_Block(aBlock)
		, m_ResultRead(QT_OK)
		, m_ResultWrite(QT_OK)
	{
	}

	CQtByteStreamT& operator<<(char c)
	{
		Write(&c, sizeof(char));
		return *this;
	}

	CQtByteStreamT& operator<<(unsigned char c)
	{
		Write(&c, sizeof(unsigned char));
		return *this;
	}

	CQtByteStreamT& operator<<(short n)
	{
		return *this << (unsigned short)n;
	}

	CQtByteStreamT& operator<<(unsigned short n)
	{
		ConvertorType::Swap(n);
		Write(&n, sizeof(unsigned short));
		return *this;
	}

	CQtByteStreamT& operator<<(int n)
	{
		return *this << (unsigned int)n;
	}
	
	CQtByteStreamT& operator<<(unsigned int n)
	{
		ConvertorType::Swap(n);
		Write(&n, sizeof(unsigned int));
		return *this;
	}

	CQtByteStreamT& operator<<(long n)
	{
		return *this << (unsigned long)n;
	}

	CQtByteStreamT& operator<<(unsigned long n)
	{
		ConvertorType::Swap(n);
		Write(&n, sizeof(unsigned long));
		return *this;
	}

	CQtByteStreamT& operator<<(float n)
	{
		ConvertorType::Swap(n);
		Write(&n, sizeof(float));
		return *this;
	}

	CQtByteStreamT& operator<<(double n)
	{
		ConvertorType::Swap(n);
		Write(&n, sizeof(double));
		return *this;
	}

	CQtByteStreamT& operator<<(const CQtString &str)
	{
		return WriteString(str.c_str(), str.length());
	}

	CQtByteStreamT& operator<<(const char *str)
	{
		unsigned short len = 0;
		if (str)
			len = strlen(str);
		return WriteString(str, len);
	}

	CQtByteStreamT& WriteString(const char *str, unsigned long ll)
	{
		QT_ASSERTE_RETURN(ll < STRING_LEN_MAX, *this);
		unsigned short len = static_cast<unsigned short>(ll);
		if (len >= STRING_LEN_MAX) {
			QT_ERROR_TRACE_THIS("CQtByteStreamT::WriteString, too long, len=" << len);
			m_ResultWrite = QT_ERROR_UNEXPECTED;
			return *this;
		}

		(*this) << len;
		if (len > 0)
			Write(str, len);
		return *this;
	}

	CQtByteStreamT& WriteStringWith4BytesLength(const CQtString &str)
	{
		return WriteStringWith4BytesLength(str.c_str(), str.length());
	}

	CQtByteStreamT& WriteStringWith4BytesLength(const char *str, unsigned long ll)
	{
		(*this) << ll;
		if (ll > 0)
			Write(str, ll);
		return *this;
	}

	CQtByteStreamT& operator>>(char& c)
	{
		Read(&c, sizeof(char));
		return *this;
	}

	CQtByteStreamT& operator>>(unsigned char& c)
	{
		Read(&c, sizeof(unsigned char));
		return *this;
	}

	CQtByteStreamT& operator>>(unsigned short& n)
	{
		Read(&n, sizeof(unsigned short));
		ConvertorType::Swap(n);
		return *this;
	}

	CQtByteStreamT& operator>>(short& n)
	{
		return *this >> (unsigned short&)n;
	}

	CQtByteStreamT& operator>>(int& n)
	{
		return *this >> (unsigned int&)n;
	}

	CQtByteStreamT& operator>>(unsigned int& n)
	{
		Read(&n, sizeof(unsigned int));
		ConvertorType::Swap(n);
		return *this;
	}

	CQtByteStreamT& operator>>(long& n)
	{
		return *this >> (unsigned long&)n;
	}

	CQtByteStreamT& operator>>(unsigned long& n)
	{
		Read(&n, sizeof(unsigned long));
		ConvertorType::Swap(n);
		return *this;
	}

	CQtByteStreamT& operator>>(float& n)
	{
		Read(&n, sizeof(float));
		ConvertorType::Swap(n);
		return *this;
	}

	CQtByteStreamT& operator>>(double& n)
	{
		Read(&n, sizeof(double));
		ConvertorType::Swap(n);
		return *this;
	}

	CQtByteStreamT& operator>>(CQtString& str)
	{
		unsigned short len = 0;
		(*this) >> len;
		QT_ASSERTE(len < STRING_LEN_MAX);
		if (len >= STRING_LEN_MAX) {
			QT_ERROR_TRACE_THIS("CQtByteStreamT::operator>>CQtString, too long, len=" << len);
			m_ResultRead = QT_ERROR_UNEXPECTED;
			return *this;
		}

		if (len > 0) {
			str.resize(0);
			str.resize(len);
			Read(const_cast<char*>(str.data()), len);
		}
		return *this;
	}

	CQtByteStreamT& ReadStringWith4BytesLength(CQtString& str)
	{
		unsigned long len = 0;
		(*this) >> len;

		//////////////////////////////////////////////////////////////////////////
		//add length protection to limit the max length about the string object to avoid crash if the length too big
		//6/9 2009 Victor Cui
		QT_ASSERTE(len < STRING_LEN_MAX);
		if (len >= STRING_LEN_MAX) {
			QT_ERROR_TRACE_THIS("CQtByteStreamT::ReadStringWith4BytesLength, too long, len=" << len);
			m_ResultRead = QT_ERROR_UNEXPECTED;
			return *this;
		}
		//////////////////////////////////////////////////////////////////////////
		if (len > 0) {
			str.resize(0);
			str.resize(len);
			Read(const_cast<char*>(str.data()), len);
		}
		return *this;
	}
	
	CQtByteStreamT& Read(void *aDst, unsigned long aCount)
	{
		if (QT_SUCCEEDED(m_ResultRead)) {
			unsigned long ulRead = 0;
			m_ResultRead = m_Block.Read(aDst, aCount, &ulRead);
#ifdef QT_DEBUG
			if (QT_SUCCEEDED(m_ResultRead))
				QT_ASSERTE(ulRead == aCount);
#endif // QT_DEBUG
		}
		if (QT_FAILED(m_ResultRead)) {
			QT_ERROR_TRACE_THIS("CQtByteStreamT::Read, can't read. m_ResultRead=" << m_ResultRead);
		}
		return *this;
	}

	CQtByteStreamT& Write(const void *aDst, unsigned long aCount)
	{
		if (QT_SUCCEEDED(m_ResultWrite)) {
			unsigned long ulWritten = 0;
			m_ResultWrite = m_Block.Write(aDst, aCount, &ulWritten);
#ifdef QT_DEBUG
			if (QT_SUCCEEDED(m_ResultWrite))
				QT_ASSERTE(ulWritten == aCount);
#endif // QT_DEBUG
		}
		if (QT_FAILED(m_ResultWrite)) {
			QT_ERROR_TRACE_THIS("CQtByteStreamT::Write, can't write. m_ResultWrite=" << m_ResultWrite);
		}
		return *this;
	}
	
	BOOL IsGood()
	{
		if (QT_SUCCEEDED(m_ResultWrite) && QT_SUCCEEDED(m_ResultRead))
			return TRUE;
		else
			return FALSE;
	}

private:
	BlockType &m_Block;
	QtResult m_ResultRead;
	QtResult m_ResultWrite;

	// Not support bool because its sizeof is not fixed.
	CQtByteStreamT& operator<<(bool n);
	CQtByteStreamT& operator>>(bool& n);

	// Not support long double.
	CQtByteStreamT& operator<<(long double n);
	CQtByteStreamT& operator>>(long double& n);
};

// mainly copied from ace/Basic_Types.h
// Byte-order (endian-ness) determination.
# if defined (BYTE_ORDER)
#   if (BYTE_ORDER == LITTLE_ENDIAN)
#     define QT_LITTLE_ENDIAN 0x0123
#     define QT_BYTE_ORDER QT_LITTLE_ENDIAN
#   elif (BYTE_ORDER == BIG_ENDIAN)
#     define QT_BIG_ENDIAN 0x3210
#     define QT_BYTE_ORDER QT_BIG_ENDIAN
#   else
#     error: unknown BYTE_ORDER!
#   endif /* BYTE_ORDER */
# elif defined (_BYTE_ORDER)
#   if (_BYTE_ORDER == _LITTLE_ENDIAN)
#     define QT_LITTLE_ENDIAN 0x0123
#     define QT_BYTE_ORDER QT_LITTLE_ENDIAN
#   elif (_BYTE_ORDER == _BIG_ENDIAN)
#     define QT_BIG_ENDIAN 0x3210
#     define QT_BYTE_ORDER QT_BIG_ENDIAN
#   else
#     error: unknown _BYTE_ORDER!
#   endif /* _BYTE_ORDER */
# elif defined (__BYTE_ORDER)
#   if (__BYTE_ORDER == __LITTLE_ENDIAN)
#     define QT_LITTLE_ENDIAN 0x0123
#     define QT_BYTE_ORDER QT_LITTLE_ENDIAN
#   elif (__BYTE_ORDER == __BIG_ENDIAN)
#     define QT_BIG_ENDIAN 0x3210
#     define QT_BYTE_ORDER QT_BIG_ENDIAN
#   else
#     error: unknown __BYTE_ORDER!
#   endif /* __BYTE_ORDER */
# elif defined (__BYTE_ORDER__)
#   if (__BYTE_ORDER__ == __LITTLE_ENDIAN__)
#     define QT_LITTLE_ENDIAN 0x0123
#     define QT_BYTE_ORDER QT_LITTLE_ENDIAN
#   elif (__BYTE_ORDER__ == __BIG_ENDIAN__)
#     define QT_BIG_ENDIAN 0x3210
#     define QT_BYTE_ORDER QT_BIG_ENDIAN
#   else
#     error: unknown __BYTE_ORDER__!
#   endif /* __BYTE_ORDER__ */
# else /* ! BYTE_ORDER && ! __BYTE_ORDER  && !__BYTE_ORDER__*/
  // We weren't explicitly told, so we have to figure it out . . .
#   if defined (i386) || defined (__i386__) || defined (_M_IX86) || \
     defined (vax) || defined (__alpha) || defined (__LITTLE_ENDIAN__) ||\
     defined (ARM) || defined (_M_IA64)
    // We know these are little endian.
#     define QT_LITTLE_ENDIAN 0x0123
#     define QT_BYTE_ORDER QT_LITTLE_ENDIAN
#   else
    // Otherwise, we assume big endian.
#     define QT_BIG_ENDIAN 0x3210
#     define QT_BYTE_ORDER QT_BIG_ENDIAN
#   endif
# endif /* ! BYTE_ORDER && ! __BYTE_ORDER */

class CQtHostNetworkConvertorNormal
{
public:
	static void Swap(unsigned long &aHostLong)
	{
#ifdef QT_LITTLE_ENDIAN
		if (8 == sizeof(long))
		{
			Swap8(&aHostLong, &aHostLong);
		}
		else
		{
			Swap4(&aHostLong, &aHostLong);
		}
#endif // QT_LITTLE_ENDIAN
	}
	
	static void Swap(unsigned int &aHostInt)
	{
#ifdef QT_LITTLE_ENDIAN
		Swap4(&aHostInt, &aHostInt);
#endif // QT_LITTLE_ENDIAN
	}

	static void Swap(unsigned short &aHostShort)
	{
#ifdef QT_LITTLE_ENDIAN
		Swap2(&aHostShort, &aHostShort);
#endif // QT_LITTLE_ENDIAN
	}

	static void Swap(float &aHostFloat)
	{
#ifdef QT_LITTLE_ENDIAN
		Swap4(&aHostFloat, &aHostFloat);
#endif // QT_LITTLE_ENDIAN
	}

	static void Swap(double &aHostDouble)
	{
#ifdef QT_LITTLE_ENDIAN
		Swap8(&aHostDouble, &aHostDouble);
#endif // QT_LITTLE_ENDIAN
	}

	// mainly copied from ACE_CDR
	static void Swap2(const void *orig, void* target)
	{
#if defined QT_PORT_CLIENT
		// fixed 374293, if that operator = is not on 4 border for SPARC system, 11/27 2009
		register unsigned short usrc;
		memcpy(&usrc, orig, sizeof(usrc)); 
		register unsigned short tmp =  (usrc << 8) | (usrc >> 8);
		memcpy(target, &tmp, sizeof(tmp));
#else
		register unsigned short usrc = 
			* reinterpret_cast<const unsigned short*>(orig);
		register unsigned short* udst = 
			reinterpret_cast<unsigned short*>(target);
		*udst = (usrc << 8) | (usrc >> 8);
#endif
	}

	static void Swap4(const void* orig, void* target)
	{
		register unsigned int x;
#if defined QT_PORT_CLIENT
		// fixed 374293, if that operator = is not on 4 border for SPARC system, 11/27 2009
		memcpy(&x, orig, sizeof(x));
#else
			x = * reinterpret_cast<const unsigned int*>(orig);
#endif
		x = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
#if defined QT_PORT_CLIENT
		// fixed 374293, if that operator = is not on 4 border for SPARC system , 11/27 2009
		memcpy(target, &x, sizeof(x));
#else
		* reinterpret_cast<unsigned int*>(target) = x;
#endif
	}

	static void Swap8(const void* orig, void* target)
	{
#if defined QT_PORT_CLIENT
		// fixed 374293, if that operator = is not on 4 border for SPARC system, 11/27 2009
		register unsigned long x = 0, y = 0;
		memcpy(&x, orig, sizeof(x));
		memcpy(&y, static_cast<const char*>(orig) + 4, sizeof(y));
#else
		register unsigned long x = 
			* reinterpret_cast<const unsigned long*>(orig);
		register unsigned long y = 
			* reinterpret_cast<const unsigned long*>(static_cast<const char*>(orig) + 4);
#endif
		x = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
		y = (y << 24) | ((y & 0xff00) << 8) | ((y & 0xff0000) >> 8) | (y >> 24);
#if defined QT_PORT_CLIENT
		// fixed 374293, if that operator = is not on 4 border for SPARC system, 11/27 2009
		memcpy(target, &y, sizeof(y));
		memcpy(static_cast<char*>(target) + 4, &x, sizeof(x));
#else
		* reinterpret_cast<unsigned long*>(target) = y;
		* reinterpret_cast<unsigned long*>(static_cast<char*>(target) + 4) = x;
#endif
	}
};

class CQtHostNetworkConvertorIntel
{
public:
	static void Swap(unsigned long &aHostLong)
	{
#if !defined QT_LITTLE_ENDIAN
		if (8 == sizeof(unsigned long))
		{
			CQtHostNetworkConvertorNormal::Swap8(&aHostLong, &aHostLong);
		}
		else
		{
			CQtHostNetworkConvertorNormal::Swap4(&aHostLong, &aHostLong);
		}
#endif // QT_LITTLE_ENDIAN
	}

	static void Swap(unsigned short &aHostShort)
	{
#if !defined QT_LITTLE_ENDIAN
		CQtHostNetworkConvertorNormal::Swap2(&aHostShort, &aHostShort);
#endif // QT_LITTLE_ENDIAN
	}
	
	static void Swap(unsigned int &aHostInt)
	{
#if !defined QT_LITTLE_ENDIAN
		CQtHostNetworkConvertorNormal::Swap4(&aHostInt, &aHostInt);
#endif // QT_LITTLE_ENDIAN
	}

	static void Swap(float &aHostFloat)
	{
#if !defined QT_LITTLE_ENDIAN
		CQtHostNetworkConvertorNormal::Swap4(&aHostFloat, &aHostFloat);
#endif // QT_LITTLE_ENDIAN
	}

	static void Swap(double &aHostDouble)
	{
#if !defined QT_LITTLE_ENDIAN
		CQtHostNetworkConvertorNormal::Swap8(&aHostDouble, &aHostDouble);
#endif // QT_LITTLE_ENDIAN
	}
};

class CQtHostNetworkConvertorNull
{
public:
	static void Swap(unsigned long &aHostLong)
	{
	}

	static void Swap(unsigned short &aHostShort)
	{
	}

	static void Swap(float &aHostFloat)
	{
	}

	static void Swap(int &aHostInt)
	{
	}
	
	static void Swap(double &aHostDouble)
	{
	}
};

#include "QtMessageBlock.h"

typedef CQtByteStreamT<CQtMessageBlock, CQtHostNetworkConvertorNormal> CQtByteStreamNetwork;
typedef CQtByteStreamT<CQtMessageBlock, CQtHostNetworkConvertorIntel> CQtByteStreamIntel;
typedef CQtByteStreamT<CQtMessageBlock, CQtHostNetworkConvertorNull> CQtByteStreamMemory;

#endif // !QTBYTESTREAM_H
