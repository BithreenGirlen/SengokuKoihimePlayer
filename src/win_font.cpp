
#include <Windows.h>
#include <dwrite_1.h>
#include <atlbase.h>

#include "win_font.h"

#pragma comment (lib,"Dwrite.lib")


class CWinFont::Impl
{
public:
	Impl()
	{
		HRESULT hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWrireFactory));
		if (FAILED(hr))return;

		hr = m_pDWrireFactory->GetSystemFontCollection(&m_pDWriteFontCollection);

		int iRet = ::GetUserDefaultLocaleName(m_localeName, sizeof(m_localeName) / sizeof(wchar_t));
	}
	~Impl()
	{

	}

	const wchar_t* const getLocaleName() const
	{
		return m_localeName;
	}

	std::wstring findLocaleFontName(const wchar_t* fontFamilyName)
	{
		UINT fontFamilyNameIndex = 0;
		BOOL doesExist = FALSE;
		HRESULT hr = m_pDWriteFontCollection->FindFamilyName(fontFamilyName, &fontFamilyNameIndex, &doesExist);
		if (FAILED(hr) || doesExist == FALSE)return {};

		CComPtr<IDWriteFontFamily> pDWriteFontFamily;
		hr = m_pDWriteFontCollection->GetFontFamily(fontFamilyNameIndex, &pDWriteFontFamily);
		if (FAILED(hr))return {};

		CComPtr<IDWriteLocalizedStrings> pDWriteLocalisedStrings;
		hr = pDWriteFontFamily->GetFamilyNames(&pDWriteLocalisedStrings);
		if (FAILED(hr))return {};

		UINT localeNameIndex = 0;
		doesExist = FALSE;
		hr = pDWriteLocalisedStrings->FindLocaleName(m_localeName, &localeNameIndex, &doesExist);
		if (FAILED(hr))return {};
		if (doesExist == FALSE)localeNameIndex = 0;

		UINT32 localeNameLength = 0;
		hr = pDWriteLocalisedStrings->GetStringLength(localeNameIndex, &localeNameLength);
		if (FAILED(hr))return {};

		std::wstring localeFontName(localeNameLength + 1UL, L'\0');
		hr = pDWriteLocalisedStrings->GetString(localeNameIndex, &localeFontName[0], static_cast<UINT32>(localeFontName.size()));
		localeFontName.pop_back();

		return localeFontName;
	}

	std::vector <std::wstring> getSystemFontFamilyNames()
	{
		std::vector<std::wstring> systemFontFamilyNames;

		UINT32 fontFamilyCount = m_pDWriteFontCollection->GetFontFamilyCount();
		for (UINT32 i = 0; i < fontFamilyCount; ++i)
		{
			CComPtr<IDWriteFontFamily> pDWriteFontFamily;
			HRESULT hr = m_pDWriteFontCollection->GetFontFamily(i, &pDWriteFontFamily);
			if (FAILED(hr))continue;

			CComPtr<IDWriteLocalizedStrings> pDWriteLocalisedStrings;
			hr = pDWriteFontFamily->GetFamilyNames(&pDWriteLocalisedStrings);
			if (FAILED(hr))continue;

			UINT localeNameIndex = 0;
			BOOL doesExist = FALSE;
			hr = pDWriteLocalisedStrings->FindLocaleName(m_localeName, &localeNameIndex, &doesExist);
			if (FAILED(hr))continue;
			if (doesExist == FALSE)localeNameIndex = 0;

			UINT32 localeNameLength = 0;
			hr = pDWriteLocalisedStrings->GetStringLength(localeNameIndex, &localeNameLength);
			if (FAILED(hr))continue;

			std::wstring fontFamilyName(localeNameLength + 1UL, L'\0');
			hr = pDWriteLocalisedStrings->GetString(localeNameIndex, &fontFamilyName[0], static_cast<UINT32>(fontFamilyName.size()));
			if (SUCCEEDED(hr))
			{
				fontFamilyName.pop_back();
				systemFontFamilyNames.push_back(fontFamilyName);
			}
		}

		return systemFontFamilyNames;
	}

	std::vector<std::wstring> findFontFilePaths(const wchar_t* fontFamilyName, bool bold, bool italic)
	{
		std::vector<std::wstring> fontFilePaths;
		if (fontFamilyName == nullptr)return fontFilePaths;

		UINT fontFamilyNameIndex = 0;
		BOOL doesExist = FALSE;
		HRESULT hr = m_pDWriteFontCollection->FindFamilyName(fontFamilyName, &fontFamilyNameIndex, &doesExist);
		if (FAILED(hr) || doesExist == FALSE)return fontFilePaths;

		CComPtr<IDWriteFontFamily> pDWriteFontFamily;
		hr = m_pDWriteFontCollection->GetFontFamily(fontFamilyNameIndex, &pDWriteFontFamily);
		if (FAILED(hr))return fontFilePaths;

		CComPtr<IDWriteFont> pDWriteFont;
		hr = pDWriteFontFamily->GetFirstMatchingFont(
			bold ? DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,
			italic ? DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
			&pDWriteFont
		);
		if (FAILED(hr))return fontFilePaths;

		CComPtr<IDWriteFontFace> pDWriteFontFace;
		hr = pDWriteFont->CreateFontFace(&pDWriteFontFace);
		if (FAILED(hr))return fontFilePaths;

		UINT32 fontFileCount = 0;
		hr = pDWriteFontFace->GetFiles(&fontFileCount, nullptr);
		if (FAILED(hr))return fontFilePaths;

		std::vector<CComPtr<IDWriteFontFile>> writeFontFiles(fontFileCount, nullptr);
		hr = pDWriteFontFace->GetFiles(&fontFileCount, &*writeFontFiles.data());
		if (FAILED(hr))return fontFilePaths;

		for (const auto& writeFontFile : writeFontFiles)
		{
			const void* pFontFileReferenceKey = nullptr;
			UINT32 fontFileReferenceKeyLength = 0;
			hr = writeFontFile->GetReferenceKey(&pFontFileReferenceKey, &fontFileReferenceKeyLength);
			if (FAILED(hr))continue;

			CComPtr<IDWriteFontFileLoader> pDWriteFonfFileLoader;
			hr = writeFontFile->GetLoader(&pDWriteFonfFileLoader);
			if (FAILED(hr))continue;

			CComPtr<IDWriteLocalFontFileLoader> pDWriteLocalFontFileLoader;
			hr = pDWriteFonfFileLoader->QueryInterface(&pDWriteLocalFontFileLoader);
			if (FAILED(hr))continue;

			UINT32 filePathLength = 0;
			hr = pDWriteLocalFontFileLoader->GetFilePathLengthFromKey(pFontFileReferenceKey, fontFileReferenceKeyLength, &filePathLength);
			if (FAILED(hr))continue;

			std::wstring fontFilePath(filePathLength + 1UL, L'\0');
			hr = pDWriteLocalFontFileLoader->GetFilePathFromKey(pFontFileReferenceKey, fontFileReferenceKeyLength, &fontFilePath[0], static_cast<UINT32>(fontFilePath.size()));
			if (SUCCEEDED(hr))
			{
				fontFilePath.pop_back();
				fontFilePaths.push_back(fontFilePath.data());
			}
		}

		return fontFilePaths;
	}
private:
	CComPtr<IDWriteFactory> m_pDWrireFactory;
	CComPtr<IDWriteFontCollection> m_pDWriteFontCollection;

	wchar_t m_localeName[LOCALE_NAME_MAX_LENGTH]{};
};

CWinFont::CWinFont()
{
	m_pImpl = new CWinFont::Impl();
}

CWinFont::~CWinFont()
{
	delete m_pImpl;
}

const wchar_t* const CWinFont::getLocaleName()
{
	return m_pImpl->getLocaleName();
}

std::wstring CWinFont::findLocaleFontName(const wchar_t* fontFamilyName)
{
	return m_pImpl->findLocaleFontName(fontFamilyName);
}

std::vector<std::wstring> CWinFont::getSystemFontFamilyNames()
{
	return m_pImpl->getSystemFontFamilyNames();
}

std::vector<std::wstring> CWinFont::findFontFilePaths(const wchar_t* fontFamilyName, bool bold, bool italic)
{
	return m_pImpl->findFontFilePaths(fontFamilyName, bold, italic);
}
