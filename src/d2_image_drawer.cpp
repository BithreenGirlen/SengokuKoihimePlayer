
#include <atlbase.h>

#include "d2_image_drawer.h"

#pragma comment (lib,"D2d1.lib")
#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"dxguid.lib")

CD2ImageDrawer::CD2ImageDrawer(HWND hWnd)
	:m_hRetWnd(hWnd)
{
	m_hrComInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(m_hrComInit))return;

	CComPtr<ID3D11Device>pD3d11Device;
	HRESULT hr = ::D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED, nullptr, 0, D3D11_SDK_VERSION,
		&pD3d11Device, nullptr, nullptr);
	if (FAILED(hr))return;

	CComPtr<IDXGIDevice1> pDxgDevice1;
	hr = pD3d11Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDxgDevice1);
	if (FAILED(hr))return;

	hr = pDxgDevice1->SetMaximumFrameLatency(1);
	if (FAILED(hr))return;

	CComPtr<IDXGIAdapter> pDxgiAdapter;
	hr = pDxgDevice1->GetAdapter(&pDxgiAdapter);
	if (FAILED(hr))return;

	CComPtr<IDXGIFactory2> pDxgiFactory2;
	hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory2));
	if (FAILED(hr))return;

	hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2d1Factory1);
	if (FAILED(hr))return;

	CComPtr<ID2D1Device> pD2d1Device;
	hr = m_pD2d1Factory1->CreateDevice(pDxgDevice1, &pD2d1Device);
	if (FAILED(hr))return;

	hr = pD2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2d1DeviceContext);
	if (FAILED(hr))return;

	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 1;
	desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	hr = pDxgiFactory2->CreateSwapChainForHwnd(pDxgDevice1, m_hRetWnd, &desc, nullptr, nullptr, &m_pDxgiSwapChain1);
	if (FAILED(hr))return;

	m_pD2d1DeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	m_pD2d1DeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
	m_pD2d1DeviceContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);

	D2D1_RENDERING_CONTROLS sRenderings{};
	m_pD2d1DeviceContext->GetRenderingControls(&sRenderings);

	sRenderings.bufferPrecision = D2D1_BUFFER_PRECISION_8BPC_UNORM_SRGB;
	m_pD2d1DeviceContext->SetRenderingControls(sRenderings);
}

CD2ImageDrawer::~CD2ImageDrawer()
{
	ReleaseBitmap();

	if (m_pDxgiSwapChain1 != nullptr)
	{
		m_pDxgiSwapChain1->Release();
		m_pDxgiSwapChain1 = nullptr;
	}

	if (m_pD2d1DeviceContext != nullptr)
	{
		m_pD2d1DeviceContext->Release();
		m_pD2d1DeviceContext = nullptr;
	}

	if (m_pD2d1Factory1 != nullptr)
	{
		m_pD2d1Factory1->Release();
		m_pD2d1Factory1 = nullptr;
	}

	if (SUCCEEDED(m_hrComInit))
	{
		::CoUninitialize();
	}
}

