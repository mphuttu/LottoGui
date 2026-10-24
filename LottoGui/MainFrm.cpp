
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "LottoGui.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

static UINT GetFrameDpi(HWND hWnd)
{
	const UINT dpi = ::GetDpiForWindow(hWnd);
	return dpi == 0 ? 96 : dpi;
}

static int ScaleForDpi(int value, UINT dpi)
{
	return ::MulDiv(value, dpi, 96);
}

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!InitializeMenuIcons())
	{
		TRACE0("Failed to initialize menu icons\n");
		return -1;
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);


	return 0;
}

BOOL CMainFrame::InitializeMenuIcons()
{
	static const struct
	{
		UINT commandId;
		int imageIndex;
	} menuIcons[] =
	{
		{ ID_FILE_NEW, 0 },
		{ ID_FILE_OPEN, 1 },
		{ ID_FILE_SAVE, 2 },
		{ ID_FILE_SAVE_AS, 3 },
		{ ID_FILE_PRINT, 4 },
		{ ID_FILE_PRINT_PREVIEW, 5 },
		{ ID_FILE_PRINT_SETUP, 6 },
		{ ID_APP_EXIT, 7 },
		{ ID_EDIT_UNDO, 8 },
		{ ID_EDIT_CUT, 9 },
		{ ID_EDIT_COPY, 10 },
		{ ID_EDIT_PASTE, 11 },
		{ ID_HELP_USAGE, 12 },
		{ ID_APP_ABOUT, 13 }
	};
	const int menuIconCount = sizeof(menuIcons) / sizeof(menuIcons[0]);

	CMenu* menu = GetMenu();
	if (menu == NULL)
		return FALSE;

	if (!RebuildMenuImageList(GetFrameDpi(m_hWnd)))
		return FALSE;

	for (int i = 0; i < menuIconCount; ++i)
	{
		m_menuIconItems[i].imageIndex = menuIcons[i].imageIndex;
		if (!SetMenuItemOwnerDraw(menu->GetSafeHmenu(), menuIcons[i].commandId,
			m_menuIconItems[i]))
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CMainFrame::RebuildMenuImageList(UINT dpi)
{
	const int imageCellSize = 32;
	const int iconSize = ScaleForDpi(16, dpi);
	const COLORREF maskColor = RGB(192, 192, 192);

	CBitmap menuBitmap;
	if (!menuBitmap.LoadBitmap(IDB_MENUICONS))
		return FALSE;

	BITMAP bitmapInfo = {};
	if (menuBitmap.GetBitmap(&bitmapInfo) == 0 ||
		bitmapInfo.bmHeight != imageCellSize ||
		bitmapInfo.bmWidth <= 0 ||
		bitmapInfo.bmWidth % bitmapInfo.bmHeight != 0)
	{
		return FALSE;
	}

	const int imageSize = bitmapInfo.bmHeight;
	const int imageCount = bitmapInfo.bmWidth / imageSize;
	CImageList menuImages;
	if (imageCount != 14 || !menuImages.Create(iconSize, iconSize,
		ILC_COLOR24 | ILC_MASK, imageCount, 0))
	{
		return FALSE;
	}

	CClientDC screenDC(this);
	CDC sourceDC;
	if (!sourceDC.CreateCompatibleDC(&screenDC))
		return FALSE;

	CBitmap* oldSourceBitmap = sourceDC.SelectObject(&menuBitmap);
	for (int imageIndex = 0; imageIndex < imageCount; ++imageIndex)
	{
		CDC imageDC;
		CBitmap imageBitmap;
		if (!imageDC.CreateCompatibleDC(&screenDC) ||
			!imageBitmap.CreateCompatibleBitmap(&screenDC, iconSize, iconSize))
		{
			sourceDC.SelectObject(oldSourceBitmap);
			return FALSE;
		}

		CBitmap* oldImageBitmap = imageDC.SelectObject(&imageBitmap);
		const int sourceX = imageIndex * imageSize;
		const int oldStretchMode = imageDC.SetStretchBltMode(COLORONCOLOR);
		const BOOL copied = imageDC.StretchBlt(0, 0, iconSize, iconSize,
			&sourceDC, sourceX, 0, imageCellSize, imageCellSize, SRCCOPY);
		imageDC.SetStretchBltMode(oldStretchMode);
		imageDC.SelectObject(oldImageBitmap);

		if (!copied || menuImages.Add(&imageBitmap, maskColor) != imageIndex)
		{
			sourceDC.SelectObject(oldSourceBitmap);
			return FALSE;
		}
	}
	sourceDC.SelectObject(oldSourceBitmap);

	m_menuImages.DeleteImageList();
	return m_menuImages.Attach(menuImages.Detach());
}

BOOL CMainFrame::SetMenuItemOwnerDraw(HMENU hMenu, UINT commandId, MenuIconItem& item)
{
	const int itemCount = ::GetMenuItemCount(hMenu);
	for (int position = 0; position < itemCount; ++position)
	{
		MENUITEMINFO menuItemInfo = {};
		menuItemInfo.cbSize = sizeof(menuItemInfo);
		menuItemInfo.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_FTYPE;
		if (!::GetMenuItemInfo(hMenu, position, TRUE, &menuItemInfo))
			return FALSE;

		if (menuItemInfo.wID == commandId)
		{
			TCHAR label[256] = {};
			MENUITEMINFO labelInfo = {};
			labelInfo.cbSize = sizeof(labelInfo);
			labelInfo.fMask = MIIM_STRING;
			labelInfo.dwTypeData = label;
			labelInfo.cch = _countof(label);
			if (!::GetMenuItemInfo(hMenu, position, TRUE, &labelInfo) ||
				labelInfo.cch >= _countof(label))
			{
				return FALSE;
			}

			item.text = label;
			menuItemInfo.fMask = MIIM_FTYPE | MIIM_DATA;
			menuItemInfo.fType = (menuItemInfo.fType & ~MFT_STRING) | MFT_OWNERDRAW;
			menuItemInfo.dwItemData = reinterpret_cast<ULONG_PTR>(&item);
			return ::SetMenuItemInfo(hMenu, position, TRUE, &menuItemInfo);
		}

		if (menuItemInfo.hSubMenu != NULL &&
			SetMenuItemOwnerDraw(menuItemInfo.hSubMenu, commandId, item))
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CMainFrame::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT measureItem)
{
	if (measureItem != NULL && measureItem->CtlType == ODT_MENU)
	{
		for (int i = 0; i < _countof(m_menuIconItems); ++i)
		{
			if (measureItem->itemData == reinterpret_cast<ULONG_PTR>(&m_menuIconItems[i]))
			{
				CDC* dc = GetDC();
				if (dc == NULL)
					break;

				const UINT dpi = GetFrameDpi(m_hWnd);
				NONCLIENTMETRICS metrics = {};
				metrics.cbSize = sizeof(metrics);
				CFont menuFont;
				CFont* font = CFont::FromHandle(
					static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT)));
				if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0) &&
					menuFont.CreateFontIndirect(&metrics.lfMenuFont))
				{
					font = &menuFont;
				}
				CFont* oldFont = dc->SelectObject(font);
				CString label = m_menuIconItems[i].text;
				CString shortcut;
				const int tabPosition = label.Find(_T('\t'));
				if (tabPosition >= 0)
				{
					shortcut = label.Mid(tabPosition + 1);
					label = label.Left(tabPosition);
				}

				const CSize labelSize = dc->GetTextExtent(label);
				const CSize shortcutSize = dc->GetTextExtent(shortcut);
				dc->SelectObject(oldFont);
				ReleaseDC(dc);

				const int iconSize = ScaleForDpi(16, dpi);
				const int shortcutGap = shortcut.IsEmpty() ? 0 : ScaleForDpi(12, dpi);
				measureItem->itemWidth = ScaleForDpi(28, dpi) + labelSize.cx +
					shortcutGap + shortcutSize.cx + ScaleForDpi(8, dpi);
				measureItem->itemHeight = max(iconSize + ScaleForDpi(4, dpi),
					max(labelSize.cy, shortcutSize.cy) + ScaleForDpi(4, dpi));
				return;
			}
		}
	}

	CFrameWnd::OnMeasureItem(nIDCtl, measureItem);
}

