/*------------------------------------------------------*/
/* the timer definition					                */
/*								                        */
/* timer.h                                              */
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

#ifndef	TIMER_INC_
#define TIMER_INC_

#include "QtUtilTemplates.h"

#if defined(_MSC_VER)
#pragma warning(disable:4307)
#pragma warning(disable:4308)
#endif

#ifndef QT_WIN32
//#define _UNIX_HIGH_PRECISION
#endif

///Warning that enum value can not be large int
#if defined(_MSC_VER) && (_MSC_VER <= 1300) //for VC7.0 or before
template <unsigned long base>
struct ExpTraits
{
	template <unsigned long exp>
		struct Exp_Inner
	{
		enum { VAL = base * ExpTraits<base>::Exp_Inner<exp - 1>::VAL };
	};
	template<> struct Exp_Inner<0>
	{
		enum { VAL = 1 };
	};
};
template<unsigned long base, unsigned long exponential>
struct ExpT
{
	enum{ VAL = ExpTraits<base>::Exp_Inner<exponential>::VAL };
};

#else

template <unsigned long base, unsigned long exponential>
struct ExpT
{
	enum { VAL = base * ExpT<base, exponential - 1>::VAL };
};
template <unsigned long base>
struct ExpT<base, 0>
{
	enum { VAL = 1};
};
#endif

//template instantiation depth exceeds maximum of 17 on MAC, so need a function replace it
template <typename ResultType>
ResultType exp_cal(unsigned long base, unsigned long exponential)
{
	ResultType val = 1;
	for(unsigned long idx = 0; idx < exponential; ++idx)
		val *= base;
	return val;
}

#ifdef QT_WIN32
  #include <time.h>
  #include <winnt.h>
#else
  #ifndef MACOS
    #include <sys/time.h>
    #include <sys/times.h>
    #include <sys/types.h>
  #else
	  #ifndef MachOSupport
	  	#define   _SC_CLK_TCK	CLOCKS_PER_SEC
	  	struct tms {
			_BSD_CLOCK_T_ tms_utime;	/* User CPU time */
			_BSD_CLOCK_T_ tms_stime;	/* System CPU time */
			_BSD_CLOCK_T_ tms_cutime;	/* User CPU time of terminated child procs */
			_BSD_CLOCK_T_ tms_cstime;	/* System CPU time of terminated child procs */
		};
	  #else
	  	#include <sys/time.h>
	    #include <sys/times.h>
	    #include <sys/types.h>
	  #endif	//MachOSupport
  #endif
  typedef long long int _int64;
#endif

const _int64 us_benchmark = 1000000; 
const _int64 us_second = us_benchmark;//microseconds in one seconds
const _int64 us_minute = us_benchmark * 60;
const _int64 us_hour = us_benchmark * 60 * 60;
const _int64 us_day = us_hour * 24;
const _int64 ms_benchmark = 1000;
const _int64 ms_hour = ms_benchmark * 60 * 60;
const _int64 ms_minute = ms_benchmark * 60;
const _int64 ms_second = ms_benchmark;//milliseconds in one seconds

static const _int64 MAX_INT64_PLUS_VAL	=	(_int64)0X7FFFFFFF << 32 | 0XFFFFFFFF;

///Warning the time will overflow from year 2038 on 32bit machine
struct QT_OS_EXPORT PretInitialationTimer {
#ifdef QT_WIN32
	typedef LARGE_INTEGER INIT_TYPE;
#else
	typedef unsigned INIT_TYPE;
#endif
	INIT_TYPE m_InitialValue;
#if defined QT_WIN32 || defined _UNIX_HIGH_PRECISION
	_int64 m_startTmfromS; //time from system started
	_int64 m_startTmfromC; //time from 1970.1.1
#ifdef _UNIX_HIGH_PRECISION
	_int64 m_startMillis;
#endif
#endif
	PretInitialationTimer();
	friend class CQtSingletonT<PretInitialationTimer>;
};

typedef CQtSingletonT<PretInitialationTimer> PretInitialationTimerSingleT;

struct QT_OS_EXPORT timer_policy
{
	///get time (us) from 1970.1.1 
	static _int64 now();
};

struct QT_OS_EXPORT tick_policy
{
	///get time (micro s) from system start or process start 
	static _int64 now();
};

struct QT_OS_EXPORT low_tick_policy//low precision tick, 100ms
{
	class low_tick_generator: public CQtTimerWrapperIDSink
	{
	public:
		enum{MIN_INTERVAL = 100000};//100ms
		void OnTimer(CQtTimerWrapperID* aId);
		_int64 now();
		static low_tick_generator *instance();
	private:
		low_tick_generator();
		low_tick_generator(const low_tick_generator &);
		low_tick_generator & operator=(const low_tick_generator &);
		_int64 m_tickNow;
		CQtTimerWrapperID	m_RefreshTimer;
		static CQtMutexThread mutex;
		static low_tick_generator *pInstance;
		static BOOL m_InitTimerSucc;
	};
public:
	///get time (micro s) from system start or process start 
	static _int64 now();
};


typedef struct QT_OS_EXPORT tick_type{
	DWORD tk_hour;
	DWORD tk_min; //minute after hour 0-59
	DWORD tk_sec;// second after minute 0-59
	DWORD tk_millisec;//millisecond after second 0-999
	DWORD tk_microsec;//microsecond after millisecond 0-999
	
	tick_type();
	tick_type(_int64 val);
protected:
	void reset(_int64 val);
}*lptick_type;

