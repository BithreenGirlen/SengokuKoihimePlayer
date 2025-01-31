

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
		std::wstring wstrFolderPath = FindPathContainingCharacterId(wstrVoiceFolder, wstrCharacterId, nullptr);
		if (wstrFolderPath.empty())return false;

		resourcePath.wstrVoiceFolder = wstrFolderPath;

		std::wstring wstrJsonFolder = wstrBaseFolder + L"adventure\\json";
		std::wstring wstrFilePath = FindPathContainingCharacterId(wstrJsonFolder, wstrCharacterId, L".json");
		if (wstrFilePath.empty())return false;

		resourcePath.wstrScenarioFile = wstrFilePath;

		return true;
	}

	enum TokenDataType
	{
		kText = 1,
		kVoice,
	};

	struct MessageData
	{
		std::wstring wstrName;
		std::wstring wstrText;
	};

	struct TokenDatum
	{
		int iType = 0;
		MessageData messageData;
		std::string strVoicePath;
	};

	/*脚本ファイル解析*/
	static void ParseScenarioFile(const std::string& strScenarioFile, std::vector<TokenDatum>& tokenData, std::string& strError)
	{
		/*音声IDとファイル名が同名なのでloadDataからの写像は作成しない。*/

		try
		{
			nlohmann::json nlJson = nlohmann::json::parse(strScenarioFile);

			const nlohmann::json& jData = nlJson.at(5).at(0).at(2);

			const nlohmann::json& jsonTokenData = jData.at("tokenData");
			for(const auto &jsonTokenDatum : jsonTokenData)
			{
				TokenDatum tokenDatum;
				if (jsonTokenDatum.contains("type"))
				{
					/*文章*/
					int iType = jsonTokenDatum["type"];
					if (iType == 0)
					{
						if (jsonTokenDatum.contains("message"))
						{
							/*発言者*/
							if (jsonTokenDatum.contains("name"))
							{
								std::string str = std::string(jsonTokenDatum["name"]);
								tokenDatum.messageData.wstrName = win_text::WidenUtf8(str);
							}
							std::string str = std::string(jsonTokenDatum["message"]);

							tokenDatum.iType = TokenDataType::kText;
							tokenDatum.messageData.wstrText = win_text::WidenUtf8(str);

							tokenData.push_back(std::move(tokenDatum));
						}
					}
					else if (iType == 1)
					{
						if (jsonTokenDatum.contains("params"))
						{
							const nlohmann::json& jParams = jsonTokenDatum["params"];
							if (jParams.size() > 3)
							{
								/*音声指定*/
								if (jParams[3].is_string() && jParams[2] == "vo")
								{
									tokenDatum.iType = TokenDataType::kVoice;
									tokenDatum.strVoicePath = std::string(jParams[3]);
									tokenData.push_back(std::move(tokenDatum));
								}
							}
						}
					}
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
bool sngk::SearchAndLoadScenarioFile(const std::wstring& wstrStillFolderPath, std::vector<adv::TextDatum>& textData)
{
	SResourcePath resourcePath;
	bool bRet = DeriveResourcePathFromStillFolderPath(wstrStillFolderPath, resourcePath);
	if (!bRet)return false;

	std::string strScenarioFile = win_filesystem::LoadFileAsString(resourcePath.wstrScenarioFile.c_str());
	if (strScenarioFile.empty())return false;

	std::vector<TokenDatum> tokenData;
	std::string strError;
	ParseScenarioFile(strScenarioFile, tokenData, strError);

	if (!strError.empty())
	{
		win_dialogue::ShowMessageBox("Parse error", strError.c_str());
		return false;
	}

	std::wstring wstrVoicePath;
	for(const auto& tokenDatum : tokenData)
	{
		switch (tokenDatum.iType)
		{
		case TokenDataType::kText:
		{
			adv::TextDatum textDatum;
			textDatum.wstrText.reserve(128);
			if (!tokenDatum.messageData.wstrName.empty())
			{
				textDatum.wstrText = tokenDatum.messageData.wstrName;
				textDatum.wstrText += L":\n";
			}
			textDatum.wstrText += tokenDatum.messageData.wstrText;
			if (!wstrVoicePath.empty())
			{
				textDatum.wstrVoicePath = wstrVoicePath;
				wstrVoicePath.clear();
			}
			textData.push_back(std::move(textDatum));
		}
		break;
		case TokenDataType::kVoice:
			wstrVoicePath = resourcePath.wstrVoiceFolder +  L"\\" + win_text::WidenUtf8(tokenDatum.strVoicePath) + L".mp3";
			break;
		}
	}

	/*台本なし・読み取り失敗*/
	if (textData.empty())
	{
		std::vector<std::wstring> audioFilePaths;
		win_filesystem::CreateFilePathList(resourcePath.wstrVoiceFolder.c_str(), L".mp3", audioFilePaths);
		for (const std::wstring& audioFilePath : audioFilePaths)
		{
			textData.emplace_back(adv::TextDatum{ L"", audioFilePath });
		}
	}

	return !textData.empty();
}
