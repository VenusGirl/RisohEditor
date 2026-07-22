// MRadWindow.hpp --- RADical development window
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2026 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include "MWindowBase.hpp"
#include "MRubberBand.hpp"
#include "MIndexLabels.hpp"
#include "MOleHost.hpp"
#include "DialogRes.hpp"
#include "IconRes.hpp"
#include "Res.hpp"
#include <map>
#include <unordered_set>     // for std::unordered_set

class MRadCtrl;
class MRadDialog;
class MRadWindow;

//////////////////////////////////////////////////////////////////////////////
// constants

// user-defined window messages
#define MYWM_CTRLMOVE           (WM_USER + 100)     // control was moved
#define MYWM_CTRLSIZE           (WM_USER + 101)     // control was resized
#ifndef MYWM_SELCHANGE
	#define MYWM_SELCHANGE      (WM_USER + 102)     // selection was changed
#endif
#define MYWM_DLGSIZE            (WM_USER + 103)     // dialog was resized
#define MYWM_DELETESEL          (WM_USER + 104)     // selection was deleted
#define MYWM_MOVESIZEREPORT     (WM_USER + 105)     // report moving/resizing
#define MYWM_CLEARSTATUS        (WM_USER + 106)     // clear status
#define MYWM_COMPILECHECK       (WM_USER + 107)     // compilation check
#define MYWM_REOPENRAD          (WM_USER + 108)     // reopen the RADical window
#define MYWM_GETUNITS           (WM_USER + 109)     // get the dialog base units
#define MYWM_UPDATEDLGRES       (WM_USER + 110)     // update the dialog res
#define MYWM_REDRAW             (WM_USER + 111)     // redraw MRadDialog
#define MYWM_RADDBLCLICK       (WM_USER + 115)     // Double-clicked on control

#define GRID_SIZE   5   // grid size

//////////////////////////////////////////////////////////////////////////////
// MRadCtrl --- the RADical controls
// NOTE: An MRadCtrl is a dialog control, subclassed by MRadDialog.

class MRadCtrl : public MWindowBase
{
public:
	DWORD           m_dwMagic;          // magic number to verify the instance
	BOOL            m_bTopCtrl;         // is it a top control?
	MRubberBand     m_hwndRubberBand;   // the rubber band window
	BOOL            m_bMoving;          // is it moving?
	BOOL            m_bSizing;          // is it resizing?
	BOOL            m_bLocking;         // is it locked?
	INT             m_nIndex;           // the control index
	POINT           m_pt;               // the position
	INT             m_nImageType;       // the image type

	// constructor
	MRadCtrl();

	// the default icon
	static HICON& Icon();

	// the default bitmap
	static HBITMAP& Bitmap();

	// is the window a group box?
	static BOOL IsGroupBox(HWND hCtrl);

	// call me after subclassing
	void PostSubclass();

	// the targets (the selected window handles)
	typedef std::unordered_set<HWND> set_type;
	static set_type& GetTargets();

	// the last selection (the selected window handle)
	static HWND& GetLastSel();

	// get the target control indexes
	static std::unordered_set<INT> GetTargetIndeces();

	// the index-to-control mapping
	typedef std::map<INT, HWND> map_type;
	static map_type& IndexToCtrlMap();

	// get the rubber band that is associated to the MRadCtrl
	MRubberBand* GetRubberBand();

	// get the MRadCtrl from a window handle
	static MRadCtrl* GetRadCtrl(HWND hwnd);

	// is the user dragging on the dialog face
	static BOOL& GetRangeSelect(void);

	// deselect the selection
	static BOOL DeselectSelection();

	static LRESULT DoSendMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return ::SendMessage(hwnd, uMsg, wParam, lParam);
	}

	// delete the selection
	static void DeleteSelection();

	// deselect this control
	void Deselect();

	// is it selected?
	BOOL IsSelected() const
	{
		return IsWindow(m_hwndRubberBand);
	}

	// select the control
	static void Select(HWND hwnd);

	static void SelectByIndex(INT nIndex);

	// move the selected RADical controls
	static void MoveSelection(HWND hwnd, INT dx, INT dy);

	// resize the selected RADical controls
	static void ResizeSelection(HWND hwnd, INT cx, INT cy);

	// range selection
	struct RANGE_SELECT
	{
		RECT rc;
		BOOL bCtrlDown;
	};

	// callback function for DoRangeSelect
	static BOOL CALLBACK
	RangeSelectProc(HWND hwnd, LPARAM lParam);

	// do range selection
	static void DoRangeSelect(HWND hwndParent, const RECT *prc, BOOL bCtrlDown);

	// the window procedure of MRadCtrl
	LRESULT CALLBACK
	WindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	struct MYHITTEST
	{
		HWND hParent;
		HWND hCandidate;
		HWND hLast;
		HWND hGroupBox;
		POINT pt;
	};

	// the helper function for hittest
	static BOOL CALLBACK EnumHitTestChildProc(HWND hwnd, LPARAM lParam);

	void DoTest();

