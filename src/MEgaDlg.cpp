// MEgaDlg.cpp --- Programming Language EGA dialog
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2020 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#include "MEgaDlg.hpp"
#include "Res.hpp"

// EGA入力関数。EGA_set_input_fnに渡される。
// この関数がfalseを返せば、EGAの実行単位が終了する。
bool EGA_dialog_input(char *buf, size_t buflen)
{
	if (buf == NULL && buflen == 0) // １つ実行単位の実行が終わったとき？
	{
		// 特殊なメッセージを投函する。
		::PostMessageW(s_hwndEga, WM_EGA_DO_PRINT, 0, 0);
		::PostMessageW(g_hMainWnd, WM_COMMAND, ID_EGAFINISH, 0);
		return true;
	}

	// 入力と終了を待つ。
	while (!EgaBridge::IsEnterPressed() || !::IsWindowVisible(s_hwndEga))
	{
		if (EgaBridge::IsStopRequested()) // 停止を要求されたか？
			return false;

		// キューから入力ファイルを取得。
		std::string pendingFile;
		if (EgaBridge::TryTakeFileInputRequest(pendingFile))
		{
			// このスレッドで実行する。
			EGA_file_input(pendingFile.c_str());
			continue;
		}

		Sleep(100); // FIXME: もっと良い待ち方があるはずだ。
	}

	// 入力を受け入れる準備をする。
	EgaBridge::ClearEnterPressed();
	EgaBridge::PrepareForInput();

	// 入力を要求する。
	::PostMessageW(s_hwndEga, WM_EGA_DO_GETINPUT, 0, 0);

	// このスレッドで実際に入力と終了要求を待つ。
	std::wstring textW;
	if (!EgaBridge::WaitAndTakeInputText(textW, INFINITE))
		return false; // Stop request

	// UTF-8化してbufに格納する。
	char szTextA[512];
	WideCharToMultiByte(CP_UTF8, 0, textW.c_str(), -1, szTextA, ARRAYSIZE(szTextA), NULL, NULL);
	StringCchCopyA(buf, buflen, szTextA);

	// 「exit」か「exit;」の場合は終了を要求するためのコマンドを投函する。
	if (lstrcmpA(szTextA, "exit") == 0 || lstrcmpA(szTextA, "exit;") == 0)
		PostMessageW(s_hwndEga, WM_COMMAND, IDCANCEL, 0);

	return true;
}

// EGA出力関数。EGA_set_print_fnに渡される。
void EGA_dialog_print(const char *fmt, va_list va)
{
	// s_hwndEgaが無効なら戻る。
	if (!IsWindow(s_hwndEga))
		return;

	// 出力バッファを確保して出力文字列を取得する。
	std::string str;
	str.resize(512);
	for (;;)
	{
		va_list va2;
		va_copy(va2, va);
		HRESULT hr = StringCbVPrintfA(&str[0], str.size(), fmt, va2);
		va_end(va2);

		if (hr != STRSAFE_E_INSUFFICIENT_BUFFER)
			break;
		str.resize(str.size() * 2);
	}

	// 文字列を整える。
	str.resize(lstrlenA(str.c_str()));
	mstr_replace_all(str, "\n", "\r\n");

	// ワイド文字列にしてブリッジに渡す。
	MAnsiToWide wide(CP_UTF8, str.c_str());
	EgaBridge::QueuePrintText(wide.c_str());
}

MEgaDlg::MEgaDlg() : MDialogBase(IDD_EGA)
{
	MTRACEA("%s\n", __FUNCTION__);

	// アイコン読み込み。
	m_hIcon = LoadIconDx(IDI_SMILY);
	m_hIconSm = LoadSmallIconDx(IDI_SMILY);

	EgaBridge::Initialize();
	EgaBridge::SetInputFn(EGA_dialog_input);
	EgaBridge::SetPrintFn(EGA_dialog_print);

	EGA_extension();
}

MEgaDlg::~MEgaDlg()
{
	MTRACEA("%s\n", __FUNCTION__);
	EgaBridge::Uninitialize();

	DeleteObject(m_hFont);
	DestroyIcon(m_hIcon);
	DestroyIcon(m_hIconSm);
}

void MEgaDlg::ExecuteEgaFile(LPCWSTR filename)
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

