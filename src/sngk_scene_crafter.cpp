

#include "sngk_scene_crafter.h"

#include "sngk.h"
#include "path_utility.h"
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

bool CSngkSceneCrafter::loadScenario(const wchar_t* stillFolderPath)
{
	if (m_pStoredD2d1DeviceContext == nullptr)return false;

	clearScenarioData();

	bool bRet = loadImages(stillFolderPath);
	if (!bRet)return false;

	std::vector<std::wstring> animationNames;
	bRet = sngk::SearchAndLoadScenarioFile(stillFolderPath, m_textData, animationNames, m_sceneData);
	if (animationNames.size() > m_images.size())return false;

	m_animationClock.restart();

	return bRet;
}

bool CSngkSceneCrafter::hasScenarioData() const noexcept
{
	return !m_sceneData.empty();
}

void CSngkSceneCrafter::getCurrentImageSize(unsigned int* uiWidth, unsigned int* uiHeight)
{
	if (m_nImageIndex < m_images.size() || m_nAnimationIndex < m_images[m_nImageIndex].size())
	{
		D2D1_SIZE_U s = m_images[m_nImageIndex][m_nAnimationIndex]->GetPixelSize();
		if (uiWidth != nullptr)*uiWidth = s.width;
		if (uiHeight != nullptr)*uiHeight = s.height;
	}
}

void CSngkSceneCrafter::getLargestImageSize(unsigned int* uiWidth, unsigned int* uiHeight)
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
void CSngkSceneCrafter::shiftScene(bool forward)
{
	if (m_sceneData.empty())return;

	if (forward)
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
bool CSngkSceneCrafter::hasReachedLastScene() const noexcept
{
	return m_nSceneIndex == m_sceneData.size() - 1;
}
/*現在の画像受け渡し*/
ID2D1Bitmap* CSngkSceneCrafter::getCurrentImage()
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
			if (!m_isPaused)
			{
				float fElapsed = m_animationClock.getElapsedTime();
				if (::isgreaterequal(fElapsed, 1 / static_cast<float>(m_fps)))
				{
					shiftAnimation();
					m_animationClock.restart();
				}
			}

			return p;
		}
	}

	return nullptr;
}

const std::wstring& CSngkSceneCrafter::getCurrentFormattedText()
{
	m_formattedText.clear();

	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;
		if (nTextIndex < m_textData.size())
		{
			m_formattedText.assign(m_textData[nTextIndex].message);
			if (!m_formattedText.empty() && m_formattedText.back() != L'\n')m_formattedText.push_back(L'\n');
			wchar_t buffer[64]{};
			::swprintf_s(buffer, L"%zu/%zu", nTextIndex + 1, m_textData.size());
			m_formattedText += buffer;
		}
	}

	return m_formattedText;
}

const wchar_t* CSngkSceneCrafter::getCurrentVoiceFilePath()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;
		if (nTextIndex < m_textData.size())
		{
			return m_textData[nTextIndex].voiceFilePath.c_str();
		}
	}

	return nullptr;
}

void CSngkSceneCrafter::setPause(bool paused)
{
	m_animationClock.restart();

	m_isPaused = paused;
}

bool CSngkSceneCrafter::isPaused() const noexcept
{
	return m_isPaused;
}
/*コマ送り加速・減速*/
void CSngkSceneCrafter::updateAnimationInterval(bool faster)
{
	if (faster)
	{
		++m_fps;
	}
	else
	{
		if (--m_fps <= 1)m_fps = 1;
	}
}
/*速度初期化*/
void CSngkSceneCrafter::resetAnimationInterval()
{
	m_fps = Constants::kDefaultFps;
}
/*消去*/
void CSngkSceneCrafter::clearScenarioData()
{
	m_textData.clear();

	m_sceneData.clear();
	m_nSceneIndex = 0;

	m_images.clear();
	m_nImageIndex = 0;
	m_nAnimationIndex = 0;

	resetAnimationInterval();
}

bool CSngkSceneCrafter::loadImages(const wchar_t* pwzStillFolderPath)
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
		std::wstring_view wstrFileName = path_utility::ExtractFileNameWithoutExtension(imageFilePath);

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

		win_image::SImageFrame sWhole{};
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
				ImportImage(sPortion, sWhole.uiStride, wait1Bitmaps);
			}
			else if (wstrFileName == L"wait2")
			{
				ImportImage(sPortion, sWhole.uiStride, wait2Bitmaps);
			}
			else if (wstrFileName == L"fin")
			{
				ImportImage(sPortion, sWhole.uiStride, finBitmaps);
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
			INT iStride = sWhole.uiStride / uiDivX;

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

							sPortion.pData = sWhole.pixels.data() + (nPortionX * iStride) + (nPortionY * sWhole.uiStride * sPortion.uiHeight);

							ImportImage(sPortion, sWhole.uiStride, bitmap);
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

	m_animationClock.restart();

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
void CSngkSceneCrafter::shiftAnimation()
{
	if (m_nImageIndex < m_images.size())
	{
		if (++m_nAnimationIndex >= m_images[m_nImageIndex].size())
		{
			m_nAnimationIndex = 0;
		}
	}
}
