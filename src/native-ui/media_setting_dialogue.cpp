/*=========================  Media player setting dialogue  =========================
 * Dialogue-box-like behavior window; modal only.
 *===================================================================================*/

#include "media_setting_dialogue.h"

#include "../mf_media_player.h"

CMediaSettingDialogue::CMediaSettingDialogue()
{
	int fontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFontW(fontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"DFKai-SB");
}

CMediaSettingDialogue::~CMediaSettingDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

bool CMediaSettingDialogue::open(HINSTANCE hInstance, HWND hWnd, void* pMediaPlayer, const wchar_t* windowName, HICON hIcon)
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
		m_pMediaPlayer = pMediaPlayer;

		UINT dpi = ::GetDpiForSystem();
		int windowWidth = ::MulDiv(100, dpi, USER_DEFAULT_SCREEN_DPI);
		int windowHeight = ::MulDiv(200, dpi, USER_DEFAULT_SCREEN_DPI);

		RECT rect{};
		::GetClientRect(hWnd, &rect);
		POINT parentClientPos{ rect.left, rect.top };
		::ClientToScreen(hWnd, &parentClientPos);

		m_hWnd = ::CreateWindowW(m_className, windowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			parentClientPos.x, parentClientPos.y, windowWidth, windowHeight, hWnd, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			messageLoop();
			return true;
		}
	}

	return false;
}

int CMediaSettingDialogue::messageLoop()
{
	MSG msg;

	for (;;)
	{
		BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
		if (bRet > 0)
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		else if (bRet == 0)
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
/*C CALLBACK*/
LRESULT CMediaSettingDialogue::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMediaSettingDialogue* pThis = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = reinterpret_cast<CMediaSettingDialogue*>(pCreateStruct->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}

	pThis = reinterpret_cast<CMediaSettingDialogue*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (pThis != nullptr)
	{
		return pThis->handleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMediaSettingDialogue::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	case WM_SIZE:
		return onSize();
	case WM_NOTIFY:
		return onNotify(wParam, lParam);
	case WM_COMMAND:
		return onCommand(wParam, lParam);
	case WM_VSCROLL:
		return onVScroll(wParam, lParam);
	default:
		break;
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMediaSettingDialogue::onCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	createSliders();
	m_volumeStatic.create(L"Volume", m_hWnd);
	m_rateStatic.create(L"Rate", m_hWnd);

	::ShowWindow(hWnd, SW_NORMAL);

	::EnableWindow(::GetWindow(m_hWnd, GW_OWNER), FALSE);

	const auto FontCallback = [](HWND hWnd, LPARAM lParam)
		-> BOOL
		{
			::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
			/* TRUE: 続行, FALSE: 終了 */
			return TRUE;
		};

	::EnumChildWindows(m_hWnd, FontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return 0;
}
/*WM_DESTROY*/
LRESULT CMediaSettingDialogue::onDestroy()
{
	::PostQuitMessage(0);
	return 0;
}
/*WM_CLOSE*/
LRESULT CMediaSettingDialogue::onClose()
{
	HWND hOwnerWnd = ::GetWindow(m_hWnd, GW_OWNER);
	::EnableWindow(hOwnerWnd, TRUE);
	::BringWindowToTop(hOwnerWnd);

	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_className, ::GetModuleHandleW(nullptr));

	return 0;
}
/*WM_PAINT*/
LRESULT CMediaSettingDialogue::onPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/*WM_SIZE*/
LRESULT CMediaSettingDialogue::onSize()
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);
	long clientWidth = rect.right - rect.left;
	long clientHeight = rect.bottom - rect.top;

	long spaceX = clientWidth / 96 * 10;
	long spaceY = clientHeight / 96;

	long textSpace = Constants::kFontSize;

	::MoveWindow(m_volumeIntSlider.getHwnd(), spaceX, spaceY + textSpace, clientWidth / 2 - spaceX * 2, clientHeight - spaceY * 2 - textSpace, TRUE);
	::MoveWindow(m_volumeStatic.getHwnd(), spaceX, spaceY, Constants::kTextWidth, Constants::kFontSize, TRUE);

	::MoveWindow(m_rateFloatSlider.getHwnd(), clientWidth / 2 + spaceX, spaceY + textSpace, clientWidth / 2 - spaceX * 2, clientHeight - spaceY * 2 - textSpace, TRUE);
	::MoveWindow(m_rateStatic.getHwnd(), clientWidth / 2 + spaceX, spaceY, Constants::kTextWidth, Constants::kFontSize, TRUE);

	return 0;
}
/*WM_NOTIFY*/
LRESULT CMediaSettingDialogue::onNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pNmhdr = reinterpret_cast<LPNMHDR>(lParam);
	if (pNmhdr != nullptr)
	{
		if (pNmhdr->code == TTN_NEEDTEXT)
		{
			if (pNmhdr->hwndFrom == m_rateFloatSlider.getToolTipHandle())
			{
				m_rateFloatSlider.onToolTipNeedText(reinterpret_cast<LPTOOLTIPTEXTW>(lParam));
			}
		}
	}
	return 0;
}
/*WM_COMMAND*/
LRESULT CMediaSettingDialogue::onCommand(WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int msgSource = LOWORD(lParam);
	if (msgSource == 0)
	{
		/*Menus*/
	}
	else
	{
		/*Controls*/
	}

	return 0;
}
/*WM_VSCROLL*/
LRESULT CMediaSettingDialogue::onVScroll(WPARAM wParam, LPARAM lParam)
{
	CMfMediaPlayer* pPlayer = static_cast<CMfMediaPlayer*>(m_pMediaPlayer);
	if (pPlayer != nullptr)
	{
		HANDLE hScroll = reinterpret_cast<HANDLE>(lParam);

		if (hScroll == m_volumeIntSlider.getHwnd())
		{
			double volume = m_volumeIntSlider.getPosition() / 100.0;
			if (volume != pPlayer->getCurrentVolume())
			{
				pPlayer->setCurrentVolume(volume);
			}
		}

		if (hScroll == m_rateFloatSlider.getHwnd())
		{
			double playbackRate = m_rateFloatSlider.getPosition();
			if (playbackRate != pPlayer->getCurrentRate())
			{
				pPlayer->setCurrentRate(playbackRate);
			}
		}
	}

	return 0;
}
/*音量調整・再生速度変更スライダ作成*/
void CMediaSettingDialogue::createSliders()
{
	m_volumeIntSlider.create(L"", m_hWnd, reinterpret_cast<HMENU>(Controls::kVolumeSlider), 0, 100, 20, true);
	m_rateFloatSlider.create(L"", m_hWnd, reinterpret_cast<HMENU>(Controls::kRateSkuder), 0.5f, 2.5f, 0.1f, 20, true);

	setSliderPosition();
}
/*現在値取得・表示*/
void CMediaSettingDialogue::setSliderPosition()
{
	CMfMediaPlayer* pPlayer = static_cast<CMfMediaPlayer*>(m_pMediaPlayer);
	if (pPlayer != nullptr)
	{
		if (m_volumeIntSlider.getHwnd() != nullptr)
		{
			double dbVolume = pPlayer->getCurrentVolume() * 100.0;
			m_volumeIntSlider.setPosition(static_cast<long long>(dbVolume));
		}

		if (m_rateFloatSlider.getHwnd() != nullptr)
		{
			double dbRate = pPlayer->getCurrentRate();
			m_rateFloatSlider.setPosition(static_cast<float>(dbRate));
		}
	}
}