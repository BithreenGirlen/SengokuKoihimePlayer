

#include "win_clock.h"

CWinClock::CWinClock()
{
	::QueryPerformanceFrequency(&m_frequency);
	restart();
}

CWinClock::~CWinClock()
{

}

float CWinClock::getElapsedTime()
{
	LARGE_INTEGER nNow = getNowCounter();
	return static_cast<float>(nNow.QuadPart - m_nLastCounter.QuadPart) / m_frequency.QuadPart;
}

void CWinClock::restart()
{
	m_nLastCounter = getNowCounter();
}

LARGE_INTEGER CWinClock::getNowCounter()
{
	LARGE_INTEGER ticks;
	::QueryPerformanceCounter(&ticks);
	return ticks;
}
