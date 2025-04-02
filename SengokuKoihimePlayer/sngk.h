#ifndef SNGK_H_
#define SNGK_H_

#include <string>
#include <vector>

#include "adv.h"

namespace sngk
{
	bool SearchAndLoadScenarioFile(
		const std::wstring& wstrStillFolderPath,
		std::vector<adv::TextDatum>& textData,
		std::vector<std::wstring>& animationNames,
		std::vector<adv::SceneDatum> &sceneData
	);
}

#endif // !SNGK_H_
