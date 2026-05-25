#ifndef TEXT_UTILITY_H_
#define TEXT_UTILITY_H_

#include <string>
#include <vector>

namespace text_utility
{
	template <typename CharType>
	void TextToLines(const std::basic_string<CharType>& text, std::vector<std::basic_string<CharType>>& lines)
	{
		std::basic_string<CharType> temp{};
		for (auto& c : text)
		{
			if (c == CharType('\r') || c == CharType('\n'))
			{
				if (!temp.empty())
				{
					lines.push_back(temp);
					temp.clear();
				}
				continue;
			}
			temp.push_back(c);
		}

		if (!temp.empty())
		{
			lines.push_back(temp);
		}
	}

	template <typename CharType>
	void SplitTextBySeparator(const std::basic_string<CharType>& text, const CharType separator, std::vector<std::basic_string<CharType>>& splits)
	{
		for (size_t nRead = 0; nRead < text.size();)
		{
			size_t nPos = text.find(separator, nRead);
			if (nPos == std::basic_string<CharType>::npos)
			{
				size_t nLen = text.size() - nRead;
				splits.emplace_back(text.substr(nRead, nLen));
				break;
			}

			size_t nLen = nPos - nRead;
			splits.emplace_back(text.substr(nRead, nLen));
			nRead += nLen + 1;
		}
	}

	template <typename CharType>
	void ReplaceAll(std::basic_string<CharType>& src, const std::basic_string<CharType>& strOld, const std::basic_string<CharType>& strNew)
	{
		if (strOld.empty() || strOld == strNew) return;

		for (size_t nRead = 0;;)
		{
			size_t nPos = src.find(strOld, nRead);
			if (nPos == std::basic_string<CharType>::npos) break;
			src.replace(nPos, strOld.size(), strNew);
			nRead = nPos + strNew.size();
		}
	}
	template <typename CharType>
	void ReplaceAll(std::basic_string<CharType>& src, const CharType* strOld, const CharType* strNew)
	{
		ReplaceAll(src, std::basic_string<CharType>(strOld), std::basic_string<CharType>(strNew));
	}

	template <typename CharType>
	void EliminateTag(const std::basic_string<CharType>& src)
	{
		std::basic_string<CharType> result{};
		result.reserve(src.size());
		int iCount = 0;
		for (const auto& c : src)
		{
			if (c == CharType('<'))
			{
				++iCount;
				continue;
			}
			else if (c == CharType('>'))
			{
				--iCount;
				continue;
			}

			if (iCount == 0)
			{
				result.push_back(c);
			}
		}
		src = result;
	}

	/* Utilities below are related path not so as to be text. */

	template <typename CharType>
	std::basic_string<CharType> ExtractDirectory(const std::basic_string<CharType>& filePath)
	{
		const std::basic_string<CharType> separators = { CharType{'\\'}, CharType{'/'} };
		size_t nPos = filePath.find_last_of(separators);
		if (nPos != std::basic_string<CharType>::npos)
		{
			return filePath.substr(0, nPos);
		}
		return filePath;
	}

	template <typename CharType>
	std::basic_string<CharType> ExtractFileName(const std::basic_string<CharType>& filePath)
	{
		const std::basic_string<CharType> separators = { CharType{'\\'}, CharType{'/'} };
		size_t nPos = filePath.find_last_of(separators);
		nPos = nPos == std::basic_string<CharType>::npos ? 0 : nPos + 1;

		size_t nPos2 = filePath.find(CharType('.'), nPos);
		if (nPos2 == std::wstring::npos)nPos2 = filePath.size() - 1;

		return filePath.substr(nPos, nPos2 - nPos);
	}
}

#endif // !TEXT_UTILITY_H_
