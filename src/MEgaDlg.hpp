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
#define WM_EGA_DO_PRINT    (WM_APP + 101)  // lParam = LPWSTR (UI thread will free)

class MEgaDlg;
extern HWND s_hwndEga;
extern HWND g_hMainWnd;
extern MIdOrString g_RES_select_type;
extern MIdOrString g_RES_select_name;
extern LANGID g_RES_select_lang;
// NOTE: The Enter/input handshake state used to live here as raw file-scope
// statics (s_bEnter, s_inputCs, s_hInputDone, s_inputBuffer). It has been
// moved into EgaBridge, which owns its lifetime together with the rest of
// the per-session bridge state (see EgaBridge::Initialize/Uninitialize).
// This guarantees the state is reset every time a new EGA session starts,
// instead of possibly carrying over a stale value from a previous session
// when the EGA dialog is closed and reopened.

static bool EGA_dialog_input(char *buf, size_t buflen)
{
	if (buf == NULL && buflen == 0)
	{
		PostMessageW(g_hMainWnd, WM_COMMAND, ID_EGAFINISH, 0);
		return true;
	}

	while (!EgaBridge::IsEnterPressed() || !::IsWindowVisible(s_hwndEga))
	{
		if (EgaBridge::IsStopRequested())
			return false;

		std::string pendingFile;
		if (EgaBridge::TryTakeFileInputRequest(pendingFile))
		{
			EGA_file_input(pendingFile.c_str());
			continue;
		}

		Sleep(100);
	}
	EgaBridge::ClearEnterPressed();

	EgaBridge::PrepareForInput();
	::PostMessageW(s_hwndEga, WM_EGA_DO_GETINPUT, 0, 0);

	// Wait for results or stop request
	std::wstring textW;
	if (!EgaBridge::WaitAndTakeInputText(textW))
		return false; // Stop request

	char szTextA[512];
	WideCharToMultiByte(CP_UTF8, 0, textW.c_str(), -1, szTextA, ARRAYSIZE(szTextA), NULL, NULL);
	StringCchCopyA(buf, buflen, szTextA);

	if (lstrcmpA(szTextA, "exit") == 0 || lstrcmpA(szTextA, "exit;") == 0)
		PostMessageW(s_hwndEga, WM_COMMAND, IDCANCEL, 0);

	return true;
}

static void EGA_dialog_print(const char *fmt, va_list va)
{
	if (!IsWindow(s_hwndEga))
		return;

	std::string str;
	str.resize(512);
	while (StringCbVPrintfA(&str[0], str.size(), fmt, va) == STRSAFE_E_INSUFFICIENT_BUFFER)
	{
		str.resize(str.size() * 2);
	}
	str.resize(lstrlenA(str.c_str()));

	mstr_replace_all(str, "\n", "\r\n");

	MAnsiToWide wide(CP_UTF8, str.c_str());

	LPWSTR heapStr = _wcsdup(wide.c_str());
	if (heapStr && !::PostMessageW(s_hwndEga, WM_EGA_DO_PRINT, 0, (LPARAM)heapStr))
		free(heapStr);
}

void EGA_extension(void);

//////////////////////////////////////////////////////////////////////////////

class MEgaDlg : public MDialogBase
{
public:
	std::wstring m_filename;

	DECLARE_DYNAMIC(MEgaDlg)

	MEgaDlg() : MDialogBase(IDD_EGA)
	{
		MTRACEA("%s\n", __FUNCTION__);

		m_hIcon = LoadIconDx(IDI_SMILY);
		m_hIconSm = LoadSmallIconDx(IDI_SMILY);

		EgaBridge::Initialize();
		EgaBridge::SetInputFn(EGA_dialog_input);
		EgaBridge::SetPrintFn(EGA_dialog_print);

		EGA_extension();
	}

