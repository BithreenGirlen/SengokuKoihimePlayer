#ifndef SNGK_IMAGE_TRANSFEROR_H_
#define SNGK_IMAGE_TRANSFEROR_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <atlbase.h>

#include <vector>
#include <string>

#include "win_clock.h"

class CSngkImageTransferor
{
public:
	CSngkImageTransferor(ID2D1DeviceContext* pD2d1DeviceContext);
	~CSngkImageTransferor();

	bool SetImages(std::vector<std::wstring>& imageFilePaths);
	void GetImageSize(unsigned int* uiWidth, unsigned int* uiHeight);

	void ShiftImage();
	ID2D1Bitmap* GetCurrentImage();

	bool TogglePause();
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

	std::vector<std::vector<CComPtr<ID2D1Bitmap>>> m_images;
	size_t m_nImageIndex = 0;
	size_t m_nAnimationIndex = 0;

	bool m_bPaused = false;

	void ClearImages();
	void ImportImage(const SPortion& sPortion, UINT uiStride, std::vector<CComPtr<ID2D1Bitmap>>& bitmaps);

	void ShiftAnimation();

	CWinClock m_animationClock;
	int m_iFps = Constants::kDefaultFps;
};

#endif // !SNGK_IMAGE_TRANSFEROR_H_
