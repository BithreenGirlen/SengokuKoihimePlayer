#ifndef VIEW_MANAGER_H_
#define VIEW_MANAGER_H_

#include <Windows.h>

class CViewManager
{
public:
    CViewManager(HWND hWnd);
    ~CViewManager();

    void SetBaseSize(unsigned int uiWidth, unsigned int uiHeight);
    void Rescale(bool bUpscale);
    void SetOffset(int iX, int iY);
    void ResetZoom();
    void OnStyleChanged();

    float GetScale() const { return m_fScale; };
    float GetXOffset() const { return m_fXOffset; };
    float GetYOffset() const { return m_fYOffset; };
private:
    enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

    HWND m_hRetWnd = nullptr;

    unsigned int m_uiBaseWidth = Constants::kBaseWidth;
    unsigned int m_uiBaseHeight = Constants::kBaseHeight;
    float m_fDefaultScale = 1.f;

    float m_fScale = 1.f;
    float m_fXOffset = 0;
    float m_fYOffset = 0;

    void WorkOutDefaultScale();
    void ResizeWindow();
    void AdjustOffset();
    void RequestRedraw();
};

#endif // !VIEW_MANAGER_H_
