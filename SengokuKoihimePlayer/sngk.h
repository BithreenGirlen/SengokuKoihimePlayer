#ifndef SNGK_H_
#define SNGK_H_

#include <string>
#include <vector>

#include "adv.h"

namespace sngk
{
	bool DeriveAudioFolderPathFromStillFolderPath(const std::wstring& wstrStillFolderPath, std::wstring& wstrAudioFolderPath);
	bool DeriveEpisodeJsonPathFromStillFolderPath(const std::wstring& wstrStillFolderPath, std::wstring& wstrEpisodeJsonFilePath);

	bool SearchAndLoadScenarioFile(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData);
}

#endif // !SNGK_H_
