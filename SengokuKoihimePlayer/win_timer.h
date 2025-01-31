#ifndef WIN_TIMER_H_
#define WIN_TIMER_H_

#include <Windows.h>

class CWinTimer
{
public:
	CWinTimer();
	~CWinTimer();

	void Start();
	void End();

	void SetCallback(void (*pFunc)(void*), void *pUserData);
	void SetDerfaultInterval(long long nInterval);

	void SetInterval(long long nInterval);
	long long GetInterval() const;
	void ResetInterval();
private:
	enum Constants {kDefaultInterval = 16};

	long long m_nDefaultInterval = Constants::kDefaultInterval;
	long long m_nInterval = Constants::kDefaultInterval;
	PTP_TIMER m_pTpTimer = nullptr;

	void (*m_pCallback)(void*) = nullptr;
	void* m_pUserData = nullptr;

	void UpdateTimerInterval(PTP_TIMER timer);
	void OnTide();
	static void CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer);
};
#endif // !WIN_TIMER_H_
