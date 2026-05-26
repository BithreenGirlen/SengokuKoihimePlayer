

#include <atlbase.h>

#include "d2_text_writer.h"

#pragma comment (lib,"Dwrite.lib")

CD2TextWriter::CD2TextWriter(ID2D1Factory1* pD2d1Factory1, ID2D1DeviceContext* pD2d1DeviceContext)
	:m_pStoredD2d1Factory1(pD2d1Factory1), m_pStoredD2d1DeviceContext(pD2d1DeviceContext)
{
	HRESULT hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
	if (FAILED(hr))return;

	setFontByFontName(nullptr);

	if (m_pStoredD2d1DeviceContext != nullptr)
	{
		m_pStoredD2d1DeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
	}

	createBrushes();

	onScaleChanged();
}

CD2TextWriter::~CD2TextWriter()
{
	releaseBrushes();

	releaseFontFace();

	releaseTextFormat();

	if (m_pDWriteFactory != nullptr)
	{
		m_pDWriteFactory->Release();
		m_pDWriteFactory = nullptr;
	}
}

bool CD2TextWriter::setFontByFontName(const wchar_t* fontFamilyName, const wchar_t* localeName, bool bold, bool italic, float fontSize)
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	releaseTextFormat();

	HRESULT hr = m_pDWriteFactory->CreateTextFormat(
		fontFamilyName == nullptr ? L"Yu mincho" : fontFamilyName,
		nullptr,
		bold ? DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_REGULAR,
		italic ? DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		pointSizeToDip(fontSize),
		localeName == nullptr ? L"en-us" : localeName,
		&m_pDWriteTextFormat);

	return SUCCEEDED(hr);
}

bool CD2TextWriter::setupOutLinedDrawing(const wchar_t* fontFilePath, bool toSimulateBold, bool toSimulateItalic, float fontSize, float thickness)
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	releaseFontFace();

	CComPtr<IDWriteFontFile> pDWriteFontFile;
	HRESULT hr = m_pDWriteFactory->CreateFontFileReference(fontFilePath, nullptr, &pDWriteFontFile);
	if (FAILED(hr))return false;

	BOOL iSupported = FALSE;
	DWRITE_FONT_FILE_TYPE fontType = DWRITE_FONT_FILE_TYPE::DWRITE_FONT_FILE_TYPE_UNKNOWN;
	DWRITE_FONT_FACE_TYPE fontFace = DWRITE_FONT_FACE_TYPE::DWRITE_FONT_FACE_TYPE_UNKNOWN;
	UINT32 uiFaceCount = 0;
	hr = pDWriteFontFile->Analyze(&iSupported, &fontType, &fontFace, &uiFaceCount);

	DWRITE_FONT_SIMULATIONS fontSimulations = DWRITE_FONT_SIMULATIONS::DWRITE_FONT_SIMULATIONS_NONE;
	if (toSimulateBold)fontSimulations |= DWRITE_FONT_SIMULATIONS::DWRITE_FONT_SIMULATIONS_BOLD;
	if (toSimulateItalic)fontSimulations |= DWRITE_FONT_SIMULATIONS::DWRITE_FONT_SIMULATIONS_OBLIQUE;

	IDWriteFontFile* pDWriteFontFiles[] = { pDWriteFontFile };
	hr = m_pDWriteFactory->CreateFontFace(fontFace, 1U, pDWriteFontFiles, 0, fontSimulations, &m_pDWriteFontFace);
	if (SUCCEEDED(hr))
	{
		m_fFontSize = fontSize;
		m_fThickness = thickness;
	}

	return SUCCEEDED(hr);
}

void CD2TextWriter::draw(const wchar_t* text, unsigned long textLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pDWriteTextFormat == nullptr || m_pD2d1SolidColorBrush == nullptr)
	{
		return;
	}
	m_pStoredD2d1DeviceContext->BeginDraw();
	m_pStoredD2d1DeviceContext->DrawText(text, textLength, m_pDWriteTextFormat, &rect, m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->EndDraw();
}

void CD2TextWriter::layedOutDraw(const wchar_t* text, unsigned long textLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pDWriteTextFormat == nullptr || m_pD2d1SolidColorBrush == nullptr)
	{
		return;
	}

	CComPtr<IDWriteTextLayout>pDWriteTextLayout;
	HRESULT hr = m_pDWriteFactory->CreateTextLayout(text, textLength, m_pDWriteTextFormat, rect.right - rect.left, rect.bottom - rect.top, &pDWriteTextLayout);

	CComPtr<IDWriteTextLayout1>pDWriteTextLayout1;
	hr = pDWriteTextLayout->QueryInterface(__uuidof(IDWriteTextLayout1), (void**)&pDWriteTextLayout1);

	DWRITE_TEXT_RANGE sRange{ 0, textLength };
	hr = pDWriteTextLayout1->SetCharacterSpacing(1.f, 1.f, 2.f, sRange);
	pDWriteTextLayout1->SetFontWeight(DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_EXTRA_BOLD, sRange);

	m_pStoredD2d1DeviceContext->BeginDraw();
	m_pStoredD2d1DeviceContext->DrawTextLayout(D2D1_POINT_2F{ rect.left, rect.top }, pDWriteTextLayout1, m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->EndDraw();
}

