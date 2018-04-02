/*------------------------------------------------------------------*/
/* compatible to standard C++                                       */
/* C++(standard) definition                                         */
/*                                                                  */
/* QtStdCpp.h                                                       */
/*                                                                  */
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

#ifndef QTSTDCPP_H
#define QTSTDCPP_H

#include "QtDefines.h"

#include <ctype.h>

#if 0
#ifndef _WIN32
#include <wctype.h>
#endif
#endif

#define QT_DISABLE_MSVCP_DLL

#if defined (_MSC_VER) && defined (_DLL) && defined (QT_DISABLE_MSVCP_DLL)
//  #pragma warning(disable:4273)

  #undef _CRTIMP
  #define _CRTIMP

  #ifdef _DEBUG
    #pragma comment(linker, "/NODEFAULTLIB:LIBQTTD")
    #pragma comment(linker, "/NODEFAULTLIB:MSVCPRTD")
    #pragma comment(lib, "LIBCPMTD")
  #else
    #pragma comment(linker, "/NODEFAULTLIB:LIBQTT")
    #pragma comment(linker, "/NODEFAULTLIB:MSVCPRT")
    #pragma comment(lib, "LIBCPMT")
  #endif // _DEBUG

  // Don't include <use_ansi.h> that using C++ dynamic library.
  #ifdef _USE_ANSI_CPP
    #error Error: please include this file before include STL head files such as <map>, etc.
  #endif // _USE_ANSI_CPP
  #define _USE_ANSI_CPP

#endif // _MSC_VER && _DLL && QT_DISABLE_MSVCP_DLL


//#if defined (_MSC_VER) && defined (_DLL) && defined (QT_DISABLE_MSVCP_DLL)
#if 0
  #pragma warning(disable:4273)

  #define QT_IMPLEMENT_VAR_FOR_MSVCP() \
    extern "C" { \
      int __mb_cur_max = 1; \
      unsigned short *_pctype = _ctype+1; \
      int __setlc_active; \
      int __unguarded_readlc_active; \
      UINT __lc_codepage = 0; \
      DWORD __lc_handle[5-0+1] = {0, 0, 0, 0, 0, 0}; \
	  \
	  char _bufin[4096]; \
      FILE _iob[_IOB_ENTRIES] = { \
        { _bufin, 0, _bufin, _IOREAD | 0x0100, 0, 0, 4096 }, \
        { NULL, 0, NULL, _IOWRT, 1, 0, 0 }, \
        { NULL, 0, NULL, _IOWRT, 2, 0, 0 },}; \
    }
#else // 
  #define QT_IMPLEMENT_VAR_FOR_MSVCP() 
#endif // _MSC_VER && _DLL && QT_DISABLE_MSVCP_DLL

#include <memory>
#include <utility>
#include <algorithm>
#include <utility>
//#include <stdexcept>

#ifdef _MSC_VER
#if _MSC_VER > 1200
#define QT_MIN _cpp_min
#define QT_MAX _cpp_max
#else
#define QT_MIN std::_cpp_min
#define QT_MAX std::_cpp_max
#endif
#else
#define QT_MIN std::min
#define QT_MAX std::max
#endif // _MSC_VER

#if defined (_MSC_VER) && defined (_DLL) && defined (QT_DISABLE_MSVCP_DLL)
  #include <xiosbase>
  #undef _DLL
  #include <fstream>
  #include <string>
  #include <locale>
//  #include <iostream>
//  #include <strstream>
  #define _DLL

  // Can't include <iostream> that depending on C static library.
  #ifdef _IOSTREAM_
    #error Error: Can not include <iostream> if using QT_DISABLE_MSVCP_DLL.
  #endif // _IOSTREAM_
  #define _IOSTREAM_

#else
    #include <string>
#endif // _MSC_VER && _DLL && QT_DISABLE_MSVCP_DLL

#include <set> 
#include <map>
#include <vector>
#include <list>
  #include <queue>
#include <stack>

#if defined (_MSC_VER) && defined (_DLL) && defined (QT_DISABLE_MSVCP_DLL)
  #undef _CRTIMP
  #define _CRTIMP __declspec(dllimport)
#endif // _MSC_VER && _DLL && QT_DISABLE_MSVCP_DLL

//define CQtWString to store wchar_t string

#if 0
class CQtWString : public std::wstring
{
public:
	typedef std::wstring SuperType;
	
public:
	CQtWString()
		: SuperType()
	{
	}
	
