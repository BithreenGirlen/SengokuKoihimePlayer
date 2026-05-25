#ifndef D2_IMAGE_DRAWER_H_
#define D2_IMAGE_DRAWER_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>
#include <d3d11.h>

#include "win_image.h"

class CD2ImageDrawer
{
public:
	CD2ImageDrawer(HWND hWnd);
	~CD2ImageDrawer();

	void Clear(const D2D1::ColorF &colour = D2D1::ColorF(255, 255, 255, 255));
	bool Draw(const SImageFrame &imageFrame, const D2D_VECTOR_2F fOffset = {0.f, 0.f}, float fScale = 1.f);
	bool Draw(ID2D1Bitmap* pD2d1Bitmap, const D2D_VECTOR_2F fOffset = { 0.f, 0.f }, float fScale = 1.f);
	void Display();

	ID2D1Factory1* GetD2Factory()const { return m_pD2d1Factory1; }
	ID2D1DeviceContext* GetD2DeviceContext()const { return m_pD2d1DeviceContext; }
private:
	HWND m_hRetWnd = nullptr;

	HRESULT m_hrComInit = E_FAIL;
	ID2D1Factory1* m_pD2d1Factory1 = nullptr;
	ID2D1DeviceContext* m_pD2d1DeviceContext = nullptr;
	IDXGISwapChain1* m_pDxgiSwapChain1 = nullptr;
	ID2D1Bitmap* m_pD2d1Bitmap = nullptr;

	unsigned int m_uiWindowWidth = 0;
	unsigned int m_uiWindowHeight = 0;

	void ReleaseBitmap();
	bool CheckBitmapSize(unsigned long uiWidth, unsigned long uiHeight);
	bool CreateBitmapForDrawing(unsigned long uiWidth, unsigned long uiHeight);
	bool CheckBufferSize();
	bool ResizeBuffer();
};

#endif // !D2_IMAGE_DRAWER_H_
