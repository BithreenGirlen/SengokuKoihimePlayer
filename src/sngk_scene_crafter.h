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

	bool loadScenario(const wchar_t* stillFolderPath);
	bool hasScenarioData() const noexcept;

	void getCurrentImageSize(unsigned int* uiWidth, unsigned int* uiHeight);
	void getLargestImageSize(unsigned int* uiWidth, unsigned int* uiHeight);

	void shiftScene(bool forward);
	bool hasReachedLastScene() const noexcept;

	ID2D1Bitmap* getCurrentImage();
	const std::wstring& getCurrentFormattedText() const noexcept;
	const wchar_t* getCurrentVoiceFilePath();

	const std::vector<adv::LabelDatum>& getLabelData() const noexcept;
	bool jumpToLabel(size_t nLabelIndex);

	void setPause(bool paused);
	bool isPaused() const noexcept;

	void shiftAnimation();

	void updateAnimationInterval(bool faster);
	void resetAnimationInterval();
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
	std::vector<adv::LabelDatum> m_labelData;

	std::wstring m_formattedText;

	bool m_isPaused = false;
	CWinClock m_animationClock;
	int m_fps = Constants::kDefaultFps;

	void clearScenarioData();
	bool loadImages(const wchar_t* stillFolderPath);
	void ImportImage(const SPortion& sPortion, UINT uiStride, std::vector<CComPtr<ID2D1Bitmap>>& bitmaps);

	void prepareScene();
	void prepareText();
};

#endif // !SNGK_SCENE_CRAFTER_H_
