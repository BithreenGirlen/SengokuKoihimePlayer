

#include "sngk.h"
#include "win_filesystem.h"
#include "win_text.h"
#include "win_dialogue.h"

#include "deps/nlohmann/json.hpp"

namespace sngk
{
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

    /*ID抽出*/
    std::wstring ExtractCharacterIdFromStillFolderPath(const std::wstring& wstrStillFolderPath)
    {
        size_t nPos = wstrStillFolderPath.find_last_of(L"\\/");
        if (nPos == std::wstring::npos)return std::wstring();

        std::wstring wstrCurrent = wstrStillFolderPath.substr(nPos + 1);
        nPos = wstrCurrent.find(L"st_");
        if (nPos == std::wstring::npos)return std::wstring();

        return wstrCurrent.substr(nPos + 3);
    }

    /*ID対応先探索*/
    std::wstring FindPathContainingCharacterId(const std::wstring& targetFolder, const std::wstring &wstrCharacterId, const wchar_t *pwzFileExtension)
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
        return folders.at(nIndex);
    }

    /*脚本ファイル解析*/
    void ParseScenarioFile(const std::string& strScenarioFile, std::vector<TokenDatum>& tokenData, std::string& strError)
    {
        /*音声IDとファイル名が同名なのでloadDataからの写像は作成しない。*/

        try
        {
            nlohmann::json nlJson = nlohmann::json::parse(strScenarioFile);

            const nlohmann::json& jData = nlJson.at(5).at(0).at(2);

            const nlohmann::json& jsonTokenData = jData.at("tokenData");
            for (size_t i = 0; i < jsonTokenData.size(); ++i)
            {
                TokenDatum tokenDatum;
                if (jsonTokenData[i].contains("type"))
                {
                    /*テキスト*/
                    int iType = jsonTokenData.at(i).at("type");
                    if (iType == 0)
                    {
                        if (jsonTokenData[i].contains("message"))
                        {
                            if (jsonTokenData[i].contains("name"))
                            {
                                std::string str = std::string(jsonTokenData.at(i).at("name"));
                                tokenDatum.messageData.wstrName = win_text::WidenUtf8(str);
                            }
                            std::string str = std::string(jsonTokenData.at(i).at("message"));

                            tokenDatum.iType = TokenDataType::kText;
                            std::wstring wstr = win_text::WidenUtf8(str);

                            tokenDatum.messageData.wstrText = wstr;
                            tokenData.emplace_back(tokenDatum);
                        }
                    }
                    else if (iType == 1)
                    {
                        if (jsonTokenData[i].contains("params"))
                        {
                            const nlohmann::json& jParams = jsonTokenData.at(i).at("params");
                            if (jParams.size() > 3)
                            {
                                /*音声指定*/
                                if (jParams[3].is_string() && jParams.at(2) == "vo")
                                {
                                    tokenDatum.iType = TokenDataType::kVoice;
                                    tokenDatum.strVoicePath = std::string(jParams.at(3));
                                    tokenData.emplace_back(tokenDatum);
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
/*音声フォルダ経路探索*/
bool sngk::DeriveAudioFolderPathFromStillFolderPath(const std::wstring& wstrStillFolderPath, std::wstring& wstrAudioFolderPath)
{
    std::wstring wstrCharacterId = ExtractCharacterIdFromStillFolderPath(wstrStillFolderPath);
    if (wstrCharacterId.empty())return false;

    size_t nPos = wstrStillFolderPath.find(L"adventure");
    if (nPos == std::wstring::npos)return false;

    std::wstring wstrVoiceFolder = wstrStillFolderPath.substr(0, nPos);
    wstrVoiceFolder += L"audios\\voice\\adv";

    std::wstring wstrFolderPath = FindPathContainingCharacterId(wstrVoiceFolder, wstrCharacterId, nullptr);
    if (wstrFolderPath.empty())return false;

    wstrAudioFolderPath = wstrFolderPath;

    return true;
}
/*脚本ファイル経路探索*/
bool sngk::DeriveEpisodeJsonPathFromStillFolderPath(const std::wstring& wstrStillFolderPath, std::wstring& wstrEpisodeJsonFilePath)
{
    std::wstring wstrCharacterId = ExtractCharacterIdFromStillFolderPath(wstrStillFolderPath);
    if (wstrCharacterId.empty())return false;

    size_t nPos = wstrStillFolderPath.find(L"adventure");
    if (nPos == std::wstring::npos)return false;

    std::wstring wstrJsonFolder = wstrStillFolderPath.substr(0, nPos);
    wstrJsonFolder += L"adventure\\json";

    std::wstring wstrFilePath = FindPathContainingCharacterId(wstrJsonFolder, wstrCharacterId, L".json");
    if (wstrFilePath.empty())return false;

    wstrEpisodeJsonFilePath = wstrFilePath;

    return true;
}
/*脚本ファイル探索と取り込み*/
bool sngk::SearchAndLoadScenarioFile(const std::wstring& wstrStillFolderPath, std::vector<adv::TextDatum>& textData)
{
    std::wstring wstrJsonFilePath;
    bool bRet = DeriveEpisodeJsonPathFromStillFolderPath(wstrStillFolderPath, wstrJsonFilePath);
    if (!bRet)return false;

    std::wstring wstrAudioFolderPath;
    bRet = DeriveAudioFolderPathFromStillFolderPath(wstrStillFolderPath, wstrAudioFolderPath);
    if (!bRet)return false;

    std::string strScenarioFile = win_filesystem::LoadFileAsString(wstrJsonFilePath.c_str());
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
    for (size_t i = 0; i < tokenData.size(); ++i)
    {
        const TokenDatum& tokenDatum = tokenData.at(i);
        switch (tokenDatum.iType)
        {
        case TokenDataType::kText:
        {
            adv::TextDatum textDatum;
            textDatum.wstrText = tokenDatum.messageData.wstrText;
            if (!wstrVoicePath.empty())
            {
                textDatum.wstrVoicePath = wstrVoicePath;
                wstrVoicePath.clear();
            }
            textData.emplace_back(textDatum);
        }
        break;
        case TokenDataType::kVoice:
            wstrVoicePath = wstrAudioFolderPath +  L"\\" + win_text::WidenUtf8(tokenDatum.strVoicePath) + L".mp3";
            break;
        }
    }

    return !textData.empty();
}