protected:
	LRESULT OnRedraw(HWND hwnd, WPARAM wParam, LPARAM lParam);
	BOOL OnEraseBkgnd(HWND hwnd, HDC hdc);
	void OnNCRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest);
	void OnNCRButtonUp(HWND hwnd, int x, int y, UINT codeHitTest) { /* eat */ }
	void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) { /* eat */ }
	void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags) { /* eat */ }
	void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags) { /* eat */ }
	UINT OnNCHitTest(HWND hwnd, int x, int y);
	void OnMove(HWND hwnd, int x, int y);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	void OnNCLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest);
	void OnNCMouseMove(HWND hwnd, int x, int y, UINT codeHitTest);
	void OnNCLButtonUp(HWND hwnd, int x, int y, UINT codeHitTest);
};

//////////////////////////////////////////////////////////////////////////////
// MRadDialog --- RADical dialog

class MRadDialog : public MDialogBase
{
public:
	BOOL            m_index_visible;        // indeces are visible
	POINT           m_ptClicked;            // the clicked position
	POINT           m_ptDragging;           // the dragging position
	MIndexLabels    m_labels;               // the labels
	BOOL            m_bMovingSizing;        // the lock of moving and/or resizing
	INT             m_xDialogBaseUnit;      // the X dialog base unit
	INT             m_yDialogBaseUnit;      // the Y dialog base unit
	HBRUSH          m_hbrBack;              // the background brush
	std::vector<std::shared_ptr<MRadCtrl>> m_rad_ctrls;

	// contructor
	MRadDialog();

	~MRadDialog();

	// the target types to get
	enum TARGET_TYPE
	{
		TARGET_NEXT,        // get the next target
		TARGET_PREV,        // get the previous target
		TARGET_FIRST,       // get the first target
		TARGET_LAST         // get the last target
	};

	// the structure to get the target control
	struct GET_TARGET
	{
		TARGET_TYPE target;
		HWND hwndTarget;
		INT m_nIndex;
		INT m_nCurrentIndex;
	};

	// EnumChildWindows' callback function to get the target
	static BOOL CALLBACK GetTargetProc(HWND hwnd, LPARAM lParam);

	HWND GetNextCtrl(HWND hwndCtrl) const;
	HWND GetPrevCtrl(HWND hwndCtrl) const;

	static HWND GetFirstCtrl(HWND hwndParent);
	static HWND GetLastCtrl(HWND hwndParent);

	// the dialog procedure of MRadDialog
	INT_PTR CALLBACK
	DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT DoSendMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	// get the normalized rectangle from two points
	void NormalizeRect(RECT *prc, POINT pt0, POINT pt1);

	// draw the dragging rectangle
	void DrawDragSelect(HWND hwnd);

	// the window procedure of MRadDialog
	LRESULT CALLBACK
	WindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	// NOTE: We have to do subclassing all the children controls and their descendants
	//       to modify the hittesting.

	// do subclassing a control and its descendants
	void DoSubclass(HWND hCtrl, INT nIndex);

	// do subclassing the children
	void DoSubclassChildren(HWND hwnd, BOOL bTop = FALSE);

	// create the background brush
	BOOL ReCreateBackBrush();

