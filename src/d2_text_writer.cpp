

#include <atlbase.h>

#include <vector>

#include "d2_text_writer.h"

#pragma comment (lib,"Dwrite.lib")

CD2TextWriter::CD2TextWriter(ID2D1Factory1* pD2d1Factory1, ID2D1DeviceContext* pD2d1DeviceContext)
	:m_pStoredD2d1Factory1(pD2d1Factory1), m_pStoredD2d1DeviceContext(pD2d1DeviceContext)
{
	HRESULT hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
	if (FAILED(hr))return;

	SetFontByFontName(nullptr);

	if (m_pStoredD2d1DeviceContext != nullptr)
	{
		m_pStoredD2d1DeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
	}

	CreateBrushes();
}

CD2TextWriter::~CD2TextWriter()
{
	ReleaseBrushes();

	ReleaseFontFace();

	ReleaseTextFormat();

	if (m_pDWriteFactory != nullptr)
	{
		m_pDWriteFactory->Release();
		m_pDWriteFactory = nullptr;
	}
}
/*字体指定*/
bool CD2TextWriter::SetFontByFontName(const wchar_t* pwzFontFamilyName, const wchar_t* pwzLocaleName, bool bBold, bool bItalic, float fFontSize)
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	ReleaseTextFormat();

	HRESULT hr = m_pDWriteFactory->CreateTextFormat(
		pwzFontFamilyName == nullptr ? L"Yu mincho" : pwzFontFamilyName,
		nullptr,
		bBold ? DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_REGULAR,
		bItalic ? DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		PointSizeToDip(fFontSize),
		pwzLocaleName == nullptr ? L"en-us" : pwzLocaleName,
		&m_pDWriteTextFormat);

	return SUCCEEDED(hr);
}
/*縁有り描画事前設定*/
bool CD2TextWriter::SetupOutLinedDrawing(const wchar_t* pwzFontFilePath, bool bSimulateBold, bool bSimulateItalic, float fFontSize, float fStrokeThickness)
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	ReleaseFontFace();

	CComPtr<IDWriteFontFile> pDWriteFontFile;
	HRESULT hr = m_pDWriteFactory->CreateFontFileReference(pwzFontFilePath, nullptr, &pDWriteFontFile);
	if (FAILED(hr))return false;

	BOOL iSupported = FALSE;
	DWRITE_FONT_FILE_TYPE fontType = DWRITE_FONT_FILE_TYPE::DWRITE_FONT_FILE_TYPE_UNKNOWN;
	DWRITE_FONT_FACE_TYPE fontFace = DWRITE_FONT_FACE_TYPE::DWRITE_FONT_FACE_TYPE_UNKNOWN;
	UINT32 uiFaceCount = 0;
	hr = pDWriteFontFile->Analyze(&iSupported, &fontType, &fontFace, &uiFaceCount);

	DWRITE_FONT_SIMULATIONS fontSimulatioms = DWRITE_FONT_SIMULATIONS::DWRITE_FONT_SIMULATIONS_NONE;
	if (bSimulateBold)fontSimulatioms |= DWRITE_FONT_SIMULATIONS::DWRITE_FONT_SIMULATIONS_BOLD;
	if (bSimulateItalic)fontSimulatioms |= DWRITE_FONT_SIMULATIONS::DWRITE_FONT_SIMULATIONS_OBLIQUE;

	IDWriteFontFile* pDWriteFontFiles[] = { pDWriteFontFile };
	hr = m_pDWriteFactory->CreateFontFace(fontFace, 1U, pDWriteFontFiles, 0, fontSimulatioms, &m_pDWriteFontFace);
	if (SUCCEEDED(hr))
	{
		m_fFontSize = fFontSize;
		m_fStrokeThickness = fStrokeThickness;
	}

	return SUCCEEDED(hr);
}
/*単純描画*/
void CD2TextWriter::NoBorderDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pDWriteTextFormat == nullptr || m_pD2d1SolidColorBrush == nullptr)
	{
		return;
	}
	m_pStoredD2d1DeviceContext->BeginDraw();
	m_pStoredD2d1DeviceContext->DrawText(wszText, ulTextLength, m_pDWriteTextFormat, &rect, m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->EndDraw();
}
/*文字間隔指定描画*/
void CD2TextWriter::LayedOutDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pDWriteTextFormat == nullptr || m_pD2d1SolidColorBrush == nullptr)
	{
		return;
	}

	CComPtr<IDWriteTextLayout>pDWriteTextLayout;
	HRESULT hr = m_pDWriteFactory->CreateTextLayout(wszText, ulTextLength, m_pDWriteTextFormat, rect.right - rect.left, rect.bottom - rect.top, &pDWriteTextLayout);
	
	CComPtr<IDWriteTextLayout1>pDWriteTextLayout1;
	hr = pDWriteTextLayout->QueryInterface(__uuidof(IDWriteTextLayout1), (void**)&pDWriteTextLayout1);

	DWRITE_TEXT_RANGE sRange{ 0, ulTextLength };
	hr = pDWriteTextLayout1->SetCharacterSpacing(1.f, 1.f, 2.f, sRange);
	pDWriteTextLayout1->SetFontWeight(DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_EXTRA_BOLD, sRange);

	m_pStoredD2d1DeviceContext->BeginDraw();
	m_pStoredD2d1DeviceContext->DrawTextLayout(D2D1_POINT_2F{ rect.left, rect.top }, pDWriteTextLayout1, m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->EndDraw();
}
/*縁有り描画*/
void CD2TextWriter::OutLinedDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pD2d1SolidColorBrush == nullptr || m_pD2dSolidColorBrushForOutline == nullptr || m_pDWriteFontFace == nullptr)
	{
		return;
	}

	/*他の描画法と違って制御コードも文字列として見てしまうので一行毎に描画する。*/
	const auto TextToLines = 
		[&wszText, &ulTextLength](std::vector<std::vector<wchar_t>>& lines, size_t nMax = SIZE_MAX)
		-> void
		{
			const wchar_t* pLineStart = nullptr;
			for (size_t i = 0; i < ulTextLength; ++i)
			{
				if (wszText[i] == L'\r' || wszText[i] == L'\n')
				{
					if (pLineStart != nullptr)
					{
						lines.emplace_back(pLineStart, &wszText[i]);
						pLineStart = nullptr;
					}
				}
				else
				{
					if (pLineStart == nullptr)
					{
						pLineStart = &wszText[i];
					}
					else
					{
						size_t nLen = &wszText[i] - pLineStart;
						if (nLen >= nMax)
						{
							lines.emplace_back(pLineStart, &wszText[i]);
							pLineStart = &wszText[i];
						}
					}
				}
			}

			if (pLineStart != nullptr)
			{
				lines.emplace_back(pLineStart, &wszText[ulTextLength]);
			}
		};

	D2D1_SIZE_F fSize = m_pStoredD2d1DeviceContext->GetSize();
	size_t nMax = static_cast<size_t>((fSize.width - (rect.left - rect.right)) / PointSizeToDip(m_fFontSize)) - 2LL;

	std::vector<std::vector<wchar_t>> lines;
	TextToLines(lines, nMax);

	m_pStoredD2d1DeviceContext->BeginDraw();
	for (size_t i = 0; i < lines.size(); ++i)
	{
		D2D1_POINT_2F fPos{ rect.left, rect.top + i * PointSizeToDip(m_fFontSize) };
		SingleLineGlyphDraw(lines[i].data(), static_cast<unsigned long>(lines[i].size()), fPos);
	}
	m_pStoredD2d1DeviceContext->EndDraw();
}