	CQtWString(const wchar_t *s)
		: SuperType(s ? s : (wchar_t*)(""))
	{
	}
	
	CQtWString(const wchar_t *s, SuperType::size_type n)
		: SuperType(s ? s : (wchar_t*)(""), s ? n : 0)
	{
	}
	
#if defined(QT_WIN32) || defined(QT_MACOS) || defined(QT_SOLARIS)
	template <class IterType>
		CQtWString(IterType s1, IterType s2)
		: SuperType(s1, s2, SuperType::allocator_type())
	{
	}
#else
	CQtWString(const wchar_t* s1, const wchar_t* s2)
		: SuperType(s1, s2 - s1)
	{
	}
#endif
	
	CQtWString(const SuperType &str)
		: SuperType(str)
	{
	}
	
	CQtWString(const CQtWString &str)
		: SuperType(str)
	{
	}

	CQtWString(const char *str)
	{
		*this = str;
	}
	
	CQtWString(SuperType::size_type n, wchar_t c)
		: SuperType(n, c, SuperType::allocator_type())
	{
	}
	CQtWString& operator = (const wchar_t *s)
	{
		SuperType::operator = (s ? s : (wchar_t*)(""));
		return *this;
	}
	
	CQtWString& operator = (const CQtWString &str)
	{
		SuperType::operator = (str);
		return *this;
	}
	
	//covert CQtString to CQtWString
	CQtWString& operator=(const char * szString)
	{
		CQtWString::size_type nLen = strlen(szString) + 1;
		wchar_t *szBuffer= new wchar_t[nLen];
		mbstowcs(szBuffer, szString, nLen);
		assign(szBuffer, nLen);
		delete [] szBuffer;
		return *this;
	}

	CQtWString& operator = (wchar_t c)
	{
		SuperType::operator = (c);
		return *this;
	}
	
	CQtWString& operator = (int n)
	{
		SuperType::operator = (n);
		return *this;
	}
	
	
	CQtWString& toUpperCase()
	{
		wchar_t * szInner = (wchar_t *)this->c_str(); 
		for(CQtWString::size_type i = 0; i < length(); ++i, szInner++)
		{
			if(iswalpha(*szInner) && iswlower(*szInner))
				*szInner = towupper(*szInner);
		}
		return *this;
	}
	
	CQtWString& toLowerCase()
	{
		wchar_t * szInner = (wchar_t *)this->c_str(); 
		for(CQtWString::size_type i = 0; i < length(); ++i, szInner++)
		{
			if(iswalpha(*szInner) && iswupper(*szInner))
				*szInner = towlower(*szInner);
		}
		return *this;
	}
};

template<class IS> 
void LTrimWString(CQtWString &aTrim, IS aIs)
{
	LPCWSTR pStart = aTrim.c_str();
	LPCWSTR pMove = pStart;
	
	for ( ; *pMove; ++pMove) {
		if (!aIs(*pMove)) {
			if (pMove != pStart) {
				size_t nLen = wcslen(pMove);
				aTrim.replace(0, nLen, pMove, nLen);
				aTrim.resize(nLen);
			}
			return;
		}
	}
};

template<class IS> 
void RTrimWString(CQtWString &aTrim, IS aIs)
{
	if (aTrim.empty())
		return;
	
	LPCWSTR pStart = aTrim.c_str();
	LPCWSTR pEnd = pStart + aTrim.length() - 1;
	LPCWSTR pMove = pEnd;
	
	for ( ; pMove >= pStart; --pMove) {
		if (!aIs(*pMove)) {
			if (pMove != pEnd)
				aTrim.resize(pMove - pStart + 1);
			return;
		}
	}
};

template<class IS> 
void TrimWString(CQtWString &aTrim, IS aIs)
{
	LTrimWString(aTrim, aIs);
	RTrimWString(aTrim, aIs);
};

#endif // 0
//typedef std::string CQtString;

class CQtString : public std::string
{
public:
	typedef std::string SuperType;

public:
	CQtString()
		: SuperType()
	{
	}
	
	CQtString(const char *s)
		: SuperType(s ? s : "")
	{
	}

	CQtString(const char *s, SuperType::size_type n)
		: SuperType(s ? s : "", s ? n : 0)
	{
	}

#if defined(QT_WIN32) || defined(QT_MACOS) || defined(QT_SOLARIS)
	template <class IterType>
	CQtString(IterType s1, IterType s2)
		: SuperType(s1, s2, SuperType::allocator_type())
	{
	}
#else
	CQtString(const char* s1, const char* s2) 
		: SuperType(s1, s2 - s1)
	{
	}
#endif