void CD2TextWriter::outLinedDraw(const wchar_t* text, size_t textLength, const D2D1_RECT_F& rect)
{
	if (m_pStoredD2d1DeviceContext == nullptr || m_pD2d1SolidColorBrush == nullptr || m_pD2dSolidColorBrushForOutline == nullptr || m_pDWriteFontFace == nullptr)
	{
		return;
	}

	DWRITE_FONT_METRICS fontMetrics;
	m_pDWriteFontFace->GetMetrics(&fontMetrics);
	const float scale = pointSizeToDip(m_fFontSize) / fontMetrics.designUnitsPerEm;

	const D2D1_SIZE_F targetSize = m_pStoredD2d1DeviceContext->GetSize();
	const float maxWidth = (targetSize.width - (rect.left - rect.right));
	float accumulatedWidth = 0.f;

	UINT32 textBuffer[kMaxLineCharacters]{};
	size_t textBufferLength = 0;
	size_t line = 0;

	const auto& drawTextBuffer = [&]()
		-> void
		{
			D2D1_POINT_2F fPos{ rect.left, rect.top + line * pointSizeToDip(m_fFontSize) };

			m_pStoredD2d1DeviceContext->BeginDraw();
			drawSingleLineGlyphai(textBuffer, textBufferLength, fPos);
			m_pStoredD2d1DeviceContext->EndDraw();

			textBufferLength = 0;
			accumulatedWidth = 0;
			++line;
		};

	/* 他の描画法と違って制御コードも文字列として見てしまうので一行毎に描画する。*/
	for (size_t nRead = 0;; ++nRead)
	{
		const wchar_t* pRead = text + nRead;
		size_t nRemained = textLength - nRead;

		if (nRead >= textLength)
		{
			drawTextBuffer();
			break;
		}

		const wchar_t c = *pRead;
		if (c == '\r')continue;
		else if (c == '\n')
		{
			drawTextBuffer();
			continue;
		}

		size_t nLastRemained = nRemained;
		UINT32 codePoint = stepUtf16(&pRead, &nRemained);
		if (nLastRemained - nRemained == 2)++nRead;

		const DWRITE_GLYPH_METRICS glyphMetrics = getSingleGlyphMetrics(codePoint);
		accumulatedWidth += glyphMetrics.advanceWidth * scale;
		if (::isgreater(accumulatedWidth, maxWidth))
		{
			drawTextBuffer();
		}

		textBuffer[textBufferLength++] = codePoint;
	}
}

D2D1_SIZE_F CD2TextWriter::getGlyphSize(const wchar_t* text, size_t textLength, size_t* nConsumed)
{
	if (m_pDWriteFontFace == nullptr)return {};
	if (text == nullptr || textLength == 0)return {};

	DWRITE_FONT_METRICS fontMetrics;
	m_pDWriteFontFace->GetMetrics(&fontMetrics);
	const float scale = m_fFontSize / fontMetrics.designUnitsPerEm;

	wchar_t buffer[3]{};
	buffer[0] = *text;
	if (textLength > 1)
	{
		buffer[1] = *(text + 1);
	}
	const wchar_t* pRead = buffer;
	size_t nRemained = textLength;
	UINT32 codePoint = stepUtf16(&pRead, &nRemained);
	if (nConsumed != nullptr)*nConsumed = textLength - nRemained;

	const DWRITE_GLYPH_METRICS glyphMetrics = getSingleGlyphMetrics(codePoint);

	return { glyphMetrics.advanceWidth * scale, glyphMetrics.advanceHeight * scale };
}

bool CD2TextWriter::hasBoldStyle() const
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

bool CD2TextWriter::hasItalicStyle() const
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

bool CD2TextWriter::getFontFamilyName(wchar_t* fontFamilyNameBuffer, unsigned long bufferSize)
{
	if (m_pDWriteTextFormat != nullptr)
	{
		if (bufferSize < m_pDWriteTextFormat->GetFontFamilyNameLength())return false;

		return m_pDWriteTextFormat->GetFontFamilyName(fontFamilyNameBuffer, bufferSize) == S_OK;
	}
	return false;
}

void CD2TextWriter::onScaleChanged()
{
	m_dpi = ::GetDpiForSystem();
}

