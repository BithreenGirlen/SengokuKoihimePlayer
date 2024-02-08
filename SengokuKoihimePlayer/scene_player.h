#ifndef SCENE_PLAYER_H_
#define SCENE_PLAYER_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d3d11.h>

#include <string>
#include <vector>

struct ImageInfo
{
    UINT uiWidth = 0;
    UINT uiHeight = 0;
    INT iStride = 0;
    std::vector<BYTE> pixels;
};

class CScenePlayer
{
public:
    CScenePlayer(HWND hWnd);
    ~CScenePlayer();

    bool SetFiles(const std::vector<std::wstring>& filePaths);
    bool DisplayImage();
    void Next();
    void UpScale();
    void DownScale();
    void ResetScale();
    void SwitchSizeLore(bool bBarHidden);
    void SetOffset(int iX, int iY);
    void SpeedUp();
    void SpeedDown();
    bool SwitchPause();
    void SetFirstScene(size_t nIndex) { m_nIndexOrigin = nIndex; }
private:

    HWND m_hRetWnd = nullptr;

    enum Portion { kInterval = 16, kAnimationLength = 7 };
    enum ImageSize{ kBaseWidth = 1280, kBaseHeight = 720, kAnimationWdith = 1024, kAnimationHeight = 576 };
    enum SceneIndex {kFirstAnimation = 1, kSecondAnimation = 8, kFin = 15 };

    HRESULT m_hrComInit = E_FAIL;
    ID2D1Factory1* m_pD2d1Factory1 = nullptr;
    ID2D1DeviceContext* m_pD2d1DeviceContext = nullptr;
    IDXGISwapChain1* m_pDxgiSwapChain1 = nullptr;
    ID2D1Bitmap* m_pD2d1Bitmap = nullptr;

    std::vector<ImageInfo> m_image_info;
    size_t m_nIndex = 0;
    size_t m_nIndexOrigin = 0;
    D2D_POINT_2U m_uiPortion{};
    size_t m_nAnimationIndex = 0;

    double m_dbScale = 1.0;
    int m_iXOffset = 0;
    int m_iYOffset = 0;
    long long m_interval = Portion::kInterval;

    bool m_bBarHidden = false;
    bool m_bPause = false;

    void Clear();
    bool LoadImageToMemory(const wchar_t* pwzFilePath);
    bool CreateBitmapForDrawing();
    void Update();
    void ResizeWindow();
    void AdjustOffset(int iXAddOffset, int iYAddOffset);
    void ResizeBuffer();
    void IncrementAnimationIndex();
    void ResetAnimationIndex();

    void StartThreadpoolTimer();
    void EndThreadpoolTimer();

    PTP_TIMER m_timer = nullptr;
    static void CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer);
};

#endif //SCENE_PLAYER_H_
