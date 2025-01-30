/*=========================  Media player setting dialogue  =========================
 * Dialogue-box-like behavior window; modal only.
 *===================================================================================*/

#include <Windows.h>
#include <CommCtrl.h>

#include <string>

#include "media_setting_dialogue.h"
#include "mf_media_player.h"
#include "Resource.h"

CMediaSettingDialogue::CMediaSettingDialogue()
{
    m_hFont = ::CreateFont(Constants::kFontSize, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"DFKai-SB");
}

CMediaSettingDialogue::~CMediaSettingDialogue()
{
    if (m_hFont != nullptr)
    {
        ::DeleteObject(m_hFont);
    }
}

bool CMediaSettingDialogue::Open(HINSTANCE hInstance, HWND hWnd, void* pMediaPlayer, const wchar_t* pwzWindowName)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_10535006));
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_ICON_10535006);
    wcex.lpszClassName = m_swzClassName;
    wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_10535006));

    if (::RegisterClassExW(&wcex))
    {
        m_hInstance = hInstance;
        m_hParentWnd = hWnd;
        m_pMediaPlayer = pMediaPlayer;

        UINT uiDpi = ::GetDpiForSystem();
        int iWindowWidth = ::MulDiv(100, uiDpi, USER_DEFAULT_SCREEN_DPI);
        int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

        m_hWnd = ::CreateWindowW(m_swzClassName, pwzWindowName, WS_OVERLAPPEDWINDOW & ~ WS_MINIMIZEBOX & ~ WS_MAXIMIZEBOX & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, hWnd, nullptr, hInstance, this);
        if (m_hWnd != nullptr)
        {
            MessageLoop();
            return true;
        }
        else
        {
            std::wstring wstrMessage = L"CreateWindowExW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
        }
    }
    else
    {
        std::wstring wstrMessage = L"RegisterClassW failed; code: " + std::to_wstring(::GetLastError());
        ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    }

	return false;
}

