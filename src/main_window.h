#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "d2_image_drawer.h"
#include "d2_text_writer.h"
#include "mf_media_player.h"
#include "view_manager.h"
#include "sngk_scene_crafter.h"
#include "win_clock.h"
#include "native-ui/font_setting_dialogue.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();

	bool create(HINSTANCE hInstance, const wchar_t* windowName, HICON hIcon = nullptr);
	int messageLoop();

	HWND getHwnd()const { return m_hWnd; }
private:
	const wchar_t* m_className = L"SengokuKoihime player window";
	const wchar_t* m_defaultWindowName = L"";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT onCreate(HWND hWnd);
	LRESULT onDestroy();
	LRESULT onClose();
	LRESULT onPaint();
	LRESULT onSize();
	LRESULT onKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT onKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT onCommand(WPARAM wParam, LPARAM lParam);
	LRESULT onTimer(WPARAM wParam);
	LRESULT onMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT onMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT onLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT onLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT onRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT onMButtonUp(WPARAM wParam, LPARAM lParam);

	struct Menu
	{
		enum
		{
			kOpenFolder = 1,
			kAudioSetting, kFontSetting,
			kPauseImage
		};
	};
	struct MenuBar { enum { kFolder, kSetting, kImage }; };

	struct MouseState
	{
		bool wasLeftPressed = false;
		bool hasLeftBeenDragged = false;
		bool wasLeftCombined = false;
		bool wasRightCombined = false;
		/// @brief Last mouse position in client coördinate
		POINT lastMousePos{};
	};

	MouseState m_mouseState;

	HMENU m_hMenuBar = nullptr;
	bool m_isMenuBarHidden = false;
	bool m_isTextHidden = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	void initialiseMenuBar();

	void menuOnOpen();
	void menuOnNextFolder();
	void menuOnForeFolder();

	void menuOnAudioSetting();
	void menuOnFontSetting();

	void menuOnPauseImage();

	void changeWindowTitle(const wchar_t* windowTitle);
	void toggleWindowBorderStyle();

	bool createFolderList(const std::wstring& folderPath);
	void setupScenario(const std::wstring& folderPath);

	void updateScreen() const;

	CD2ImageDrawer* m_pD2ImageDrawer = nullptr;
	CD2TextWriter* m_pD2TextWriter = nullptr;
	CMfMediaPlayer* m_pAudioPlayer = nullptr;
	CViewManager* m_pViewManager = nullptr;
	CSngkSceneCrafter* m_pSngkSceneCrafter = nullptr;

	CFontSettingDialogue* m_pFontSettingDialogue = nullptr;

	CWinClock m_textClock;

	void checkTextClock();
	void shiftText(bool forward);
	void updateText();
	void autoTexting();
};

#endif //MAIN_WINDOW_H_