#ifndef SNGK_SCENE_CRAFTER_H_
#define SNGK_SCENE_CRAFTER_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <atlbase.h>

#include <vector>
#include <string>

#include "adv.h"
#include "win_clock.h"

class CSngkSceneCrafter
{
public:
	CSngkSceneCrafter(ID2D1DeviceContext* pD2d1DeviceContext);
	~CSngkSceneCrafter();

	bool LoadScenario(const wchar_t* pwzStillFolderPath);

	void GetCurrentImageSize(unsigned int* uiWidth, unsigned int* uiHeight);
	void GetLargestImageSize(unsigned int* uiWidth, unsigned int* uiHeight);

	void ShiftScene(bool bForward);
	bool HasReachedLastScene();

	ID2D1Bitmap* GetCurrentImage();
	std::wstring GetCurrentFormattedText();
	const wchar_t* GetCurrentVoiceFilePath();

	bool TogglePause();
	bool IsPaused() const;

	void ShiftAnimation();

	void UpdateAnimationInterval(bool bFaster);
	void ResetAnimationInterval();
private:
	enum Constants
	{
		kDefaultFps = 60,
		kBaseWidth = 1280, kBaseHeight = 720,
		kAnimationWdith = 2048, kAnimationHeight = 1728
	};

	struct SPortion
	{
		UINT uiWidth = Constants::kBaseWidth;
		UINT uiHeight = Constants::kBaseHeight;
		void* pData = nullptr;
	};

	ID2D1DeviceContext* m_pStoredD2d1DeviceContext = nullptr;

	std::vector<adv::TextDatum> m_textData;
	std::vector<std::vector<CComPtr<ID2D1Bitmap>>> m_images;
	size_t m_nImageIndex = 0;
	size_t m_nAnimationIndex = 0;

	std::vector<adv::SceneDatum> m_sceneData;
	size_t m_nSceneIndex = 0;

	bool m_bPaused = false;

	void ClearScenarioData();
	bool LoadImages(const wchar_t* pwzStillFolderPath);

	void ImportImage(const SPortion& sPortion, UINT uiStride, std::vector<CComPtr<ID2D1Bitmap>>& bitmaps);

	CWinClock m_animationClock;
	int m_iFps = Constants::kDefaultFps;
};

#endif // !SNGK_SCENE_CRAFTER_H_
