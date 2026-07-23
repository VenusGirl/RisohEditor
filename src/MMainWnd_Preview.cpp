// MMainWnd_Preview.cpp --- Preview function implementations for MMainWnd
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2021 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#include "MMainWnd.hpp"
#include "ToolbarRes.hpp"
#include "LineNumEdit.hpp"
#include "Utils.h"
#include "MStrBin.hpp"

// preview the icon resource
BOOL MMainWnd::PreviewIcon(HWND hwnd, const EntryBase& entry)
{
	// create a bitmap object from the entry and set it to m_hBmpView
	BITMAP bm;
	m_hBmpView.SetBitmap(CreateBitmapFromIconOrPngDx(hwnd, entry, bm));

	// create the icon
	MStringW str;
	HICON hIcon = PackedDIB_CreateIcon(&entry[0], entry.size(), bm, TRUE);

	// dump info to m_hCodeEditor
	if (hIcon)
	{
		str = DumpIconInfo(bm, TRUE);
	}
	else
	{
		str = DumpBitmapInfo(m_hBmpView.m_hBitmap);
	}
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// destroy the icon
	DestroyIcon(hIcon);

	// show
	SetShowMode(SHOW_CODEANDBMP);

	return TRUE;
}

// preview the cursor resource
BOOL MMainWnd::PreviewCursor(HWND hwnd, const EntryBase& entry)
{
	// create a cursor object from the entry and set it to m_hBmpView
	BITMAP bm;
	HCURSOR hCursor = PackedDIB_CreateIcon(&entry[0], entry.size(), bm, FALSE);
	HBITMAP hbm = CreateBitmapFromIconDx(hCursor, bm.bmWidth, bm.bmHeight, TRUE);
	m_hBmpView.SetBitmap(hbm);

	// dump info to m_hCodeEditor
	MStringW str = DumpIconInfo(bm, FALSE);
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// destroy the cursor
	DestroyCursor(hCursor);

	// show
	SetShowMode(SHOW_CODEANDBMP);

	return !!hbm;
}

// preview the group icon resource
BOOL MMainWnd::PreviewGroupIcon(HWND hwnd, const EntryBase& entry)
{
	// create a bitmap object from the entry and set it to m_hBmpView
	HBITMAP hbm = CreateBitmapFromIconsDx(hwnd, entry);
	m_hBmpView.SetBitmap(hbm);

	// dump the text to m_hCodeEditor
	ResToText res2text;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// show
	SetShowMode(SHOW_CODEANDBMP);

	return !!hbm;
}

// preview the group cursor resource
BOOL MMainWnd::PreviewGroupCursor(HWND hwnd, const EntryBase& entry)
{
	// create a bitmap object from the entry and set it to m_hBmpView
	HBITMAP hbm = CreateBitmapFromCursorsDx(hwnd, entry);
	m_hBmpView.SetBitmap(hbm);
	assert(m_hBmpView);

	// dump the text to m_hCodeEditor
	ResToText res2text;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// show
	SetShowMode(SHOW_CODEANDBMP);

	return !!hbm;
}

// preview the bitmap resource
BOOL MMainWnd::PreviewBitmap(HWND hwnd, const EntryBase& entry)
{
	// create a bitmap object from the entry and set it to m_hBmpView
	HBITMAP hbm = PackedDIB_CreateBitmapFromMemory(&entry[0], entry.size());
	m_hBmpView.SetBitmap(hbm);

	// dump the text to m_hCodeEditor
	ResToText res2text;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// show
	SetShowMode(SHOW_CODEANDBMP);

	return !!hbm;
}

// preview the image resource
BOOL MMainWnd::PreviewImage(HWND hwnd, const EntryBase& entry)
{
	// dump the text to m_hCodeEditor
	ResToText res2text;
	MStringW str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// set the entry image to m_hBmpView
	m_hBmpView.SetImage(&entry[0], entry.size());

	// show
	SetShowMode(SHOW_CODEANDBMP);

	return !!m_hBmpView.m_hBitmap;
}

// preview the WAVE resource
BOOL MMainWnd::PreviewWAVE(HWND hwnd, const EntryBase& entry)
{
	// dump the text to m_hCodeEditor
	ResToText res2text;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// make it playable
	m_hBmpView.SetPlay();

	// show
	SetShowMode(SHOW_CODEANDBMP);

	return TRUE;
}

// preview the MP3 resource
BOOL MMainWnd::PreviewMP3(HWND hwnd, const EntryBase& entry)
{
	// dump the text to m_hCodeEditor
	ResToText res2text;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// make it playable
	m_hBmpView.SetPlay();

	// show
	SetShowMode(SHOW_CODEANDBMP);

	return TRUE;
}

