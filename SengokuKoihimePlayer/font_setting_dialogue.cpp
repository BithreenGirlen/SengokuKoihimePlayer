
#include "font_setting_dialogue.h"

#include "dialogue_template.h"
#include "win_font.h"
#include "d2_text_writer.h"

CFontSettingDialogue::CFontSettingDialogue()
{
	int iFontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(iFontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"yumin");
}

CFontSettingDialogue::~CFontSettingDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

HWND CFontSettingDialogue::Open(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, void* pTextWriter)
{
	CDialogueTemplate sWinDialogueTemplate;
	sWinDialogueTemplate.SetWindowSize(160, 160);
	std::vector<unsigned char> dialogueTemplate = sWinDialogueTemplate.Generate(pwzWindowName);

	m_pTextWriter = pTextWriter;

	return ::CreateDialogIndirectParam(hInstance, (LPCDLGTEMPLATE)dialogueTemplate.data(), hWndParent, (DLGPROC)DialogProc, (LPARAM)this);
}
/*C CALLBACK*/
LRESULT CFontSettingDialogue::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		::SetWindowLongPtr(hWnd, DWLP_USER, lParam);
	}

	auto pThis = reinterpret_cast<CFontSettingDialogue*>(::GetWindowLongPtr(hWnd, DWLP_USER));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
