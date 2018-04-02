
#include "QtBase.h"
#include "QtTimeValue.h"

const CQtTimeValue CQtTimeValue::s_tvZero;
const CQtTimeValue CQtTimeValue::s_tvMax(LONG_MAX, QT_ONE_SECOND_IN_USECS-1);

void CQtTimeValue::Normalize()
{
//	m_lSec += m_lUsec / QT_ONE_SECOND_IN_USECS;
//	m_lUsec %= QT_ONE_SECOND_IN_USECS;
	if (m_lUsec >= QT_ONE_SECOND_IN_USECS) {
		do {
			m_lSec++;
			m_lUsec -= QT_ONE_SECOND_IN_USECS;
		}
		while (m_lUsec >= QT_ONE_SECOND_IN_USECS);
	}
	else if (m_lUsec <= -QT_ONE_SECOND_IN_USECS) {
		do {
			m_lSec--;
			m_lUsec += QT_ONE_SECOND_IN_USECS;
		}
		while (m_lUsec <= -QT_ONE_SECOND_IN_USECS);
	}

	if (m_lSec >= 1 && m_lUsec < 0) {
		m_lSec--;
		m_lUsec += QT_ONE_SECOND_IN_USECS;
	}
	else if (m_lSec < 0 && m_lUsec > 0) {
		m_lSec++;
		m_lUsec -= QT_ONE_SECOND_IN_USECS;
	}
}

#ifdef QT_WIN32
CQtTimeValue CQtTimeValue::GetTimeOfDay()
{
	FILETIME tfile;
	::GetSystemTimeAsFileTime(&tfile);

	ULARGE_INTEGER _100ns;
	_100ns.LowPart = tfile.dwLowDateTime;
	_100ns.HighPart = tfile.dwHighDateTime;
	_100ns.QuadPart -= (DWORDLONG)0x19db1ded53e8000;
	return CQtTimeValue((long)(_100ns.QuadPart / (10000 * 1000)), 
					  (long)((_100ns.QuadPart % (10000 * 1000)) / 10));
}
#endif // QT_WIN32

