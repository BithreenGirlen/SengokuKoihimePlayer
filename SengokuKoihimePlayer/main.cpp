

#include "framework.h"
#include "main_window.h"
#include "Resource.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    int iRet = 0;
    CMainWindow sMainWindow;
    bool bRet = sMainWindow.Create(hInstance, L"SengokuKoihime player", ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_10535006)));
    if (bRet)
    {
        ::ShowWindow(sMainWindow.GetHwnd(), nCmdShow);
        iRet = sMainWindow.MessageLoop();
    }

    return iRet;
}