// preview the AVI resource
BOOL MMainWnd::PreviewAVI(HWND hwnd, const EntryBase& entry)
{
	// dump the text to m_hCodeEditor
	ResToText res2text;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// set the AVI
	m_hBmpView.SetMedia(&entry[0], entry.size(), L"avi");

	// show movie
	SetShowMode(SHOW_MOVIE);

	return TRUE;
}

// preview the RT_ACCELERATOR resource
BOOL MMainWnd::PreviewAccel(HWND hwnd, const EntryBase& entry)
{
	// entry.m_data --> stream --> accel
	AccelRes accel;
	MByteStreamEx stream(entry.m_data);
	if (accel.LoadFromStream(stream))
	{
		// dump the text to m_hCodeEditor
		MString str = GetLanguageStatement(entry.m_lang);
		str += accel.Dump(entry.m_name);
		SetWindowTextW(m_hCodeEditor, str.c_str());
		return TRUE;
	}
	return FALSE;
}

// preview the message table resource
BOOL MMainWnd::PreviewMessage(HWND hwnd, const EntryBase& entry)
{
	// entry.m_data --> stream --> mes
	MessageRes mes;
	MByteStreamEx stream(entry.m_data);
	if (mes.LoadFromStream(stream))
	{
		// dump the text to m_hCodeEditor
		MString str = GetLanguageStatement(entry.m_lang);
		str += mes.Dump(entry.m_name);
		SetWindowTextW(m_hCodeEditor, str.c_str());
		return TRUE;
	}
	return FALSE;
}

// preview the string resource
BOOL MMainWnd::PreviewString(HWND hwnd, const EntryBase& entry)
{
	// entry.m_data --> stream --> str_res
	StringRes str_res;
	MByteStreamEx stream(entry.m_data);
	WORD nNameID = entry.m_name.m_id;
	if (str_res.LoadFromStream(stream, nNameID))
	{
		// dump the text to m_hCodeEditor
		MStringW str = str_res.Dump(nNameID);
		SetWindowTextW(m_hCodeEditor, str.c_str());
		return TRUE;
	}
	return FALSE;
}

// preview the HTML resource
BOOL MMainWnd::PreviewHtml(HWND hwnd, const EntryBase& entry)
{
	// load a text file
	MTextType type;
	type.nNewLine = MNEWLINE_CRLF;
	MStringW str;
	if (entry.size())
		str = mstr_from_bin(&entry.m_data[0], entry.m_data.size(), &type);

	// dump the text to m_hCodeEditor
	SetWindowTextW(m_hCodeEditor, str.c_str());

	return TRUE;
}

// preview the menu resource
BOOL MMainWnd::PreviewMenu(HWND hwnd, const EntryBase& entry)
{
	// entry.m_data --> stream --> menu_res
	MenuRes menu_res;
	MByteStreamEx stream(entry.m_data);
	if (menu_res.LoadFromStream(stream))
	{
		// dump the text to m_hCodeEditor
		MString str = GetLanguageStatement(entry.m_lang);
		str += menu_res.Dump(entry.m_name);
		SetWindowTextW(m_hCodeEditor, str.c_str());
		return TRUE;
	}
	return FALSE;
}

// preview the TOOLBAR resource
BOOL MMainWnd::PreviewToolbar(HWND hwnd, const EntryBase& entry)
{
	// entry.m_data --> stream --> toolbar_res
	ToolbarRes toolbar_res;
	MByteStreamEx stream(entry.m_data);
	if (toolbar_res.LoadFromStream(stream))
	{
		// dump the text to m_hCodeEditor
		MString str = GetLanguageStatement(entry.m_lang);
		str += toolbar_res.Dump(entry.m_name);
		SetWindowTextW(m_hCodeEditor, str.c_str());
		return TRUE;
	}
	return FALSE;
}

// preview the version resource
BOOL MMainWnd::PreviewVersion(HWND hwnd, const EntryBase& entry)
{
	// entry.m_data --> ver_res
	VersionRes ver_res;
	if (ver_res.LoadFromData(entry.m_data))
	{
		// dump the text to m_hCodeEditor
		MString str = GetLanguageStatement(entry.m_lang);
		str += ver_res.Dump(entry.m_name);
		SetWindowTextW(m_hCodeEditor, str.c_str());
		return TRUE;
	}
	return FALSE;
}

// preview the unknown resource
BOOL MMainWnd::PreviewUnknown(HWND hwnd, const EntryBase& entry)
{
	// dump the text to m_hCodeEditor
	ResToText res2text;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());
	return TRUE;
}

BOOL MMainWnd::PreviewTypeLib(HWND hwnd, const EntryBase& entry)
{
	// dump the text to m_hCodeEditor
	ResToText res2text;
	res2text.m_hwnd = m_hwnd;
	res2text.m_bHumanReadable = TRUE;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());
	return TRUE;
}

