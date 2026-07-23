// MAddCtrlDlg.hpp --- "Add Control" Dialog
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2018 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include "DialogRes.hpp"
#include "MString.hpp"
#include "MToolBarCtrl.hpp"
#include "MComboBoxAutoComplete.hpp"
#include "Common.hpp"
#include "Res.hpp"

class MAddCtrlDlg;

//////////////////////////////////////////////////////////////////////////////

class MAddCtrlDlg : public MDialogBase
{
public:
	DialogRes&      m_dialog_res;
	BOOL            m_bUpdating;
	DWORD           m_dwStyle;
	DWORD           m_dwExStyle;
	POINT           m_pt;
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
	std::vector<BYTE> m_data;
	std::vector<MStringA> m_str_list;

	MAddCtrlDlg(DialogRes& dialog_res, POINT pt);
	~MAddCtrlDlg();

	void ApplySelection(HWND hLst, std::vector<BYTE>& sel);
	void ApplySelection(HWND hLst, ConstantsDB::TableType& table,
						std::vector<BYTE>& sel, DWORD dwValue);

	void UpdateClass(HWND hwnd, HWND hLst1, const MString& strClass);

	INT_PTR CALLBACK
	DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

protected:
	void InitTables(LPCTSTR pszClass);
	void InitToolBar();

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
