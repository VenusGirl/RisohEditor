// MCloneInNewTypeDlg.hpp --- "Clone In New Type" Dialog
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2018 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include "resource.h"
#include "MWindowBase.hpp"
#include "ConstantsDB.hpp"
#include "Res.hpp"
#include "MComboBoxAutoComplete.hpp"
#include "Common.hpp"

//////////////////////////////////////////////////////////////////////////////

class MCloneInNewTypeDlg : public MDialogBase
{
public:
	EntryPtr m_entry;
	MIdOrString m_old_type;
	MIdOrString m_new_type;
	LANGID m_lang;
	MComboBoxAutoComplete m_cmb2;

	MCloneInNewTypeDlg(EntryBase *entry)
		: MDialogBase(IDD_CLONEINNEWTYPE), m_entry(g_res.get_shared(entry)),
		  m_old_type(entry->m_type)
	{
	}

	INT_PTR CALLBACK
	DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
			HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
			HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		}
		return 0;
	}

	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
	{
		// for Old Type
		HWND hCmb1 = GetDlgItem(hwnd, cmb1);
		InitResTypeComboBox(hCmb1, m_old_type);
		EnableWindow(hCmb1, FALSE);

		// for New Type
		HWND hCmb2 = GetDlgItem(hwnd, cmb2);
		InitResTypeComboBox(hCmb2, m_new_type);
		SubclassChildDx(m_cmb2, cmb2);

		CenterWindowDx();
		return TRUE;
	}

	void OnOK(HWND hwnd)
	{
		MIdOrString type;
		HWND hCmb2 = GetDlgItem(hwnd, cmb2);
		if (!CheckTypeComboBox(hCmb2, type))
			return;

		if (m_old_type == type)
		{
			ErrorBoxDx(IDS_SAMETYPE);
			return;
		}

		if (g_res.find(ET_TYPE, type))
		{
			if (MsgBoxDx(IDS_EXISTSOVERWRITE, MB_ICONINFORMATION | MB_YESNOCANCEL) != IDYES)
				return;
		}

		m_new_type = type;

		EndDialog(IDOK);
	}

	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch (id)
		{
		case IDOK:
			OnOK(hwnd);
			break;
		case IDCANCEL:
			EndDialog(IDCANCEL);
			break;
		case psh1:
			OnPsh1(hwnd);
			break;
		case cmb2:
			if (codeNotify == CBN_EDITCHANGE)
			{
				m_cmb2.OnEditChange();
			}
			break;
		}
	}

	void OnPsh1(HWND hwnd)
	{
		SendMessage(GetParent(hwnd), WM_COMMAND, ID_IDLIST, 0);
	}
};
