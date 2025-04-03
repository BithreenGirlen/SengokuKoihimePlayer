﻿
#include <Windows.h>
#include <CommCtrl.h>

#include "dialogue_controls.h"


CListView::CListView()
{

}

CListView::~CListView()
{

}
/*ListView作成*/
bool CListView::Create(HWND hParentWnd, const std::vector<std::wstring>& columnNames, bool bHasCheckBox)
{
	m_hWnd = ::CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_ALIGNLEFT | WS_TABSTOP | LVS_SINGLESEL, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(nullptr), nullptr);
	if (m_hWnd != nullptr)
	{
		ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | (bHasCheckBox ? LVS_EX_CHECKBOXES : 0) | LVS_EX_HEADERDRAGDROP);

		LVCOLUMNW lvColumn{};
		lvColumn.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		for (size_t i = 0; i < columnNames.size(); ++i)
		{
			lvColumn.iSubItem = static_cast<int>(i);
			lvColumn.pszText = const_cast<LPWSTR>(columnNames.at(i).data());
			ListView_InsertColumn(m_hWnd, i, &lvColumn);
		}
	}
	return m_hWnd != nullptr;
}
/*ListView幅調整*/
void CListView::AdjustWidth()
{
	if (m_hWnd != nullptr)
	{
		int iColumnCount = GetColumnCount();
		if (iColumnCount != -1)
		{
			RECT rect;
			::GetClientRect(m_hWnd, &rect);
			int iWindowWidth = rect.right - rect.left;

			LVCOLUMNW lvColumn{};
			lvColumn.mask = LVCF_WIDTH;
			lvColumn.cx = iWindowWidth / iColumnCount;
			for (int i = 0; i < iColumnCount; ++i)
			{
				ListView_SetColumn(m_hWnd, i, &lvColumn);
			}
		}
	}
}
/*項目追加*/
bool CListView::Add(const std::vector<std::wstring>& columns, bool ToBottom)
{
	if (m_hWnd == nullptr)return false;

	int iItem = GetItemCount();
	if (iItem == -1)return false;

	LRESULT lResult = -1;
	for (size_t i = 0; i < columns.size(); ++i)
	{
		LVITEM lvItem{};
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;

		lvItem.iItem = ToBottom ? iItem : 0;
		lvItem.iSubItem = static_cast<int>(i);
		lvItem.pszText = const_cast<wchar_t*>(columns.at(i).c_str());

		if (i == 0)
		{
			lResult = ::SendMessage(m_hWnd, LVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&lvItem));
			if (lResult == -1)return false;
			iItem = static_cast<int>(lResult);
		}
		else
		{
			lResult = ::SendMessage(m_hWnd, LVM_SETITEMTEXT, iItem, reinterpret_cast<LPARAM>(&lvItem));
			if (lResult == -1)return false;
		}
	}

	return true;
}
/*リスト項目消去*/
void CListView::Clear()
{
	if (m_hWnd != nullptr)
	{
		ListView_DeleteAllItems(m_hWnd);
	}
}
/*単要素リスト構築*/
void CListView::CreateSingleList(const std::vector<std::wstring>& names)
{
	if (m_hWnd != nullptr)
	{
		Clear();
		for (const auto& name : names)
		{
			std::vector<std::wstring> columns;
			columns.push_back(name);
			Add(columns);
		}
	}
}
/*全選択項目文字列拾い上げ*/
std::vector<std::wstring> CListView::PickupCheckedItems()
{
	std::vector<std::wstring> checkedItems;
	if (m_hWnd != nullptr)
	{
		int iCount = GetItemCount();
		if (iCount != -1 && iCount != 0)
		{
			checkedItems.reserve(iCount);
			for (int i = 0; i < iCount; ++i)
			{
				UINT uiRet = ListView_GetCheckState(m_hWnd, i);
				if (uiRet == 1)
				{
					std::wstring wstr = GetItemText(i, 0);
					if (!wstr.empty())
					{
						checkedItems.push_back(wstr);
					}
				}
			}
		}
	}
	return checkedItems;
}
/*名称総数取得*/
int CListView::GetColumnCount()
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessage(m_hWnd, LVM_GETHEADER, 0, 0);
		if (lResult != 0)
		{
			HWND hHeaderWnd = reinterpret_cast<HWND>(lResult);

			lResult = ::SendMessage(hHeaderWnd, HDM_GETITEMCOUNT, 0, 0);
			return static_cast<int>(lResult);
		}
	}
	return -1;
}
/*項目数取得*/
int CListView::GetItemCount()
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessage(m_hWnd, LVM_GETITEMCOUNT, 0, 0);
		return static_cast<int>(lResult);
	}
	return -1;
}
/*指定項目の文字列取得*/
std::wstring CListView::GetItemText(int iRow, int iColumn)
{
	std::wstring wstrResult;
	if (m_hWnd != nullptr)
	{
		LV_ITEM lvItem{};
		lvItem.iSubItem = iColumn;

		for (int iSize = 256; iSize < 1025; iSize *= 2)
		{
			std::vector<wchar_t> vBuffer(iSize, L'\0');

			lvItem.cchTextMax = iSize;
			lvItem.pszText = vBuffer.data();
			int iLen = static_cast<int>(::SendMessage(m_hWnd, LVM_GETITEMTEXT, iRow, reinterpret_cast<LPARAM>(&lvItem)));
			if (iLen < iSize - 1)
			{
				wstrResult = vBuffer.data();
				break;
			}
		}
	}
	return wstrResult;
}