// preview the RT_RCDATA resource
BOOL MMainWnd::PreviewRCData(HWND hwnd, const EntryBase& entry)
{
	// dump the text to m_hCodeEditor
	ResToText res2text;
	res2text.m_hwnd = m_hwnd;
	res2text.m_bHumanReadable = TRUE;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());
	return TRUE;
}

// preview the DLGINIT resource
BOOL MMainWnd::PreviewDlgInit(HWND hwnd, const EntryBase& entry)
{
	// dump the text to m_hCodeEditor
	ResToText res2text;
	MString str = res2text.DumpEntry(entry);
	SetWindowTextW(m_hCodeEditor, str.c_str());
	return !str.empty();
}

// preview the dialog template resource
BOOL MMainWnd::PreviewDialog(HWND hwnd, const EntryBase& entry)
{
	// entry.m_data --> stream --> dialog_res
	DialogRes dialog_res;
	MByteStreamEx stream(entry.m_data);
	if (dialog_res.LoadFromStream(stream))
	{
		// dump the text to m_hCodeEditor
		MString str = GetLanguageStatement(entry.m_lang);
		str += dialog_res.Dump(entry.m_name, !!g_settings.bAlwaysControl);
		SetWindowTextW(m_hCodeEditor, str.c_str());
		return TRUE;
	}
	return FALSE;
}

// preview the animation icon resource
BOOL MMainWnd::PreviewAniIcon(HWND hwnd, const EntryBase& entry, BOOL bIcon)
{
	HICON hIcon = NULL;

	{
		WCHAR szPath[MAX_PATH], szTempFile[MAX_PATH];
		GetTempPathW(_countof(szPath), szPath);
		GetTempFileNameW(szPath, L"ani", 0, szTempFile);

		MFile file;
		DWORD cbWritten = 0;
		if (file.OpenFileForOutput(szTempFile) &&
			file.WriteFile(&entry[0], entry.size(), &cbWritten))
		{
			file.FlushFileBuffers();    // flush
			file.CloseHandle();         // close the handle

			if (bIcon)
			{
				hIcon = (HICON)LoadImage(NULL, szTempFile, IMAGE_ICON,
					0, 0, LR_LOADFROMFILE);
			}
			else
			{
				hIcon = (HICON)LoadImage(NULL, szTempFile, IMAGE_CURSOR,
					0, 0, LR_LOADFROMFILE);
			}
		}
		DeleteFileW(szTempFile);
	}

	if (hIcon)
	{
		m_hBmpView.SetIcon(hIcon, bIcon);

		ResToText res2text;
		MString str = res2text.DumpEntry(entry);
		SetWindowTextW(m_hCodeEditor, str.c_str());
	}
	else
	{
		m_hBmpView.DestroyView();
	}

	// show
	SetShowMode(SHOW_CODEANDBMP);
	return !!hIcon;
}

// preview the string table resource
BOOL MMainWnd::PreviewStringTable(HWND hwnd, const EntryBase& entry)
{
	// search the strings
	EntrySet found;
	g_res.search(found, ET_LANG, RT_STRING, BAD_NAME, entry.m_lang);

	// found --> str_res
	StringRes str_res;
	for (auto e : found)
	{
		MByteStreamEx stream(e->m_data);
		if (!str_res.LoadFromStream(stream, e->m_name.m_id))
			return FALSE;
	}

	// dump the text to m_hCodeEditor
	MString str = GetLanguageStatement(entry.m_lang);
	str += str_res.Dump();
	SetWindowTextW(m_hCodeEditor, str.c_str());

	// show code only
	SetShowMode(SHOW_CODEONLY);
	return TRUE;
}

// preview the message table resource
BOOL MMainWnd::PreviewMessageTable(HWND hwnd, const EntryBase& entry)
{
	// search the message tables
	EntrySet found;
	g_res.search(found, ET_LANG, RT_MESSAGETABLE, entry.m_name, entry.m_lang);

	// dump the text to m_hCodeEditor
	MString str = GetLanguageStatement(entry.m_lang);
	{
		// found --> msg_res
		MessageRes msg_res;
		for (auto e : found)
		{
			MByteStreamEx stream(e->m_data);
			if (!msg_res.LoadFromStream(stream))
			{ 
				return FALSE;
			}
		}

		str += L"#ifdef MCDX_INVOKED\r\n";
		str += msg_res.Dump(entry.m_name);
		str += L"#endif\r\n\r\n";
	}
	::SetWindowTextW(m_hCodeEditor, str.c_str());
	::SendMessageW(m_hCodeEditor, LNEM_CLEARLINEMARKS, 0, 0);

	// show code only
	SetShowMode(SHOW_CODEONLY);

	return TRUE;
}