bool CD2TextWriter::HasBoldStyle() const
{
	if (m_pDWriteFontFace != nullptr)
	{
		DWRITE_FONT_SIMULATIONS fontSimulatioms = m_pDWriteFontFace->GetSimulations();
		return fontSimulatioms & DWRITE_FONT_SIMULATIONS::DWRITE_FONT_SIMULATIONS_BOLD;
	}
	else if (m_pDWriteTextFormat != nullptr)
	{
		DWRITE_FONT_WEIGHT eFontWeight = m_pDWriteTextFormat->GetFontWeight();
		return eFontWeight >= DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_BOLD;
	}

	return false;
}

bool CD2TextWriter::HasItalicStyle() const
{
	if (m_pDWriteFontFace != nullptr)
	{
		DWRITE_FONT_SIMULATIONS fontSimulatioms = m_pDWriteFontFace->GetSimulations();
		return fontSimulatioms & DWRITE_FONT_SIMULATIONS::DWRITE_FONT_SIMULATIONS_OBLIQUE;
	}
	else if (m_pDWriteTextFormat != nullptr)
	{
		DWRITE_FONT_STYLE eFontStyle = m_pDWriteTextFormat->GetFontStyle();
		return eFontStyle == DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_ITALIC;
	}

	return false;
}

bool CD2TextWriter::GetFontFamilyName(wchar_t* pwzFontFamilyName, unsigned long ulNameLength)
{
	if (m_pDWriteTextFormat != nullptr)
	{
		if (ulNameLength < m_pDWriteTextFormat->GetFontFamilyNameLength())return false;

		return m_pDWriteTextFormat->GetFontFamilyName(pwzFontFamilyName, ulNameLength) == S_OK;
	}
	return false;
}
/*文字書式情報解放*/
void CD2TextWriter::ReleaseTextFormat()
{
	if (m_pDWriteTextFormat != nullptr)
	{
		m_pDWriteTextFormat->Release();
		m_pDWriteTextFormat = nullptr;
	}
}
/*字体形状情報解放*/
void CD2TextWriter::ReleaseFontFace()
{
	if (m_pDWriteFontFace != nullptr)
	{
		m_pDWriteFontFace->Release();
		m_pDWriteFontFace = nullptr;
	}
}
/*塗りつぶし色作成*/
bool CD2TextWriter::CreateBrushes()
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	ReleaseBrushes();

	HRESULT hr = m_pStoredD2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pD2d1SolidColorBrush);
	hr &= m_pStoredD2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pD2dSolidColorBrushForOutline);

	return SUCCEEDED(hr);
}
/*塗りつぶし色解放*/
void CD2TextWriter::ReleaseBrushes()
{
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
}
/*一行彫刻*/
bool CD2TextWriter::SingleLineGlyphDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_POINT_2F& fRawPos)
{
	std::vector<UINT32> codePoints;
	codePoints.resize(ulTextLength);
	for (unsigned long i = 0; i < ulTextLength; ++i)
	{
		codePoints[i] = wszText[i];
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
	m_pStoredD2d1DeviceContext->DrawGeometry(pD2d1PathGeometry, m_bColourReversed ? m_pD2d1SolidColorBrush :m_pD2dSolidColorBrushForOutline, PointSizeToDip(m_fStrokeThickness));
	m_pStoredD2d1DeviceContext->FillGeometry(pD2d1PathGeometry, m_bColourReversed ? m_pD2dSolidColorBrushForOutline : m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(0.f, 0.f));

	return true;
}
