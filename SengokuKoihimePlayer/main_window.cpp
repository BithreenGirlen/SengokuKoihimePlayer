
#include <Windows.h>
#include <CommCtrl.h>


#include "main_window.h"
#include "win_dialogue.h"
#include "win_filesystem.h"
#include "sngk.h"
#include "media_setting_dialogue.h"
#include "Resource.h"


CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName)
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

int CMainWindow::MessageLoop()
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
    case WM_KEYUP:
        return OnKeyUp(wParam, lParam);
    case WM_COMMAND:
        return OnCommand(wParam);
    case WM_TIMER:
        return OnTimer(wParam);
    case WM_MOUSEWHEEL:
        return OnMouseWheel(wParam, lParam);
    case WM_LBUTTONDOWN:
        return OnLButtonDown(wParam, lParam);
    case WM_LBUTTONUP:
        return OnLButtonUp(wParam, lParam);
    case WM_MBUTTONUP:
        return OnMButtonUp(wParam, lParam);
    case EventMessage::kAudioPlayer:
        OnAudioPlayerEvent(static_cast<unsigned long>(lParam));
        break;
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    InitialiseMenuBar();

    m_pScenePlayer = new CScenePlayer(m_hWnd);

    m_pMfAudioPlayer = new CMfMediaPlayer();
    m_pMfAudioPlayer->SetPlaybackWindow(m_hWnd, EventMessage::kAudioPlayer);

    return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
    ::PostQuitMessage(0);

    if (m_pScenePlayer != nullptr)
    {
        delete m_pScenePlayer;
        m_pScenePlayer = nullptr;
    }

    if (m_pMfAudioPlayer != nullptr)
    {
        delete m_pMfAudioPlayer;
        m_pMfAudioPlayer = nullptr;
    }
    return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_swzClassName, m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
    PAINTSTRUCT ps{};
    ::BeginPaint(m_hWnd, &ps);

    if (m_pScenePlayer != nullptr)
    {
        m_pScenePlayer->DrawImage();
        if (!m_textData.empty() && !m_bTextHidden)
        {
            const adv::TextDatum& t = m_textData.at(m_nTextIndex);
            std::wstring wstr = t.wstrText + L"\r\n " + std::to_wstring(m_nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
            m_pScenePlayer->DrawString(wstr.c_str(), static_cast<unsigned long>(wstr.size()));
        }
        m_pScenePlayer->Display();
    }

    ::EndPaint(m_hWnd, &ps);

    return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
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
    case 0x54:// T key
        m_bTextHidden ^= true;
        UpdateScreen();
        break;
    }
    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam)
{
    int wmKind = HIWORD(wParam);
    int wmId = LOWORD(wParam);
    if (wmKind == 0)
    {
        /*Menus*/
        switch (wmId)
        {
        case Menu::kOpenFolder:
            MenuOnOpen();
            break;
        case Menu::kAudioLoop:
            MenuOnLoop();
            break;
        case Menu::kAudioSetting:
            MenuOnVolume();
            break;
        case Menu::kPauseImage:
            MenuOnPauseImage();
            break;
        }
    }
    if (wmKind > 1)
    {
        /*Controls*/
    }

    return 0;
}
/*WM_TIMER*/
LRESULT CMainWindow::OnTimer(WPARAM wParam)
{
    switch (wParam)
    {
    case Timer::kText:
        AutoTexting();
        break;
    default:
        break;
    }
    return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
    WORD wKey = LOWORD(wParam);

    if (wKey == 0 && m_pScenePlayer != nullptr)
    {
        if (iScroll > 0)
        {
            m_pScenePlayer->UpScale();
        }
        else
        {
            m_pScenePlayer->DownScale();
        }
    }

    if (wKey == MK_RBUTTON && m_pMfAudioPlayer != nullptr)
    {
        ShiftText(iScroll > 0);
    }

    if (wKey == MK_LBUTTON && m_pScenePlayer != nullptr)
    {
        if (iScroll > 0)
        {
            m_pScenePlayer->SpeedUp();
        }
        else
        {
            m_pScenePlayer->SpeedDown();
        }
        m_bSpeedSet = true;
    }

    return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    ::GetCursorPos(&m_CursorPos);

    return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (m_bSpeedSet == true)
    {
        m_bSpeedSet = false;
        return 0;
    }

    WORD usKey = LOWORD(wParam);

    if (usKey == MK_RBUTTON && m_bHideBar)
    {
        ::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_DOWN;
        ::SendInput(1, &input, sizeof(input));
    }

    if (usKey == 0 && m_pScenePlayer != nullptr)
    {
        POINT pt{};
        ::GetCursorPos(&pt);
        int iX = m_CursorPos.x - pt.x;
        int iY = m_CursorPos.y - pt.y;

        if (iX == 0 && iY == 0)
        {
            m_pScenePlayer->Next();
        }
        else
        {
            m_pScenePlayer->SetOffset(iX, iY);
        }

    }
    return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);
    if (usKey == 0 && m_pScenePlayer != nullptr)
    {
        m_pScenePlayer->ResetScale();
    }

    if (usKey == MK_RBUTTON && m_pScenePlayer != nullptr)
    {
        SwitchWindowMode();
    }

    return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
    HMENU hMenuFolder = nullptr;
    HMENU hMenuAudio = nullptr;
    HMENU kMenuImage = nullptr;
    HMENU hMenuBar = nullptr;
    BOOL iRet = FALSE;

    if (m_hMenuBar != nullptr)return;

    hMenuFolder = ::CreateMenu();
    if (hMenuFolder == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuFolder, MF_STRING, Menu::kOpenFolder, "Open");
    if (iRet == 0)goto failed;

    hMenuAudio = ::CreateMenu();
    if (hMenuAudio == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioLoop, "Loop");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuAudio, MF_STRING, Menu::kAudioSetting, "Setting");
    if (iRet == 0)goto failed;

    kMenuImage = ::CreateMenu();
    iRet = ::AppendMenuA(kMenuImage, MF_STRING, Menu::kPauseImage, "Pause");
    if (iRet == 0)goto failed;

    hMenuBar = ::CreateMenu();
    if (hMenuBar == nullptr) goto failed;

    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFolder), "Folder");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuAudio), "Audio");
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
    if (hMenuAudio != nullptr)
    {
        ::DestroyMenu(hMenuAudio);
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
    std::wstring wstrFolder = win_dialogue::SelectWorkFolder(nullptr);
    if (!wstrFolder.empty())
    {
        SetupScenarioResources(wstrFolder.c_str());
        CreateFolderList(wstrFolder.c_str());
    }
}
/*次フォルダに移動*/
void CMainWindow::MenuOnNextFolder()
{
    if (m_folders.empty())return;

    ++m_nIndex;
    if (m_nIndex >= m_folders.size())m_nIndex = 0;
    SetupScenarioResources(m_folders.at(m_nIndex).c_str());
}
/*前フォルダに移動*/
void CMainWindow::MenuOnForeFolder()
{
    if (m_folders.empty())return;

    --m_nIndex;
    if (m_nIndex >= m_folders.size())m_nIndex = m_folders.size() - 1;
    SetupScenarioResources(m_folders.at(m_nIndex).c_str());
}
/*音声ループ設定変更*/
void CMainWindow::MenuOnLoop()
{
    if (m_pMfAudioPlayer != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kAudio);
            if (hMenu != nullptr)
            {
                BOOL iRet = m_pMfAudioPlayer->SwitchLoop();
                ::CheckMenuItem(hMenu, Menu::kAudioLoop, iRet == TRUE ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}
/*音量・再生速度変更*/
void CMainWindow::MenuOnVolume()
{
    CMediaSettingDialogue sMediaSettingDialogue;
    sMediaSettingDialogue.Open(m_hInstance, m_hWnd, m_pMfAudioPlayer, L"Audio");
}
/*一時停止*/
void CMainWindow::MenuOnPauseImage()
{
    if (m_pScenePlayer != nullptr)
    {
        HMENU hMenuBar = ::GetMenu(m_hWnd);
        if (hMenuBar != nullptr)
        {
            HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kImage);
            if (hMenu != nullptr)
            {
                bool bRet = m_pScenePlayer->SwitchPause();
                ::CheckMenuItem(hMenu, Menu::kPauseImage, bRet ? MF_CHECKED : MF_UNCHECKED);
            }
        }
    }
}
/*標題変更*/
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

    m_bHideBar ^= true;

    if (m_bHideBar)
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

    if (m_pScenePlayer != nullptr)
    {
        m_pScenePlayer->SwitchSizeLore(m_bHideBar);
    }

}
/*フォルダ一覧表作成*/
bool CMainWindow::CreateFolderList(const wchar_t* pwzFolderPath)
{
    if (pwzFolderPath == nullptr)return false;

    m_folders.clear();
    m_nIndex = 0;
    win_filesystem::GetFilePathListAndIndex(pwzFolderPath, nullptr, m_folders, &m_nIndex);

    return m_folders.size() > 0;
}
/*再生素材ファイル一覧作成*/
void CMainWindow::SetupScenarioResources(const wchar_t* pwzFolderPath)
{
    if (pwzFolderPath == nullptr)return;

    SetImageList(pwzFolderPath);
    CreateMessageList(pwzFolderPath);
}
/*連続画像一覧設定*/
void CMainWindow::SetImageList(const wchar_t* pwzImageFolderPath)
{
    bool bRet = false;

    std::vector<std::wstring> imageFiles;
    bRet = win_filesystem::CreateFilePathList(pwzImageFolderPath, L".jpg", imageFiles);
    win_filesystem::CreateFilePathList(pwzImageFolderPath, L".png", imageFiles);
    if (bRet && m_pScenePlayer != nullptr)
    {
        /*
         * 名前順に整理しているので取得時の順番は{fin, output[1-14], wait1, wait2}となっているはず。
         * これを{wait2, output[1-14], fin, wait1}の順番に入れ換え、開始場面を"wait1"とする。
         */
        auto IsWait1 = [](const std::wstring& wstr)
            -> bool
            {
                return wcsstr(wstr.c_str(), L"wait1") != nullptr;
            };

        auto iter = std::find_if(imageFiles.begin(), imageFiles.end(), IsWait1);
        if (iter != imageFiles.end())
        {
            size_t nWait1Index = std::distance(imageFiles.begin(), iter);
            std::swap(imageFiles.at(0), imageFiles.at(nWait1Index));
            ++nWait1Index;
            if (nWait1Index < imageFiles.size())
            {
                std::swap(imageFiles.at(0), imageFiles.at(nWait1Index));
            }
            m_pScenePlayer->SetFirstScene(nWait1Index);
        }

        m_bPlayReady = m_pScenePlayer->SetFiles(imageFiles);
    }

    ChangeWindowTitle(m_bPlayReady ? pwzImageFolderPath : nullptr);
}
/*文章一覧作成*/
bool CMainWindow::CreateMessageList(const wchar_t* pwzFolderPath)
{
    m_textData.clear();
    m_nTextIndex = 0;
    sngk::SearchAndLoadScenarioFile(pwzFolderPath, m_textData);
    if (m_textData.empty())
    {
        std::wstring wstrAudioFolderPath;
        sngk::DeriveAudioFolderPathFromStillFolderPath(pwzFolderPath, wstrAudioFolderPath);
        if (!wstrAudioFolderPath.empty())
        {
            std::vector<std::wstring> audioFiles;
            win_filesystem::CreateFilePathList(wstrAudioFolderPath.c_str(), L".mp3", audioFiles);
            for (const std::wstring& audioFile : audioFiles)
            {
                m_textData.emplace_back(adv::TextDatum{ L"", audioFile });
            }
        }
    }

    UpdateText();

    return !m_textData.empty();
}
/*再描画要求*/
void CMainWindow::UpdateScreen()
{
    ::InvalidateRect(m_hWnd, nullptr, FALSE);
}
/*文章送り・戻し*/
void CMainWindow::ShiftText(bool bForward)
{
    if (bForward)
    {
        ++m_nTextIndex;
        if (m_nTextIndex >= m_textData.size())m_nTextIndex = 0;
    }
    else
    {
        --m_nTextIndex;
        if (m_nTextIndex >= m_textData.size())m_nTextIndex = m_textData.size() - 1;
    }
    UpdateText();
}
/*文章更新*/
void CMainWindow::UpdateText()
{
    if (!m_textData.empty())
    {
        const adv::TextDatum& t = m_textData.at(m_nTextIndex);
        if (!t.wstrVoicePath.empty())
        {
            if (m_pMfAudioPlayer != nullptr)
            {
                m_pMfAudioPlayer->Play(t.wstrVoicePath.c_str());
            }

            ::KillTimer(m_hWnd, Timer::kText);
        }
        else
        {
            constexpr unsigned int kTimerInterval = 2000;
            ::SetTimer(m_hWnd, Timer::kText, kTimerInterval, nullptr);
        }
    }

    UpdateScreen();
}
/*IMFMediaEngineNotify::EventNotify*/
void CMainWindow::OnAudioPlayerEvent(unsigned long ulEvent)
{
    switch (ulEvent)
    {
    case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:

        break;
    case MF_MEDIA_ENGINE_EVENT_ENDED:
        AutoTexting();
        break;
    default:
        break;
    }
}
/*自動送り*/
void CMainWindow::AutoTexting()
{
    if (m_nTextIndex < m_textData.size() - 1)ShiftText(true);
}