/* ==================== ListBox ====================
* 
* ListBox lacks the equivalent to ListView's LVS_EX_DOUBLEBUFFER,
* so takes longer time than ListView in its scrolling.
*/


CListBox::CListBox()
{

}

CListBox::~CListBox()
{

}
/*ListBox作成*/
bool CListBox::Create(HWND hParentWnd)
{
	m_hWnd = ::CreateWindowExW(0, WC_LISTBOX, L"ListBox", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_SORT | LBS_NOINTEGRALHEIGHT | WS_VSCROLL, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(nullptr), nullptr);
	return m_hWnd != nullptr;
}
/*項目追加*/
void CListBox::Add(const wchar_t* szText, bool ToBottom)
{
	if (m_hWnd != nullptr)
	{
		if (ToBottom)
		{
			::SendMessage(m_hWnd, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szText));
		}
		else
		{
			::SendMessage(m_hWnd, LB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(szText));
		}
	}
}
/*全消去*/
void CListBox::Clear()
{
	if (m_hWnd != nullptr)
	{
		::SendMessage(m_hWnd, LB_RESETCONTENT, 0, 0);
	}
}
/*選択項目文字列取得*/
std::wstring CListBox::GetSelectedItemName()
{
	if (m_hWnd != nullptr)
	{
		long long llSelected = GetSelectedItemIndex();
		if (llSelected != -1)
		{
			LRESULT lResult = ::SendMessage(m_hWnd, LB_GETTEXTLEN, 0, llSelected);
			if (lResult != LB_ERR)
			{
				std::vector<wchar_t> vBuffer(lResult + 1LL, L'\0');
				lResult = ::SendMessage(m_hWnd, LB_GETTEXT, 0, reinterpret_cast<LPARAM>(vBuffer.data()));
				if (lResult != LB_ERR)
				{
					return vBuffer.data();
				}
			}
		}
	}
	return std::wstring();
}
/*選択項目番号取得*/
long long CListBox::GetSelectedItemIndex()
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessage(m_hWnd, LB_GETCURSEL, 0, 0);

		return lResult;
	}
	return LB_ERR;
}

/* ==================== ComboBox ==================== */

CComboBox::CComboBox()
{

}

CComboBox::~CComboBox()
{

}
/*作成*/
bool CComboBox::Create(HWND hParentWnd)
{
	m_hWnd = ::CreateWindowEx(0, WC_COMBOBOXW, L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(NULL), nullptr);
	return m_hWnd != nullptr;
}
/*項目構築*/
void CComboBox::Setup(const std::vector<std::wstring>& itemTexts)
{
	Clear();

	if (m_hWnd != nullptr)
	{
		for (const auto& itemText : itemTexts)
		{
			::SendMessageW(m_hWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(itemText.c_str()));
		}
		SetSelectedItem(0);
	}
}
/*選択項目番号取得*/
int CComboBox::GetSelectedItemIndex()
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0);
		return static_cast<int>(lResult);
	}
	return CB_ERR;
}
/*選択項目文字列取得*/
std::wstring CComboBox::GetSelectedItemText()
{
	if (m_hWnd != nullptr)
	{
		int iIndex = GetSelectedItemIndex();
		if (iIndex != CB_ERR)
		{
			LRESULT lResult = ::SendMessage(m_hWnd, CB_GETLBTEXTLEN, iIndex, 0);
			if (lResult != CB_ERR)
			{
				std::vector<wchar_t> vBuffer(lResult + 1, L'\0');
				lResult = ::SendMessageW(m_hWnd, CB_GETLBTEXT, iIndex, reinterpret_cast<LPARAM>(vBuffer.data()));
				if (lResult != CB_ERR)
				{
					return vBuffer.data();
				}
			}
		}
	}
	return std::wstring();
}

int CComboBox::FindIndex(const wchar_t* szName)
{
	if (m_hWnd != nullptr)
	{
		return static_cast<int>(::SendMessage(m_hWnd, CB_FINDSTRING, -1, reinterpret_cast<LPARAM>(szName)));
	}
	return CB_ERR;
}
/*選択項目指定*/
bool CComboBox::SetSelectedItem(int iIndex)
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessage(m_hWnd, CB_SETCURSEL, iIndex, 0);
		return iIndex == -1 ? lResult == CB_ERR : lResult == iIndex;
	}
	return false;
}
/*消去*/
void CComboBox::Clear()
{
	if (m_hWnd != nullptr)
	{
		::SendMessage(m_hWnd, CB_RESETCONTENT, 0, 0);
	}
}

