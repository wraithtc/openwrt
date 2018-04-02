/*------------------------------------------------------*/
/* wrapper class for time value                         */
/*                                                      */
/* QtTimeValue.h                                        */
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

#ifndef QTTIMEVALUE_H
#define QTTIMEVALUE_H

#include "QtDefines.h"

#define QT_ONE_SECOND_IN_MSECS 1000L
#define QT_ONE_SECOND_IN_USECS 1000000L
#define QT_ONE_SECOND_IN_NSECS 1000000000L

#ifdef _MSC_VER
  // -------------------------------------------------------------------
  // These forward declarations are only used to circumvent a bug in
  // MSVC 6.0 compiler.  They shouldn't cause any problem for other
  // compilers.
  class CQtTimeValue;
  QT_OS_EXPORT CQtTimeValue operator + (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
  QT_OS_EXPORT CQtTimeValue operator - (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
  QT_OS_EXPORT int operator < (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
  QT_OS_EXPORT int operator > (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
  QT_OS_EXPORT int operator <= (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
  QT_OS_EXPORT int operator >= (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
  QT_OS_EXPORT int operator == (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
  QT_OS_EXPORT int operator != (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
#endif // _MSC_VER

class QT_OS_EXPORT CQtTimeValue  
{
public:
	// add the follwoing two functions to avoid call Normalize().
	CQtTimeValue();
	CQtTimeValue(long aSec);
	CQtTimeValue(long aSec, long aUsec);
	CQtTimeValue(const timeval &aTv);
	CQtTimeValue(double aSec);
	
	void Set(long aSec, long aUsec);
	void Set(const timeval &aTv);
	void Set(double aSec);

	long GetSec() const ;
	long GetUsec() const ;

	void SetByTotalMsec(long aMilliseconds);
	long GetTotalInMsec() const;

	void operator += (const CQtTimeValue &aRight);
	void operator -= (const CQtTimeValue &aRight);

	friend QT_OS_EXPORT CQtTimeValue operator + (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
	friend QT_OS_EXPORT CQtTimeValue operator - (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
	friend QT_OS_EXPORT int operator < (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
	friend QT_OS_EXPORT int operator > (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
	friend QT_OS_EXPORT int operator <= (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
	friend QT_OS_EXPORT int operator >= (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
	friend QT_OS_EXPORT int operator == (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);
	friend QT_OS_EXPORT int operator != (const CQtTimeValue &aLeft, const CQtTimeValue &aRight);

	static CQtTimeValue GetTimeOfDay();
	static const CQtTimeValue s_tvZero;
	static const CQtTimeValue s_tvMax;
	
private:
	void Normalize();
	
	long m_lSec;
	long m_lUsec;
};


// inline functions
inline CQtTimeValue::CQtTimeValue()
	: m_lSec(0)
	, m_lUsec(0)
{
}

inline CQtTimeValue::CQtTimeValue(long aSec)
	: m_lSec(aSec)
	, m_lUsec(0)
{
}

inline CQtTimeValue::CQtTimeValue(long aSec, long aUsec)
{
	Set(aSec, aUsec);
}

inline CQtTimeValue::CQtTimeValue(const timeval &aTv)
{
	Set(aTv);
}

inline CQtTimeValue::CQtTimeValue(double aSec)
{
	Set(aSec);
}

inline void CQtTimeValue::Set(long aSec, long aUsec)
{
	m_lSec = aSec;
	m_lUsec = aUsec;
	Normalize();
}

inline void CQtTimeValue::Set(const timeval &aTv)
{
	m_lSec = aTv.tv_sec;
	m_lUsec = aTv.tv_usec;
	Normalize();
}

inline void CQtTimeValue::Set(double aSec)
{
	long l = (long)aSec;
	m_lSec = l;
	m_lUsec = (long)((aSec - (double)l) * QT_ONE_SECOND_IN_USECS);
	Normalize();
}

inline void CQtTimeValue::SetByTotalMsec(long aMilliseconds)
{
	m_lSec = aMilliseconds / 1000;
	m_lUsec = (aMilliseconds - (m_lSec * 1000)) * 1000;
}

inline long CQtTimeValue::GetSec() const 
{
	return m_lSec;
}

inline long CQtTimeValue::GetUsec() const 
{
	return m_lUsec;
}

inline long CQtTimeValue::GetTotalInMsec() const
{
	return m_lSec * 1000 + m_lUsec / 1000;
}

#ifndef QT_WIN32
inline CQtTimeValue CQtTimeValue::GetTimeOfDay()
{
	timeval tvCur;
	::gettimeofday(&tvCur, NULL);
	return CQtTimeValue(tvCur);
}
#endif // !QT_WIN32

inline int operator > (const CQtTimeValue &aLeft, const CQtTimeValue &aRight)
{
	if (aLeft.GetSec() > aRight.GetSec())
		return 1;
	else if (aLeft.GetSec() == aRight.GetSec() && aLeft.GetUsec() > aRight.GetUsec())
		return 1;
	else
		return 0;
}

inline int operator >= (const CQtTimeValue &aLeft, const CQtTimeValue &aRight)
{
	if (aLeft.GetSec() > aRight.GetSec())
		return 1;
	else if (aLeft.GetSec() == aRight.GetSec() && aLeft.GetUsec() >= aRight.GetUsec())
		return 1;
	else
		return 0;
}

inline int operator < (const CQtTimeValue &aLeft, const CQtTimeValue &aRight)
{
	return !(aLeft >= aRight);
}

inline int operator <= (const CQtTimeValue &aLeft, const CQtTimeValue &aRight)
{
	return aRight >= aLeft;
}

inline int operator == (const CQtTimeValue &aLeft, const CQtTimeValue &aRight)
{
	return aLeft.GetSec() == aRight.GetSec() && 
		   aLeft.GetUsec() == aRight.GetUsec();
}

inline int operator != (const CQtTimeValue &aLeft, const CQtTimeValue &aRight)
{
	return !(aLeft == aRight);
}

inline void CQtTimeValue::operator += (const CQtTimeValue &aRight)
{
	m_lSec = GetSec() + aRight.GetSec();
	m_lUsec = GetUsec() + aRight.GetUsec();
	Normalize();
}

inline void CQtTimeValue::operator -= (const CQtTimeValue &aRight)
{
	m_lSec = GetSec() - aRight.GetSec();
	m_lUsec = GetUsec() - aRight.GetUsec();
	Normalize();
}

inline CQtTimeValue operator + 
(const CQtTimeValue &aLeft, const CQtTimeValue &aRight)
{
	return CQtTimeValue(aLeft.GetSec() + aRight.GetSec(), 
					  aLeft.GetUsec() + aRight.GetUsec());
}

inline CQtTimeValue operator - 
(const CQtTimeValue &aLeft, const CQtTimeValue &aRight)
{
	return CQtTimeValue(aLeft.GetSec() - aRight.GetSec(), 
					  aLeft.GetUsec() - aRight.GetUsec());
}

#endif // !QTTIMEVALUE_H
