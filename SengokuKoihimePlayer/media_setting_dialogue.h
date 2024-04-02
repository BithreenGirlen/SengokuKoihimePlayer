#ifndef MEDIA_SETTING_DIALOGUE_H_
#define MEDIA_SETTING_DIALOGUE_H_

#include <Windows.h>

class CMediaSettingDialogue
{
public:
	CMediaSettingDialogue();
	~CMediaSettingDialogue();
	bool Open(HINSTANCE hInstance, HWND hWnd, void* pMediaPlayer, const wchar_t* pwzWindowName);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd; }
private:
	const wchar_t* m_swzClassName = L"Media player setting dialogue";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;
	HWND m_hParentWnd = nullptr;
	void* m_pMediaPlayer = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnVScroll(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam);

	enum Constants { kFontSize = 16, kTextWidth = 70};
	enum Controls{kVolumeSlider = 1, kRateSkuder};
	HFONT m_hFont = nullptr;
	HWND m_hVolumeSlider = nullptr;
	HWND m_hVolumeText = nullptr;
	HWND m_hRateSlider = nullptr;
	HWND m_hRateText = nullptr;

	void CreateSliders();
	void SetSliderPosition();
	void GetClientAreaSize(long* width, long* height);
	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);
};

#endif //MEDIA_SETTING_DIALOGUE_H_