/* ==================== Button ==================== */

CButton::CButton()
{

}

CButton::~CButton()
{

}

bool CButton::Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, bool bHasCheckBox)
{
	m_hWnd = ::CreateWindowEx(0, WC_BUTTON, szText, WS_VISIBLE | WS_CHILD | WS_TABSTOP | (bHasCheckBox ? BS_CHECKBOX : 0), 0, 0, 0, 0, hParentWnd, hMenu, ::GetModuleHandle(NULL), nullptr);
	return m_hWnd != nullptr;
}

void CButton::SetCheckBox(bool bToBeChecked)
{
	if (m_hWnd != nullptr)
	{
		/*BM_SETCHECK always return 0*/
		::SendMessage(m_hWnd, BM_SETCHECK, bToBeChecked ? BST_CHECKED : BST_UNCHECKED, 0);
	}
}

bool CButton::IsChecked()
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessage(m_hWnd, BM_GETCHECK, 0, 0);
		return lResult == BST_CHECKED;
	}

	return false;
}

/* ==================== Integer trackbar ==================== */

CSlider::CSlider()
{

}

CSlider::~CSlider()
{

}

bool CSlider::Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, unsigned short usMin, unsigned short usMax, unsigned int uiRange, bool bVertical)
{
	m_hWnd = ::CreateWindowExW(
		0, TRACKBAR_CLASS, szText,
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_TOOLTIPS | TBS_BOTH | (bVertical ? TBS_VERT : 0),
		0, 0, 0, 0, hParentWnd, hMenu, ::GetModuleHandleA(nullptr), nullptr
	);

	if (m_hWnd != nullptr)
	{
		::SendMessage(m_hWnd, TBM_SETRANGE, TRUE, MAKELONG(usMin, usMax));
		::SendMessage(m_hWnd, TBM_SETPAGESIZE, TRUE, uiRange);
	}

	return m_hWnd != nullptr;
}

long long CSlider::GetPosition() const
{
	return ::SendMessage(m_hWnd, TBM_GETPOS, 0, 0);
}

void CSlider::SetPosition(long long llPos) const
{
	::SendMessage(m_hWnd, TBM_SETPOS, TRUE, llPos);
}

HWND CSlider::GetToolTipHandle()
{
	return reinterpret_cast<HWND>(::SendMessage(m_hWnd, TBM_GETTOOLTIPS, 0, 0));
}

/* ==================== Float trackBar ==================== */

CFloatSlider::CFloatSlider()
{

}

CFloatSlider::~CFloatSlider()
{

}

bool CFloatSlider::Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, float fMin, float fMax, float fRange, unsigned int uiRatio, bool bVertical)
{
	m_hWnd = ::CreateWindowExW(
		0, TRACKBAR_CLASS, szText,
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_TOOLTIPS | TBS_BOTH | (bVertical ? TBS_VERT : 0),
		0, 0, 0, 0, hParentWnd, hMenu, ::GetModuleHandleA(nullptr), nullptr
	);

	if (uiRatio > 0)m_uiRatio = uiRatio;

	if (m_hWnd != nullptr)
	{
		unsigned int uiMin = static_cast<unsigned int>(fMin * m_uiRatio);
		unsigned int uiMax = static_cast<unsigned int>(fMax * m_uiRatio);
		unsigned int uiRange = static_cast<unsigned int>(fRange * m_uiRatio);

		::SendMessage(m_hWnd, TBM_SETRANGE, TRUE, MAKELONG(uiMin, uiMax));
		::SendMessage(m_hWnd, TBM_SETPAGESIZE, TRUE, uiRange);
	}

	return m_hWnd != nullptr;
}

float CFloatSlider::GetPosition() const
{
	return ::SendMessage(m_hWnd, TBM_GETPOS, 0, 0) / static_cast<float>(m_uiRatio);
}

void CFloatSlider::SetPosition(float fPos) const
{
	::SendMessage(m_hWnd, TBM_SETPOS, TRUE, static_cast<LPARAM>(fPos * m_uiRatio));
}

HWND CFloatSlider::GetToolTipHandle()
{
	return reinterpret_cast<HWND>(::SendMessage(m_hWnd, TBM_GETTOOLTIPS, 0, 0));
}

void CFloatSlider::OnToolTipNeedText(LPNMTTDISPINFOW pNmtTextDispInfo)
{
	if (pNmtTextDispInfo != nullptr)
	{
		long n = 0;
		for (long l = m_uiRatio; l > 1; ++n, l = l / kDefaultRatio);
		/* LPNMTTDISPINFOW::szText is 80 chars in length. */
		swprintf_s(pNmtTextDispInfo->szText, L"%0.*f", n, GetPosition());
	}

}

/* ==================== Static ==================== */

CStatic::CStatic()
{

}

CStatic::~CStatic()
{

}

bool CStatic::Create(const wchar_t* szText, HWND hParentWnd)
{
	m_hWnd = ::CreateWindowEx(0, WC_STATIC, szText, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(NULL), nullptr);

	return m_hWnd;
}
