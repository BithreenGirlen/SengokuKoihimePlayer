
#include "main_window.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "media_setting_dialogue.h"


CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName, HICON hIcon)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszClassName = m_swzClassName;
	if (hIcon != nullptr)
	{
		wcex.hIcon = hIcon;
		wcex.hIconSm = hIcon;
	}

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;
		if (pwzWindowName != nullptr)m_wstrDefaultWindowName = pwzWindowName;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		m_hWnd = ::CreateWindowW(m_swzClassName, pwzWindowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			return true;
		}
		else
		{
			std::wstring wstrMessage = L"CreateWindowW failed; code: " + std::to_wstring(::GetLastError());
			::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
		}
	}
	else
	{
		std::wstring wstrMessage = L"RegisterClassExW failed; code: " + std::to_wstring(::GetLastError());
		::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
	}

	return false;
}

int CMainWindow::MessageLoop()
{
	MSG msg;

	for (;;)
	{
		BOOL iRet = ::GetMessageW(&msg, 0, 0, 0);
		if (iRet > 0)
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		else if (iRet == 0)
		{
			/*ループ終了*/
			return static_cast<int>(msg.wParam);
		}
		else
		{
			/*ループ異常*/
			std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
			::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
			return -1;
		}
	}
	return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMainWindow* pThis = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}

	pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return OnCreate(hWnd);
	case WM_DESTROY:
		return OnDestroy();
	case WM_CLOSE:
		return OnClose();
	case WM_PAINT:
		return OnPaint();
	case WM_ERASEBKGND:
		return 1;
	case WM_KEYDOWN:
		return OnKeyDown(wParam, lParam);
	case WM_KEYUP:
		return OnKeyUp(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_TIMER:
		return OnTimer(wParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, lParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
	case WM_MBUTTONUP:
		return OnMButtonUp(wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	InitialiseMenuBar();

	m_pD2ImageDrawer = new CD2ImageDrawer(m_hWnd);

	m_pAudioPlayer = new CMfMediaPlayer();

	m_pD2TextWriter = new CD2TextWriter(m_pD2ImageDrawer->GetD2Factory(), m_pD2ImageDrawer->GetD2DeviceContext());
	m_pD2TextWriter->SetupOutLinedDrawing(L"C:\\Windows\\Fonts\\yumindb.ttf");

	m_pViewManager = new CViewManager(m_hWnd);

	m_pSngkSceneCrafter = new CSngkSceneCrafter(m_pD2ImageDrawer->GetD2DeviceContext());

	m_pFontSettingDialogue = new CFontSettingDialogue();

	return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
	::PostQuitMessage(0);

	return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
	if (m_pFontSettingDialogue != nullptr)
	{
		if (m_pFontSettingDialogue->GetHwnd() != nullptr)
		{
			::SendMessage(m_pFontSettingDialogue->GetHwnd(), WM_CLOSE, 0, 0);
			delete m_pFontSettingDialogue;
			m_pFontSettingDialogue = nullptr;
		}
	}

	if (m_pSngkSceneCrafter != nullptr)
	{
		delete m_pSngkSceneCrafter;
		m_pSngkSceneCrafter = nullptr;
	}

	if (m_pViewManager != nullptr)
	{
		delete m_pViewManager;
		m_pViewManager = nullptr;
	}

	if (m_pD2TextWriter != nullptr)
	{
		delete m_pD2TextWriter;
		m_pD2TextWriter = nullptr;
	}

	if (m_pD2ImageDrawer != nullptr)
	{
		delete m_pD2ImageDrawer;
		m_pD2ImageDrawer = nullptr;
	}

	if (m_pAudioPlayer != nullptr)
	{
		delete m_pAudioPlayer;
		m_pAudioPlayer = nullptr;
	}

	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_swzClassName, m_hInstance);

	return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
	PAINTSTRUCT ps{};
	HDC hDC = ::BeginPaint(m_hWnd, &ps);

	if (m_pD2ImageDrawer == nullptr || m_pD2TextWriter == nullptr || m_pViewManager == nullptr || m_pSngkSceneCrafter == nullptr)
	{
		::EndPaint(m_hWnd, &ps);
		return 0;
	}

	bool bRet = false;

	m_pD2ImageDrawer->Clear();

	ID2D1Bitmap* pImage = m_pSngkSceneCrafter->GetCurrentImage();
	if (pImage != nullptr)
	{
		bRet = m_pD2ImageDrawer->Draw(pImage, { m_pViewManager->GetXOffset(), m_pViewManager->GetYOffset() }, m_pViewManager->GetScale());
	}

	if (bRet)
	{
		if (!m_bTextHidden)
		{
			const std::wstring wstr = m_pSngkSceneCrafter->GetCurrentFormattedText();
			m_pD2TextWriter->OutLinedDraw(wstr.c_str(), static_cast<unsigned long>(wstr.size()));
		}
		m_pD2ImageDrawer->Display();

		UpdateScreen();

		CheckTextClock();
	}

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
	return 0;
}
/*WM_KEYDOWN*/
LRESULT CMainWindow::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_RIGHT:
		AutoTexting();
		break;
	case VK_LEFT:
		ShiftText(false);
		break;
	default:

		break;
	}

	return 0;
}
/*WM_KEYUP*/
LRESULT CMainWindow::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_ESCAPE:
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		break;
	case VK_UP:
		MenuOnForeFolder();
		break;
	case VK_DOWN:
		MenuOnNextFolder();
		break;
	case 'C':
		if (m_pD2TextWriter != nullptr)
		{
			m_pD2TextWriter->SwitchTextColour();
		}
		break;
	case 'T':
		m_bTextHidden ^= true;
		break;
	}
	return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmKind = LOWORD(lParam);
	if (wmKind == 0)
	{
		/*Menus*/
		switch (wmId)
		{
		case Menu::kOpenFolder:
			MenuOnOpen();
			break;
		case Menu::kAudioSetting:
			MenuOnAudioSetting();
			break;
		case Menu::kFontSetting:
			MenuOnFontSetting();
			break;
		case Menu::kPauseImage:
			MenuOnPauseImage();
			break;
		}
	}
	else
	{
		/*Controls*/
	}

	return 0;
}
/*WM_TIMER*/
LRESULT CMainWindow::OnTimer(WPARAM wParam)
{
	return 0;
}
/*WM_MOUSEMOVE*/
LRESULT CMainWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);
	if (usKey == MK_LBUTTON)
	{
		if (m_bLeftCombinated)return 0;

		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_cursorPos.x - pt.x;
		int iY = m_cursorPos.y - pt.y;

		if (m_pViewManager != nullptr)
		{
			m_pViewManager->SetOffset(iX, iY);
		}

		m_cursorPos = pt;
		m_bLeftDragged = true;
	}

	return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
	WORD usKey = LOWORD(wParam);

	if (usKey == 0)
	{
		if (m_pViewManager != nullptr)
		{
			m_pViewManager->Rescale(iScroll > 0);
		}
	}

	if (usKey == MK_LBUTTON)
	{
		if (m_pSngkSceneCrafter != nullptr)
		{
			m_pSngkSceneCrafter->UpdateAnimationInterval(iScroll > 0);
		}
		m_bLeftCombinated = true;
	}

	if (usKey == MK_RBUTTON)
	{
		ShiftText(iScroll > 0);
	}

	return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::GetCursorPos(&m_cursorPos);

	m_bLeftDowned = true;

	return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_bLeftCombinated || m_bLeftDragged)
	{
		m_bLeftCombinated = false;
		m_bLeftDragged = false;
		m_bLeftDowned = false;
		return 0;
	}

	WORD usKey = LOWORD(wParam);

	if (usKey == MK_RBUTTON && m_bBarHidden)
	{
		::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
		INPUT input{};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_DOWN;
		::SendInput(1, &input, sizeof(input));
	}

	if (usKey == 0 && m_bLeftDowned)
	{
		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_cursorPos.x - pt.x;
		int iY = m_cursorPos.y - pt.y;

		if (iX == 0 && iY == 0)
		{
			if (m_pSngkSceneCrafter != nullptr)
			{
				if (m_pSngkSceneCrafter->IsPaused())
				{
					m_pSngkSceneCrafter->ShiftAnimation();
				}
			}
		}
	}

	m_bLeftDowned = false;

	return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);
	if (usKey == 0)
	{
		if (m_pViewManager != nullptr)
		{
			m_pViewManager->ResetZoom();
		}

		if (m_pSngkSceneCrafter != nullptr)
		{
			m_pSngkSceneCrafter->ResetAnimationInterval();
		}
	}

	if (usKey == MK_RBUTTON)
	{
		SwitchWindowMode();
	}

	return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
	HMENU hMenuFolder = nullptr;
	HMENU hMenuSetting = nullptr;
	HMENU kMenuImage = nullptr;
	HMENU hMenuBar = nullptr;
	BOOL iRet = FALSE;

	if (m_hMenuBar != nullptr)return;

	hMenuFolder = ::CreateMenu();
	if (hMenuFolder == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuFolder, MF_STRING, Menu::kOpenFolder, "Open");
	if (iRet == 0)goto failed;

	hMenuSetting = ::CreateMenu();
	if (hMenuSetting == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuSetting, MF_STRING, Menu::kAudioSetting, "Audio");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuSetting, MF_STRING, Menu::kFontSetting, "Font");
	if (iRet == 0)goto failed;

	kMenuImage = ::CreateMenu();
	iRet = ::AppendMenuA(kMenuImage, MF_STRING, Menu::kPauseImage, "Pause");
	if (iRet == 0)goto failed;

	hMenuBar = ::CreateMenu();
	if (hMenuBar == nullptr) goto failed;

	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFolder), "Folder");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuSetting), "Setting");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(kMenuImage), "Image");
	if (iRet == 0)goto failed;

	iRet = ::SetMenu(m_hWnd, hMenuBar);
	if (iRet == 0)goto failed;

	m_hMenuBar = hMenuBar;

	/*正常終了*/
	return;

