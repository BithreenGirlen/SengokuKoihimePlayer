#ifndef SNGK_H_
#define SNGK_H_

#include <string>
#include <vector>

#include "adv.h"

namespace sngk
{
	bool SearchAndLoadScenarioFile(
		const std::wstring& stillFolderPath,
		std::vector<adv::TextDatum>& textData,
		std::vector<adv::SceneDatum>& sceneData,
		std::vector<adv::LabelDatum>& labelData
	);
}

#endif // !SNGK_H_
