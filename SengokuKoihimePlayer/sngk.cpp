

#include "sngk.h"
#include "win_filesystem.h"
#include "win_text.h"
#include "win_dialogue.h"

#include "deps/nlohmann/json.hpp"

namespace sngk
{
	/*ID抽出*/
	static std::wstring ExtractCharacterIdFromStillFolderPath(const std::wstring& wstrStillFolderPath)
	{
		size_t nPos = wstrStillFolderPath.find_last_of(L"\\/");
		if (nPos == std::wstring::npos)return std::wstring();

		nPos = wstrStillFolderPath.find(L"st_", nPos);
		if (nPos == std::wstring::npos)return std::wstring();

		return wstrStillFolderPath.substr(nPos + 3);
	}

	/*ID対応先探索*/
	static std::wstring FindPathContainingCharacterId(const std::wstring& targetFolder, const std::wstring &wstrCharacterId, const wchar_t *pwzFileExtension)
	{
		std::vector<std::wstring> folders;
		win_filesystem::CreateFilePathList(targetFolder.c_str(), pwzFileExtension, folders);
		const auto IsContained = [&wstrCharacterId](const std::wstring& wstr)
			-> bool
			{
				return wcsstr(wstr.c_str(), wstrCharacterId.c_str()) != nullptr;
			};

		const auto iter = std::find_if(folders.begin(), folders.end(), IsContained);
		if (iter == folders.cend())return std::wstring();

		size_t nIndex = std::distance(folders.begin(), iter);
		return folders[nIndex];
	}

	struct SResourcePath
	{
		std::wstring wstrVoiceFolder;
		std::wstring wstrScenarioFile;
	};

	/*音声フォルダ・台本ファイル経路導出*/
	static bool DeriveResourcePathFromStillFolderPath(const std::wstring& wstrStillFolderPath, SResourcePath &resourcePath)
	{
		std::wstring wstrCharacterId = ExtractCharacterIdFromStillFolderPath(wstrStillFolderPath);
		if (wstrCharacterId.empty())return false;

		size_t nPos = wstrStillFolderPath.find(L"adventure");
		if (nPos == std::wstring::npos)return false;

		std::wstring wstrBaseFolder = wstrStillFolderPath.substr(0, nPos);

		std::wstring wstrVoiceFolder = wstrBaseFolder + L"audios\\voice\\adv";
		resourcePath.wstrVoiceFolder = FindPathContainingCharacterId(wstrVoiceFolder, wstrCharacterId, nullptr);
		if (resourcePath.wstrVoiceFolder.empty())return false;

		std::wstring wstrScenarioFolder = wstrBaseFolder + L"adventure\\json";
		resourcePath.wstrScenarioFile = FindPathContainingCharacterId(wstrScenarioFolder, wstrCharacterId, L".json");
		if (resourcePath.wstrScenarioFile.empty())return false;

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
	static void ParseScenarioFile(const std::string& strScenarioFile, std::vector<STokenDatum>& tokenData, std::string& strError)
	{
		/*音声IDとファイル名が同名なのでloadDataからの写像は作成しない。*/

		try
		{
			nlohmann::json nlJson = nlohmann::json::parse(strScenarioFile);

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
		catch (nlohmann::json::exception e)
		{
			strError = e.what();
		}
	}
}
/*脚本ファイル探索と取り込み*/
bool sngk::SearchAndLoadScenarioFile(const std::wstring& wstrStillFolderPath, std::vector<adv::TextDatum>& textData, std::vector<std::wstring>& animationNames, std::vector<adv::SceneDatum>& sceneData)
{
	SResourcePath resourcePath;
	bool bRet = DeriveResourcePathFromStillFolderPath(wstrStillFolderPath, resourcePath);
	if (!bRet)return false;

	std::string strScenarioFile = win_filesystem::LoadFileAsString(resourcePath.wstrScenarioFile.c_str());
	if (strScenarioFile.empty())return false;

	std::vector<STokenDatum> tokenData;
	std::string strError;
	ParseScenarioFile(strScenarioFile, tokenData, strError);

	if (!strError.empty())
	{
		win_dialogue::ShowMessageBox("Parse error", strError.c_str());
		return false;
	}

	std::wstring voicePathBuffer;
	adv::SceneDatum sceneDatumBuffer;

	for(const auto& tokenDatum : tokenData)
	{
		if (tokenDatum.type == ETokenDataType::kText)
		{
			adv::TextDatum textDatum;
			textDatum.wstrText.reserve(128);
			textDatum.wstrText = win_text::WidenUtf8(tokenDatum.strData);
			if (!voicePathBuffer.empty())
			{
				textDatum.wstrVoicePath = voicePathBuffer;
				voicePathBuffer.clear();
			}
			textData.push_back(std::move(textDatum));

			sceneDatumBuffer.nTextIndex = textData.size() - 1;
			sceneData.push_back(sceneDatumBuffer);
		}
		else if (tokenDatum.type == ETokenDataType::kVoice)
		{
			voicePathBuffer = resourcePath.wstrVoiceFolder + L"\\" + win_text::WidenUtf8(tokenDatum.strData) + L".mp3";
		}
		else if (tokenDatum.type == ETokenDataType::kAnimation)
		{
			if (tokenDatum.strData.find("flash") == std::string::npos)
			{
				std::wstring wstr = win_text::WidenUtf8(tokenDatum.strData);

				const auto& iter = std::find(animationNames.begin(), animationNames.end(), wstr);
				if (iter == animationNames.cend())
				{
					if (!animationNames.empty())
					{
						++sceneDatumBuffer.nImageIndex;
					}
					animationNames.push_back(std::move(wstr));
				}
			}
		}
	}

	return !textData.empty();
}
