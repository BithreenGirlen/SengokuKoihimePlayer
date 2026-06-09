#ifndef D2_IMAGE_DRAWER_H_
#define D2_IMAGE_DRAWER_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d3d11.h>

class CD2ImageDrawer
{
public:
	CD2ImageDrawer(HWND hWnd);
	~CD2ImageDrawer();

	void clear(const D2D1::ColorF& colour = D2D1::ColorF(255, 255, 255, 255));
	bool draw(ID2D1Bitmap* pD2d1Bitmap, const D2D1_POINT_2F* targetOffset = nullptr, const D2D1_RECT_F* srcRect = nullptr);
	void display();

	ID2D1Factory1* const getD2Factory()const { return m_pD2d1Factory1; }
	ID2D1DeviceContext* const getD2DeviceContext()const { return m_pD2d1DeviceContext; }
private:
	HRESULT m_hrComInit = E_FAIL;
	ID2D1Factory1* m_pD2d1Factory1 = nullptr;
	ID2D1DeviceContext* m_pD2d1DeviceContext = nullptr;
	IDXGISwapChain1* m_pDxgiSwapChain1 = nullptr;

	unsigned int m_bufferWidth = 0;
	unsigned int m_bufferHeight = 0;

	bool checkBufferSize();
	bool resizeBuffer();
};

#endif // !D2_IMAGE_DRAWER_H_
