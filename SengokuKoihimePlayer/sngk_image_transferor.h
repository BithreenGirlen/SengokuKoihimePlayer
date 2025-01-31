#ifndef SNGK_IMAGE_TRANSFEROR_H_
#define SNGK_IMAGE_TRANSFEROR_H_

#include <Windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <atlbase.h>

#include <vector>
#include <string>

#include "win_timer.h"

class CSngkImageTransferor
{
public:
	CSngkImageTransferor(ID2D1DeviceContext* pD2d1DeviceContext, HWND hWnd);
	~CSngkImageTransferor();

	bool SetImages(std::vector<std::wstring>& imageFilePaths);
	void GetImageSize(unsigned int* uiWidth, unsigned int* uiHeight);

	void ShiftImage();
	ID2D1Bitmap* GetCurrentImage();

	bool SwitchPause();
	void RescaleTimer(bool bFaster);
	void ResetSpeed();
private:
	enum Constants
	{
		kInterval = 16,
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
	HWND m_hRenderWindow = nullptr;

	std::vector<std::vector<CComPtr<ID2D1Bitmap>>> m_images;
	size_t m_nImageIndex = 0;
	size_t m_nAnimationIndex = 0;

	bool m_bPaused = false;

	void ClearImages();
	void ImportImage(const SPortion& sPortion, UINT uiStride, std::vector<CComPtr<ID2D1Bitmap>>& bitmaps);

	void ShiftAnimation();

	CWinTimer m_winTimer;

	static void TimerCallback(void* pData);
};

#endif // !SNGK_IMAGE_TRANSFEROR_H_
