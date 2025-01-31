

#include "sngk_image_transferor.h"

#include "text_utility.h"
#include "win_filesystem.h"
#include "win_image.h"

CSngkImageTransferor::CSngkImageTransferor(ID2D1DeviceContext* pD2d1DeviceContext, HWND hWnd)
	: m_pStoredD2d1DeviceContext(pD2d1DeviceContext), m_hRenderWindow(hWnd)
{
	m_winTimer.SetCallback(&CSngkImageTransferor::TimerCallback, this);
	m_winTimer.SetDerfaultInterval(Constants::kInterval);
}

CSngkImageTransferor::~CSngkImageTransferor()
{
	m_winTimer.End();
}

bool CSngkImageTransferor::SetImages(std::vector<std::wstring>& imageFilePaths)
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	ClearImages();

	/*
	* 動作名: 画像名
	* wait1: wait1
	* wait2: wait2
	* anim1: output[1-7]
	* anim2: output2[8-14]
	* fin: fin
	*/

	std::vector<CComPtr<ID2D1Bitmap>> wait1Bitmaps;
	std::vector<CComPtr<ID2D1Bitmap>> wait2Bitmaps;
	std::vector<CComPtr<ID2D1Bitmap>> anim1Bitmaps;
	std::vector<CComPtr<ID2D1Bitmap>> anim2Bitmaps;
	std::vector<CComPtr<ID2D1Bitmap>> finBitmaps;

	bool bLatterAnimation = false;

	for (const auto& imageFilePath : imageFilePaths)
	{
		std::wstring wstrFileName = text_utility::ExtractFileName(imageFilePath);

		/*分割画像は予め拡大して大きさを揃える。*/
		const auto GetImageScale = [&imageFilePath]()
			-> float
			{
				if (imageFilePath.empty())return 1.f;

				unsigned int uiWidth = 0;
				unsigned int uiHeight = 0;
				win_image::SkimImageSize(imageFilePath.c_str(), &uiWidth, &uiHeight);

				return uiWidth == Constants::kAnimationWdith ? 1.25f : 1.f;
			};

		float fImageScale = GetImageScale();

		SImageFrame sWhole{};
		bool bRet = win_image::LoadImageToMemory(imageFilePath.c_str(), &sWhole, fImageScale);
		if (!bRet)continue;

		UINT uiDivX = sWhole.uiWidth / Constants::kBaseWidth;
		UINT uiDivY = sWhole.uiHeight / Constants::kBaseHeight;

		if (uiDivX == 0 || uiDivY == 0)continue;

		if (uiDivX == 1)
		{
			SPortion sPortion{};
			sPortion.pData = sWhole.pixels.data();

			if (wstrFileName == L"wait1")
			{
				ImportImage(sPortion, sWhole.iStride, wait1Bitmaps);
			}
			else if (wstrFileName == L"wait2")
			{
				ImportImage(sPortion, sWhole.iStride, wait2Bitmaps);
			}
			else if (wstrFileName == L"fin")
			{
				ImportImage(sPortion, sWhole.iStride, finBitmaps);
			}
		}
		else
		{
			auto& bitmap = bLatterAnimation ? anim2Bitmaps : anim1Bitmaps;
			bool bResidual = false;

			if (wstrFileName == L"output7")
			{
				bLatterAnimation = true;
				bResidual = true;
			}
			else if (wstrFileName == L"output14")
			{
				bResidual = true;
			}

			SPortion sPortion{};
			sPortion.uiWidth = sWhole.uiWidth / uiDivX;
			sPortion.uiHeight = sWhole.uiHeight / uiDivY;
			INT iStride = sWhole.iStride / uiDivX;

			/*
			* 次の順序で分割:
			*  1  4
			*  2  5
			*  3  6
			*/
			const auto SplitAnimationImages = [&]()
				-> void
				{
					for (size_t nPortionX = 0; nPortionX < uiDivX; ++nPortionX)
					{
						for (size_t nPortionY = 0; nPortionY < uiDivY; ++nPortionY)
						{
							/*白色画像なので打ち切り*/
							if (bResidual && nPortionX >= 1 && nPortionY >= 1)return;

							sPortion.pData = sWhole.pixels.data() + (nPortionX * iStride) + (nPortionY * sWhole.iStride * sPortion.uiHeight);

							ImportImage(sPortion, sWhole.iStride, bitmap);
						}
					}
				};

			SplitAnimationImages();
		}
	}

	if (!wait1Bitmaps.empty())m_images.push_back(std::move(wait1Bitmaps));
	if (!wait2Bitmaps.empty())m_images.push_back(std::move(wait2Bitmaps));
	if (!anim1Bitmaps.empty())m_images.push_back(std::move(anim1Bitmaps));
	if (!anim2Bitmaps.empty())m_images.push_back(std::move(anim2Bitmaps));
	if (!finBitmaps.empty())m_images.push_back(std::move(finBitmaps));

	if (!m_images.empty())
	{
		m_winTimer.Start();
	}
	else
	{
		m_winTimer.End();
	}

	return !m_images.empty();
}
/*画像寸法取得*/
void CSngkImageTransferor::GetImageSize(unsigned int* uiWidth, unsigned int* uiHeight)
{
	if (m_nImageIndex < m_images.size() || m_nAnimationIndex < m_images[m_nImageIndex].size())
	{
		D2D1_SIZE_U s = m_images[m_nImageIndex][m_nAnimationIndex]->GetPixelSize();
		*uiWidth = s.width;
		*uiHeight = s.height;
	}
}
/*画像移行*/
void CSngkImageTransferor::ShiftImage()
{
	if (m_nImageIndex >= m_images.size() || m_nAnimationIndex >= m_images[m_nImageIndex].size())
	{
		return;
	}

	if (m_bPaused)
	{
		ShiftAnimation();
	}
	else
	{
		m_nAnimationIndex = 0;
		if (++m_nImageIndex >= m_images.size())m_nImageIndex = 0;
	}
}
/*現在の画像受け渡し*/
ID2D1Bitmap* CSngkImageTransferor::GetCurrentImage()
{
	if (m_nImageIndex >= m_images.size() || m_nAnimationIndex >= m_images[m_nImageIndex].size())
	{
		return nullptr;
	}

	ID2D1Bitmap* p = m_images[m_nImageIndex][m_nAnimationIndex];
	if (!m_bPaused)
	{
		ShiftAnimation();
	}

	return p;
}
/*停止切り替え*/
bool CSngkImageTransferor::SwitchPause()
{
	m_bPaused ^= true;
	return m_bPaused;
}
/*コマ送り加速・減速*/
void CSngkImageTransferor::RescaleTimer(bool bFaster)
{
	long long llInterval = m_winTimer.GetInterval();

	if (bFaster)
	{
		if (--llInterval <= 1)llInterval = 1;
	}
	else
	{
		++llInterval;
	}

	m_winTimer.SetInterval(llInterval);
}
/*速度初期化*/
void CSngkImageTransferor::ResetSpeed()
{
	m_winTimer.ResetInterval();
}
/*消去*/
void CSngkImageTransferor::ClearImages()
{
	m_images.clear();
	m_nImageIndex = 0;
	m_nAnimationIndex = 0;

	ResetSpeed();
}
/*画像流し込み*/
void CSngkImageTransferor::ImportImage(const SPortion& sPortion, UINT uiStride, std::vector<CComPtr<ID2D1Bitmap>>& bitmaps)
{
	CComPtr<ID2D1Bitmap> pD2d1Bitmap;

	HRESULT hr = m_pStoredD2d1DeviceContext->CreateBitmap
	(
		D2D1::SizeU(sPortion.uiWidth, sPortion.uiHeight),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
		&pD2d1Bitmap
	);

	D2D1_RECT_U rc{ 0, 0, sPortion.uiWidth, sPortion.uiHeight };

	hr = pD2d1Bitmap->CopyFromMemory(&rc, sPortion.pData, uiStride);
	if (SUCCEEDED(hr))
	{
		bitmaps.push_back(std::move(pD2d1Bitmap));
	}
}
/*コマ送り*/
void CSngkImageTransferor::ShiftAnimation()
{
	if (m_nImageIndex >= m_images.size() || m_nAnimationIndex >= m_images[m_nImageIndex].size())
	{
		return;
	}

	if (++m_nAnimationIndex >= m_images[m_nImageIndex].size())
	{
		m_nAnimationIndex = 0;
	}
}
/*再描画要求*/
void CSngkImageTransferor::TimerCallback(void* pData)
{
	auto pThis = static_cast<CSngkImageTransferor*>(pData);
	if (pThis != nullptr)
	{
		HWND hWnd = pThis->m_hRenderWindow;
		if (::IsWindow(hWnd))
		{
			::InvalidateRect(hWnd, nullptr, FALSE);
		}
	}
}
