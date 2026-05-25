#ifndef MEDIA_SETTING_DIALOGUE_H_
#define MEDIA_SETTING_DIALOGUE_H_

#include <Windows.h>

#include "dialogue_controls.h"

class CMediaSettingDialogue
{
public:
	CMediaSettingDialogue();
	~CMediaSettingDialogue();

	bool open(HINSTANCE hInstance, HWND hWnd, void* pMediaPlayer, const wchar_t* windowName, HICON hIcon = nullptr);

	HWND getHwnd()const { return m_hWnd; }
private:
	const wchar_t* m_className = L"Media player setting dialogue";
	HWND m_hWnd = nullptr;

	void* m_pMediaPlayer = nullptr;

	int messageLoop();
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT onCreate(HWND hWnd);
	LRESULT onDestroy();
	LRESULT onClose();
	LRESULT onPaint();
	LRESULT onSize();
	LRESULT onNotify(WPARAM wParam, LPARAM lParam);
	LRESULT onCommand(WPARAM wParam, LPARAM lParam);
	LRESULT onVScroll(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16, kTextWidth = 70 };
	enum Controls { kVolumeSlider = 1, kRateSkuder };
	HFONT m_hFont = nullptr;

	CSlider m_volumeIntSlider;
	CStatic m_volumeStatic;

	CFloatSlider m_rateFloatSlider;
	CStatic m_rateStatic;

	void createSliders();
	void setSliderPosition();
};

#endif //MEDIA_SETTING_DIALOGUE_H_
