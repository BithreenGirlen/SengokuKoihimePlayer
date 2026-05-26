

#include "sngk.h"
#include "win_filesystem.h"
#include "win_text.h"
#include "win_dialogue.h"

#include "deps/nlohmann/json.hpp"

namespace sngk
{
	/*ID抽出*/
	static std::wstring_view ExtractCharacterIdFromStillFolderPath(const std::wstring_view& stillFolderPath)
	{
		static constexpr std::wstring_view stillFolderPrefix = L"st_";

		size_t nPos = stillFolderPath.find_last_of(L"\\/");
		if (nPos == std::wstring_view::npos)return {};
		nPos += 1;

		nPos = stillFolderPath.find(stillFolderPrefix, nPos);
		if (nPos == std::wstring_view::npos)return {};
		nPos += stillFolderPrefix.length();

		return stillFolderPath.substr(nPos);
	}

	/*ID対応先探索*/
	static std::wstring FindPathContainingCharacterId(const std::wstring_view& folderPath, const std::wstring_view &characterId, const std::wstring_view& fileSpec)
	{
		/* Folder path if filteSpec is empty, and filepath if not. */
		std::vector<std::wstring> paths;
		win_filesystem::CreateFilePathList(folderPath, fileSpec, paths);

		const auto& iter = std::find_if(paths.begin(), paths.end(), [&characterId](const std::wstring& path)
			-> bool
			{
				return path.find(characterId) != std::wstring::npos;
			}
		);
		if (iter == paths.cend())return {};

		size_t nIndex = std::distance(paths.begin(), iter);
		return paths[nIndex];
	}

	struct SResourcePath
	{
		std::wstring voiceFolderPath;
		std::wstring scenarioFilePath;
	};

	/*音声フォルダ・台本ファイル経路導出*/
	static bool DeriveResourcePathFromStillFolderPath(const std::wstring& stillFolderPath, SResourcePath &resourcePath)
	{
		std::wstring_view characterId = ExtractCharacterIdFromStillFolderPath(stillFolderPath);
		if (characterId.empty())return false;

		size_t nPos = stillFolderPath.find(L"adventure");
		if (nPos == std::wstring_view::npos)return false;

		wchar_t pathBuffer[1024]{};
		int length = swprintf_s(pathBuffer, L"%.*s%s", static_cast<int>(nPos), &stillFolderPath[0], L"audios\\voice\\adv");
		if (length == -1)return false;

		resourcePath.voiceFolderPath = FindPathContainingCharacterId(std::wstring_view(pathBuffer, length), characterId, {});
		if (resourcePath.voiceFolderPath.empty())return false;

		length = swprintf_s(pathBuffer, L"%.*s%s", static_cast<int>(nPos), &stillFolderPath[0],L"adventure\\json");
		if (length == -1)return false;

		resourcePath.scenarioFilePath = FindPathContainingCharacterId(std::wstring_view(pathBuffer, length), characterId, L".json");
		if (resourcePath.scenarioFilePath.empty())return false;

		return true;
	}

	enum class ETokenDataType
	{
		kUnknown = -1,
		kText,
		kVoice,
		kAnimation
	};

	struct STokenDatum
	{
		ETokenDataType type = ETokenDataType::kUnknown;
		std::string strData;
	};

