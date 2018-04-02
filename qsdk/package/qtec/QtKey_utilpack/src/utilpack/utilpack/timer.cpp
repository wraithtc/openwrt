
#include "QtBase.h"
#include "timer.h"

PretInitialationTimer::PretInitialationTimer()
{
#ifdef QT_WIN32
	QueryPerformanceFrequency(&m_InitialValue);
	LARGE_INTEGER tv;
	if(!QueryPerformanceCounter(&tv))
		m_startTmfromS = (static_cast<_int64>(clock()) * us_second)/ CLOCKS_PER_SEC;
	else
	{
		m_startTmfromS = static_cast<_int64>((((double)tv.QuadPart * ms_second) / (double)m_InitialValue.QuadPart) * ms_second);
	}
	m_startTmfromC = static_cast<_int64>(time(NULL)) * us_second;
#else
	m_InitialValue = (INIT_TYPE)((double)ms_second) /  sysconf(_SC_CLK_TCK);
#ifdef _UNIX_HIGH_PRECISION
	struct timeval tv;
	struct tms tm;
	gettimeofday(&tv, 0);
	m_startTmfromC = static_cast<_int64>(tv.tv_sec) * us_second + tv.tv_usec;
	m_startTmfromS = (static_cast<_int64>(times(&tm)) * m_InitialValue) * ms_second;
	m_startMillis = m_startTmfromS / us_second;		
#endif
#endif
}

_int64 timer_policy::now()
{
#ifdef QT_WIN32
	LARGE_INTEGER tv;
	if(!QueryPerformanceCounter(&tv))
		return (static_cast<_int64>(clock()) * us_second)/ CLOCKS_PER_SEC - CQtSingletonT<PretInitialationTimer>::Instance()->m_startTmfromS + 
		CQtSingletonT<PretInitialationTimer>::Instance()->m_startTmfromC;
	else
	{
		return static_cast<_int64>((((double)tv.QuadPart * ms_second) / (double)PretInitialationTimerSingleT::Instance()->m_InitialValue.QuadPart) * ms_second)  - 
			CQtSingletonT<PretInitialationTimer>::Instance()->m_startTmfromS + 
			CQtSingletonT<PretInitialationTimer>::Instance()->m_startTmfromC;
	}
#else
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (static_cast<_int64>(tv.tv_sec) * us_second + tv.tv_usec);
#endif	
}

_int64 tick_policy::now()
{
#ifdef QT_WIN32
#if 0
	LARGE_INTEGER tv;
	if(!QueryPerformanceCounter(&tv))
		return (static_cast<_int64>(clock()) * us_second)/ CLOCKS_PER_SEC;
	else
	{
		return static_cast<_int64>((((double)tv.QuadPart * ms_second) / (double)PretInitialationTimerSingleT::Instance()->m_InitialValue.QuadPart) * ms_second);
	}
#else
	_int64 _now = (_int64)::GetTickCount();
	return _now * ms_benchmark ;
#endif

#else
	struct tms tm;
#ifdef _UNIX_HIGH_PRECISION
	struct timeval tv;
	gettimeofday(&tv, 0);
	//first get milliseconds lapsed from PretInitialationTimerSingleT created
	_int64 _amend = (static_cast<_int64>(tv.tv_sec) * us_second + tv.tv_usec - 
		PretInitialationTimerSingleT::Instance()->m_startTmfromC) % us_second;
	//correct it if timer has been adjusted
	if(_amend < 0)
		_amend += us_second;
	_int64 val = (static_cast<_int64>(times(&tm)) * (PretInitialationTimerSingleT::Instance()->m_InitialValue));
	val -= (val % ms_second);
	val *= ms_second;
	val += (_amend + PretInitialationTimerSingleT::Instance()->m_startMillis);
	return val;

#else
	return (static_cast<_int64>(times(&tm)) * (PretInitialationTimerSingleT::Instance()->m_InitialValue)) * ms_second;
#endif
#endif
}

tick_type::tick_type()
{
	reset(tick_policy::now());
}

tick_type::tick_type(_int64 val)
{
	reset(val);
}

void tick_type::reset(_int64 val)
{
	tk_microsec = static_cast<DWORD>(val % ms_second);
	tk_millisec = static_cast<DWORD>((val / ms_second) % ms_second);
	tk_sec = static_cast<DWORD>((val / us_second) % 60);
	tk_min = static_cast<DWORD>((val / us_minute) % 60);
	tk_hour = static_cast<DWORD>(val / us_hour);
}
	

formatted_ticker::VALUE_TYPE formatted_ticker::now()
{
	tick_type lstore;
	return lstore;
}

time_type::time_type()
{
	_int64 localtm = timer_policy::now();
	reset(localtm);
}

time_type::time_type(_int64 val)
{
	reset(val);
}
void time_type::reset(_int64 val)
{
	tm_microsec = static_cast<DWORD>(val % ms_second);
	tm_millisec = static_cast<DWORD>((val / ms_second) % ms_second);
	time_t timer_sec = static_cast<time_t>(val / us_second);
#ifdef QT_WIN32
	//it is not thread safe on windows
	struct tm *tm_v = localtime(&timer_sec);		
#else
	struct tm *tm_v, tmstore;
	tm_v = localtime_r(&timer_sec, &tmstore);
#endif
	if(tm_v)
	{
		tm_sec = tm_v->tm_sec;
		tm_min = tm_v->tm_min;
		tm_hour = tm_v->tm_hour;
		tm_mday = tm_v->tm_mday;
		tm_mon = tm_v->tm_mon + 1;
		tm_year = tm_v->tm_year + 1900;
		tm_wday = tm_v->tm_wday;
		tm_yday = tm_v->tm_yday;
		tm_isdst = tm_v->tm_isdst;		
	}
	else
	{
		tm_sec = 0;
		tm_min = 0;
		tm_hour = 0;
		tm_mday = 0;
		tm_mon = 0;
		tm_year = 0;
		tm_wday = 0;
		tm_yday = 0;
		tm_isdst = 0;		

	}
}

formatted_timer::VALUE_TYPE formatted_timer::now()
{
	time_type lstore;
	return lstore;
}

///

CQtMutexThread low_tick_policy::low_tick_generator::mutex;
low_tick_policy::low_tick_generator *low_tick_policy::low_tick_generator::pInstance = NULL;
BOOL low_tick_policy::low_tick_generator::m_InitTimerSucc = TRUE;

void low_tick_policy::low_tick_generator::OnTimer(CQtTimerWrapperID* aId)
{
	m_tickNow = tick_policy::now();
}
_int64 low_tick_policy::low_tick_generator::now()
{
	QT_ASSERTE_RETURN(m_InitTimerSucc, tick_policy::now());
	return m_tickNow;
}
low_tick_policy::low_tick_generator::low_tick_generator()
{
	m_tickNow = tick_policy::now();
	QtResult rv = m_RefreshTimer.Schedule(this, CQtTimeValue(0, MIN_INTERVAL));
	QT_ASSERTE(QT_OK==rv);
	if(QT_FAILED(rv))
		m_InitTimerSucc = FALSE;
}

low_tick_policy::low_tick_generator *low_tick_policy::low_tick_generator::instance()
{
	if(!pInstance)
	{
		CQtMutexGuardT<CQtMutexThread> guard(mutex);
		if(!pInstance)
			pInstance = new low_tick_generator();
	}
	return pInstance;
}

_int64 low_tick_policy::now()
{
	return low_tick_generator::instance()->now();
}