// close the preview
VOID MMainWnd::HidePreview(STV stv)
{
	// destroy the RADical window if any
	if (IsWindow(m_rad_window))
	{
		m_rad_window.DestroyWindow();
	}

	// clear m_hHexViewer
	SetWindowTextW(m_hHexViewer, NULL);
	Edit_SetModify(m_hHexViewer, FALSE);

	// clear m_hCodeEditor
	if (stv == STV_RESETTEXT || stv == STV_RESETTEXTANDMODIFIED)
	{
		SetWindowTextW(m_hCodeEditor, NULL);
		::SendMessageW(m_hCodeEditor, LNEM_CLEARLINEMARKS, 0, 0);
	}
	if (stv != STV_DONTRESET)
	{
		Edit_SetModify(m_hCodeEditor, FALSE);
	}

	// close and hide m_hBmpView
	m_hBmpView.DestroyView();

	// Code Viewer only
	SetShowMode(SHOW_CODEONLY);

	// recalculate the splitter
	PostMessageDx(WM_SIZE);
}

// do preview the resource item
BOOL MMainWnd::Preview(HWND hwnd, const EntryBase *entry, STV stv)
{
	// close the preview
	HidePreview(stv);

	if (stv == STV_DONTRESET)
		return IsEntryTextEditable(entry);

	// show the binary
	MStringW str = DumpBinaryAsText(entry->m_data);
	SetWindowTextW(m_hHexViewer, str.c_str());

	// code only
	SetShowMode(SHOW_CODEONLY);

	// do preview the resource item
	BOOL bCanPreview = TRUE;
	if (entry->m_type.m_id != 0)
	{
		WORD wType = entry->m_type.m_id;
		if (wType == (WORD)(UINT_PTR)RT_ICON)
		{
			bCanPreview = PreviewIcon(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_CURSOR)
		{
			bCanPreview = PreviewCursor(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_GROUP_ICON)
		{
			bCanPreview = PreviewGroupIcon(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_GROUP_CURSOR)
		{
			bCanPreview = PreviewGroupCursor(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_BITMAP)
		{
			bCanPreview = PreviewBitmap(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_ACCELERATOR)
		{
			bCanPreview = PreviewAccel(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_STRING)
		{
			bCanPreview = PreviewString(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_MENU)
		{
			bCanPreview = PreviewMenu(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_TOOLBAR)
		{
			bCanPreview = PreviewToolbar(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_DIALOG)
		{
			bCanPreview = PreviewDialog(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_ANIICON)
		{
			bCanPreview = PreviewAniIcon(hwnd, *entry, TRUE);
		}
		else if (wType == (WORD)(UINT_PTR)RT_ANICURSOR)
		{
			bCanPreview = PreviewAniIcon(hwnd, *entry, FALSE);
		}
		else if (wType == (WORD)(UINT_PTR)RT_MESSAGETABLE)
		{
			bCanPreview = PreviewMessage(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_MANIFEST || wType == (WORD)(UINT_PTR)RT_HTML)
		{
			bCanPreview = PreviewHtml(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_VERSION)
		{
			bCanPreview = PreviewVersion(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_RCDATA)
		{
			bCanPreview = PreviewRCData(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_DLGINIT)
		{
			bCanPreview = PreviewDlgInit(hwnd, *entry);
		}
		else if (wType == (WORD)(UINT_PTR)RT_MESSAGETABLE)
		{
			bCanPreview = PreviewMessageTable(hwnd, *entry);
		}
		else
		{
			bCanPreview = PreviewUnknown(hwnd, *entry);
		}
	}
	else
	{
		if (entry->m_type == L"PNG" || entry->m_type == L"GIF" ||
			entry->m_type == L"JPEG" || entry->m_type == L"TIFF" ||
			entry->m_type == L"JPG" || entry->m_type == L"TIF" ||
			entry->m_type == L"EMF" || entry->m_type == L"ENHMETAFILE" ||
			entry->m_type == L"ENHMETAPICT" ||
			entry->m_type == L"WMF" || entry->m_type == L"IMAGE")
		{
			bCanPreview = PreviewImage(hwnd, *entry);
		}
		else if (entry->m_type == L"WAVE")
		{
			bCanPreview = PreviewWAVE(hwnd, *entry);
		}
		else if (entry->m_type == L"MP3")
		{
			bCanPreview = PreviewMP3(hwnd, *entry);
		}
		else if (entry->m_type == L"AVI")
		{
			bCanPreview = PreviewAVI(hwnd, *entry);
		}
		else if (entry->m_type == L"TYPELIB")
		{
			bCanPreview = PreviewTypeLib(hwnd, *entry);
		}
		else
		{
			bCanPreview = PreviewUnknown(hwnd, *entry);
		}
	}

	// recalculate the splitter
	PostMessageDx(WM_SIZE);

	return bCanPreview && IsEntryTextEditable(entry);
}