struct QT_OS_EXPORT formatted_ticker: public tick_type
{
	typedef tick_type   VALUE_TYPE;
	static tick_type now();
};

typedef struct QT_OS_EXPORT time_type: public tm
{
	DWORD tm_millisec; //millisecond after second, 0 - 999
	DWORD tm_microsec; //microsecond after millisecond 0-999
	time_type();
	time_type(_int64 val);
protected:
	void reset(_int64 val);
}*lptime_type;

struct QT_OS_EXPORT formatted_timer: public tick_type
{
	typedef time_type  VALUE_TYPE;
	static VALUE_TYPE now();
};

///rigour to millisecond
template<typename policy_t>
struct QT_OS_EXPORT timer_fact
{
	typedef _int64	VALUE_TYPE;
	///Constructor
	timer_fact(VALUE_TYPE initTime = -1)
	{
#ifdef QT_WIN32
		if(0 == ::SetThreadAffinityMask(GetCurrentThread(), 1))
		{
			QT_WARNING_TRACE_THIS("timer_fact::timer_fact let the thread run on a fixed CPU, thread ID = " << GetCurrentThread() << " failed, errno = " << ::GetLastError());
		}
		else
		{
			QT_INFO_TRACE_THIS("timer_fact::timer_fact let the thread run on a fixed CPU, thread ID = " << GetCurrentThread());
		}
#endif
		reset(initTime);
	}
	///reset the time tag
	void reset(VALUE_TYPE initTime = -1)
	{
		if(initTime == -1)
			tag_ = policy_t::now();
		else
			tag_ = initTime;
	}

	///get the elapsed times from reset(microseconds)
	VALUE_TYPE elapsed()
	{
		VALUE_TYPE nowt = policy_t::now();
		VALUE_TYPE remain = nowt >= tag_ ? 0 : MAX_INT64_PLUS_VAL - tag_;
#ifdef QT_WIN32
		static const _int64 MAX_SUPPORT_CYCLE_US = 31536000000000; //365 * 24 * 3600 * 1000 * 1000
		if(remain >= MAX_SUPPORT_CYCLE_US) //only support less than a year
		{
			QT_WARNING_TRACE_THIS("timer_fact::elapsed over one year, no support, remain = " << remain);
			QT_INFO_TRACE_THIS("timer_fact::elapsed now_t = " << nowt << " tag_ = " << tag_ << " remain = " << remain);
			return 0;
		}
#endif
		return remain == 0 ? nowt -  tag_ : nowt + remain;
	}

	VALUE_TYPE elapsed_mills()
	{
		VALUE_TYPE nowt = policy_t::now() / ms_benchmark;
		VALUE_TYPE remain = nowt >= tag_  / ms_benchmark ? 0 : (MAX_INT64_PLUS_VAL - tag_) / ms_benchmark;
#ifdef QT_WIN32
		static const _int64 MAX_SUPPORT_CYCLE_MS = 31536000000; //365 * 24 * 3600 * 1000
		if(remain >= MAX_SUPPORT_CYCLE_MS) //only support over less than a year
		{
			QT_WARNING_TRACE_THIS("timer_fact::elapsed_mills over one year, no support, remain = " << remain);
			QT_INFO_TRACE_THIS("timer_fact::elapsed_mills now_t = " << nowt << " tag_ = " << (tag_ / ms_benchmark) << " remain = " << remain);
			return 0;
		}
#endif
		return remain == 0 ? nowt -  tag_ / ms_benchmark : nowt + remain;
	}

	VALUE_TYPE elapsed_sec()
	{
		VALUE_TYPE nowt = policy_t::now() / us_benchmark;
		VALUE_TYPE remain = nowt >= tag_  / us_benchmark ? 0 : (MAX_INT64_PLUS_VAL - tag_) / us_benchmark;
#ifdef QT_WIN32
		static const _int64 MAX_SUPPORT_CYCLE_S = 31536000; //365 * 24 * 3600
		if(remain >= MAX_SUPPORT_CYCLE_S) //only support over less than a year
		{
			QT_WARNING_TRACE_THIS("timer_fact::elapsed_sec over one year, no support, remain = " << remain);
			QT_INFO_TRACE_THIS("timer_fact::elapsed_sec now_t = " << nowt << " tag_ = " << (tag_ / us_benchmark) << " remain = " << remain);
			return 0;
		}
#endif
		return remain == 0 ? nowt -  tag_ / us_benchmark : nowt + remain;
	}

	///get the elapsed microseconds
	const static VALUE_TYPE now() 
	{
		return policy_t::now();
	}

	bool overtime(VALUE_TYPE microTimes)
	{
		return elapsed() >= microTimes;
	}

	bool overtime_mills(VALUE_TYPE millsTimes)
	{
		return elapsed_mills()  >= millsTimes;
	}

	bool overtime_sec(VALUE_TYPE secTimes)
	{
		return elapsed_sec() >= secTimes;
	}

	VALUE_TYPE latest_tag() const
	{
		return tag_;
	}

private:
	///the time tag
	VALUE_TYPE	tag_;	
};

typedef timer_fact<timer_policy>		timer;
typedef timer_fact<tick_policy>			ticker;
typedef timer_fact<low_tick_policy>		low_ticker;

#endif
