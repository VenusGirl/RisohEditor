// MCtrlPropDlg.hpp --- "Properties for Control" Dialog
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2026 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include "resource.h"
#include "MToolBarCtrl.hpp"
#include "RisohSettings.hpp"
#include "ConstantsDB.hpp"
#include "MComboBoxAutoComplete.hpp"
#include "DialogRes.hpp"
#include "MString.hpp"
#include "Common.hpp"

#include <unordered_set>     // for std::unordered_set
#include <oledlg.h>

//////////////////////////////////////////////////////////////////////////////

class MCtrlPropDlg : public MDialogBase
{
public:
	static constexpr LPCTSTR MULTIPLE_TEXT = TEXT("<multiple>");
	enum Flags
	{
		F_NONE = 0,
		F_HELP = 0x0001,
		F_STYLE = 0x0002,
		F_EXSTYLE = 0x0004,
		F_X = 0x0008,
		F_Y = 0x0010,
		F_CX = 0x0020,
		F_CY = 0x0040,
		F_ID = 0x0080,
		F_CLASS = 0x0100,
		F_TITLE = 0x0200,
		F_EXTRA = 0x0400,
		F_SLIST = 0x0800,
		F_ALL = 0x0FFF
	};
	DialogRes&          m_dialog_res;
	BOOL                m_bUpdating = FALSE;
	std::unordered_set<INT>       m_indeces;
	DWORD               m_flags;
	BOOL                m_bClassMixed = FALSE;
	BOOL                m_bTitleMixed = FALSE;
	BOOL                m_bIDMixed = FALSE;
	DWORD               m_dwStyle = 0;
	DWORD               m_dwExStyle = 0;
	DialogItem          m_item;
	ConstantsDB::TableType  m_style_table;
	ConstantsDB::TableType  m_exstyle_table;
	std::vector<BYTE>       m_style_selection;
	std::vector<BYTE>       m_exstyle_selection;
	MToolBarCtrl            m_hTB;
	HIMAGELIST              m_himlControls;
	std::vector<std::wstring> m_vecControls;
	MComboBoxAutoComplete m_cmb1;
	MComboBoxAutoComplete m_cmb2;
	MComboBoxAutoComplete m_cmb3;
	MComboBoxAutoComplete m_cmb4;
	MComboBoxAutoComplete m_cmb5;

	MCtrlPropDlg(DialogRes& dialog_res, const std::unordered_set<INT>& indeces);
	~MCtrlPropDlg();

	void GetInfo();
	BOOL SetInfo(DWORD flags);

	DWORD GetItemAndFlags(DialogItem& item);

	void ApplySelection(HWND hLst, std::vector<BYTE>& sel);
	void ApplySelection(HWND hLst, ConstantsDB::TableType& table,
						std::vector<BYTE>& sel, DWORD dwValue);

	void UpdateClass(HWND hwnd, HWND hLst1, const MString& strClass);

	INT_PTR CALLBACK
	DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

protected:
	void InitToolBar();
	void InitTables(LPCTSTR pszClass);

	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnOK(HWND hwnd);
	void OnLst1(HWND hwnd);
	void OnLst2(HWND hwnd);
	void OnEdt6(HWND hwnd);
	void OnEdt7(HWND hwnd);
	void OnPsh1(HWND hwnd);
	void OnPsh2(HWND hwnd);
	void OnPsh3(HWND hwnd);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr);
};
