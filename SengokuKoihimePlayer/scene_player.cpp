

#include "scene_player.h"

#include <math.h>
#include <shobjidl.h>
#include <atlbase.h>
#include <wincodec.h>

CScenePlayer::CScenePlayer(HWND hWnd)
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

	hr = pDxgiFactory2->CreateSwapChainForHwnd(pDxgDevice1, hWnd, &desc, nullptr, nullptr, &m_pDxgiSwapChain1);
	if (FAILED(hr))return;

	m_pD2d1DeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	m_pD2d1DeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
	m_pD2d1DeviceContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
	D2D1_RENDERING_CONTROLS sRenderings{};
	m_pD2d1DeviceContext->GetRenderingControls(&sRenderings);
	sRenderings.bufferPrecision = D2D1_BUFFER_PRECISION_8BPC_UNORM_SRGB;
	m_pD2d1DeviceContext->SetRenderingControls(sRenderings);

}

CScenePlayer::~CScenePlayer()
{
	EndThreadpoolTimer();

	if (m_pD2Text != nullptr)
	{
		delete m_pD2Text;
		m_pD2Text = nullptr;
	}

	Clear();

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
/*ファイル設定*/
bool CScenePlayer::SetFiles(const std::vector<std::wstring>& filePaths)
{
	Clear();
	for (const std::wstring& filePath : filePaths)
	{
		LoadImageToMemory(filePath.c_str());
	}
	ResetScale();
	if (m_nIndexOrigin < m_image_info.size())
	{
		m_nIndex = m_nIndexOrigin;
	}
	return CreateBitmapForDrawing();
}
/*描画*/
bool CScenePlayer::DrawImage()
{
	if (m_image_info.empty() || m_nIndex >= m_image_info.size() || m_pD2d1DeviceContext == nullptr || m_pDxgiSwapChain1 == nullptr || m_pD2d1Bitmap == nullptr)
	{
		return false;
	}

	ImageInfo s = m_image_info.at(m_nIndex);

	HRESULT hr = E_FAIL;
	UINT uiWidth = s.uiWidth;
	UINT uiHeight = s.uiHeight;
	INT iStride = s.iStride;

	UINT uiDivX = s.uiWidth / ImageSize::kBaseWidth;
	UINT uiDivY = s.uiHeight / ImageSize::kBaseHeight;
	if (uiDivX > 1 && uiDivY > 2)
	{
		StartThreadpoolTimer();

		uiHeight = s.uiHeight / uiDivY;
		uiWidth = s.uiWidth / uiDivX;
		iStride = s.iStride / uiDivX;
		D2D1_RECT_U rc = { 0, 0, uiWidth, uiHeight };
		hr = m_pD2d1Bitmap->CopyFromMemory(&rc, s.pixels.data() + iStride * m_uiPortion.x + s.iStride * m_uiPortion.y * uiHeight, s.iStride);
		if (!m_bPause)
		{
			IncrementAnimationIndex();
		}
	}
	else
	{
		D2D1_RECT_U rc = { 0, 0, uiWidth, uiHeight };
		hr = m_pD2d1Bitmap->CopyFromMemory(&rc, s.pixels.data(), s.iStride);
	}

	if (SUCCEEDED(hr))
	{
		FLOAT fScale = static_cast<FLOAT>(::round(m_dbScale * 1000) / 1000);

		CComPtr<ID2D1Effect> pD2d1Effect;
		hr = m_pD2d1DeviceContext->CreateEffect(CLSID_D2D1Scale, &pD2d1Effect);
		pD2d1Effect->SetInput(0, m_pD2d1Bitmap);
		hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_CENTER_POINT, D2D1::Vector2F(static_cast<FLOAT>(m_iXOffset), static_cast<FLOAT>(m_iYOffset)));
		hr = pD2d1Effect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(fScale, fScale));
		m_pD2d1DeviceContext->BeginDraw();
		m_pD2d1DeviceContext->DrawImage(pD2d1Effect, D2D1::Point2F(0.f, 0.f), D2D1::RectF(static_cast<FLOAT>(m_iXOffset), static_cast<FLOAT>(m_iYOffset), uiWidth * fScale, uiHeight * fScale), D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC, D2D1_COMPOSITE_MODE_SOURCE_COPY);
		m_pD2d1DeviceContext->EndDraw();
	}

	return true;
}
/*文字列描画*/
bool CScenePlayer::DrawString(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pD2d1DeviceContext == nullptr || m_pDxgiSwapChain1 == nullptr)
	{
		return false;
	}

	if (m_pD2Text == nullptr)
	{
		m_pD2Text = new CD2TextWriter(m_pD2d1Factory1, m_pD2d1DeviceContext);
		m_pD2Text->SetupOutLinedDrawing(L"C:\\Windows\\Fonts\\yumindb.ttf");
	}

	m_pD2d1DeviceContext->BeginDraw();
	if (m_pD2Text != nullptr)
	{
		RECT rc;
		::GetClientRect(m_hRetWnd, &rc);

		unsigned int uiWidth = rc.right - rc.left;
		unsigned int uiHeight = rc.bottom - rc.top;

		D2D1_RECT_F fRect = { 0.f, 0.f, static_cast<float>(uiWidth), uiHeight / 4.f };
		m_pD2Text->OutLinedDraw(wszText, ulTextLength, fRect);
	}
	m_pD2d1DeviceContext->EndDraw();
	return false;
}
/*転写*/
void CScenePlayer::Display()
{
	if (m_pDxgiSwapChain1 != nullptr && m_pD2d1Bitmap != nullptr)
	{
		DXGI_PRESENT_PARAMETERS params{};
		m_pDxgiSwapChain1->Present1(1, 0, &params);
	}
}
/*次画像*/
void CScenePlayer::Next()
{
	if (m_nIndex < SceneIndex::kFirstAnimation)
	{
		EndThreadpoolTimer();
		m_nIndex = SceneIndex::kFirstAnimation;
	}
	else if (m_nIndex >= SceneIndex::kFirstAnimation && m_nIndex < SceneIndex::kSecondAnimation)
	{
		if (m_bPause)IncrementAnimationIndex();
		else
		{
			ResetAnimationIndex();
			m_nIndex = SceneIndex::kSecondAnimation;
		}

	}
	else if (m_nIndex >= SceneIndex::kSecondAnimation && m_nIndex < SceneIndex::kFin)
	{
		if (m_bPause)IncrementAnimationIndex();
		else
		{
			ResetAnimationIndex();
			m_nIndex = SceneIndex::kFin;
		}
	}
	else
	{
		EndThreadpoolTimer();
		++m_nIndex;
	}
	if (m_nIndex >= m_image_info.size())m_nIndex = 0;
	Update();
}
/*拡大*/
void CScenePlayer::UpScale()
{
	if (m_dbScale < 3.99)
	{
		m_dbScale += 0.05;
		ResizeWindow();
	}
}
/*縮小*/
void CScenePlayer::DownScale()
{
	if (m_dbScale > 0.51)
	{
		m_dbScale -= 0.05;
		ResizeWindow();
	}
}
/*原寸表示*/
void CScenePlayer::ResetScale()
{
	m_dbScale = 1.0;
	m_iXOffset = 0;
	m_iYOffset = 0;
	m_interval = Portion::kInterval;
	ResizeWindow();
}
/*窓枠寸法計算法切り替え*/
void CScenePlayer::SwitchSizeLore(bool bBarHidden)
{
	m_bBarHidden = bBarHidden;
	ResizeWindow();
}
/*原点位置移動*/
void CScenePlayer::SetOffset(int iX, int iY)
{
	AdjustOffset(iX, iY);
	Update();
}
/*コマ送り加速*/
void CScenePlayer::SpeedUp()
{
	if (m_interval > 2)
	{
		--m_interval;
	}
}
/*コマ送り減速*/
void CScenePlayer::SpeedDown()
{
	if (m_interval < 1000)
	{
		++m_interval;
	}
}
/*一時停止切り替え*/
bool CScenePlayer::SwitchPause()
{
	m_bPause ^= true;
	return m_bPause;
}
/*消去*/
void CScenePlayer::Clear()
{
	m_image_info.clear();
	m_nIndex = 0;
	if (m_pD2d1Bitmap != nullptr)
	{
		m_pD2d1Bitmap->Release();
		m_pD2d1Bitmap = nullptr;
	}
}
/*画像ファイル取り込み*/
bool CScenePlayer::LoadImageToMemory(const wchar_t* pzFilePath)
{
	ImageInfo s;

	CComPtr<IWICImagingFactory> pWicImageFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
	hr = pWicImageFactory->CreateDecoderFromFilename(pzFilePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pWicBitmapDecoder);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapFrameDecode> pWicFrameDecode;
	hr = pWicBitmapDecoder->GetFrame(0, &pWicFrameDecode);
	if (FAILED(hr))return false;

	CComPtr<IWICFormatConverter> pWicFormatConverter;
	hr = pWicImageFactory->CreateFormatConverter(&pWicFormatConverter);
	if (FAILED(hr))return false;

	pWicFormatConverter->Initialize(pWicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
	if (FAILED(hr))return false;

	hr = pWicFormatConverter->GetSize(&s.uiWidth, &s.uiHeight);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapScaler> pWicBmpScaler;
	hr = pWicImageFactory->CreateBitmapScaler(&pWicBmpScaler);
	if (FAILED(hr))return false;

	/*予め拡大して大きさを揃える。*/
	FLOAT fScale = 1.f;
	if (s.uiWidth % ImageSize::kAnimationWdith == 0 && s.uiHeight % ImageSize::kAnimationHeight == 0)
	{
		fScale = 1.25f;
	}

	hr = pWicBmpScaler->Initialize(pWicFormatConverter, static_cast<UINT>(s.uiWidth * fScale), static_cast<UINT>(s.uiHeight * fScale), WICBitmapInterpolationMode::WICBitmapInterpolationModeCubic);
	if (FAILED(hr))return false;
	hr = pWicBmpScaler.p->GetSize(&s.uiWidth, &s.uiHeight);

	CComPtr<IWICBitmap> pWicBitmap;
	hr = pWicImageFactory->CreateBitmapFromSource(pWicBmpScaler, WICBitmapCacheOnDemand, &pWicBitmap);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapLock> pWicBitmapLock;
	WICRect wicRect{ 0, 0, static_cast<INT>(s.uiWidth), static_cast<INT>(s.uiHeight) };
	hr = pWicBitmap->Lock(&wicRect, WICBitmapLockRead, &pWicBitmapLock);
	if (FAILED(hr))return false;

	UINT uiStride;
	hr = pWicBitmapLock->GetStride(&uiStride);
	if (FAILED(hr))return false;

	s.iStride = static_cast<INT>(uiStride);
	s.pixels.resize(static_cast<size_t>(s.iStride * s.uiHeight));
	hr = pWicBitmap->CopyPixels(nullptr, uiStride, static_cast<UINT>(s.pixels.size()), s.pixels.data());
	if (FAILED(hr))return false;

	m_image_info.push_back(s);

	return true;
}
/*複写枠作成*/
bool CScenePlayer::CreateBitmapForDrawing()
{
	UINT uiWidth = 0;;
	UINT uiHeight = 0;
	for (size_t i = 0; i < m_image_info.size(); ++i)
	{
		uiWidth = uiWidth < m_image_info.at(i).uiWidth ? m_image_info.at(i).uiWidth : uiWidth;
		uiHeight = uiHeight < m_image_info.at(i).uiWidth ? m_image_info.at(i).uiHeight : uiHeight;
	}

	HRESULT hr = m_pD2d1DeviceContext->CreateBitmap(D2D1::SizeU(uiWidth, uiHeight),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
		&m_pD2d1Bitmap);
	return SUCCEEDED(hr);
}
/*画面更新*/
void CScenePlayer::Update()
{
	if (m_hRetWnd != nullptr)
	{
		::InvalidateRect(m_hRetWnd, NULL, TRUE);
	}
}
/*窓枠寸法調整*/
void CScenePlayer::ResizeWindow()
{
	if (!m_image_info.empty() && m_hRetWnd != nullptr)
	{
		RECT rect;
		if (!m_bBarHidden)
		{
			::GetWindowRect(m_hRetWnd, &rect);
		}
		else
		{
			::GetClientRect(m_hRetWnd, &rect);
		}

		int iX = static_cast<int>(::round(m_image_info.at(0).uiWidth * (m_dbScale * 1000) / 1000));
		int iY = static_cast<int>(::round(m_image_info.at(0).uiHeight * (m_dbScale * 1000) / 1000));
		rect.right = iX + rect.left;
		rect.bottom = iY + rect.top;
		if (!m_bBarHidden)
		{
			::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);
			::SetWindowPos(m_hRetWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
			RECT rc;
			::GetWindowRect(m_hRetWnd, &rc);
			::MoveWindow(m_hRetWnd, rc.left, rc.top, rect.right, rect.bottom, TRUE);
		}

		ResizeBuffer();
		AdjustOffset(0, 0);
		Update();
	}

}
/*原点位置調整*/
void CScenePlayer::AdjustOffset(int iXAddOffset, int iYAddOffset)
{
	if (!m_image_info.empty() && m_hRetWnd != nullptr)
	{
		int iX = static_cast<int>(::round(m_image_info.at(0).uiWidth * (m_dbScale * 1000) / 1000));
		int iY = static_cast<int>(::round(m_image_info.at(0).uiHeight * (m_dbScale * 1000) / 1000));

		RECT rc;
		::GetClientRect(m_hRetWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iXOffsetMax = iX > iClientWidth ? static_cast<int>(::floor((iX - iClientWidth) / ((m_dbScale * 1000) / 1000))) : 0;
		int iYOffsetMax = iY > iClientHeight ? static_cast<int>(::floor((iY - iClientHeight) / ((m_dbScale * 1000) / 1000))) : 0;

		m_iXOffset += iXAddOffset;
		if (m_iXOffset < 0) m_iXOffset = 0;
		if (m_iXOffset > iXOffsetMax)m_iXOffset = iXOffsetMax;
		m_iYOffset += iYAddOffset;
		if (m_iYOffset < 0) m_iYOffset = 0;
		if (m_iYOffset > iYOffsetMax)m_iYOffset = iYOffsetMax;
	}

}
/*原版寸法変更*/
void CScenePlayer::ResizeBuffer()
{
	if (m_pDxgiSwapChain1 != nullptr && m_pD2d1DeviceContext != nullptr && m_hRetWnd != nullptr)
	{
		m_pD2d1DeviceContext->SetTarget(nullptr);

		RECT rc;
		::GetClientRect(m_hRetWnd, &rc);
		HRESULT hr = m_pDxgiSwapChain1->ResizeBuffers(0, rc.right - rc.left, rc.bottom - rc.top, DXGI_FORMAT_B8G8R8A8_UNORM, 0);

		CComPtr<IDXGISurface> pDxgiSurface;
		hr = m_pDxgiSwapChain1->GetBuffer(0, IID_PPV_ARGS(&pDxgiSurface));

		CComPtr<ID2D1Bitmap1> pD2d1Bitmap1;
		hr = m_pD2d1DeviceContext->CreateBitmapFromDxgiSurface(pDxgiSurface, nullptr, &pD2d1Bitmap1);

		m_pD2d1DeviceContext->SetTarget(pD2d1Bitmap1);
	}
}
/*コマ番号逓加*/
void CScenePlayer::IncrementAnimationIndex()
{
	if (m_nIndex < m_image_info.size())
	{
		UINT uiDivX = m_image_info.at(m_nIndex).uiWidth / ImageSize::kBaseWidth;
		UINT uiDivY = m_image_info.at(m_nIndex).uiHeight / ImageSize::kBaseHeight;
		if (uiDivX > 1 && uiDivY > 2)
		{
			++m_uiPortion.y;
			if (m_uiPortion.y >= uiDivY)
			{
				m_uiPortion.y = 0;
				++m_uiPortion.x;
				if (m_uiPortion.x >= uiDivX)
				{
					m_uiPortion.x = 0;
				}
			}

			bool bWhite = m_nIndex % kAnimationLength == 0 && m_uiPortion.x >= 1 && m_uiPortion.y >= 1;
			bool bEnd = m_uiPortion.x == 0 && m_uiPortion.y == 0;
			if (bWhite || bEnd)
			{
				++m_nAnimationIndex;
				if (m_nAnimationIndex >= kAnimationLength)
				{
					ResetAnimationIndex();
					if (m_nIndex >= SceneIndex::kFirstAnimation && m_nIndex < SceneIndex::kSecondAnimation)
					{
						m_nIndex = SceneIndex::kFirstAnimation;
					}
					else if (m_nIndex >= SceneIndex::kSecondAnimation && m_nIndex < SceneIndex::kFin)
					{
						m_nIndex = SceneIndex::kSecondAnimation;
					}
				}
				else
				{
					++m_nIndex;
				}
			}
		}
	}
}

void CScenePlayer::ResetAnimationIndex()
{
	m_uiPortion = D2D_POINT_2U{};
	m_nAnimationIndex = 0;
}
/*コマ送り開始*/
void CScenePlayer::StartThreadpoolTimer()
{
	if (m_timer != nullptr)return;

	m_timer = ::CreateThreadpoolTimer(TimerCallback, this, nullptr);
	if (m_timer != nullptr)
	{
		ResetAnimationIndex();
		FILETIME FileDueTime{};
		ULARGE_INTEGER ulDueTime{};
		ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * m_interval));
		FileDueTime.dwHighDateTime = ulDueTime.HighPart;
		FileDueTime.dwLowDateTime = ulDueTime.LowPart;
		::SetThreadpoolTimer(m_timer, &FileDueTime, 0, 0);
	}

}
/*コマ送り終了*/
void CScenePlayer::EndThreadpoolTimer()
{
	if (m_timer != nullptr)
	{
		::SetThreadpoolTimer(m_timer, nullptr, 0, 0);
		::WaitForThreadpoolTimerCallbacks(m_timer, TRUE);
		::CloseThreadpoolTimer(m_timer);
		m_timer = nullptr;
	}
}
/*コマ送り処理スレッドプール*/
void CScenePlayer::TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
	CScenePlayer* pThis = static_cast<CScenePlayer*>(Context);
	if (pThis != nullptr)
	{
		pThis->Update();

		FILETIME FileDueTime{};
		ULARGE_INTEGER ulDueTime{};
		ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * pThis->m_interval));
		FileDueTime.dwHighDateTime = ulDueTime.HighPart;
		FileDueTime.dwLowDateTime = ulDueTime.LowPart;

		::SetThreadpoolTimer(Timer, &FileDueTime, 0, 0);
	}
}
// class CScenePlayer


