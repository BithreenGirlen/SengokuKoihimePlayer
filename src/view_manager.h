#ifndef VIEW_MANAGER_H_
#define VIEW_MANAGER_H_

#include <Windows.h>

class CViewManager
{
public:
    CViewManager(HWND hWnd);
    ~CViewManager();

    void setBaseSize(unsigned int uiWidth, unsigned int uiHeight);
    void rescale(bool toUpscale);
    void addOffset(int iX, int iY);
    void resetZoom();
    void onStyleChanged();

    float getScale() const { return m_fScale; };
    float getOffsetX() const { return m_fOffsetX; };
    float getOffsetY() const { return m_fOffsetY; };
private:
    enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

    HWND m_hRetWnd = nullptr;

    unsigned int m_uiBaseWidth = Constants::kBaseWidth;
    unsigned int m_uiBaseHeight = Constants::kBaseHeight;
    float m_fDefaultScale = 1.f;

    float m_fScale = 1.f;
    float m_fOffsetX = 0;
    float m_fOffsetY = 0;

    void workOutDefaultScale();
    void resizeWindow();
    void adjustOffset();
    void requestRedraw() const;
};

#endif // !VIEW_MANAGER_H_
