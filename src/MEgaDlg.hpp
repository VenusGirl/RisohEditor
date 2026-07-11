// MEgaDlg.hpp --- Programming Language EGA dialog
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2020 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include "resource.h"
#include "MResizable.hpp"
#include "RisohSettings.hpp"
#include "../EGA/ega.hpp"
#include "EgaBridge.hpp"

using namespace EGA;

#define WM_EGA_DO_GETINPUT (WM_APP + 100)  // UI thread reads edt2 and clear
// WM_EGA_DO_PRINT is defined in EgaBridge.hpp (included above), since
// EgaBridge itself now posts it from QueuePrintText().

class MEgaDlg;
extern HWND s_hwndEga;
extern HWND g_hMainWnd;
extern MIdOrString g_RES_select_type;
extern MIdOrString g_RES_select_name;
extern LANGID g_RES_select_lang;

bool EGA_dialog_input(char *buf, size_t buflen);
void EGA_dialog_print(const char *fmt, va_list va);
void EGA_extension(void);

//////////////////////////////////////////////////////////////////////////////

class MEgaDlg : public MDialogBase
{
public:
	std::wstring m_filename;

	DECLARE_DYNAMIC(MEgaDlg)

	MEgaDlg();
	virtual ~MEgaDlg();

	void ExecuteEgaFile(LPCWSTR filename = nullptr);

	virtual INT_PTR CALLBACK
	DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HFONT m_hFont;
	HICON m_hIcon;
	HICON m_hIconSm;
	MResizable m_resizable;
	INT m_cchEdt1 = 0;

	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnOK(HWND hwnd);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnDestroy(HWND hwnd);
	HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	void OnMove(HWND hwnd, int x, int y);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnEgaGetInput(HWND hwnd);
	void OnEgaPrint(HWND hwnd);
};
