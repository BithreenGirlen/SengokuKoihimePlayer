
#include "font_setting_dialogue.h"

#include "dialogue_template.h"
#include "../win_font.h"
#include "../d2_text_writer.h"

CFontSettingDialogue::CFontSettingDialogue()
{
	int fontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFontW(fontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Yu mincho");
}

CFontSettingDialogue::~CFontSettingDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

HWND CFontSettingDialogue::open(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, void* pTextWriter, void(*pFontChangeCallback)(void* pUserDatum, FontCallbackDatum* pFontCallbackDatum), void* pCallbackUserDatum)
{
	CDialogueTemplate dialogueTemplate;
	dialogueTemplate.setWindowSize(160, 160);

	m_pTextWriter = pTextWriter;
	m_pFontChangeCallback = pFontChangeCallback;
	m_pCallbackUserDatum = pCallbackUserDatum;

	return ::CreateDialogIndirectParam(hInstance, (LPCDLGTEMPLATE)dialogueTemplate.generate(pwzWindowName), hWndParent, (DLGPROC)DialogProc, (LPARAM)this);
}
/* C CALLBACK */
LRESULT CFontSettingDialogue::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		::SetWindowLongPtr(hWnd, DWLP_USER, lParam);
	}

	auto pThis = reinterpret_cast<CFontSettingDialogue*>(::GetWindowLongPtr(hWnd, DWLP_USER));
	if (pThis != nullptr)
	{
		return pThis->handleMessage(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
/* メッセージ処理 */
LRESULT CFontSettingDialogue::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return onInit(hWnd);
	case WM_SIZE:
		return onSize();
	case WM_CLOSE:
		return onClose();
	case WM_NOTIFY:
		return onNotify(wParam, lParam);
	case WM_COMMAND:
		return onCommand(wParam, lParam);
	case WM_VSCROLL:
		return onVScroll(wParam, lParam);
	default:
		break;
	}
	return FALSE;
}
/* WM_INITDIALOG */
LRESULT CFontSettingDialogue::onInit(HWND hWnd)
{
	m_hWnd = hWnd;

	m_fontNameStatic.create(L"Font name", m_hWnd);
	m_fontNameComboBox.create(m_hWnd);

	m_fontSizeStatic.create(L"Size", m_hWnd);
	m_fontSizeSlider.create(L"", m_hWnd, reinterpret_cast<HMENU>(Controls::kFontSizeSlider), 8, 64, 1);

	m_fontThicknessStatic.create(L"Thickness", m_hWnd);
	m_fontThicknessSlider.create(L"", m_hWnd, reinterpret_cast<HMENU>(Controls::kFontThicknessSlider), 0.f, 6.f, 0.1f);

	m_boldCheckButton.create(L"Bold", m_hWnd, reinterpret_cast<HMENU>(Controls::kBoldCheckButton), true);
	m_italicCheckButton.create(L"Italic", m_hWnd, reinterpret_cast<HMENU>(Controls::kItalicCheckButton), true);

	m_applyButton.create(L"Apply", m_hWnd, reinterpret_cast<HMENU>(Controls::kApplyButton));

	CWinFont winFont;

	std::vector<std::wstring> fontNames = winFont.getSystemFontFamilyNames();
	m_fontNameComboBox.setup(fontNames);

	if (m_pTextWriter != nullptr)
	{
		CD2TextWriter* pD2TextWriter = static_cast<CD2TextWriter*>(m_pTextWriter);

		m_boldCheckButton.setCheckBox(pD2TextWriter->hasBoldStyle());
		m_italicCheckButton.setCheckBox(pD2TextWriter->hasItalicStyle());

		wchar_t fontFamilyName[LOCALE_NAME_MAX_LENGTH]{};
		pD2TextWriter->getFontFamilyName(fontFamilyName, sizeof(fontFamilyName) / sizeof(wchar_t));
		if (fontFamilyName[0] != 'L\0')
		{
			int index = m_fontNameComboBox.findIndex(fontFamilyName);
			if (index != -1)
			{
				m_fontNameComboBox.setSelectedItem(index);
			}
			else
			{
				/*実行環境の言語・文字による表記で再探索。*/
				std::wstring localeFontName = winFont.findLocaleFontName(fontFamilyName);
				if (!localeFontName.empty())
				{
					index = m_fontNameComboBox.findIndex(localeFontName.c_str());
					if (index != -1)
					{
						m_fontNameComboBox.setSelectedItem(index);
					}
				}
			}
		}
	}

	resizeControls();

	setSliderPosition();

	const auto FontCallback = [](HWND hWnd, LPARAM lParam)
		-> BOOL
		{
			::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
			/* TRUE: 続行, FALSE: 終了 */
			return TRUE;
		};

	::EnumChildWindows(m_hWnd, FontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return TRUE;
}
/* WM_CLOSE */
LRESULT CFontSettingDialogue::onClose()
{
	::DestroyWindow(m_hWnd);
	m_hWnd = nullptr;

	return 0;
}
/* WM_SIZE */
LRESULT CFontSettingDialogue::onSize()
{
	resizeControls();

	return 0;
}
/* WM_NOTIFY */
LRESULT CFontSettingDialogue::onNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pNmhdr = reinterpret_cast<LPNMHDR>(lParam);
	if (pNmhdr != nullptr)
	{
		if (pNmhdr->code == TTN_NEEDTEXT)
		{
			if (pNmhdr->hwndFrom == m_fontThicknessSlider.getToolTipHandle())
			{
				m_fontThicknessSlider.onToolTipNeedText(reinterpret_cast<LPTOOLTIPTEXTW>(lParam));
			}
		}
	}
	return 0;
}
/* WM_COMMAND */
LRESULT CFontSettingDialogue::onCommand(WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int msgSource = LOWORD(lParam);
	if (msgSource == 0)
	{
		/*Menus*/
	}
	else
	{
		/*Controls*/

		WORD usCode = HIWORD(wParam);
		if (usCode == CBN_SELCHANGE)
		{
			/*Notification code*/
		}
		else
		{
			switch (id)
			{
			case Controls::kApplyButton:
				onApplyButton();
				break;
			case Controls::kBoldCheckButton:
				break;
			case Controls::kItalicCheckButton:
				break;
			default:
				break;
			}
		}
	}

	return 0;
}
/* WM_VSCROLL */
LRESULT CFontSettingDialogue::onVScroll(WPARAM wParam, LPARAM lParam)
{
	return 0;
}
/* 再配置 */
void CFontSettingDialogue::resizeControls()
{
	RECT clientRect;
	::GetClientRect(m_hWnd, &clientRect);

	long clientWidth = clientRect.right - clientRect.left;
	long clientHeight = clientRect.bottom - clientRect.top;

	long spaceX = clientWidth / 24;
	long spaceY = clientHeight / 96;

	long x = spaceX;
	long y = spaceY * 2;
	long w = clientWidth - spaceX * 2;
	long h = clientHeight * 8 / 10;

	int fontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);

	if (m_fontNameStatic.getHwnd() != nullptr)
	{
		::MoveWindow(m_fontNameStatic.getHwnd(), x, y, w, h, TRUE);
	}

	y += fontHeight;
	if (m_fontNameComboBox.getHwnd() != nullptr)
	{
		::MoveWindow(m_fontNameComboBox.getHwnd(), x, y, w, h, TRUE);
	}

	y += clientHeight * 1 / 6;
	h = clientHeight * 1 / 6;
	::MoveWindow(m_fontSizeStatic.getHwnd(), x, y, w, h, TRUE);

	y += fontHeight;
	::MoveWindow(m_fontSizeSlider.getHwnd(), x, y, w, h, TRUE);

	y += clientHeight * 1 / 6;
	::MoveWindow(m_fontThicknessStatic.getHwnd(), x, y, w, h, TRUE);

	y += fontHeight;
	::MoveWindow(m_fontThicknessSlider.getHwnd(), x, y, w, h, TRUE);

	y += clientHeight * 1 / 6;
	h = fontHeight;
	w = clientWidth / 4;
	::MoveWindow(m_boldCheckButton.getHwnd(), x, y, w, h, TRUE);

	y += h + spaceY;
	::MoveWindow(m_italicCheckButton.getHwnd(), x, y, w, h, TRUE);

	w = clientWidth / 4;
	h = static_cast<int>(fontHeight * 1.5);
	x = clientWidth - w - spaceX * 2;
	y = clientHeight - h - spaceY * 2;
	if (m_applyButton.getHwnd() != nullptr)
	{
		::MoveWindow(m_applyButton.getHwnd(), x, y, w, h, TRUE);
	}
}
/* 適用ボタン */
void CFontSettingDialogue::onApplyButton()
{
	if (m_pTextWriter == nullptr)return;

	std::wstring fontName = m_fontNameComboBox.getSelectedItemText();
	if (!fontName.empty())
	{
		CWinFont winFont;

		bool bold = m_boldCheckButton.isChecked();
		bool italic = m_italicCheckButton.isChecked();

		auto filePaths = winFont.findFontFilePaths(fontName.c_str(), bold, italic);
		if (!filePaths.empty())
		{
			float fontSize = static_cast<float>(m_fontSizeSlider.getPosition());
			float fontThickness = m_fontThicknessSlider.getPosition();

			CD2TextWriter* pD2TextWriter = static_cast<CD2TextWriter*>(m_pTextWriter);
			pD2TextWriter->setFontByFontName(fontName.c_str(), winFont.getLocaleName(), bold, italic);
			pD2TextWriter->setupOutLinedDrawing(filePaths[0].c_str(), bold, italic, fontSize, fontThickness);

			if (m_pFontChangeCallback != nullptr)
			{
				FontCallbackDatum fontCallbackdatum
				{
					.fontFamilyName = fontName.c_str(),
					.localeName = winFont.getLocaleName(),
					.fontFilePath = filePaths[0].c_str(),
					.fontSize = fontSize,
					.fontThickness = fontThickness,
					.bold = bold,
					.italic = italic
				};
				m_pFontChangeCallback(m_pCallbackUserDatum, &fontCallbackdatum);
			}
		}
	}
}

void CFontSettingDialogue::setSliderPosition()
{
	if (m_pTextWriter != nullptr)
	{
		CD2TextWriter* pD2TextWriter = static_cast<CD2TextWriter*>(m_pTextWriter);

		float fFontSize = pD2TextWriter->getFontSize();
		float fStrokeThickness = pD2TextWriter->getThickness();

		m_fontSizeSlider.setPosition(static_cast<long long>(fFontSize));
		m_fontThicknessSlider.setPosition(fStrokeThickness);
	}
}
