#ifndef D2_TEXT_WRITER_H_
#define D2_TEXT_WRITER_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dwrite_1.h>

class CD2TextWriter
{
public:
	CD2TextWriter(ID2D1Factory1* pD2d1Factory1, ID2D1DeviceContext* pD2d1DeviceContext);
	~CD2TextWriter();

	/// @brief Set system font to be used in draw() and in layedoutDraw()
	bool setFontByFontName(const wchar_t* fontFamilyName, const wchar_t* localeName = nullptr, bool bold = true, bool italic = false, float fontSize = kfDefaultFontSize);
	/// @brief Set font to be used in outLinedDraw()
	bool setupOutLinedDrawing(const wchar_t* fontFilePath, bool toSmulateBold = true, bool toSimulateItalic = true, float fontSize = kfDefaultFontSize, float thickness = kfDefaultThickness);

	/// @brief Draw without outline
	void draw(const wchar_t* text, unsigned long textLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
	/// @brief Draw with fixed space between characters
	void layedOutDraw(const wchar_t* text, unsigned long textLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
	/// @brief Draw characters having outline
	void outLinedDraw(const wchar_t* text, size_t textLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});

	/// @brief Get the width and height of a character when drawn with outline
	D2D1_SIZE_F getGlyphSize(const wchar_t* text, size_t textLength, size_t* nConsumed = nullptr);

	void toggleTextColour() { m_isColourReversed ^= true; }

	float getFontSize() const { return m_fFontSize; }
	float getThickness() const { return m_fThickness; }

	/// @brief Bold style is simulated or not
	bool hasBoldStyle()const;
	/// @brief Italic style is simulated or not
	bool hasItalicStyle()const;

	/// @brief Write font family name to buffer; Pass wchar_t buffer[LOCALE_NAME_MAX_LENGTH]
	bool getFontFamilyName(wchar_t* fontFamilyNamebuffer, unsigned long bufferSize);

	void onScaleChanged();
private:
	static constexpr float kfDefaultFontSize = 24.f;
	static constexpr float kfDefaultThickness = 3.2f;
	static constexpr size_t kMaxLineCharacters = 512;

	ID2D1Factory1* m_pStoredD2d1Factory1 = nullptr;
	ID2D1DeviceContext* m_pStoredD2d1DeviceContext = nullptr;

	IDWriteFactory* m_pDWriteFactory = nullptr;
	IDWriteTextFormat* m_pDWriteTextFormat = nullptr;
	IDWriteFontFace* m_pDWriteFontFace = nullptr;

	ID2D1SolidColorBrush* m_pD2d1SolidColorBrush = nullptr;
	ID2D1SolidColorBrush* m_pD2dSolidColorBrushForOutline = nullptr;

	float m_fFontSize = kfDefaultFontSize;
	float m_fThickness = kfDefaultThickness;
	unsigned int m_dpi = 96;

	bool m_isColourReversed = false;

	float pointSizeToDip(float fPointSize)const;

	void releaseTextFormat();
	void releaseFontFace();

	bool createBrushes();
	void releaseBrushes();

	bool drawSingleLineGlyphai(const UINT32* codePoints, size_t codePointLength, const D2D1_POINT_2F& originalPos = D2D1_POINT_2F{});

	DWRITE_GLYPH_METRICS getSingleGlyphMetrics(UINT32 codePoint);
	UINT32 stepUtf16(const wchar_t** pRead, size_t* nRemained);
};

#endif // D2_TEXT_WRITER_H_
