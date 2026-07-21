// RisohEditor.hpp --- RisohEditor header
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2021 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include <initguid.h>
#include <windows.h>
#include <windowsx.h>
#ifdef __GNUC__ // Workaround
	#define WINAPI_FAMILY_ONE_PARTITION(vset, v) ((WINAPI_FAMILY & vset) == v)
#endif
#include <shlobj.h>
#include <shlwapi.h>
#include <dlgs.h>
#include <tchar.h>
#include <commctrl.h>
#include <commdlg.h>
#include <mbstring.h>
#include <mmsystem.h>
#include <process.h>
#include <uxtheme.h>
#include <urlmon.h>
#include <wininet.h>
#ifdef ATL_SUPPORT
	#include <cguid.h>
	#include <atlbase.h>
	#include <atlhost.h>
#endif

#include <algorithm>    // for std::sort
#include <string>       // for std::string, std::wstring
#include <cassert>      // for assert macro
#include <vector>       // for std::vector
#include <map>          // for std::map
#include <unordered_map>
#include <cstdio>
#include <clocale>
#include <strsafe.h>

////////////////////////////////////////////////////////////////////////////

INT LogMessageBoxW(HWND hwnd, LPCWSTR text, LPCWSTR title, UINT uType);

#include "WonSetThreadUILanguage.h"

#include "resource.h"
#include "MWindowBase.hpp"
#include "MEditCtrl.hpp"
#include "MSplitterWnd.hpp"
#include "MBitmapDx.hpp"
#include "Res.hpp"
#include "ConstantsDB.hpp"
#include "MacroParser.hpp"
#include "MWaitCursor.hpp"
#include "RisohSettings.hpp"

// RisohEditor.cpp
BOOL GetPathOfShortcutDx(HWND hwnd, LPCWSTR pszLnkFile, LPWSTR pszPath);
HBITMAP CreateBitmapFromIconDx(HICON hIcon, INT width, INT height, BOOL bCursor);

#define _CP_UTF16 1200

////////////////////////////////////////////////////////////////////////////

#include "MRadWindow.hpp"
#include "MReplaceBinDlg.hpp"
#include "MBmpView.hpp"
#include "MIDListDlg.hpp"
#include "MAdviceResHDlg.hpp"
#include "MTestParentWnd.hpp"
#include "MEgaDlg.hpp"
#include "MDropdownArrow.hpp"
#include "MTabCtrl.hpp"

#include "MString.hpp"
#include "MByteStream.hpp"
#include "AccelRes.hpp"
#include "IconRes.hpp"
#include "MenuRes.hpp"
#include "MessageRes.hpp"
#include "StringRes.hpp"
#include "DialogRes.hpp"
#include "VersionRes.hpp"
#include "DlgInitRes.hpp"

#include "ConstantsDB.hpp"
#include "PackedDIB.hpp"
#include "Res.hpp"
#include "ResHeader.hpp"

#include "MFile.hpp"
#include "MProcessMaker.hpp"

#include "ResToText.hpp"

struct CODEPAGE_INFO
{
	UINT number;
	const char *name;
};

typedef HRESULT (WINAPI *SETWINDOWTHEME)(HWND, LPCWSTR, LPCWSTR);

#define CX_STATUS_PART  80      // status bar part width
#define ERROR_LINE_COLOR RGB(255, 191, 191)

#define MYWM_UPDATEARROW (WM_USER + 114)
#define MYWM_GETDLGHEADLINES (WM_USER + 250)

// the window class libraries
typedef std::unordered_set<HMODULE> wclib_t;
extern wclib_t s_wclib;