/*メッセージ処理*/
LRESULT CFontSettingDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return OnInit(hWnd);
	case WM_SIZE:
		return OnSize();
	case WM_CLOSE:
		return OnClose();
	case WM_NOTIFY:
		return OnNotify(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_VSCROLL:
		return OnVScroll(wParam, lParam);
	default:
		break;
	}
	return FALSE;
}
/*WM_INITDIALOG*/
LRESULT CFontSettingDialogue::OnInit(HWND hWnd)
{
	m_hWnd = hWnd;

	m_fontNameStatic.Create(L"Font name", m_hWnd);
	m_fontNameComboBox.Create(m_hWnd);

	m_fontSizeStatic.Create(L"Size", m_hWnd);
	m_fontSizeSlider.Create(L"", m_hWnd, reinterpret_cast<HMENU>(Controls::kFontSizeSlider), 8, 64, 1);

	m_fontThicknessStatic.Create(L"Thickness", m_hWnd);
	m_fontThicknessSlider.Create(L"", m_hWnd, reinterpret_cast<HMENU>(Controls::kFontThicknessSlider), 0.f, 6.f, 0.1f);

	m_boldCheckButton.Create(L"Bold", m_hWnd, reinterpret_cast<HMENU>(Controls::kBoldCheckButton), true);
	m_italicCheckButton.Create(L"Italic", m_hWnd, reinterpret_cast<HMENU>(Controls::kItalicCheckButton), true);

	m_applyButton.Create(L"Apply", m_hWnd, reinterpret_cast<HMENU>(Controls::kApplyButton));

	CWinFont sWinFont;

	std::vector<std::wstring> fontNames = sWinFont.GetSystemFontFamilyNames();
	m_fontNameComboBox.Setup(fontNames);

	if (m_pTextWriter != nullptr)
	{
		CD2TextWriter* pD2TextWriter = static_cast<CD2TextWriter*>(m_pTextWriter);

		m_boldCheckButton.SetCheckBox(pD2TextWriter->HasBoldStyle());
		m_italicCheckButton.SetCheckBox(pD2TextWriter->HasItalicStyle());

		wchar_t sBuffer[LOCALE_NAME_MAX_LENGTH]{};
		pD2TextWriter->GetFontFamilyName(sBuffer, sizeof(sBuffer) / sizeof(wchar_t));
		if (sBuffer[0] != 'L\0')
		{
			int iIndex = m_fontNameComboBox.FindIndex(sBuffer);
			if (iIndex != -1)
			{
				m_fontNameComboBox.SetSelectedItem(iIndex);
			}
			else
			{
				/*実行環境の言語・文字による表記で再探索。*/
				std::wstring wstrLocaleFontName = sWinFont.FindLocaleFontName(sBuffer);
				if (!wstrLocaleFontName.empty())
				{
					int iIndex = m_fontNameComboBox.FindIndex(wstrLocaleFontName.c_str());
					if (iIndex != -1)
					{
						m_fontNameComboBox.SetSelectedItem(iIndex);
					}
				}
			}
		}
	}

	ResizeControls();

	SetSliderPosition();

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return TRUE;
}
/*WM_CLOSE*/
LRESULT CFontSettingDialogue::OnClose()
{
	::DestroyWindow(m_hWnd);
	m_hWnd = nullptr;

	return 0;
}
/*WM_SIZE*/
LRESULT CFontSettingDialogue::OnSize()
{
	ResizeControls();

	return 0;
}
/*WM_NOTIFY*/
LRESULT CFontSettingDialogue::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pNmhdr = reinterpret_cast<LPNMHDR>(lParam);
	if (pNmhdr != nullptr)
	{
		if (pNmhdr->code == TTN_NEEDTEXT)
		{
			if (pNmhdr->hwndFrom == m_fontThicknessSlider.GetToolTipHandle())
			{
				m_fontThicknessSlider.OnToolTipNeedText(reinterpret_cast<LPTOOLTIPTEXT>(lParam));
			}
		}
	}
	return 0;
}
/*WM_COMMAND*/
LRESULT CFontSettingDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmKind = LOWORD(lParam);
	if (wmKind == 0)
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
			switch (wmId)
			{
			case Controls::kApplyButton:
				OnApplyButton();
				break;
			case Controls::kBoldCheckButton:
				m_boldCheckButton.SetCheckBox(!m_boldCheckButton.IsChecked());
				break;
			case Controls::kItalicCheckButton:
				m_italicCheckButton.SetCheckBox(!m_italicCheckButton.IsChecked());
				break;
			default:
				break;
			}
		}
	}

	return 0;
}
/*WM_VSCROLL*/
LRESULT CFontSettingDialogue::OnVScroll(WPARAM wParam, LPARAM lParam)
{
	return 0;
}
/*EnumChildWindows CALLBACK*/
BOOL CFontSettingDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	/*TRUE: 続行, FALSE: 終了*/
	return TRUE;
}
/*再配置*/
void CFontSettingDialogue::ResizeControls()
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

	int iFontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);

	if (m_fontNameStatic.GetHwnd() != nullptr)
	{
		::MoveWindow(m_fontNameStatic.GetHwnd(), x, y, w, h, TRUE);
	}

	y += iFontHeight;
	if (m_fontNameComboBox.GetHwnd() != nullptr)
	{
		::MoveWindow(m_fontNameComboBox.GetHwnd(), x, y, w, h, TRUE);
	}

	y += clientHeight * 1 / 6;
	h = clientHeight * 1 / 6;
	::MoveWindow(m_fontSizeStatic.GetHwnd(), x, y, w, h, TRUE);

	y += iFontHeight;
	::MoveWindow(m_fontSizeSlider.GetHwnd(), x, y, w, h, TRUE);

	y += clientHeight * 1 / 6;
	::MoveWindow(m_fontThicknessStatic.GetHwnd(), x, y, w, h, TRUE);

	y += iFontHeight;
	::MoveWindow(m_fontThicknessSlider.GetHwnd(), x, y, w, h, TRUE);

	y += clientHeight * 1 / 6;
	h = iFontHeight;
	w = clientWidth / 4;
	::MoveWindow(m_boldCheckButton.GetHwnd(), x, y, w, h, TRUE);

	y += h + spaceY;
	::MoveWindow(m_italicCheckButton.GetHwnd(), x, y, w, h, TRUE);

	w = clientWidth / 4;
	h = static_cast<int>(iFontHeight * 1.5);
	x = clientWidth - w - spaceX * 2;
	y = clientHeight - h - spaceY * 2;
	if (m_applyButton.GetHwnd() != nullptr)
	{
		::MoveWindow(m_applyButton.GetHwnd(), x, y, w, h, TRUE);
	}
}
/*適用ボタン*/
void CFontSettingDialogue::OnApplyButton()
{
	if (m_pTextWriter == nullptr)return;

	std::wstring wstrFontName = m_fontNameComboBox.GetSelectedItemText();
	if (!wstrFontName.empty())
	{
		CWinFont sWinFont;

		bool bBold = m_boldCheckButton.IsChecked();
		bool bItalic = m_italicCheckButton.IsChecked();

		auto filePaths = sWinFont.FindFontFilePaths(wstrFontName.c_str(), bBold, bItalic);
		if (!filePaths.empty())
		{
			float fFontSize = static_cast<float>(m_fontSizeSlider.GetPosition());
			float fThickness = m_fontThicknessSlider.GetPosition();

			CD2TextWriter* pD2TextWriter = static_cast<CD2TextWriter*>(m_pTextWriter);
			pD2TextWriter->SetFontByFontName(wstrFontName.c_str(), sWinFont.GetLocaleName(), bBold, bItalic);
			pD2TextWriter->SetupOutLinedDrawing(filePaths[0].c_str(), bBold, bItalic, fFontSize, fThickness);
		}
	}
}

void CFontSettingDialogue::SetSliderPosition()
{
	if (m_pTextWriter != nullptr)
	{
		CD2TextWriter* pD2TextWriter = static_cast<CD2TextWriter*>(m_pTextWriter);

		float fFontSize = pD2TextWriter->GetFontSize();
		float fStrokeThickness = pD2TextWriter->GetStrokeThickness();

		m_fontSizeSlider.SetPosition(static_cast<long long>(fFontSize));
		m_fontThicknessSlider.SetPosition(fStrokeThickness);
	}
}
