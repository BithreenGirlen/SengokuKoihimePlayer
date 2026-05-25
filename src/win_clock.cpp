

#include "win_clock.h"

CWinClock::CWinClock()
{
	Restart();
}

CWinClock::~CWinClock()
{

}

float CWinClock::GetElapsedTime()
{
	LARGE_INTEGER freq;
	::QueryPerformanceFrequency(&freq);

	LARGE_INTEGER nNow = GetNowCounter();
	return static_cast<float>(nNow.QuadPart - m_nLastCounter.QuadPart) / freq.QuadPart * 1000;
}

void CWinClock::Restart()
{
	m_nLastCounter = GetNowCounter();
}

LARGE_INTEGER CWinClock::GetNowCounter()
{
	LARGE_INTEGER ticks;
	::QueryPerformanceCounter(&ticks);
	return ticks;
}
