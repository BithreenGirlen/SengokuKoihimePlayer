#ifndef WIN_FILESYSTEM_H_
#define WIN_FILESYSTEM_H_

#include <string>
#include <vector>

namespace win_filesystem
{
	bool CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileExtension, std::vector<std::wstring>& paths);
	void GetFolderListAndIndex(const std::wstring& wstrFolderPath, std::vector<std::wstring>& folders, size_t* nIndex);
}
#endif // WIN_FILESYSTEM_H_