failed:
	std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
	::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
	/*SetMenu成功後はウィンドウ破棄時に破棄されるが、今は紐づけ前なのでここで破棄する。*/
	if (hMenuFolder != nullptr)
	{
		::DestroyMenu(hMenuFolder);
	}
	if (hMenuSetting != nullptr)
	{
		::DestroyMenu(hMenuSetting);
	}
	if (kMenuImage != nullptr)
	{
		::DestroyMenu(kMenuImage);
	}
	if (hMenuBar != nullptr)
	{
		::DestroyMenu(hMenuBar);
	}
}
/*フォルダ選択*/
void CMainWindow::MenuOnOpen()
{
	std::wstring wstrFolder = win_dialogue::SelectWorkFolder(m_hWnd);
	if (!wstrFolder.empty())
	{
		SetupScenario(wstrFolder.c_str());
		CreateFolderList(wstrFolder.c_str());
	}
}
/*次フォルダに移動*/
void CMainWindow::MenuOnNextFolder()
{
	if (m_folders.empty())return;

	++m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = 0;
	SetupScenario(m_folders[m_nFolderIndex].c_str());
}
/*前フォルダに移動*/
void CMainWindow::MenuOnForeFolder()
{
	if (m_folders.empty())return;

	--m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = m_folders.size() - 1;
	SetupScenario(m_folders[m_nFolderIndex].c_str());
}
/*音量・再生速度変更*/
void CMainWindow::MenuOnAudioSetting()
{
	CMediaSettingDialogue sMediaSettingDialogue;
	sMediaSettingDialogue.Open(m_hInstance, m_hWnd, m_pAudioPlayer, L"Audio", reinterpret_cast<HICON>(::GetClassLongPtr(m_hWnd, GCLP_HICON)));
}
/*書体設定*/
void CMainWindow::MenuOnFontSetting()
{
	if (m_pFontSettingDialogue != nullptr)
	{
		if (m_pFontSettingDialogue->GetHwnd() == nullptr)
		{
			HWND hWnd = m_pFontSettingDialogue->Open(m_hInstance, m_hWnd, L"Font", m_pD2TextWriter);
			::SendMessage(hWnd, WM_SETICON, ICON_SMALL, ::GetClassLongPtr(m_hWnd, GCLP_HICON));
			::ShowWindow(hWnd, SW_SHOWNORMAL);
		}
		else
		{
			::SetFocus(m_pFontSettingDialogue->GetHwnd());
		}
	}
}
/*一時停止*/
void CMainWindow::MenuOnPauseImage()
{
	if (m_pSngkSceneCrafter != nullptr)
	{
		HMENU hMenuBar = ::GetMenu(m_hWnd);
		if (hMenuBar != nullptr)
		{
			HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kImage);
			if (hMenu != nullptr)
			{
				bool bRet = m_pSngkSceneCrafter->TogglePause();
				::CheckMenuItem(hMenu, Menu::kPauseImage, bRet ? MF_CHECKED : MF_UNCHECKED);
			}
		}
	}
}
/*表題変更*/
void CMainWindow::ChangeWindowTitle(const wchar_t* pzTitle)
{
	std::wstring wstr;
	if (pzTitle != nullptr)
	{
		std::wstring wstrTitle = pzTitle;
		size_t pos = wstrTitle.find_last_of(L"\\/");
		wstr = pos == std::wstring::npos ? wstrTitle : wstrTitle.substr(pos + 1);
	}

	::SetWindowTextW(m_hWnd, wstr.empty() ? m_wstrDefaultWindowName.c_str() : wstr.c_str());
}
/*表示形式変更*/
void CMainWindow::SwitchWindowMode()
{
	if (!m_bPlayReady)return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_bBarHidden ^= true;

	if (m_bBarHidden)
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU);
		::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		::SetMenu(m_hWnd, nullptr);
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU);
		::SetMenu(m_hWnd, m_hMenuBar);
	}

	if (m_pViewManager != nullptr)
	{
		m_pViewManager->OnStyleChanged();
	}
}
/*フォルダ一覧表作成*/
bool CMainWindow::CreateFolderList(const wchar_t* pwzFolderPath)
{
	if (pwzFolderPath == nullptr)return false;

	m_folders.clear();
	m_nFolderIndex = 0;
	win_filesystem::GetFilePathListAndIndex(pwzFolderPath, nullptr, m_folders, &m_nFolderIndex);

	return m_folders.size() > 0;
}
/*寸劇構築*/
void CMainWindow::SetupScenario(const wchar_t* pwzFolderPath)
{
	if (pwzFolderPath == nullptr)return;

	if (m_pSngkSceneCrafter != nullptr)
	{
		m_bPlayReady = m_pSngkSceneCrafter->LoadScenario(pwzFolderPath);
		if (m_bPlayReady)
		{
			m_textClock.Restart();

			unsigned int uiWidth = 0;
			unsigned int uiHeight = 0;
			m_pSngkSceneCrafter->GetCurrentImageSize(&uiWidth, &uiHeight);

			if (m_pViewManager != nullptr)
			{
				m_pViewManager->SetBaseSize(uiWidth, uiHeight);
				m_pViewManager->ResetZoom();
			}

			UpdateText();
		}
	}

	ChangeWindowTitle(m_bPlayReady ? pwzFolderPath : nullptr);
}
/*再描画要求*/
void CMainWindow::UpdateScreen() const
{
	::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/*文章表示経過時間確認*/
void CMainWindow::CheckTextClock()
{
	if (m_pAudioPlayer != nullptr)
	{
		float fElapsed = m_textClock.GetElapsedTime();
		if (::isgreaterequal(fElapsed, 2000.f))
		{
			m_textClock.Restart();
			if (m_pAudioPlayer->IsEnded())
			{
				AutoTexting();
			}
		}
	}
}
/*文章送り・戻し*/
void CMainWindow::ShiftText(bool bForward)
{
	if (m_pSngkSceneCrafter != nullptr)
	{
		m_pSngkSceneCrafter->ShiftScene(bForward);
		UpdateText();
	}
}
/*文章更新*/
void CMainWindow::UpdateText()
{
	if (m_pSngkSceneCrafter != nullptr)
	{
		if (m_pAudioPlayer != nullptr)
		{
			const wchar_t* pwzVoiceFilePath = m_pSngkSceneCrafter->GetCurrentVoiceFilePath();
			if (pwzVoiceFilePath != nullptr && *pwzVoiceFilePath != L'\0')
			{
				m_pAudioPlayer->Play(pwzVoiceFilePath);
			}
		}
	}
}
/*自動送り*/
void CMainWindow::AutoTexting()
{
	if (m_pSngkSceneCrafter != nullptr)
	{
		if (!m_pSngkSceneCrafter->HasReachedLastScene())
		{
			ShiftText(true);
		}
	}
}
