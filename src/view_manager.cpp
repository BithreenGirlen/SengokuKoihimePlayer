

#include "view_manager.h"


CViewManager::CViewManager(HWND hWnd)
	:m_hRetWnd(hWnd)
{

}

CViewManager::~CViewManager()
{

}
/*基準長設定*/
void CViewManager::setBaseSize(unsigned int uiWidth, unsigned int uiHeight)
{
	m_uiBaseWidth = uiWidth;
	m_uiBaseHeight = uiHeight;
	workOutDefaultScale();
}
/*尺度変更*/
void CViewManager::rescale(bool toUpscale)
{
	constexpr float fScaleMin = 0.25f;
	constexpr float fScalePortion = 0.05f;
	if (toUpscale)
	{
		m_fScale += fScalePortion;
	}
	else
	{
		m_fScale -= fScalePortion;
		if (m_fScale < fScaleMin) m_fScale = fScaleMin;
	}
	resizeWindow();
}
/*原点位置移動*/
void CViewManager::addOffset(int iX, int iY)
{
	m_fOffsetX += iX;
	m_fOffsetY += iY;
	adjustOffset();
	requestRedraw();
}
/*原寸表示*/
void CViewManager::resetZoom()
{
	m_fScale = m_fDefaultScale;
	m_fOffsetX = 0;
	m_fOffsetY = 0;

	resizeWindow();
}
/*表示形式変更通知*/
void CViewManager::onStyleChanged()
{
	resizeWindow();
}
/*基準尺度算出*/
void CViewManager::workOutDefaultScale()
{
	/* 基準長がモニタ解像度より大きい場合には予め縮小する */

	unsigned int uiMonitorWidth = static_cast<unsigned int>(::GetSystemMetrics(SM_CXSCREEN));
	unsigned int uiMonitorHeight = static_cast<unsigned int>(::GetSystemMetrics(SM_CYSCREEN));
	if (m_uiBaseWidth > uiMonitorWidth || m_uiBaseHeight > uiMonitorHeight)
	{
		if (uiMonitorWidth > uiMonitorHeight)
		{
			m_fDefaultScale = static_cast<float>(uiMonitorHeight) / m_uiBaseHeight;
		}
		else
		{
			m_fDefaultScale = static_cast<float>(uiMonitorWidth) / m_uiBaseWidth;
		}
	}
	else
	{
		m_fDefaultScale = ::GetDpiForWindow(m_hRetWnd) / 96.f;
	}

	m_fScale = m_fDefaultScale;
}
/*窓寸法調整*/
void CViewManager::resizeWindow()
{
	if (m_hRetWnd != nullptr)
	{
		const auto IsWidowBarHidden = [this]()
			-> bool
			{
				if (m_hRetWnd != nullptr)
				{
					LONG lStyle = ::GetWindowLong(m_hRetWnd, GWL_STYLE);
					return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
				}
				return false;
			};

		RECT rect;
		::GetWindowRect(m_hRetWnd, &rect);
		int iX = static_cast<int>(m_uiBaseWidth * m_fScale);
		int iY = static_cast<int>(m_uiBaseHeight * m_fScale);

		rect.right = iX + rect.left;
		rect.bottom = iY + rect.top;
		LONG lStyle = ::GetWindowLong(m_hRetWnd, GWL_STYLE);
		bool bBarHidden = IsWidowBarHidden();
		::AdjustWindowRect(&rect, lStyle, bBarHidden ? FALSE : TRUE);
		::SetWindowPos(m_hRetWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
	}

	adjustOffset();
	requestRedraw();
}
/*原点位置調整*/
void CViewManager::adjustOffset()
{
	if (m_hRetWnd != nullptr)
	{
		int iScaledWidth = static_cast<int>(m_uiBaseWidth * m_fScale);
		int iScaledHeight = static_cast<int>(m_uiBaseHeight * m_fScale);

		RECT rc;
		::GetClientRect(m_hRetWnd, &rc);

		int iClientWidth = rc.right - rc.left;
		int iClientHeight = rc.bottom - rc.top;

		int iXOffsetMax = iScaledWidth > iClientWidth ? static_cast<int>((iScaledWidth - iClientWidth) / m_fScale) : 0;
		int iYOffsetMax = iScaledHeight > iClientHeight ? static_cast<int>((iScaledHeight - iClientHeight) / m_fScale) : 0;

		if (m_fOffsetX < 0) m_fOffsetX = 0;
		if (m_fOffsetY < 0) m_fOffsetY = 0;

		if (m_fOffsetX > iXOffsetMax)m_fOffsetX = static_cast<float>(iXOffsetMax);
		if (m_fOffsetY > iYOffsetMax)m_fOffsetY = static_cast<float>(iYOffsetMax);
	}
}
/*再描画要求*/
void CViewManager::requestRedraw() const
{
	if (m_hRetWnd != nullptr)
	{
		::InvalidateRect(m_hRetWnd, nullptr, FALSE);
	}
}