#ifndef VIEW_MANAGER_H_
#define VIEW_MANAGER_H_

#include <Windows.h>

class CViewManager
{
public:
    CViewManager(HWND hWnd);
    ~CViewManager();

    void setBaseSize(unsigned int width, unsigned int height);
    void getBaseSize(unsigned int* width, unsigned int* height);

    void setScale(float fScale);
    float getScale() const;
    void rescale(bool upscale);

    void addOffset(int iX, int iY);
    float offsetX() const;
    float offsetY() const;

    void resetScale();

    void onStyleChanged();

private:
    enum Constants { kBaseWidth = 1280, kBaseHeight = 720 };

    HWND m_hRenderTargetWnd = nullptr;

    unsigned int m_baseWidth = Constants::kBaseWidth;
    unsigned int m_baseHeight = Constants::kBaseHeight;
    float m_fDefaultScale = 1.f;

    float m_fScale = 1.f;
    float m_fOffsetX = 0;
    float m_fOffsetY = 0;

    void workOutDefaultScale();
    void adjustOffset();

    void resizeWindow();
    void requestRedraw() const;
};

#endif // !VIEW_MANAGER_H_
