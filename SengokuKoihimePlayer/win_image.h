#ifndef WIN_IMAGE_H_
#define WIN_IMAGE_H_

#include <vector>

struct SImageFrame
{
	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;
	int iStride = 0;
	std::vector<unsigned char> pixels;
};

namespace win_image
{
	enum class ERotation
	{
		None, Deg90, Deg180, Deg270
	};
	bool LoadImageToMemory(const wchar_t* wpzFilePath, SImageFrame* pImageFrame, float fScale = 1.f, ERotation rotation = ERotation::None);
	bool SkimImageSize(const wchar_t* wpzFilePath, unsigned int* uiWidth, unsigned int* uiHeight);

	bool SaveImageAsPng(const wchar_t* wpzFilePath, SImageFrame* pImageFrame);
	bool SaveImagesAsGif(const wchar_t* wpzFilePath, const std::vector<SImageFrame> &imageFrames);
}
#endif // !WIN_IMAGE_H_
