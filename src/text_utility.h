#ifndef TEXT_UTILITY_H_
#define TEXT_UTILITY_H_

#include <string>
#include <vector>

namespace text_utility
{
	template <typename CharType>
	void TextToLines(const std::basic_string<CharType>& text, std::vector<std::basic_string_view<CharType>>& lines)
	{
		constexpr CharType key[] = { static_cast<CharType>('\r'), static_cast<CharType>('\n'), static_cast<CharType>('\0') };
		for (size_t nRead = 0;;)
		{
			size_t nPos = text.find_first_of(key, nRead);
			if (nPos == std::basic_string<CharType>::npos)
			{
				lines.emplace_back(std::basic_string_view(&text[nRead], text.size() - nRead));
				break;
			}
			size_t nLen = nPos - nRead;
			if (nLen > 1)
			{
				lines.emplace_back(std::basic_string_view(&text[nRead], nLen));
			}

			nRead = nPos + 1;
		}
	}

	template <typename CharType>
	void SplitTextBySeparator(const std::basic_string_view<CharType>& text, const CharType separator, std::vector<std::basic_string<CharType>>& splits)
	{
		for (size_t nRead = 0;;)
		{
			size_t nPos = text.find(separator, nRead);
			if (nPos == std::basic_string<CharType>::npos)
			{
				splits.emplace_back(&text[nRead], text.size() - nRead);
				break;
			}

			splits.emplace_back(&text[nRead], nPos - nRead);
			nRead = nPos + 1;
		}
	}
	template <typename CharType>
	void SplitTextBySeparator(const std::basic_string<CharType>& text, const CharType separator, std::vector<std::basic_string<CharType>>& splits)
	{
		return SplitTextBySeparator(std::basic_string_view<CharType>(text), separator, splits);
	}

	template <typename CharType>
	void ReplaceAll(std::basic_string<CharType>& src, const std::basic_string<CharType>& strOld, const std::basic_string<CharType>& strNew)
	{
		if (strOld.empty() || strOld == strNew) return;

		for (size_t nPos = 0;;)
		{
			nPos = src.find(strOld, nPos);
			if (nPos == std::basic_string<CharType>::npos)break;
			src.replace(nPos, strOld.size(), strNew);
			nPos += strNew.size();
		}
	}
	template <typename CharType, size_t sizeOld, size_t sizeNew>
	void ReplaceAll(std::basic_string<CharType>& src, const CharType(&strOld)[sizeOld], const CharType(&strNew)[sizeNew])
	{
		constexpr size_t lenOld = sizeOld - 1;
		constexpr size_t lenNew = sizeNew - 1;

		if (lenOld == 0) return;

		for (size_t nPos = 0;;)
		{
			nPos = src.find(strOld, nPos, lenOld);
			if (nPos == std::basic_string<CharType>::npos)break;
			src.replace(nPos, lenOld, strNew, lenNew);
			nPos += lenNew;
		}
	}
#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || (defined(__cplusplus) && __cplusplus >= 202002L)
	template <size_t sizeOld, size_t sizeNew>
	void ReplaceAll(std::string& src, const char8_t(&strOld)[sizeOld], const char8_t(&strNew)[sizeNew])
	{
		constexpr size_t lenOld = sizeOld - 1;
		constexpr size_t lenNew = sizeNew - 1;

		const char* pOld = reinterpret_cast<const char*>(strOld);
		const char* pNew = reinterpret_cast<const char*>(strNew);

		for (size_t nPos = 0;;)
		{
			nPos = src.find(pOld, nPos, lenOld);
			if (nPos == std::string::npos) break;

			src.replace(nPos, lenOld, pNew, lenNew);
			nPos += lenNew;
		}
	}
#endif

	template <typename CharType>
	void EliminateTagInPlace(std::basic_string<CharType>& str)
	{
		size_t nWritten = 0;
		int iCount = 0;
		for (const auto& c : str)
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
				str[nWritten++] = c;
			}
		}

		str.resize(nWritten);
	}

	template <typename CharType>
	void ToXmlTags(const std::basic_string<CharType>& strText, const CharType* tagName, std::vector<std::basic_string<CharType>>& tags)
	{
		std::basic_string<CharType> strStart{ CharType('<') };
		if (tagName != nullptr)strStart += tagName;

		for (size_t nRead = 0;;)
		{
			size_t nPos = strText.find(strStart, nRead);
			if (nPos == std::basic_string<CharType>::npos)break;
			nRead = nPos + strStart.size() - 1;

			size_t nEnd = strText.find(CharType('>'), nRead);
			if (nEnd == std::basic_string<CharType>::npos)break;
			++nEnd;

			tags.push_back(strText.substr(nPos, nEnd - nPos));

			nRead = nEnd;
		}
	}

	template <typename CharType>
	void GetXmlAttributes(const std::basic_string<CharType>& strTag, std::vector<std::pair<std::basic_string<CharType>, std::basic_string<CharType>>>& attributes, bool bSingleQuote = false)
	{
		const CharType cQuote = bSingleQuote ? CharType('\'') : CharType('"');

		size_t nPos = strTag.find(CharType('<'));
		if (nPos == std::basic_string<CharType>::npos)return;
		++nPos;

		size_t nEnd = strTag.find(CharType('>'), nPos);
		if (nEnd == std::basic_string<CharType>::npos)return;

		attributes.clear();

		for (; nPos < nEnd && strTag[nPos] != CharType(' '); ++nPos);

		size_t nRead = ++nPos;
		for (; nPos < nEnd; ++nPos)
		{
			const char& c = strTag[nPos];

			if (c == '=')
			{
				std::basic_string<CharType> strName = strTag.substr(nRead, nPos - nRead);

				size_t nValueStart = strTag.find(cQuote, nPos);
				if (nValueStart == std::basic_string<CharType>::npos)break;
				++nValueStart;

				nPos = strTag.find(cQuote, nValueStart);
				if (nPos == std::basic_string<CharType>::npos)break;

				std::basic_string<CharType> strValue = strTag.substr(nValueStart, nPos - nValueStart);
				attributes.push_back({ strName, strValue });

				for (; nPos < nEnd && strTag[nPos] != CharType(' '); ++nPos);
				nRead = ++nPos;
			}
		}
	}

	template <typename CharType>
	size_t UnescapeInPlace(CharType* pSrc)
	{
		CharType* pStart = pSrc;
		CharType* pDst = pSrc;
		while (*pSrc)
		{
			if (*pSrc == CharType('\\'))
			{
				pSrc++;
				if (!*pSrc) break;

				switch (*pSrc)
				{
				case CharType('b'):  *pDst++ = CharType('\b'); break;
				case CharType('f'):  *pDst++ = CharType('\f'); break;
				case CharType('n'):  *pDst++ = CharType('\n'); break;
				case CharType('r'):  *pDst++ = CharType('\r'); break;
				case CharType('t'):  *pDst++ = CharType('\t'); break;
				default: *pDst++ = *pSrc; break;
				}
			}
			else
			{
				*pDst++ = *pSrc;
			}
			pSrc++;
		}

		*pDst = CharType('\0');

		return pDst - pStart;
	}
} /* namespace text_utility */

#endif // !TEXT_UTILITY_H_
