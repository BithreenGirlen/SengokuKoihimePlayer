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
	/// @brief Draw CPU pixel array
	bool draw(const void* srcData, const UINT32 width, const UINT32 height, const UINT32 stride, const D2D_VECTOR_2F& fOffset = { 0.f, 0.f }, float fScale = 1.f);
	/// @brief Drae GPU resource
	bool draw(ID2D1Bitmap* pD2d1Bitmap);
	void display();

	ID2D1Factory1* const getD2Factory()const { return m_pD2d1Factory1; }
	ID2D1DeviceContext* const getD2DeviceContext()const { return m_pD2d1DeviceContext; }
private:
	HRESULT m_hrComInit = E_FAIL;
	ID2D1Factory1* m_pD2d1Factory1 = nullptr;
	ID2D1DeviceContext* m_pD2d1DeviceContext = nullptr;
	IDXGISwapChain1* m_pDxgiSwapChain1 = nullptr;
	ID2D1Bitmap* m_pD2d1Bitmap = nullptr;

	unsigned int m_bufferWidth = 0;
	unsigned int m_bufferHeight = 0;

	void releaseBitmap();
	bool checkBitmapSize(unsigned long width, unsigned long height);
	bool createBitmapForDrawing(unsigned long width, unsigned long height);
	bool checkBufferSize();
	bool resizeBuffer();
};

#endif // !D2_IMAGE_DRAWER_H_
