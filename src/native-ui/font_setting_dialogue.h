#ifndef FONT_SETTING_DIALOGUE_H_
#define FONT_SETTING_DIALOGUE_H_

#include <Windows.h>

#include "dialogue_controls.h"

class CFontSettingDialogue
{
public:
	CFontSettingDialogue();
	~CFontSettingDialogue();

	HWND open(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, void *pTextWriter);

	HWND getHwnd()const { return m_hWnd; }
private:
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT onInit(HWND hWnd);
	LRESULT onClose();
	LRESULT onSize();
	LRESULT onNotify(WPARAM wParam, LPARAM lParam);
	LRESULT onCommand(WPARAM wParam, LPARAM lParam);
	LRESULT onVScroll(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16 };
	enum Controls
	{
		kApplyButton = 1,
		kFontSizeSlider, kFontThicknessSlider,
		kBoldCheckButton, kItalicCheckButton
	};

	HFONT m_hFont = nullptr;

	CStatic m_fontNameStatic;
	CComboBox m_fontNameComboBox;

	CStatic m_fontSizeStatic;
	CSlider m_fontSizeSlider;

	CStatic m_fontThicknessStatic;
	CFloatSlider m_fontThicknessSlider;

	CButton m_boldCheckButton;
	CButton m_italicCheckButton;

	CButton m_applyButton;

	void resizeControls();

	void onApplyButton();

	void setSliderPosition();

	void* m_pTextWriter = nullptr;
};
#endif // !FONT_SETTING_DIALOGUE_H_