	CQtString(const SuperType &str)
#if defined QT_WIN32
		: SuperType(str.c_str(), str.length())
#else
		: SuperType(str)
#endif
	{
	}

	CQtString(const CQtString &str)
#if defined QT_WIN32
		: SuperType(str.c_str(), str.length())
#else
		: SuperType(str)
#endif
	{
	}

#if 0
	CQtString(const CQtWString &szWString)
	{
		*this = szWString;
	}
#endif

	CQtString(SuperType::size_type n, char c)
		: SuperType(n, c, SuperType::allocator_type())
	{
	}

	CQtString& operator = (const char *s)
	{
#if defined QT_WIN32
		if(s > this->c_str() && s < (this->c_str() + this->length())) //in the string scope
		{
			CQtString tmpStr(s);
			*this = tmpStr;
			return *this;
		}
#endif
		SuperType::operator = (s ? s : "");
		return *this;
	}

	CQtString& operator = (const CQtString &str)
	{
#if defined QT_WIN32
		if(&str == this) //if same, do nothing, otherwise it should be has some memory issue
			return *this;
		SuperType::assign(str.c_str(), str.length());
#else
		SuperType::operator = (str);
#endif
		return *this;
	}

#if  0
	//covert CQtWString to CQtString
	CQtString& operator=(const CQtWString &szWString)
	{
		CQtString::size_type nLen = szWString.length() + 1;
		char *szBuffer= new char[nLen];
		wcstombs(szBuffer, szWString.c_str(), nLen);
		assign(szBuffer, nLen);;
		delete [] szBuffer;
		return *this;
	}

#endif
	CQtString& operator = (char c)
	{
		SuperType::operator = (c);
		return *this;
	}

	CQtString& operator = (int n)
	{
		SuperType::operator = (n);
		return *this;
	}

	CQtString& toUpperCase()
	{
		char * szInner = (char *)this->c_str(); 
		for(CQtString::size_type i = 0; i < length(); ++i, szInner++)
		{
			if(isalpha(*szInner) && islower(*szInner))
				*szInner = toupper(*szInner);
		}
		return *this;
	}

	CQtString& toLowerCase()
	{
		char * szInner = (char *)this->c_str(); 
		for(CQtString::size_type i = 0; i < length(); ++i, szInner++)
		{
			if(isalpha(*szInner) && isupper(*szInner))
				*szInner = tolower(*szInner);
		}
		return *this;
	}

};

class CQtIsSpace
{
public:
	int operator() (const char c) {
		return c == ' ';
	}

#if 0
	int operator() (const wchar_t c)
	{
		return c == (wchar_t)' ';
	}
#endif
};

template<class IS> 
void LTrimString(CQtString &aTrim, IS aIs)
{
	LPCSTR pStart = aTrim.c_str();
	LPCSTR pMove = pStart;

	for ( ; *pMove; ++pMove) {
		if (!aIs(*pMove)) {
			if (pMove != pStart) {
				size_t nLen = strlen(pMove);
				aTrim.replace(0, nLen, pMove, nLen);
				aTrim.resize(nLen);
			}
			return;
		}
	}
};

template<class IS> 
void RTrimString(CQtString &aTrim, IS aIs)
{
	if (aTrim.empty())
		return;

	LPCSTR pStart = aTrim.c_str();
	LPCSTR pEnd = pStart + aTrim.length() - 1;
	LPCSTR pMove = pEnd;

	for ( ; pMove >= pStart; --pMove) {
		if (!aIs(*pMove)) {
			if (pMove != pEnd)
				aTrim.resize(pMove - pStart + 1);
			return;
		}
	}
};

template<class IS> 
void TrimString(CQtString &aTrim, IS aIs)
{
	LTrimString(aTrim, aIs);
	RTrimString(aTrim, aIs);
};



//covert CQtWString to CQtString
/*
CQtString& CQtString::operator=(const CQtWString &szWString)
{
	CQtString::size_type nLen = szWString.length() + 1;
	char *szBuffer= new char[nLen];
	wcstombs(szBuffer, szWString.c_str(), szWString.length());
	assign(szBuffer, nLen);;
	delete [] szBuffer;
	return *this;
}	


*/
#endif // QTSTDCPP_H
