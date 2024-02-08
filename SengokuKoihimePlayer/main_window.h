#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>

#include "scene_player.h"
#include "media_player.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();
	bool Create(HINSTANCE hInstance);
	int MessageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	std::wstring m_class_name = L"SengokuKoihime player window";
	std::wstring m_window_name = L"SengokuKoihime player";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	enum Menu{kOpen = 1, kNextFolder, kForeFolder,
		kNextAudio, kBack, kPlay, kLoop, kVolume,
		kPauseImage,};
	enum MenuBar{kFolder, kAudio, kImage};
	POINT m_CursorPos{};
	bool m_bSpeedSet = false;

	HMENU m_hMenuBar = nullptr;
	bool m_bHideBar = false;
	bool m_bPlayReady = false;

	std::vector<std::wstring> m_folders;
	size_t m_nIndex = 0;

	void InitialiseMenuBar();

	void MenuOnOpen();
	void MenuOnNextFolder();
	void MenuOnForeFolder();

	void MenuOnNextAudio();
	void MenuOnBack();
	void MenuOnPlay();
	void MenuOnLoop();
	void MenuOnVolume();

	void MenuOnPauseImage();

	void ChangeWindowTitle(const wchar_t* pzTitle);
	void SwitchWindowMode();

	bool CreateFolderList(const wchar_t* pwzFolderPath);
	void SetPlayerFolder(const wchar_t* pwzFolderPath);
	void SetPlayFiles(const wchar_t* pwzImageFolderPath, const wchar_t* pwzAudioFolderPath);
	bool GetAudioFolderPath(const std::wstring& wstrImageFolderPath, std::wstring& wstrAudioFolderPath);

	CScenePlayer* m_pScenePlayer = nullptr;
	CMediaPlayer* m_pMediaPlayer = nullptr;
};

#endif //MAIN_WINDOW_H_