/*画面消去*/
void CD2ImageDrawer::Clear(const D2D1::ColorF& colour)
{
	if (m_pD2d1DeviceContext != nullptr)
	{
		bool bRet = CheckBufferSize();
		if (!bRet)return;
		m_pD2d1DeviceContext->BeginDraw();
		m_pD2d1DeviceContext->Clear(colour);
		m_pD2d1DeviceContext->EndDraw();
	}
}
/*画像描画*/
bool CD2ImageDrawer::Draw(const SImageFrame& imageFrame, const D2D_VECTOR_2F fOffset, float fScale)
{
	if ( m_pD2d1DeviceContext == nullptr || m_pDxgiSwapChain1 == nullptr)
	{
		return false;
	}

	if (imageFrame.uiWidth == 0 && imageFrame.uiHeight == 0)return false;

	bool bRet = CheckBitmapSize(imageFrame.uiWidth, imageFrame.uiHeight);
	if (!bRet)return false;

	bRet = CheckBufferSize();
	if (!bRet)return false;

	const SImageFrame& s = imageFrame;
	HRESULT hr = E_FAIL;
	UINT uiWidth = s.uiWidth;
	UINT uiHeight = s.uiHeight;
	INT iStride = s.iStride;

	D2D1_RECT_U rc = { 0, 0, uiWidth, uiHeight };
	hr = m_pD2d1Bitmap->CopyFromMemory(&rc, s.pixels.data(), s.iStride);
	if (SUCCEEDED(hr))
	{
		CComPtr<ID2D1Effect> pD2d1Effect;
		hr = m_pD2d1DeviceContext->CreateEffect(CLSID_D2D1Scale, &pD2d1Effect);

		pD2d1Effect->SetInput(0, m_pD2d1Bitmap);
		hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_CENTER_POINT, fOffset);
		hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(fScale, fScale));

		m_pD2d1DeviceContext->BeginDraw();
		m_pD2d1DeviceContext->DrawImage(pD2d1Effect, D2D1::Point2F(0.f, 0.f), D2D1::RectF(fOffset.x, fOffset.y, uiWidth * fScale, uiHeight * fScale), D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC, D2D1_COMPOSITE_MODE_SOURCE_OVER);
		m_pD2d1DeviceContext->EndDraw();
	}

	return SUCCEEDED(hr);
}
/*描画*/
bool CD2ImageDrawer::Draw(ID2D1Bitmap* pD2d1Bitmap, const D2D_VECTOR_2F fOffset, float fScale)
{
	if (pD2d1Bitmap == nullptr)return false;
	HRESULT hr = E_FAIL;

	D2D1_SIZE_U s = pD2d1Bitmap->GetPixelSize();
	bool bRet = CheckBitmapSize(s.width, s.height);
	if (!bRet)return false;

	CComPtr<ID2D1Effect> pD2d1Effect;
	hr = m_pD2d1DeviceContext->CreateEffect(CLSID_D2D1Scale, &pD2d1Effect);

	pD2d1Effect->SetInput(0, pD2d1Bitmap);
	hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_CENTER_POINT, fOffset);
	hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(fScale, fScale));

	m_pD2d1DeviceContext->BeginDraw();
	m_pD2d1DeviceContext->DrawImage(pD2d1Effect, D2D1::Point2F(0.f, 0.f), D2D1::RectF(fOffset.x, fOffset.y, s.width * fScale, s.height * fScale), D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC, D2D1_COMPOSITE_MODE_SOURCE_OVER);
	m_pD2d1DeviceContext->EndDraw();

	return SUCCEEDED(hr);
}
/*転写*/
void CD2ImageDrawer::Display()
{
	if (m_pDxgiSwapChain1 != nullptr)
	{
		DXGI_PRESENT_PARAMETERS params{};
		m_pDxgiSwapChain1->Present1(1, 0, &params);
	}
}
/*複写枠解放*/
void CD2ImageDrawer::ReleaseBitmap()
{
	if (m_pD2d1Bitmap != nullptr)
	{
		m_pD2d1Bitmap->Release();
		m_pD2d1Bitmap = nullptr;
	}
}
/*複写枠寸法確認*/
bool CD2ImageDrawer::CheckBitmapSize(unsigned long uiWidth, unsigned long uiHeight)
{
	if (m_pD2d1Bitmap == nullptr)
	{
		return CreateBitmapForDrawing(uiWidth, uiHeight);
	}
	else
	{
		const D2D1_SIZE_U& uBitmapSize = m_pD2d1Bitmap->GetPixelSize();
		if (uiWidth > uBitmapSize.width && uiHeight > uBitmapSize.height)
		{
			return CreateBitmapForDrawing(uiWidth, uiHeight);
		}
		else
		{
			return true;
		}
	}
	return false;
}
/*複写枠作成*/
bool CD2ImageDrawer::CreateBitmapForDrawing(unsigned long uiWidth, unsigned long uiHeight)
{
	ReleaseBitmap();

	HRESULT hr = m_pD2d1DeviceContext->CreateBitmap(D2D1::SizeU(uiWidth, uiHeight),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
		&m_pD2d1Bitmap);

	return SUCCEEDED(hr);
}
/*原版寸法確認*/
bool CD2ImageDrawer::CheckBufferSize()
{
	RECT rc;
	::GetClientRect(m_hRetWnd, &rc);

	unsigned int uiWidth = rc.right - rc.left;
	unsigned int uiHeight = rc.bottom - rc.top;

	if (m_uiWindowWidth != uiWidth || m_uiWindowHeight != uiHeight)
	{
		m_uiWindowWidth = uiWidth;
		m_uiWindowHeight = uiHeight;
		return ResizeBuffer();
	}
	else
	{
		return true;
	}
	return false;
}
/*原版寸法変更*/
bool CD2ImageDrawer::ResizeBuffer()
{
	if (m_pDxgiSwapChain1 != nullptr && m_pD2d1DeviceContext != nullptr && m_hRetWnd != nullptr)
	{
		m_pD2d1DeviceContext->SetTarget(nullptr);

		HRESULT hr = m_pDxgiSwapChain1->ResizeBuffers(0, m_uiWindowWidth, m_uiWindowHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0);

		CComPtr<IDXGISurface> pDxgiSurface;
		hr = m_pDxgiSwapChain1->GetBuffer(0, IID_PPV_ARGS(&pDxgiSurface));

		CComPtr<ID2D1Bitmap1> pD2d1Bitmap1;
		hr = m_pD2d1DeviceContext->CreateBitmapFromDxgiSurface(pDxgiSurface, nullptr, &pD2d1Bitmap1);

		m_pD2d1DeviceContext->SetTarget(pD2d1Bitmap1);
		return SUCCEEDED(hr);
	}
	return false;
}