BOOL MEgaDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	MTRACEA("%s\n", __FUNCTION__);
	s_hwndEga = hwnd; // Remember

	// Make it a resizable dialog
	m_resizable.OnParentCreate(hwnd);
	m_resizable.SetLayoutAnchor(grp1, mzcLA_TOP_LEFT, mzcLA_BOTTOM_RIGHT);
	m_resizable.SetLayoutAnchor(edt1, mzcLA_TOP_LEFT, mzcLA_BOTTOM_RIGHT);
	m_resizable.SetLayoutAnchor(stc1, mzcLA_BOTTOM_LEFT);
	m_resizable.SetLayoutAnchor(edt2, mzcLA_BOTTOM_LEFT, mzcLA_BOTTOM_RIGHT);
	m_resizable.SetLayoutAnchor(IDOK, mzcLA_BOTTOM_RIGHT);

	// Set dialog icon
	SendMessageDx(WM_SETICON, ICON_BIG, (LPARAM)m_hIcon);
	SendMessageDx(WM_SETICON, ICON_SMALL, (LPARAM)m_hIconSm);

	// No limit
	SendDlgItemMessageW(hwnd, edt1, EM_SETLIMITTEXT, 0, 0);

	// Create font
	LOGFONTW lf;
	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = -12;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	m_hFont = CreateFontIndirectW(&lf);
	SendDlgItemMessageW(hwnd, edt1, WM_SETFONT, (WPARAM)m_hFont, TRUE);

	// edt1 starts out empty; track its length ourselves from here on
	// (see the m_cchEdt1 comment in MEgaDlg.hpp).
	m_cchEdt1 = GetWindowTextLengthW(GetDlgItem(hwnd, edt1));

	// Move and resize
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

	// Start the EGA thread
	EgaBridge::StartInteractive();
	::SetFocus(::GetDlgItem(hwnd, edt2));

	return TRUE;
}

void MEgaDlg::OnOK(HWND hwnd) // Enterキーが押された？
{
	MTRACEA("%s\n", __FUNCTION__);

	// リソース項目の選択情報をクリアする。
	g_RES_select_type = BAD_TYPE;
	g_RES_select_name = BAD_NAME;
	g_RES_select_lang = BAD_LANG;

	// Enterを押したことを伝える。
	EgaBridge::NotifyEnterPressed();

	// フォーカスを edt2 に移動。
	::SetFocus(::GetDlgItem(hwnd, edt2));
}

void MEgaDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
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

void MEgaDlg::OnDestroy(HWND hwnd)
{
	MTRACEA("%s\n", __FUNCTION__);

	// 終了前に特殊なメッセージを投函する。
	PostMessageW(g_hMainWnd, WM_COMMAND, ID_EGAFINISH, 0);

	// EGAスレッドを終了する。
	EgaBridge::StopInteractive(true);

	// EGAを破棄する。
	EgaBridge::Uninitialize();

	s_hwndEga = NULL; // Forget
}

HBRUSH MEgaDlg::OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
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

void MEgaDlg::OnMove(HWND hwnd, int x, int y)
{
	if (IsWindowVisible(hwnd) && !IsMinimized(hwnd) && !IsMaximized(hwnd))
	{
		RECT rc;
		GetWindowRect(hwnd, &rc);
		g_settings.nEgaX = rc.left;
		g_settings.nEgaY = rc.top;
	}
}

void MEgaDlg::OnSize(HWND hwnd, UINT state, int cx, int cy)
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

// WM_EGA_DO_GETINPUT
void MEgaDlg::OnEgaGetInput(HWND hwnd)
{
	MTRACEA("%s\n", __FUNCTION__);

	// edt2から文字列を取得して、edt2をクリア。
	WCHAR szTextW[512];
	GetDlgItemTextW(hwnd, edt2, szTextW, ARRAYSIZE(szTextW));
	mstr_trim(szTextW);
	SetDlgItemTextW(hwnd, edt2, L"");

	// 入力文字列を投函。
	EgaBridge::SubmitInputText(szTextW);
}

// WM_EGA_DO_PRINT
void MEgaDlg::OnEgaPrint(HWND hwnd)
{
	// 未処理の出力文字列を取得する。
	std::wstring text;
	if (!EgaBridge::TakePendingPrintText(text))
		return; // そんなものはない。

	// 巨大になりすぎたら古い部分を削除（簡易版）
	if (m_cchEdt1 > 10'000'000) // 約10MB超
	{
		SendDlgItemMessageW(hwnd, edt1, EM_SETSEL, 0, 500'000); // 先頭50万文字削除
		SendDlgItemMessageW(hwnd, edt1, EM_REPLACESEL, FALSE, (LPARAM)L"");
		m_cchEdt1 = GetWindowTextLengthW(GetDlgItem(hwnd, edt1));
	}

	// 追記。
	SendDlgItemMessageW(hwnd, edt1, EM_SETSEL, m_cchEdt1, m_cchEdt1);
	SendDlgItemMessageW(hwnd, edt1, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
	SendDlgItemMessageW(hwnd, edt1, EM_SCROLLCARET, 0, 0);
	m_cchEdt1 += (INT)text.size(); // 今後のためにも、文字列長を覚えておく。

	// カーソル強制復元（念のため）
	::SetCursor(::LoadCursorW(NULL, IDC_ARROW));
}

INT_PTR CALLBACK
MEgaDlg::DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	case WM_EGA_DO_GETINPUT: // 入力を取得する。
		OnEgaGetInput(hwnd);
		return 0;
	case WM_EGA_DO_PRINT: // EGA出力を行う。
		OnEgaPrint(hwnd);
		return 0;
	case WM_EGA_DO_RUN_ON_UI: // UIタスクを実行。
		EgaBridge::ExecuteUITask((void*)lParam);
		return 0;
	default:
		return DefaultProcDx();
	}
}
