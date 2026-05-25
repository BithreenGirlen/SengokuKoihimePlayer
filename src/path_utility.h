#ifndef PATH_UTILITY_H_
#define PATH_UTILITY_H_

#include <string>

namespace path_utility
{
	template <typename CharType>
	std::basic_string_view<CharType> TruncateFilePath(std::basic_string_view<CharType> filePath)
	{
		constexpr CharType separators[] = { static_cast<CharType>('\\'), static_cast<CharType>('/'), static_cast <CharType>('\0') };

		size_t nPos = filePath.find_last_of(separators);
		if (nPos == std::basic_string_view<CharType>::npos)nPos = 0;
		else ++nPos;

		return { &filePath[nPos], filePath.size() - nPos };
	}
	template <typename CharType>
	std::basic_string_view<CharType> TruncateFilePath(const std::basic_string<CharType>& filePath)
	{
		return TruncateFilePath(std::basic_string_view<CharType>(filePath));
	}

	template <typename CharType>
	std::basic_string_view<CharType> ExtractFileNameWithoutExtension(std::basic_string_view<CharType> filePath)
	{
		constexpr CharType separators[] = { static_cast<CharType>('\\'), static_cast<CharType>('/'), static_cast <CharType>('\0') };

		size_t nPos1 = filePath.find_last_of(separators);
		if (nPos1 == std::basic_string_view<CharType>::npos)nPos1 = 0;
		else ++nPos1;

		size_t nPos2 = filePath.find(CharType('.'), nPos1);
		if (nPos2 == std::basic_string_view<CharType>::npos)nPos2 = filePath.size();

		return { &filePath[nPos1], nPos2 - nPos1 };
	}
	template <typename CharType>
	std::basic_string_view<CharType> ExtractFileNameWithoutExtension(const std::basic_string<CharType>& filePath)
	{
		return ExtractFileNameWithoutExtension(std::basic_string_view<CharType>(filePath));
	}

	template <typename CharType>
	std::basic_string_view<CharType> ExtractParentPath(std::basic_string_view<CharType> filePath)
	{
		constexpr CharType separators[] = { static_cast<CharType>('\\'), static_cast<CharType>('/'), static_cast <CharType>('\0') };

		size_t nPos = filePath.find_last_of(separators);
		if (nPos == std::basic_string_view<CharType>::npos)nPos = filePath.size();

		return { filePath.data(), nPos };
	}
	template <typename CharType>
	std::basic_string_view<CharType> ExtractParentPath(const std::basic_string<CharType>& filePath)
	{
		return ExtractParentPath(std::basic_string_view<CharType>(filePath));
	}

	template <typename CharType>
	std::basic_string<CharType> ReplaceFullExtension(std::basic_string_view<CharType> filePath, std::basic_string_view<CharType> extension)
	{
		constexpr CharType separators[] = { static_cast<CharType>('\\'), static_cast<CharType>('/'), static_cast <CharType>('\0') };

		size_t nPos1 = filePath.find_last_of(separators);
		if (nPos1 == std::basic_string_view<CharType>::npos)nPos1 = 0;
		else ++nPos1;

		size_t nPos2 = filePath.find(CharType('.'), nPos1);
		if (nPos2 == std::basic_string_view<CharType>::npos)nPos2 = filePath.size();

		std::basic_string<CharType> replaced(filePath.data(), nPos2);
		
		return replaced.append(extension);
	}
}
#endif // !PATH_UTILITY_H_
