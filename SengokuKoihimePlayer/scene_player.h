#ifndef SCENE_PLAYER_H_
#define SCENE_PLAYER_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dwrite_1.h>

#include <string>
#include <vector>

struct ImageInfo
{
    UINT uiWidth = 0;
    UINT uiHeight = 0;
    INT iStride = 0;
    std::vector<BYTE> pixels;
};

class CD2TextWriter;

class CScenePlayer
{
public:
    CScenePlayer(HWND hWnd);
    ~CScenePlayer();

    bool SetFiles(const std::vector<std::wstring>& filePaths);
    bool DrawImage();
    bool DrawString(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
    void Display();
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

    CD2TextWriter* m_pD2Text = nullptr;
};

class CD2TextWriter
{
public:
    CD2TextWriter(ID2D1Factory1* pD2d1Factory1, ID2D1DeviceContext* pD2d1DeviceContext);
    ~CD2TextWriter();

    bool SetupOutLinedDrawing(const wchar_t* pwzFontFilePath);

    void NoBorderDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
    void OutLinedDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
    void LayedOutDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
private:
    ID2D1Factory1* m_pStoredD2d1Factory1 = nullptr;
    ID2D1DeviceContext* m_pStoredD2d1DeviceContext = nullptr;

    IDWriteFactory* m_pDWriteFactory = nullptr;
    IDWriteTextFormat* m_pDWriteFormat = nullptr;
    IDWriteFontFace* m_pDWriteFontFace = nullptr;

    ID2D1SolidColorBrush* m_pD2d1SolidColorBrush = nullptr;
    ID2D1SolidColorBrush* m_pD2dSolidColorBrushForOutline = nullptr;

    const wchar_t* m_swzFontFamilyName = L"yumin";
    const float m_fStrokeWidth = 4.f;
    float m_fFontSize = 24.f;

    float PointSizeToDip(float fPointSize)const { return (fPointSize / 72.f) * 96.f; };

    bool SetupFont(const wchar_t* pwzFontFilePath);
    bool CreateBrushes();

    bool SingleLineGlyphDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_POINT_2F& fRawPos = D2D1_POINT_2F{});
};

#endif //SCENE_PLAYER_H_