	virtual ~MEgaDlg()
	{
		MTRACEA("%s\n", __FUNCTION__);
		EgaBridge::Uninitialize();

		DeleteObject(m_hFont);
		DestroyIcon(m_hIcon);
		DestroyIcon(m_hIconSm);
	}

	void Run(LPCWSTR filename)
	{
		MTRACEA("%s\n", __FUNCTION__);
		char szFileName[MAX_PATH];
		if (filename && filename[0])
		{
			WideCharToMultiByte(CP_ACP, 0, filename, -1, szFileName, _countof(szFileName), NULL, NULL);
			g_RES_select_type = BAD_TYPE;
			g_RES_select_name = BAD_NAME;
			g_RES_select_lang = BAD_LANG;

			EgaBridge::RequestFileInput(szFileName);
		}
	}

	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
	{
		MTRACEA("%s\n", __FUNCTION__);
		s_hwndEga = hwnd;
		m_resizable.OnParentCreate(hwnd);

		m_resizable.SetLayoutAnchor(grp1, mzcLA_TOP_LEFT, mzcLA_BOTTOM_RIGHT);
		m_resizable.SetLayoutAnchor(edt1, mzcLA_TOP_LEFT, mzcLA_BOTTOM_RIGHT);
		m_resizable.SetLayoutAnchor(stc1, mzcLA_BOTTOM_LEFT);
		m_resizable.SetLayoutAnchor(edt2, mzcLA_BOTTOM_LEFT, mzcLA_BOTTOM_RIGHT);
		m_resizable.SetLayoutAnchor(IDOK, mzcLA_BOTTOM_RIGHT);
		SendMessageDx(WM_SETICON, ICON_BIG, (LPARAM)m_hIcon);
		SendMessageDx(WM_SETICON, ICON_SMALL, (LPARAM)m_hIconSm);

		LOGFONTW lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = -12;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
		m_hFont = CreateFontIndirectW(&lf);
		SendDlgItemMessageW(hwnd, edt1, WM_SETFONT, (WPARAM)m_hFont, TRUE);

		if (g_settings.nEgaX != CW_USEDEFAULT && g_settings.nEgaWidth != CW_USEDEFAULT)
		{
			SetWindowPos(hwnd, NULL,
				g_settings.nEgaX, g_settings.nEgaY,
				g_settings.nEgaWidth, g_settings.nEgaHeight,
				SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		}
		else if (g_settings.nEgaX != CW_USEDEFAULT)
		{
			SetWindowPos(hwnd, NULL,
				g_settings.nEgaX, g_settings.nEgaY,
				g_settings.nEgaWidth, g_settings.nEgaHeight,
				SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		}
		else if (g_settings.nEgaWidth != CW_USEDEFAULT)
		{
			SetWindowPos(hwnd, NULL,
				g_settings.nEgaX, g_settings.nEgaY,
				g_settings.nEgaWidth, g_settings.nEgaHeight,
				SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		}
		else
		{
			CenterWindowDx();
		}

		// Start the interactive loop if needed
		EgaBridge::StartInteractive();
		::SetFocus(::GetDlgItem(hwnd, edt2));

		return TRUE;
	}

	void OnOK(HWND hwnd)
	{
		MTRACEA("%s\n", __FUNCTION__);
		g_RES_select_type = BAD_TYPE;
		g_RES_select_name = BAD_NAME;
		g_RES_select_lang = BAD_LANG;
		EgaBridge::NotifyEnterPressed();
		::SetFocus(::GetDlgItem(hwnd, edt2));
	}

	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch (id)
		{
		case IDOK: // Enter
			OnOK(hwnd);
			break;
		case IDCANCEL: // Close
			::DestroyWindow(hwnd);
			break;
		}
	}

	void OnDestroy(HWND hwnd)
	{
		MTRACEA("%s\n", __FUNCTION__);
		// NOTE: We used to also force-feed "exit" into edt2 and set the
		// (now-removed) s_bEnter flag here to try to unblock the EGA
		// worker thread. That was both redundant and a source of a bug:
		// StopInteractive() below already unblocks the worker thread
		// reliably via the stop event (see EGA_dialog_input's polling
		// loop and EgaBridge::WaitAndTakeInputText), and forcing the
		// enter-flag here could leave stale state for the *next* EGA
		// session if this dialog is reopened later.
		PostMessageW(g_hMainWnd, WM_COMMAND, ID_EGAFINISH, 0);
		EgaBridge::StopInteractive(true);
		s_hwndEga = NULL;
	}

	HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
	{
		UINT id;
		switch (type)
		{
		case CTLCOLOR_EDIT:
		case CTLCOLOR_STATIC:
		case CTLCOLOR_BTN:
			id = GetDlgCtrlID(hwndChild);
			switch (id)
			{
			case edt1:
			case edt2:
				SetTextColor(hdc, RGB(0, 255, 0));
				SetBkColor(hdc, RGB(0, 0, 0));
				return GetStockBrush(BLACK_BRUSH);
			}
		}
		return (HBRUSH)(COLOR_3DFACE + 1);
	}

	void OnMove(HWND hwnd, int x, int y)
	{
		if (IsWindowVisible(hwnd) && !IsMinimized(hwnd) && !IsMaximized(hwnd))
		{
			RECT rc;
			GetWindowRect(hwnd, &rc);
			g_settings.nEgaX = rc.left;
			g_settings.nEgaY = rc.top;
		}
	}

	void OnSize(HWND hwnd, UINT state, int cx, int cy)
	{
		m_resizable.OnSize();

		if (IsWindowVisible(hwnd) && !IsMinimized(hwnd) && !IsMaximized(hwnd))
		{
			RECT rc;
			GetWindowRect(hwnd, &rc);
			g_settings.nEgaWidth = rc.right - rc.left;
			g_settings.nEgaHeight = rc.bottom - rc.top;
		}
	}

	void OnEgaGetInput(HWND hwnd)
	{
		MTRACEA("%s\n", __FUNCTION__);
		WCHAR szTextW[512];
		GetDlgItemTextW(hwnd, edt2, szTextW, ARRAYSIZE(szTextW));
		mstr_trim(szTextW);
		SetDlgItemTextW(hwnd, edt2, L"");

		EgaBridge::SubmitInputText(szTextW);
	}

	void OnEgaPrint(HWND hwnd, LPWSTR text)
	{
		MTRACEA("%s(%ls)\n", __FUNCTION__, text);
		if (!text) return;
		INT cch = GetWindowTextLengthW(GetDlgItem(hwnd, edt1));
		SendDlgItemMessageW(hwnd, edt1, EM_SETSEL, cch, cch);
		SendDlgItemMessageW(hwnd, edt1, EM_REPLACESEL, FALSE, (LPARAM)text);
		SendDlgItemMessageW(hwnd, edt1, EM_SCROLLCARET, 0, 0);
		free(text);
	}

	virtual INT_PTR CALLBACK
	DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hwnd, WM_CTLCOLOREDIT, OnCtlColor);
		HANDLE_MSG(hwnd, WM_CTLCOLORSTATIC, OnCtlColor);
		HANDLE_MSG(hwnd, WM_MOVE, OnMove);
		HANDLE_MSG(hwnd, WM_SIZE, OnSize);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		case WM_EGA_DO_GETINPUT:
			OnEgaGetInput(hwnd);
			return 0;
		case WM_EGA_DO_PRINT:
			OnEgaPrint(hwnd, (LPWSTR)lParam);
			return 0;
		case WM_EGA_DO_RUN_ON_UI:
			EgaBridge::ExecuteUITask((void*)lParam);
			return 0;
		default:
			return DefaultProcDx();
		}
	}

protected:
	HFONT m_hFont;
	HICON m_hIcon;
	HICON m_hIconSm;
	MResizable m_resizable;
};