CD2TextWriter::CD2TextWriter(ID2D1Factory1* pD2d1Factory1, ID2D1DeviceContext* pD2d1DeviceContext)
	:m_pStoredD2d1Factory1(pD2d1Factory1), m_pStoredD2d1DeviceContext(pD2d1DeviceContext)
{
	HRESULT hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
	if (FAILED(hr))return;

	hr = m_pDWriteFactory->CreateTextFormat(m_swzFontFamilyName, nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL,
		PointSizeToDip(m_fFontSize), L"", &m_pDWriteFormat);
	if (FAILED(hr))return;

	CreateBrushes();
}

CD2TextWriter::~CD2TextWriter()
{
	if (m_pD2dSolidColorBrushForOutline != nullptr)
	{
		m_pD2dSolidColorBrushForOutline->Release();
		m_pD2dSolidColorBrushForOutline = nullptr;
	}

	if (m_pD2d1SolidColorBrush != nullptr)
	{
		m_pD2d1SolidColorBrush->Release();
		m_pD2d1SolidColorBrush = nullptr;
	}

	if (m_pDWriteFontFace != nullptr)
	{
		m_pDWriteFontFace->Release();
		m_pDWriteFontFace = nullptr;
	}

	if (m_pDWriteFormat != nullptr)
	{
		m_pDWriteFormat->Release();
		m_pDWriteFormat = nullptr;
	}

	if (m_pDWriteFactory != nullptr)
	{
		m_pDWriteFactory->Release();
		m_pDWriteFactory = nullptr;
	}
}
/*縁有り描画事前設定*/
bool CD2TextWriter::SetupOutLinedDrawing(const wchar_t* pwzFontFilePath)
{
	if (m_pDWriteFontFace == nullptr)
	{
		bool bRet = SetupFont(pwzFontFilePath);
		if (!bRet)return false;
	}

	m_pStoredD2d1DeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

	return true;
}
/*単純描画*/
void CD2TextWriter::NoBorderDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pDWriteFormat == nullptr || m_pD2d1SolidColorBrush == nullptr)
	{
		return;
	}
	m_pStoredD2d1DeviceContext->DrawText(wszText, ulTextLength, m_pDWriteFormat, &rect, m_pD2d1SolidColorBrush);
}
/*縁有り描画*/
void CD2TextWriter::OutLinedDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr
		|| m_pD2d1SolidColorBrush == nullptr || m_pD2dSolidColorBrushForOutline == nullptr
		|| m_pDWriteFontFace == nullptr)
	{
		return;
	}

	/*他の描画法と違って制御コードも文字列として見てしまうので一行毎に描画する。*/
	if (wszText == nullptr)return;

	std::vector<std::vector<wchar_t>> lines;
	for (size_t nRead = 0, nLen = 0;;)
	{
		const wchar_t* p = wcsstr(&wszText[nRead], L"\r\n");
		if (p == nullptr)
		{
			nLen = ulTextLength - nRead;
			std::vector<wchar_t> wchars;
			wchars.reserve(nLen);
			for (size_t i = nRead; i < ulTextLength; ++i)
			{
				wchars.push_back(wszText[i]);
			}
			lines.push_back(wchars);
			break;
		}
		nLen = p - &wszText[nRead];
		std::vector<wchar_t> wchars;
		wchars.reserve(nLen);
		for (size_t i = 0; i < nLen; ++i)
		{
			wchars.push_back(wszText[nRead + i]);
		}
		lines.push_back(wchars);
		nRead += nLen + 2;
	}

	for (size_t i = 0; i < lines.size(); ++i)
	{
		D2D1_POINT_2F fPos{ rect.left, rect.top + i * PointSizeToDip(m_fFontSize) };
		SingleLineGlyphDraw(lines.at(i).data(), static_cast<unsigned long>(lines.at(i).size()), fPos);
	}
}
/*文字間隔指定描画*/
void CD2TextWriter::LayedOutDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pDWriteFormat == nullptr || m_pD2d1SolidColorBrush == nullptr)
	{
		return;
	}

	CComPtr<IDWriteTextLayout>pDWriteTextLayout;
	HRESULT hr = m_pDWriteFactory->CreateTextLayout(wszText, ulTextLength, m_pDWriteFormat, rect.right - rect.left, rect.bottom - rect.top, &pDWriteTextLayout);
	CComPtr<IDWriteTextLayout1>pDWriteTextLayout1;
	hr = pDWriteTextLayout->QueryInterface(__uuidof(IDWriteTextLayout1), (void**)&pDWriteTextLayout1);

	DWRITE_TEXT_RANGE sRange{ 0, ulTextLength };
	hr = pDWriteTextLayout1->SetCharacterSpacing(1.f, 1.f, 2.f, sRange);
	pDWriteTextLayout1->SetFontWeight(DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_EXTRA_BOLD, sRange);

	m_pStoredD2d1DeviceContext->DrawTextLayout(D2D1_POINT_2F{ rect.left, rect.top }, pDWriteTextLayout1, m_pD2d1SolidColorBrush);
}
/*字体ファイル設定*/
bool CD2TextWriter::SetupFont(const wchar_t* pwzFontFilePath)
{
	if (m_pDWriteFactory == nullptr)return false;

	CComPtr<IDWriteFontFile> pDWriteFontFile;
	HRESULT hr = m_pDWriteFactory->CreateFontFileReference(pwzFontFilePath, nullptr, &pDWriteFontFile);
	if (FAILED(hr))return false;

	IDWriteFontFile* pDWriteFontFiles[] = { pDWriteFontFile };
	hr = m_pDWriteFactory->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE, 1U, pDWriteFontFiles, 0, DWRITE_FONT_SIMULATIONS_BOLD | DWRITE_FONT_SIMULATIONS_OBLIQUE, &m_pDWriteFontFace);

	return SUCCEEDED(hr);
}
/*塗りつぶし色作成*/
bool CD2TextWriter::CreateBrushes()
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	if (m_pD2d1SolidColorBrush != nullptr)
	{
		m_pD2d1SolidColorBrush->Release();
		m_pD2d1SolidColorBrush = nullptr;
	}
	if (m_pD2dSolidColorBrushForOutline != nullptr)
	{
		m_pD2dSolidColorBrushForOutline->Release();
		m_pD2dSolidColorBrushForOutline = nullptr;
	}

	HRESULT hr = m_pStoredD2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pD2d1SolidColorBrush);
	if (SUCCEEDED(hr))
	{
		hr = m_pStoredD2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pD2dSolidColorBrushForOutline);
	}

	return SUCCEEDED(hr);
}
/*一行彫刻*/
bool CD2TextWriter::SingleLineGlyphDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_POINT_2F& fRawPos)
{
	std::vector<UINT32> codePoints;
	codePoints.reserve(ulTextLength);
	for (unsigned long i = 0; i < ulTextLength; ++i)
	{
		codePoints.push_back(wszText[i]);
	}

	std::vector<UINT16> glyphai;
	glyphai.resize(ulTextLength);
	HRESULT hr = m_pDWriteFontFace->GetGlyphIndicesW(codePoints.data(), static_cast<unsigned long>(codePoints.size()), glyphai.data());
	if (FAILED(hr))return false;

	CComPtr<ID2D1PathGeometry>pD2d1PathGeometry;
	hr = m_pStoredD2d1Factory1->CreatePathGeometry(&pD2d1PathGeometry);
	if (FAILED(hr))return false;

	CComPtr<ID2D1GeometrySink> pD2d1GeometrySink;
	hr = pD2d1PathGeometry->Open(&pD2d1GeometrySink);
	if (FAILED(hr))return false;

	pD2d1GeometrySink->SetFillMode(D2D1_FILL_MODE::D2D1_FILL_MODE_WINDING);
	pD2d1GeometrySink->SetSegmentFlags(D2D1_PATH_SEGMENT::D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN);

	hr = m_pDWriteFontFace->GetGlyphRunOutline(PointSizeToDip(m_fFontSize), glyphai.data(), nullptr, nullptr, static_cast<unsigned long>(glyphai.size()), FALSE, FALSE, pD2d1GeometrySink);
	if (FAILED(hr))return false;

	pD2d1GeometrySink->Close();

	D2D1_RECT_F fGeoRect{};
	pD2d1PathGeometry->GetBounds(nullptr, &fGeoRect);
	D2D1_POINT_2F fPos = { fRawPos.x - fGeoRect.left, fRawPos.y - fGeoRect.top };
	m_pStoredD2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(fPos.x, fPos.y));
	m_pStoredD2d1DeviceContext->DrawGeometry(pD2d1PathGeometry, m_pD2dSolidColorBrushForOutline, PointSizeToDip(m_fStrokeWidth));
	m_pStoredD2d1DeviceContext->FillGeometry(pD2d1PathGeometry, m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(0.f, 0.f));
	return true;
}