	/*脚本ファイル解析*/
	static bool ParseScenarioFile(const std::string& scenarioFile, std::vector<STokenDatum>& tokenData)
	{
		/*音声IDとファイル名が同名なのでloadDataからの写像は作成しない。*/

		try
		{
			nlohmann::json nlJson = nlohmann::json::parse(scenarioFile);

			const nlohmann::json& jData = nlJson.at(5).at(0).at(2);

			const nlohmann::json& jTokenData = jData.at("tokenData");
			for(const auto &jTokenDatum : jTokenData)
			{
				if (!jTokenDatum.contains("type"))continue;

				STokenDatum tokenDatum;
				int iType = jTokenDatum["type"];
				if (iType == 0)
				{
					if (jTokenDatum.contains("message"))
					{
						/*文章*/
						tokenDatum.type = ETokenDataType::kText;
						/*発言者*/
						if (jTokenDatum.contains("name"))
						{
							tokenDatum.strData = jTokenDatum["name"].get<std::string>();
							tokenDatum.strData += ":\n";
						}
						tokenDatum.strData += jTokenDatum["message"].get<std::string>();
					}
				}
				else if (iType == 1)
				{
					if (jTokenDatum.contains("params") && jTokenDatum.contains("commandIdx"))
					{
						const int commandId = jTokenDatum["commandIdx"];
						const nlohmann::json& jParams = jTokenDatum["params"];
						if (commandId == 12)
						{
							/*音声指定*/
							tokenDatum.type = ETokenDataType::kVoice;
							tokenDatum.strData = jParams.at(3).get<std::string>();
						}
						else if (commandId == 42)
						{
							/*動作名指定*/
							tokenDatum.type = ETokenDataType::kAnimation;
							tokenDatum.strData = jParams.at(1).get<std::string>();
						}
					}
				}

				if (tokenDatum.type != ETokenDataType::kUnknown)
				{
					tokenData.push_back(std::move(tokenDatum));
				}
			}
		}
		catch (const nlohmann::json::exception& e)
		{
			win_dialogue::ShowMessageBox("Parse error", e.what());
			return false;
		}

		return true;
	}
}
/*脚本ファイル探索と取り込み*/
bool sngk::SearchAndLoadScenarioFile(const std::wstring& stillFolderPath, std::vector<adv::TextDatum>& textData, std::vector<adv::SceneDatum>& sceneData, std::vector<adv::LabelDatum>& labelData)
{
	SResourcePath resourcePath;
	bool bRet = DeriveResourcePathFromStillFolderPath(stillFolderPath, resourcePath);
	if (!bRet)return false;

	std::string scenarioFile = win_filesystem::LoadFileAsString(resourcePath.scenarioFilePath.c_str());
	if (scenarioFile.empty())return false;

	std::vector<STokenDatum> tokenData;
	bRet = ParseScenarioFile(scenarioFile, tokenData);
	if (!bRet)return false;

	std::wstring voicePathBuffer;
	adv::SceneDatum sceneDatumBuffer;
	std::vector<std::wstring> animationNames;
	bool toCreateLabel = false;

	for(const auto& tokenDatum : tokenData)
	{
		if (tokenDatum.type == ETokenDataType::kText)
		{
			adv::TextDatum textDatum
			{
				.message = win_text::WidenUtf8(tokenDatum.strData),
			};
			if (!voicePathBuffer.empty())
			{
				textDatum.voiceFilePath = voicePathBuffer;
				voicePathBuffer.clear();
			}
			textData.push_back(std::move(textDatum));

			sceneDatumBuffer.nTextIndex = textData.size() - 1;
			sceneData.push_back(sceneDatumBuffer);

			if (toCreateLabel && !animationNames.empty())
			{
				adv::LabelDatum labelDatum
				{
					.caption = animationNames.back(),
					.nSceneIndex = sceneData.size() - 1
				};
				labelData.push_back(std::move(labelDatum));

				toCreateLabel = false;
			}
		}
		else if (tokenDatum.type == ETokenDataType::kVoice)
		{
			voicePathBuffer.assign(resourcePath.voiceFolderPath).append(L"\\").append(win_text::WidenUtf8(tokenDatum.strData)).append(L".mp3");
		}
		else if (tokenDatum.type == ETokenDataType::kAnimation)
		{
			if (tokenDatum.strData.find("flash") == std::string::npos)
			{
				std::wstring animationName = win_text::WidenUtf8(tokenDatum.strData);

				const auto& iter = std::find(animationNames.begin(), animationNames.end(), animationName);
				if (iter == animationNames.cend())
				{
					if (!animationNames.empty())
					{
						++sceneDatumBuffer.nImageIndex;
					}
					animationNames.push_back(std::move(animationName));
					toCreateLabel = true;
				}
			}
		}
	}

	return !textData.empty();
}