	// show/hide the labels
	void ShowHideLabels(BOOL bShow = TRUE);

protected:
	void OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	LRESULT OnRedraw(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnSelChange(HWND hwnd, WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
	void OnCaptureChanged(HWND hwnd);
	void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
	BOOL OnEraseBkgnd(HWND hwnd, HDC hdc);
	LRESULT OnRadDblClick(HWND hwnd, WPARAM wParam, LPARAM lParam);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	LRESULT OnDeleteSel(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnCtrlMove(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnCtrlSize(HWND hwnd, WPARAM wParam, LPARAM lParam);
	void OnNCLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest);
	void OnNCLButtonUp(HWND hwnd, int x, int y, UINT codeHitTest);
	void OnNCRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest);
	void OnNCRButtonUp(HWND hwnd, int x, int y, UINT codeHitTest);
	void OnNCMouseMove(HWND hwnd, int x, int y, UINT codeHitTest);
	void OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	void OnSysColorChange(HWND hwnd);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
};

//////////////////////////////////////////////////////////////////////////////
// MRadWindow --- the RADical window
// NOTE: An MRadWindow contains an MRadDialog.

class MRadWindow : public MWindowBase
{
public:
	INT             m_xDialogBaseUnit;      // the X dialog base unit
	INT             m_yDialogBaseUnit;      // the Y dialog base unit
	MRadDialog      m_rad_dialog;           // the MRadDialog instance
	DialogRes       m_dialog_res;           // the dialog resource
	HICON           m_hIcon;                // the icon
	HICON           m_hIconSm;              // the small icon
	MTitleToBitmap  m_title_to_bitmap;      // a title-to-bitmap mapping
	MTitleToIcon    m_title_to_icon;        // a title-to-icon mapping
	DialogItemClipboard m_clipboard;        // a clipboard manager
	std::shared_ptr<MOleHost> m_pOleHost;

	// constructor
	MRadWindow();

	// create the mappings
	void create_maps(LANGID lang);

	// clear the mappings
	void clear_maps();

	// destructor
	~MRadWindow();

	VOID DestroyWindow();

	// create an MRadWindow window
	BOOL CreateDx(HWND hwndParent);

	// convert the coordinates
	void ClientToDialog(POINT *ppt);
	void ClientToDialog(SIZE *psiz);
	void ClientToDialog(RECT *prc);
	void DialogToClient(POINT *ppt);
	void DialogToClient(SIZE *psiz);
	void DialogToClient(RECT *prc);

	static HWND GetPrimaryControl(HWND hwnd, HWND hwndDialog);

	// adjust MRadWindow's position and size to MRadDialog's client area
	void FitToRadDialog();

	// the window class name
	LPCTSTR GetWndClassNameDx() const override;

	void ModifyWndClassDx(WNDCLASSEX& wcx) override;

	LRESULT DoSendMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// recreate the MRadDialog window
	BOOL ReCreateRadDialog(HWND hwnd, INT nSelectStartIndex = -1);

	// update the mappings
	void update_maps();

	// the window procedure of MRadWindow
	LRESULT CALLBACK
	WindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	// update the resource
	void UpdateRes();

	// get the selected dialog items
	BOOL GetSelectedItems(DialogItems& items);

	// able to make it top index?
	BOOL CanIndexTop() const;

	// make it top index
	void IndexTop(HWND hwnd);

	// able to make it bottom index?
	BOOL CanIndexBottom() const;

	// make it bottom index
	void IndexBottom(HWND hwnd);

	// able to decrement the control index?
	BOOL CanIndexMinus() const;

	// decrement the control index
	void IndexMinus(HWND hwnd);

	// able to increment the control index?
	BOOL CanIndexPlus() const;

	// increment the control index
	void IndexPlus(HWND hwnd);

	// select all the RADical controls
	void SelectAll(HWND hwnd);

	// get the dialog base units
	BOOL GetBaseUnits(INT& xDialogBaseUnit, INT& yDialogBaseUnit);

	// fit the coordinates to the grid
	void FitToGrid(POINT *ppt);
	void FitToGrid(SIZE *psiz);
	void FitToGrid(RECT *prc);

	void OnAddCtrl(HWND hwnd);
	void OnCtrlProp(HWND hwnd);
	void OnDlgProp(HWND hwnd);
	void OnShowHideIndex(HWND hwnd);
	void OnTopAlign(HWND hwnd);
	void OnBottomAlign(HWND hwnd);
	void OnLeftAlign(HWND hwnd);
	void OnRightAlign(HWND hwnd);
	void OnFitToGrid(HWND hwnd);
	void OnRefresh(HWND hwnd);

protected:
	BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
	void OnDestroy(HWND hwnd);
	void OnSysColorChange(HWND hwnd);
	LRESULT OnRadDblClick(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnGetUnits(HWND hwnd, WPARAM wParam, LPARAM lParam);
	void OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
	LRESULT OnSelChange(HWND hwnd, WPARAM wParam, LPARAM lParam);
	void OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu);
	LRESULT OnDeleteSel(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnCtrlMove(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnCtrlSize(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnDlgSize(HWND hwnd, WPARAM wParam, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	void OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos);
	void OnMove(HWND hwnd, int x, int y);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
};
