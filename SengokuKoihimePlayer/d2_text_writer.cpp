

#include <atlbase.h>

#include <vector>

#include "d2_text_writer.h"

#pragma comment (lib,"Dwrite.lib")

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
	m_pStoredD2d1DeviceContext->BeginDraw();
	m_pStoredD2d1DeviceContext->DrawText(wszText, ulTextLength, m_pDWriteFormat, &rect, m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->EndDraw();
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
	const auto TextToLines = 
		[&wszText, &ulTextLength](std::vector<std::vector<wchar_t>>& lines)
		-> void
		{
			std::vector<wchar_t> wchars;
			for (size_t i = 0; i < ulTextLength; ++i)
			{
				if (wszText[i] == '\r' || wszText[i] == '\n')
				{
					if (!wchars.empty())
					{
						lines.push_back(wchars);
						wchars.clear();
					}
					continue;
				}
				wchars.push_back(wszText[i]);
			}

			if (!wchars.empty())
			{
				lines.push_back(wchars);
			}
		};

	std::vector<std::vector<wchar_t>> lines;
	TextToLines(lines);

	m_pStoredD2d1DeviceContext->BeginDraw();
	for (size_t i = 0; i < lines.size(); ++i)
	{
		D2D1_POINT_2F fPos{ rect.left, rect.top + i * PointSizeToDip(m_fFontSize) };
		SingleLineGlyphDraw(lines.at(i).data(), static_cast<unsigned long>(lines.at(i).size()), fPos);
	}
	m_pStoredD2d1DeviceContext->EndDraw();
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

	m_pStoredD2d1DeviceContext->BeginDraw();
	m_pStoredD2d1DeviceContext->DrawTextLayout(D2D1_POINT_2F{ rect.left, rect.top }, pDWriteTextLayout1, m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->EndDraw();
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
	m_pStoredD2d1DeviceContext->DrawGeometry(pD2d1PathGeometry, m_bColourReversed ? m_pD2d1SolidColorBrush :m_pD2dSolidColorBrushForOutline, PointSizeToDip(m_fStrokeWidth));
	m_pStoredD2d1DeviceContext->FillGeometry(pD2d1PathGeometry, m_bColourReversed ? m_pD2dSolidColorBrushForOutline : m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(0.f, 0.f));
	return true;
}
