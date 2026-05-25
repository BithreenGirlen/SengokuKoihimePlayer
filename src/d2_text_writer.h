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

	bool SetFontByFontName(const wchar_t* pwzFontFamilyName, const wchar_t* pwzLocaleName = nullptr, bool bBold = true, bool bItalic = false, float fFontSize = kfDefaultFontSize);
	bool SetupOutLinedDrawing(const wchar_t* pwzFontFilePath, bool bSimulateBold = true, bool bSimulateItalic = true, float fFontSize = kfDefaultFontSize, float fStrokeThickness = kfDefaultStrokeThickness);

	void NoBorderDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});
	void LayedOutDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});

	void OutLinedDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_RECT_F& rect = D2D1_RECT_F{});

	void SwitchTextColour() { m_bColourReversed ^= true; }

	float GetFontSize() const { return m_fFontSize; }
	float GetStrokeThickness() const { return m_fStrokeThickness; }

	bool HasBoldStyle()const;
	bool HasItalicStyle()const;

	bool GetFontFamilyName(wchar_t* pwzFontFamilyName, unsigned long ulNameLength);
private:
	static constexpr float kfDefaultFontSize = 24.f;
	static constexpr float kfDefaultStrokeThickness = 3.2f;

	ID2D1Factory1* m_pStoredD2d1Factory1 = nullptr;
	ID2D1DeviceContext* m_pStoredD2d1DeviceContext = nullptr;

	IDWriteFactory* m_pDWriteFactory = nullptr;
	IDWriteTextFormat* m_pDWriteTextFormat = nullptr;
	IDWriteFontFace* m_pDWriteFontFace = nullptr;

	ID2D1SolidColorBrush* m_pD2d1SolidColorBrush = nullptr;
	ID2D1SolidColorBrush* m_pD2dSolidColorBrushForOutline = nullptr;

	float m_fFontSize = kfDefaultFontSize;
	float m_fStrokeThickness = kfDefaultStrokeThickness;

	bool m_bColourReversed = false;

	float PointSizeToDip(float fPointSize)const { return (fPointSize / 72.f) * 96.f; };

	void ReleaseTextFormat();
	void ReleaseFontFace();

	bool CreateBrushes();
	void ReleaseBrushes();

	bool SingleLineGlyphDraw(const wchar_t* wszText, unsigned long ulTextLength, const D2D1_POINT_2F& fRawPos = D2D1_POINT_2F{});
};

#endif // D2_TEXT_WRITER_H_
