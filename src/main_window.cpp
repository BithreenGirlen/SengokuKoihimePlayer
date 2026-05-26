
#include "main_window.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "native-ui/media_setting_dialogue.h"
#include "native-ui/window_menu.h"


CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::create(HINSTANCE hInstance, const wchar_t* windowName, HICON hIcon)
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
	wcex.lpszClassName = m_className;
	if (hIcon != nullptr)
	{
		wcex.hIcon = hIcon;
		wcex.hIconSm = hIcon;
	}

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;
		if (windowName != nullptr)m_defaultWindowName = windowName;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		m_hWnd = ::CreateWindowW(m_className, windowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			return true;
		}
	}

	return false;
}

int CMainWindow::messageLoop()
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
			return static_cast<int>(msg.wParam);
		}
		else
		{
			return -1;
		}
	}
	return 0;
}
/* C CALLBACK */
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
/* メッセージ処理 */
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return onCreate(hWnd);
	case WM_DESTROY:
		return onDestroy();
	case WM_CLOSE:
		return onClose();
	case WM_PAINT:
		return onPaint();
	case WM_ERASEBKGND:
		return 1;
	case WM_KEYDOWN:
		return onKeyDown(wParam, lParam);
	case WM_KEYUP:
		return onKeyUp(wParam, lParam);
	case WM_COMMAND:
		return onCommand(wParam, lParam);
	case WM_TIMER:
		return onTimer(wParam);
	case WM_MOUSEMOVE:
		return onMouseMove(wParam, lParam);
	case WM_MOUSEWHEEL:
		return onMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return onLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return onLButtonUp(wParam, lParam);
	case WM_RBUTTONUP:
		return onRButtonUp(wParam, lParam);
	case WM_MBUTTONUP:
		return onMButtonUp(wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/* WM_CREATE */
LRESULT CMainWindow::onCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	initialiseMenuBar();

	m_pD2ImageDrawer = new CD2ImageDrawer(m_hWnd);

	m_pAudioPlayer = new CMfMediaPlayer();

	m_pD2TextWriter = new CD2TextWriter(m_pD2ImageDrawer->getD2Factory(), m_pD2ImageDrawer->getD2DeviceContext());
	m_pD2TextWriter->setupOutLinedDrawing(L"C:\\Windows\\Fonts\\yumindb.ttf");

	m_pViewManager = new CViewManager(m_hWnd);

	m_pSngkSceneCrafter = new CSngkSceneCrafter(m_pD2ImageDrawer->getD2DeviceContext());

	m_pFontSettingDialogue = new CFontSettingDialogue();

	return 0;
}
/* WM_DESTROY */
LRESULT CMainWindow::onDestroy()
{
	::PostQuitMessage(0);

	return 0;
}
/* WM_CLOSE */
LRESULT CMainWindow::onClose()
{
	if (m_pFontSettingDialogue != nullptr)
	{
		if (m_pFontSettingDialogue->getHwnd() != nullptr)
		{
			::SendMessage(m_pFontSettingDialogue->getHwnd(), WM_CLOSE, 0, 0);
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
	::UnregisterClassW(m_className, m_hInstance);

	return 0;
}
/* WM_PAINT */
LRESULT CMainWindow::onPaint()
{
	PAINTSTRUCT ps{};
	HDC hDC = ::BeginPaint(m_hWnd, &ps);

	if (m_pD2ImageDrawer == nullptr || m_pD2TextWriter == nullptr || m_pViewManager == nullptr || m_pSngkSceneCrafter == nullptr || !m_pSngkSceneCrafter->hasScenarioData())
	{
		::EndPaint(m_hWnd, &ps);
		return 0;
	}

	m_pD2ImageDrawer->clear();

	ID2D1Bitmap* pD2d1Bitmap = m_pSngkSceneCrafter->getCurrentImage();
	if (pD2d1Bitmap != nullptr)
	{
		RECT rc;
		::GetClientRect(m_hWnd, &rc);

		int targetWidth = rc.right - rc.left;
		int targetHeight = rc.bottom - rc.top;

		D2D1_SIZE_U srcSize = pD2d1Bitmap->GetPixelSize();

		const float fScale = m_pViewManager->getScale();
		const float fX = (srcSize.width * fScale - targetWidth) / 2 + m_pViewManager->offsetX() / 2;
		const float fY = (srcSize.height * fScale - targetHeight) / 2 + m_pViewManager->offsetY() / 2;

		const D2D1_MATRIX_3X2_F scaleMatrix = D2D1::Matrix3x2F::Scale(fScale, fScale);
		const D2D1_MATRIX_3X2_F translateMatrix = D2D1::Matrix3x2F::Translation(-fX, -fY);
		const D2D1_MATRIX_3X2_F transformMatrix = scaleMatrix * translateMatrix;

		m_pD2ImageDrawer->getD2DeviceContext()->SetTransform(transformMatrix);
		m_pD2ImageDrawer->draw(pD2d1Bitmap);
		m_pD2ImageDrawer->getD2DeviceContext()->SetTransform(D2D1::Matrix3x2F::Identity());
	}

	if (!m_isTextHidden)
	{
		const std::wstring& wstr = m_pSngkSceneCrafter->getCurrentFormattedText();
		m_pD2TextWriter->outLinedDraw(wstr.c_str(), static_cast<unsigned long>(wstr.size()));
	}
	m_pD2ImageDrawer->display();

	updateScreen();

	checkTextClock();

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/* WM_SIZE */
LRESULT CMainWindow::onSize()
{
	return 0;
}
/* WM_KEYDOWN */
LRESULT CMainWindow::onKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_RIGHT:
		autoTexting();
		break;
	case VK_LEFT:
		shiftText(false);
		break;
	default:

		break;
	}

	return 0;
}
/* WM_KEYUP */
LRESULT CMainWindow::onKeyUp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_ESCAPE:
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		break;
	case VK_UP:
		menuOnForeFolder();
		break;
	case VK_DOWN:
		menuOnNextFolder();
		break;
	case 'C':
		if (m_pD2TextWriter != nullptr)
		{
			m_pD2TextWriter->toggleTextColour();
		}
		break;
	case 'T':
		m_isTextHidden ^= true;
		break;
	}
	return 0;
}
/* WM_COMMAND */
LRESULT CMainWindow::onCommand(WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int msgSource = LOWORD(lParam);
	if (msgSource == 0)
	{
		/* Menus */
		switch (id)
		{
		case Menu::kOpenFolder:
			menuOnOpen();
			break;
		case Menu::kAudioSetting:
			menuOnAudioSetting();
			break;
		case Menu::kFontSetting:
			menuOnFontSetting();
			break;
		case Menu::kPauseImage:
			menuOnPauseImage();
			break;
		}
	}
	else
	{
		/* Controls */
	}

	return 0;
}
/* WM_TIMER */
LRESULT CMainWindow::onTimer(WPARAM wParam)
{
	return 0;
}
/* WM_MOUSEMOVE */
LRESULT CMainWindow::onMouseMove(WPARAM wParam, LPARAM lParam)
{
	WORD pressedKey = LOWORD(wParam);
	if (pressedKey == MK_LBUTTON)
	{
		POINT pt{};
		::GetCursorPos(&pt);

		if (m_mouseState.hasLeftBeenDragged)
		{
			if (m_pViewManager != nullptr)
			{
				int iX = m_mouseState.lastMousePos.x - pt.x;
				int iY = m_mouseState.lastMousePos.y - pt.y;

				m_pViewManager->addOffset(iX, iY);
				updateScreen();
			}
		}

		m_mouseState.lastMousePos = pt;
		m_mouseState.hasLeftBeenDragged = true;
	}

	return 0;
}
/* WM_MOUSEWHEEL */
LRESULT CMainWindow::onMouseWheel(WPARAM wParam, LPARAM lParam)
{
	short scroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
	WORD pressedKey = LOWORD(wParam);

	if (pressedKey == 0)
	{
		if (m_pViewManager != nullptr)
		{
			m_pViewManager->rescale(scroll > 0);
		}
	}
	else if (pressedKey == MK_LBUTTON)
	{
		if (m_pSngkSceneCrafter != nullptr)
		{
			m_pSngkSceneCrafter->updateAnimationInterval(scroll > 0);
		}

		m_mouseState.wasLeftCombined = true;
	}
	else if (pressedKey == MK_RBUTTON)
	{
		shiftText(scroll > 0);

		m_mouseState.wasRightCombined = true;
	}

	return 0;
}
/* WM_LBUTTONDOWN */
LRESULT CMainWindow::onLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::GetCursorPos(&m_mouseState.lastMousePos);

	/* In case menu item is selected, WM_LBUTTONUP happens, but WM_LBUTTONDOWN not. */
	m_mouseState.wasLeftPressed = true;

	return 0;
}
/* WM_LBUTTONUP */
LRESULT CMainWindow::onLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_mouseState.wasLeftCombined)
	{
		m_mouseState.hasLeftBeenDragged = false;
		m_mouseState.wasLeftCombined = false;
		m_mouseState.wasLeftPressed = false;

		return 0;
	}

	WORD pressedKey = LOWORD(wParam);

	if (pressedKey == MK_RBUTTON && m_isMenuBarHidden)
	{
		::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
		INPUT input{};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_DOWN;
		::SendInput(1, &input, sizeof(input));
	}
	else if (pressedKey == 0 && m_mouseState.wasLeftPressed)
	{
		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_mouseState.lastMousePos.x - pt.x;
		int iY = m_mouseState.lastMousePos.y - pt.y;

		if (iX == 0 && iY == 0)
		{
			if (m_pSngkSceneCrafter != nullptr)
			{
				if (m_pSngkSceneCrafter->isPaused())
				{
					m_pSngkSceneCrafter->shiftAnimation();
				}
			}
		}

		m_mouseState.lastMousePos = pt;
	}

	m_mouseState.wasLeftPressed = false;

	return 0;
}
/* WM_RBUTTONUP */
LRESULT CMainWindow::onRButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_mouseState.wasRightCombined)
	{
		m_mouseState.wasRightCombined = false;
		return 0;
	}

	WORD pressedKey = LOWORD(wParam);
	if (pressedKey == 0 && m_pSngkSceneCrafter != nullptr && m_pSngkSceneCrafter->hasScenarioData())
	{
		const auto& labelData = m_pSngkSceneCrafter->getLabelData();
		HMENU hPopupMenu = ::CreatePopupMenu();
		if (hPopupMenu != nullptr)
		{
			for (size_t i = 0; i < labelData.size(); ++i)
			{
				::AppendMenuW(hPopupMenu, MF_STRING, i + 1, labelData[i].caption.c_str());
			}

			POINT point{};
			::GetCursorPos(&point);
			BOOL menuIndex = ::TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, 0, m_hWnd, nullptr);
			if (menuIndex > 0)
			{
				size_t labelIndex = static_cast<size_t>(menuIndex - 1);
				m_pSngkSceneCrafter->jumpToLabel(labelIndex);

				updateText();
			}

			::DestroyMenu(hPopupMenu);
		}
	}

	return 0;
}
/* WM_MBUTTONUP */
LRESULT CMainWindow::onMButtonUp(WPARAM wParam, LPARAM lParam)
{
	WORD pressedKey = LOWORD(wParam);
	if (pressedKey == 0)
	{
		if (m_pViewManager != nullptr)
		{
			m_pViewManager->resetScale();
		}

		if (m_pSngkSceneCrafter != nullptr)
		{
			m_pSngkSceneCrafter->resetAnimationInterval();
		}
	}
	else if (pressedKey == MK_RBUTTON)
	{
		toggleWindowBorderStyle();

		m_mouseState.wasRightCombined = true;
	}

	return 0;
}
/* 操作欄作成 */
void CMainWindow::initialiseMenuBar()
{
	if (m_hMenuBar != nullptr)return;

	HMENU hMenu = window_menu::MenuBuilder(
		{
			{0, L"Folder", window_menu::MenuBuilder(
				{
					{ Menu::kOpenFolder, L"Open"},
				}).get()
			},
			{0, L"Setting", window_menu::MenuBuilder(
				{
					{ Menu::kAudioSetting, L"Audio"},
					{ Menu::kFontSetting, L"Font"}
				}).get()
			},
			{0, L"Image", window_menu::MenuBuilder(
				{
					{ Menu::kPauseImage, L"Pause"},
				}).get()
			}
		}
	).get();

	if (::IsMenu(hMenu))
	{
		if (::SetMenu(m_hWnd, hMenu))
		{
			m_hMenuBar = hMenu;
		}
		else
		{
			::DestroyMenu(hMenu);
		}
	}
}
/* フォルダ選択 */
void CMainWindow::menuOnOpen()
{
	std::wstring selectedFolderPath = win_dialogue::SelectFolder(L"Select stillAnimation/st_XXXXXXXX folder", m_hWnd);
	if (!selectedFolderPath.empty())
	{
		setupScenario(selectedFolderPath.c_str());
		createFolderList(selectedFolderPath.c_str());
	}
}
/* 次フォルダに移動 */
void CMainWindow::menuOnNextFolder()
{
	if (m_folders.empty())return;

	++m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = 0;

	setupScenario(m_folders[m_nFolderIndex]);
}
/* 前フォルダに移動 */
void CMainWindow::menuOnForeFolder()
{
	if (m_folders.empty())return;

	--m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = m_folders.size() - 1;

	setupScenario(m_folders[m_nFolderIndex]);
}
/* 音量・再生速度変更 */
void CMainWindow::menuOnAudioSetting()
{
	CMediaSettingDialogue mediaSettingDialogue;
	mediaSettingDialogue.open(m_hInstance, m_hWnd, m_pAudioPlayer, L"Audio", reinterpret_cast<HICON>(::GetClassLongPtr(m_hWnd, GCLP_HICON)));
}
/* 書体設定 */
void CMainWindow::menuOnFontSetting()
{
	if (m_pFontSettingDialogue != nullptr)
	{
		if (m_pFontSettingDialogue->getHwnd() == nullptr)
		{
			HWND hWnd = m_pFontSettingDialogue->open(m_hInstance, m_hWnd, L"Font", m_pD2TextWriter);
			::SendMessage(hWnd, WM_SETICON, ICON_SMALL, ::GetClassLongPtr(m_hWnd, GCLP_HICON));
			::ShowWindow(hWnd, SW_SHOWNORMAL);
		}
		else
		{
			::SetFocus(m_pFontSettingDialogue->getHwnd());
		}
	}
}
/* 一時停止 */
void CMainWindow::menuOnPauseImage()
{
	if (m_pSngkSceneCrafter != nullptr)
	{
		HMENU hMenuBar = ::GetMenu(m_hWnd);
		if (hMenuBar != nullptr)
		{
			HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kImage);
			if (hMenu != nullptr)
			{
				bool wasPaused = m_pSngkSceneCrafter->isPaused();
				m_pSngkSceneCrafter->setPause(!wasPaused);
				::CheckMenuItem(hMenu, Menu::kPauseImage, wasPaused ? MF_CHECKED : MF_UNCHECKED);
			}
		}
	}
}
/* 表題変更 */
void CMainWindow::changeWindowTitle(const wchar_t* windowTitle)
{
	const wchar_t* truncatedWindowTitle = windowTitle;
	if (truncatedWindowTitle != nullptr)
	{
		for (;;)
		{
			const wchar_t* pPos = wcspbrk(truncatedWindowTitle, L"\\/");
			if (pPos == nullptr)break;
			truncatedWindowTitle = pPos + 1;
		}
	}

	::SetWindowTextW(m_hWnd, truncatedWindowTitle == nullptr ? m_defaultWindowName : truncatedWindowTitle);
}
/* 表示形式変更 */
void CMainWindow::toggleWindowBorderStyle()
{
	if (m_pSngkSceneCrafter == nullptr || !m_pSngkSceneCrafter->hasScenarioData())return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_isMenuBarHidden ^= true;

	if (m_isMenuBarHidden)
	{
		MONITORINFO monitorInfo{ .cbSize = sizeof(MONITORINFO) };
		if (HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST); hMonitor != nullptr)
		{
			[[maybe_unused]] BOOL iRet = ::GetMonitorInfoW(hMonitor, &monitorInfo);
		}

		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU);
		::SetWindowPos(m_hWnd, nullptr, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		::SetMenu(m_hWnd, nullptr);
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU);
		::SetMenu(m_hWnd, m_hMenuBar);
	}

	if (m_pViewManager != nullptr)
	{
		m_pViewManager->onStyleChanged();
	}
}
/* フォルダ一覧表作成 */
bool CMainWindow::createFolderList(const std::wstring& stillFolderPath)
{
	m_folders.clear();
	m_nFolderIndex = 0;
	win_filesystem::GetFilePathListAndIndex(stillFolderPath, {}, m_folders, m_nFolderIndex);

	return m_folders.size() > 0;
}
/* 寸劇構築 */
void CMainWindow::setupScenario(const std::wstring& stillFolderPath)
{
	if (m_pSngkSceneCrafter == nullptr)return;

	bool bRet = m_pSngkSceneCrafter->loadScenario(stillFolderPath.data());
	if (bRet)
	{
		m_textClock.restart();

		unsigned int uiWidth = 0;
		unsigned int uiHeight = 0;
		m_pSngkSceneCrafter->getCurrentImageSize(&uiWidth, &uiHeight);

		if (m_pViewManager != nullptr)
		{
			m_pViewManager->setBaseSize(uiWidth, uiHeight);
			m_pViewManager->resetScale();
		}

		updateText();
	}

	changeWindowTitle(m_pSngkSceneCrafter->hasScenarioData() ? stillFolderPath.data() : nullptr);
}
/* 再描画要求 */
void CMainWindow::updateScreen() const
{
	::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/* 文章表示経過時間確認 */
void CMainWindow::checkTextClock()
{
	if (m_pAudioPlayer != nullptr)
	{
		float fElapsed = m_textClock.getElapsedTime();
		if (::isgreaterequal(fElapsed, 2.f))
		{
			m_textClock.restart();
			if (m_pAudioPlayer->isEnded())
			{
				autoTexting();
			}
		}
	}
}
/* 文章送り・戻し */
void CMainWindow::shiftText(bool forward)
{
	if (m_pSngkSceneCrafter != nullptr)
	{
		m_pSngkSceneCrafter->shiftScene(forward);
		updateText();
	}
}
/* 文章更新 */
void CMainWindow::updateText()
{
	if (m_pSngkSceneCrafter != nullptr)
	{
		if (m_pAudioPlayer != nullptr)
		{
			const wchar_t* pwzVoiceFilePath = m_pSngkSceneCrafter->getCurrentVoiceFilePath();
			if (pwzVoiceFilePath != nullptr && *pwzVoiceFilePath != L'\0')
			{
				m_pAudioPlayer->play(pwzVoiceFilePath);
			}
		}
	}
}
/* 自動送り */
void CMainWindow::autoTexting()
{
	if (m_pSngkSceneCrafter != nullptr)
	{
		if (!m_pSngkSceneCrafter->hasReachedLastScene())
		{
			shiftText(true);
		}
	}
}