void CMainFrame::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT drawItem)
{
	if (drawItem != NULL && drawItem->CtlType == ODT_MENU)
	{
		for (int i = 0; i < _countof(m_menuIconItems); ++i)
		{
			if (drawItem->itemData == reinterpret_cast<ULONG_PTR>(&m_menuIconItems[i]))
			{
				const BOOL selected = (drawItem->itemState & ODS_SELECTED) != 0;
				const BOOL disabled = (drawItem->itemState & (ODS_DISABLED | ODS_GRAYED)) != 0;
				const COLORREF textColor = ::GetSysColor(
					disabled ? COLOR_GRAYTEXT : (selected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));
				HBRUSH backgroundBrush = ::GetSysColorBrush(
					selected ? COLOR_HIGHLIGHT : COLOR_MENU);
				::FillRect(drawItem->hDC, &drawItem->rcItem, backgroundBrush);

				const UINT dpi = GetFrameDpi(m_hWnd);
				const int iconSize = ScaleForDpi(16, dpi);
				const int iconLeft = ScaleForDpi(6, dpi);
				const int textLeft = ScaleForDpi(28, dpi);
				const int textRight = ScaleForDpi(8, dpi);
				const int shortcutGap = ScaleForDpi(12, dpi);
				const int itemHeight = drawItem->rcItem.bottom - drawItem->rcItem.top;
				const int iconY = drawItem->rcItem.top + (itemHeight - iconSize) / 2;
				UINT imageStyle = ILD_TRANSPARENT;
				if (disabled)
					imageStyle |= ILD_BLEND50;
				::ImageList_Draw(m_menuImages.GetSafeHandle(),
					m_menuIconItems[i].imageIndex, drawItem->hDC,
					drawItem->rcItem.left + iconLeft, iconY, imageStyle);

				CString label = m_menuIconItems[i].text;
				CString shortcut;
				const int tabPosition = label.Find(_T('\t'));
				if (tabPosition >= 0)
				{
					shortcut = label.Mid(tabPosition + 1);
					label = label.Left(tabPosition);
				}

				::SetBkMode(drawItem->hDC, TRANSPARENT);
				::SetTextColor(drawItem->hDC, textColor);
				UINT textFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
				if (drawItem->itemState & ODS_NOACCEL)
					textFlags |= DT_HIDEPREFIX;

				RECT textRect = drawItem->rcItem;
				textRect.left += textLeft;
				textRect.right -= textRight;
				RECT shortcutRect = textRect;
				if (!shortcut.IsEmpty())
				{
					CSize shortcutSize;
					::GetTextExtentPoint32(drawItem->hDC, shortcut, shortcut.GetLength(),
						&shortcutSize);
					shortcutRect.left = shortcutRect.right - shortcutSize.cx;
					textRect.right = shortcutRect.left - shortcutGap;
					::DrawText(drawItem->hDC, shortcut, shortcut.GetLength(),
						&shortcutRect, textFlags | DT_RIGHT);
				}

				::DrawText(drawItem->hDC, label, label.GetLength(), &textRect, textFlags);
				return;
			}
		}
	}

	CFrameWnd::OnDrawItem(nIDCtl, drawItem);
}

LRESULT CMainFrame::OnDpiChanged(WPARAM wParam, LPARAM lParam)
{
	const LRESULT result = Default();
	const UINT dpi = HIWORD(wParam);
	if (!RebuildMenuImageList(dpi))
	{
		TRACE0("Failed to rebuild menu icons for the new DPI\n");
	}
	return result;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers
