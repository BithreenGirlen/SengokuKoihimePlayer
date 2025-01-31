

#include "win_timer.h"

CWinTimer::CWinTimer()
{
	DEVMODE sDevMode{};
	::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &sDevMode);
	SetInterval(1000 / sDevMode.dmDisplayFrequency);
}

CWinTimer::~CWinTimer()
{
	End();
}

void CWinTimer::Start()
{
	if (m_pTpTimer != nullptr)return;

	m_pTpTimer = ::CreateThreadpoolTimer(TimerCallback, this, nullptr);
	if (m_pTpTimer != nullptr)
	{
		UpdateTimerInterval(m_pTpTimer);
	}
}

void CWinTimer::End()
{
	if (m_pTpTimer != nullptr)
	{
		::SetThreadpoolTimer(m_pTpTimer, nullptr, 0, 0);
		::WaitForThreadpoolTimerCallbacks(m_pTpTimer, TRUE);
		::CloseThreadpoolTimer(m_pTpTimer);
		m_pTpTimer = nullptr;
	}
}

void CWinTimer::SetCallback(void(*pFunc)(void*), void* pUserData)
{
	m_pCallback = pFunc;
	m_pUserData = pUserData;
}

void CWinTimer::SetDerfaultInterval(long long nInterval)
{
	m_nDefaultInterval = nInterval;
	m_nInterval = nInterval;
}

void CWinTimer::SetInterval(long long nInterval)
{
	m_nInterval = nInterval;
}

long long CWinTimer::GetInterval() const
{
	return m_nInterval;
}

void CWinTimer::ResetInterval()
{
	m_nInterval = m_nDefaultInterval;
}

void CWinTimer::UpdateTimerInterval(PTP_TIMER timer)
{
	if (timer != nullptr)
	{
		FILETIME sFileDueTime{};
		ULARGE_INTEGER ulDueTime{};
		ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * m_nInterval));
		sFileDueTime.dwHighDateTime = ulDueTime.HighPart;
		sFileDueTime.dwLowDateTime = ulDueTime.LowPart;
		::SetThreadpoolTimer(timer, &sFileDueTime, 0, 0);
	}
}

void CWinTimer::OnTide()
{
	if (m_pCallback != nullptr)
	{
		m_pCallback(m_pUserData);
	}
}

void CWinTimer::TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
	CWinTimer* pThis = static_cast<CWinTimer*>(Context);
	if (pThis != nullptr)
	{
		pThis->OnTide();
		pThis->UpdateTimerInterval(Timer);
	}
}
