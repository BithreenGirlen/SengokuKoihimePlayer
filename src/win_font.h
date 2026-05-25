#ifndef WIN_FONT_H_
#define WIN_FONT_H_

#include <string>
#include <vector>

class CWinFont
{
public:
	CWinFont();
	~CWinFont();

	/// <summary>
	/// 実行環境の言語・地域名取得
	/// </summary>
	/// <returns>言語・地域名への内部保有ポインタ</returns>
	const wchar_t* const GetLocaleName();

	/// <summary>
	/// 或る地域に於ける書体名を基に実行環境の書体名探索
	/// </summary>
	/// <param name="pwzFontFamilyName">何処かしらかの地域の書体名</param>
	/// <returns>実行環境の言語・文字で表される書体名</returns>
	std::wstring FindLocaleFontName(const wchar_t* pwzFontFamilyName);

	/// <summary>
	/// 搭載書体名一覧取得
	/// </summary>
	/// <returns>実行環境の言語・文字で表される書体名一覧</returns>
	std::vector<std::wstring> GetSystemFontFamilyNames();

	/// <summary>
	/// 搭載書体名からファイル経路探索
	/// </summary>
	/// <param name="pwzFontFamilyName">書体名</param>
	/// <param name="bBold">太字是否</param>
	/// <param name="bItalic">斜体是否</param>
	/// <returns>候補となる書体ファイル経路</returns>
	std::vector<std::wstring> FindFontFilePaths(const wchar_t* pwzFontFamilyName, bool bBold, bool bItalic);
private:
	class Impl;
	Impl* m_pImpl = nullptr;
};

#endif // !WIN_FONT_H_
