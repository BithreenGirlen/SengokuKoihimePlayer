

#include "sngk_scene_crafter.h"

#include "sngk.h"
#include "text_utility.h"
#include "win_filesystem.h"
#include "win_image.h"

CSngkSceneCrafter::CSngkSceneCrafter(ID2D1DeviceContext* pD2d1DeviceContext)
	:m_pStoredD2d1DeviceContext(pD2d1DeviceContext)
{

}

CSngkSceneCrafter::~CSngkSceneCrafter()
{

}

bool CSngkSceneCrafter::LoadScenario(const wchar_t* pwzStillFolderPath)
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	ClearScenarioData();

	bool bRet = LoadImages(pwzStillFolderPath);
	if (!bRet)return false;

	std::vector<std::wstring> wstrNames;
	bRet = sngk::SearchAndLoadScenarioFile(pwzStillFolderPath, m_textData, wstrNames, m_sceneData);
	if (wstrNames.size() > m_images.size())return false;

	m_animationClock.Restart();

	return bRet;
}

void CSngkSceneCrafter::GetCurrentImageSize(unsigned int* uiWidth, unsigned int* uiHeight)
{
	if (m_nImageIndex < m_images.size() || m_nAnimationIndex < m_images[m_nImageIndex].size())
	{
		D2D1_SIZE_U s = m_images[m_nImageIndex][m_nAnimationIndex]->GetPixelSize();
		if (uiWidth != nullptr)*uiWidth = s.width;
		if (uiHeight != nullptr)*uiHeight = s.height;
	}
}

void CSngkSceneCrafter::GetLargestImageSize(unsigned int* uiWidth, unsigned int* uiHeight)
{
	unsigned int uiMaxWidth = 0;
	unsigned int uiMaxHeight = 0;

	for (const auto& imageList : m_images)
	{
		for (const auto& pD2Bitmap : imageList)
		{
			D2D1_SIZE_U s = pD2Bitmap->GetPixelSize();

			uiMaxWidth = (std::max)(uiMaxWidth, s.width);
			uiMaxHeight = (std::max)(uiMaxHeight, s.height);;
		}
	}

	if (uiWidth != nullptr)*uiWidth = uiMaxWidth;
	if (uiHeight != nullptr)*uiHeight = uiMaxHeight;
}
/*場面移行*/
void CSngkSceneCrafter::ShiftScene(bool bForward)
{
	if (m_sceneData.empty())return;

	if (bForward)
	{
		if (++m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = 0;
		}
	}
	else
	{
		if (--m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = m_sceneData.size() - 1;
		}
	}
}
/*最終場面是否*/
bool CSngkSceneCrafter::HasReachedLastScene()
{
	return m_nSceneIndex == m_sceneData.size() - 1;
}
/*現在の画像受け渡し*/
ID2D1Bitmap* CSngkSceneCrafter::GetCurrentImage()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		m_nImageIndex = m_sceneData[m_nSceneIndex].nImageIndex;
		if (m_nImageIndex < m_images.size())
		{
			if (m_nAnimationIndex >= m_images[m_nImageIndex].size())
			{
				m_nAnimationIndex = 0;
			}

			ID2D1Bitmap* p = m_images[m_nImageIndex][m_nAnimationIndex];
			if (!m_bPaused)
			{
				float fElapsed = m_animationClock.GetElapsedTime();
				if (::isgreaterequal(fElapsed, 1000 / static_cast<float>(m_iFps)))
				{
					ShiftAnimation();
					m_animationClock.Restart();
				}
			}

			return p;
		}
	}

	return nullptr;
}

std::wstring CSngkSceneCrafter::GetCurrentFormattedText()
{
	std::wstring wstr;
	if (m_nSceneIndex < m_sceneData.size())
	{
		wstr.reserve(128);
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;

		if (nTextIndex < m_textData.size())
		{
			wstr = m_textData[nTextIndex].wstrText;
			if (!wstr.empty() && wstr.back() != L'\n')wstr.push_back(L'\n');
			wstr += std::to_wstring(nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
		}
	}

	return wstr;
}

const wchar_t* CSngkSceneCrafter::GetCurrentVoiceFilePath()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;
		if (nTextIndex < m_textData.size())
		{
			return m_textData[nTextIndex].wstrVoicePath.c_str();
		}
	}

	return nullptr;
}
/*停止切り替え*/
bool CSngkSceneCrafter::TogglePause()
{
	m_animationClock.Restart();

	m_bPaused ^= true;
	return m_bPaused;
}

bool CSngkSceneCrafter::IsPaused() const
{
	return m_bPaused;
}
/*コマ送り加速・減速*/
void CSngkSceneCrafter::UpdateAnimationInterval(bool bFaster)
{
	if (bFaster)
	{
		++m_iFps;
	}
	else
	{
		if (--m_iFps <= 1)m_iFps = 1;
	}
}
/*速度初期化*/
void CSngkSceneCrafter::ResetAnimationInterval()
{
	m_iFps = Constants::kDefaultFps;
}
/*消去*/
void CSngkSceneCrafter::ClearScenarioData()
{
	m_textData.clear();

	m_sceneData.clear();
	m_nSceneIndex = 0;

	m_images.clear();
	m_nImageIndex = 0;
	m_nAnimationIndex = 0;

	ResetAnimationInterval();
}

bool CSngkSceneCrafter::LoadImages(const wchar_t* pwzStillFolderPath)
{
	std::vector<std::wstring> imageFilePaths;
	bool bRet = win_filesystem::CreateFilePathList(pwzStillFolderPath, L".jpg", imageFilePaths);
	win_filesystem::CreateFilePathList(pwzStillFolderPath, L".png", imageFilePaths);

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

	m_animationClock.Restart();

	return !m_images.empty();
}
/*画像流し込み*/
void CSngkSceneCrafter::ImportImage(const SPortion& sPortion, UINT uiStride, std::vector<CComPtr<ID2D1Bitmap>>& bitmaps)
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
void CSngkSceneCrafter::ShiftAnimation()
{
	if (m_nImageIndex < m_images.size())
	{
		if (++m_nAnimationIndex >= m_images[m_nImageIndex].size())
		{
			m_nAnimationIndex = 0;
		}
	}
}