int CMediaSettingDialogue::MessageLoop()
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
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMediaSettingDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    case WM_SIZE:
        return OnSize();
    case WM_VSCROLL:
        return OnVScroll(wParam, lParam);
    case WM_COMMAND:
        return OnCommand(wParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMediaSettingDialogue::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    CreateSliders();
    m_hVolumeText = ::CreateWindowExW(0, WC_STATIC, L"Volume", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, m_hInstance, nullptr);
    m_hRateText = ::CreateWindowExW(0, WC_STATIC, L"Rate", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, m_hWnd, nullptr, m_hInstance, nullptr);

    ::ShowWindow(hWnd, SW_NORMAL);

    ::EnableWindow(m_hParentWnd, FALSE);

    ::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

    return 0;
}
/*WM_DESTROY*/
LRESULT CMediaSettingDialogue::OnDestroy()
{
    ::PostQuitMessage(0);
    return 0;
}
/*WM_CLOSE*/
LRESULT CMediaSettingDialogue::OnClose()
{
    ::EnableWindow(m_hParentWnd, TRUE);
    ::BringWindowToTop(m_hParentWnd);

    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_swzClassName, m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMediaSettingDialogue::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    ::EndPaint(m_hWnd, &ps);

    return 0;
}
/*WM_SIZE*/
LRESULT CMediaSettingDialogue::OnSize()
{
    long w, h;
    GetClientAreaSize(&w, &h);
    long x_space = w / 100 * 10;
    long y_space = h / 100;

    long lTextSpace = Constants::kFontSize;

    if (m_hVolumeSlider != nullptr)
    {
        ::MoveWindow(m_hVolumeSlider, x_space, y_space + lTextSpace, w / 2 - x_space * 2, h - y_space * 2 - lTextSpace, TRUE);
    }

    if (m_hVolumeText != nullptr)
    {
        ::MoveWindow(m_hVolumeText, x_space, y_space, Constants::kTextWidth, Constants::kFontSize, TRUE);
    }


    if (m_hRateSlider != nullptr)
    {
        ::MoveWindow(m_hRateSlider, w / 2 + x_space, y_space + lTextSpace, w / 2 - x_space * 2, h - y_space * 2 - lTextSpace, TRUE);
    }

    if (m_hRateText != nullptr)
    {
        ::MoveWindow(m_hRateText, w / 2 + x_space, y_space, Constants::kTextWidth, Constants::kFontSize, TRUE);
    }

    return 0;
}
/*WM_VSCROLL*/
LRESULT CMediaSettingDialogue::OnVScroll(WPARAM wParam, LPARAM lParam)
{
    CMfMediaPlayer* pPlayer = static_cast<CMfMediaPlayer*>(m_pMediaPlayer);
    if (pPlayer != nullptr)
    {
        HANDLE hScroll = reinterpret_cast<HANDLE>(lParam);

        if (hScroll == m_hVolumeSlider)
        {
            double dbVolume = ::SendMessage(m_hVolumeSlider, TBM_GETPOS, 0, 0) / 100.0;
            if (dbVolume != pPlayer->GetCurrentVolume())
            {
                pPlayer->SetCurrentVolume(dbVolume);
            }
        }

        if (hScroll == m_hRateSlider)
        {
            double dbRate = ::SendMessage(m_hRateSlider, TBM_GETPOS, 0, 0) / 10.0;
            if (dbRate != pPlayer->GetCurrentRate())
            {
                pPlayer->SetCurrentRate(dbRate);
            }
        }
    }

    return 0;
}
/*WM_COMMAND*/
LRESULT CMediaSettingDialogue::OnCommand(WPARAM wParam)
{
    int wmKind = HIWORD(wParam);
    int wmId = LOWORD(wParam);
    if (wmKind == 0)
    {
        /*Menus*/
    }
    if (wmKind > 1)
    {
        /*Controls*/
    }

    return 0;
}
/*音量調整・再生速度変更スライダ作成*/
void CMediaSettingDialogue::CreateSliders()
{
    m_hVolumeSlider = ::CreateWindowExW(0, TRACKBAR_CLASS, L"Volume Slider",
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_VERT | TBS_TOOLTIPS | TBS_BOTH,
        0, 0, 0, 0,
        m_hWnd, reinterpret_cast<HMENU>(Controls::kVolumeSlider), m_hInstance, nullptr);

    if (m_hVolumeSlider != nullptr)
    {
        ::SendMessage(m_hVolumeSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
        ::SendMessage(m_hVolumeSlider, TBM_SETPAGESIZE, TRUE, 20);
    }

    m_hRateSlider = ::CreateWindowExW(0, TRACKBAR_CLASS, L"Rate Slider",
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_VERT | TBS_TOOLTIPS | TBS_BOTH,
        0, 0, 0, 0,
        m_hWnd, reinterpret_cast<HMENU>(Controls::kRateSkuder), m_hInstance, nullptr);

    if (m_hRateSlider != nullptr)
    {
        ::SendMessage(m_hRateSlider, TBM_SETRANGE, TRUE, MAKELONG(5, 25));
        ::SendMessage(m_hRateSlider, TBM_SETPAGESIZE, TRUE, 1);
    }

    SetSliderPosition();

}
/*現在値取得・表示*/
void CMediaSettingDialogue::SetSliderPosition()
{
    CMfMediaPlayer* pPlayer = static_cast<CMfMediaPlayer*>(m_pMediaPlayer);
    if (pPlayer != nullptr)
    {
        if (m_hVolumeSlider != nullptr)
        {
            double dbVolume = pPlayer->GetCurrentVolume() * 100.0;
            ::SendMessage(m_hVolumeSlider, TBM_SETPOS, TRUE, static_cast<LPARAM>(dbVolume));
        }

        if (m_hRateSlider != nullptr)
        {
            double dbRate = pPlayer->GetCurrentRate() * 10.0;
            ::SendMessage(m_hRateSlider, TBM_SETPOS, TRUE, static_cast<LPARAM>(dbRate));
        }
    }
}
/*描画領域の大きさ取得*/
void CMediaSettingDialogue::GetClientAreaSize(long* width, long* height)
{
    RECT rect;
    ::GetClientRect(m_hWnd, &rect);
    *width = rect.right - rect.left;
    *height = rect.bottom - rect.top;
}
/*EnumChildWindows CALLBACK*/
BOOL CMediaSettingDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
    ::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
    /*TRUE: 続行, FALSE: 終了*/
    return TRUE;
}