float CD2TextWriter::pointSizeToDip(float fPointSize) const
{
	return (fPointSize / 72.f) * m_dpi;
}
/* 文字書式情報解放 */
void CD2TextWriter::releaseTextFormat()
{
	if (m_pDWriteTextFormat != nullptr)
	{
		m_pDWriteTextFormat->Release();
		m_pDWriteTextFormat = nullptr;
	}
}
/* 字体形状情報解放 */
void CD2TextWriter::releaseFontFace()
{
	if (m_pDWriteFontFace != nullptr)
	{
		m_pDWriteFontFace->Release();
		m_pDWriteFontFace = nullptr;
	}
}
/* 塗りつぶし色作成 */
bool CD2TextWriter::createBrushes()
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	releaseBrushes();

	HRESULT hr = m_pStoredD2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pD2d1SolidColorBrush);
	hr &= m_pStoredD2d1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pD2dSolidColorBrushForOutline);

	return SUCCEEDED(hr);
}
/* 塗りつぶし色解放 */
void CD2TextWriter::releaseBrushes()
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

bool CD2TextWriter::drawSingleLineGlyphai(const UINT32* codePoints, size_t codePointLength, const D2D1_POINT_2F& originalPos)
{
	UINT16 glyphai[kMaxLineCharacters]{};
	size_t count = codePointLength < kMaxLineCharacters ? codePointLength : kMaxLineCharacters;
	HRESULT hr = m_pDWriteFontFace->GetGlyphIndices(codePoints, static_cast<unsigned long>(codePointLength), glyphai);
	if (FAILED(hr))return false;

	CComPtr<ID2D1PathGeometry> pD2d1PathGeometry;
	hr = m_pStoredD2d1Factory1->CreatePathGeometry(&pD2d1PathGeometry);
	if (FAILED(hr))return false;

	CComPtr<ID2D1GeometrySink> pD2d1GeometrySink;
	hr = pD2d1PathGeometry->Open(&pD2d1GeometrySink);
	if (FAILED(hr))return false;

	pD2d1GeometrySink->SetFillMode(D2D1_FILL_MODE::D2D1_FILL_MODE_WINDING);
	pD2d1GeometrySink->SetSegmentFlags(D2D1_PATH_SEGMENT::D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN);

	hr = m_pDWriteFontFace->GetGlyphRunOutline(pointSizeToDip(m_fFontSize), glyphai, nullptr, nullptr, static_cast<unsigned long>(count), FALSE, FALSE, pD2d1GeometrySink);
	if (FAILED(hr))return false;

	pD2d1GeometrySink->Close();

	D2D1_RECT_F fGeoRect{};
	pD2d1PathGeometry->GetBounds(nullptr, &fGeoRect);
	D2D1_POINT_2F fPos = { originalPos.x - fGeoRect.left, originalPos.y - fGeoRect.top };

	m_pStoredD2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(fPos.x, fPos.y));
	m_pStoredD2d1DeviceContext->DrawGeometry(pD2d1PathGeometry, m_isColourReversed ? m_pD2d1SolidColorBrush : m_pD2dSolidColorBrushForOutline, pointSizeToDip(m_fThickness));
	m_pStoredD2d1DeviceContext->FillGeometry(pD2d1PathGeometry, m_isColourReversed ? m_pD2dSolidColorBrushForOutline : m_pD2d1SolidColorBrush);
	m_pStoredD2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

	return true;
}

DWRITE_GLYPH_METRICS CD2TextWriter::getSingleGlyphMetrics(UINT32 codePoint)
{
	if (m_pDWriteFontFace == nullptr)return {};

	UINT16 glyph{};
	HRESULT hr = m_pDWriteFontFace->GetGlyphIndices(&codePoint, 1, &glyph);
	if (FAILED(hr))return {};

	DWRITE_GLYPH_METRICS dWriteGlyphMetrics{};
	hr = m_pDWriteFontFace->GetDesignGlyphMetrics(&glyph, 1, &dWriteGlyphMetrics);

	return dWriteGlyphMetrics;
}

UINT32 CD2TextWriter::stepUtf16(const wchar_t** pRead, size_t* nRemained)
{
	static constexpr UINT32 replacementCharacter = 0xFFFD;

	if (pRead == nullptr || *pRead == nullptr || nRemained == nullptr)
	{
		return 0;
	}

	if (IS_HIGH_SURROGATE(*pRead[0]))
	{
		if (*nRemained < 2)
		{
			return replacementCharacter;
		}

		if (IS_LOW_SURROGATE(*pRead[1]))
		{
			UINT32 codePoint =
				((static_cast<UINT32>(*pRead[0]) - 0xD800) << 10)
				+ (static_cast<UINT32>(*pRead[1]) - 0xDC00)
				+ 0x10000;

			(*pRead) += 2;
			(*nRemained) -= 2;

			return codePoint;
		}
	}

	UINT32 codePoint = static_cast<UINT32>(*pRead[0]);
	(*pRead)++;
	(*nRemained)--;

	return codePoint;
}
