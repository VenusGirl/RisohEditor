// MMainWnd_Search.cpp --- RisohEditor MMainWnd search
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2026 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#include "RisohEditor.hpp"
#include "MMainWnd.hpp"
#include <thread>

// check the text for item search
static bool CheckTextForSearch(ITEM_SEARCH *pSearch, EntryBase *entry, MString text)
{
	// make the text uppercase to ignore case
	if (pSearch->bIgnoreCases)
		CharUpperW(&text[0]);

	// find?
	if (text.find(pSearch->strText) == MString::npos)
		return false;   // not found

	if (pSearch->bDownward)     // go downward
	{
		// check the position
		if (pSearch->pCurrent == nullptr || *pSearch->pCurrent < *entry)
		{
			if (pSearch->pFound)    // already found
			{
				if (*entry < *pSearch->pFound)  // compare with the found one
				{
					pSearch->pFound = g_res.get_shared(entry);
					return true;    // found
				}
			}
			else    // not found yet
			{
				pSearch->pFound = g_res.get_shared(entry);    // found
				return true;    // found
			}
		}
	}
	else    // go upward
	{
		// check the position
		if (pSearch->pCurrent == nullptr || *entry < *pSearch->pCurrent)
		{
			if (pSearch->pFound)    // already found
			{
				if (*pSearch->pFound < *entry)  // compare with the found one
				{
					pSearch->pFound = g_res.get_shared(entry);
					return true;    // found
				}
			}
			else
			{
				pSearch->pFound = g_res.get_shared(entry);
				return true;    // found
			}
		}
	}

	return false;    // not found
}

// a function for item search
unsigned __stdcall search_proc(void *arg)
{
	auto pSearch = (ITEM_SEARCH *)arg;
	MString text;

	BOOL bFound = FALSE;
	for (auto entry : g_res)
	{
		if (!entry->valid())
			continue;

		if (pSearch->bCancelled)
			break;

		EntryBase e = *entry;

		// check label
		text = e.m_strLabel;
		if (CheckTextForSearch(pSearch, entry, text))
		{
			bFound = TRUE;
			::PostMessage(g_hMainWnd, MYWM_ITEMSEARCH, IDYES, (WPARAM)pSearch);
			//MessageBoxW(NULL, e.m_strLabel.c_str(), L"OK", 0);
			break;
		}

		// check internal text
		switch (e.m_et)
		{
		case ET_LANG:
			break;
		case ET_STRING:
			// ignore the name
			e.m_name.clear();
			break;
		default:
			continue;
		}
		text = pSearch->res2text.DumpEntry(e);
		if (CheckTextForSearch(pSearch, entry, text))
		{
			// found
			bFound = TRUE;
			::PostMessage(g_hMainWnd, MYWM_ITEMSEARCH, IDYES, (WPARAM)pSearch);
			//MessageBoxW(NULL, (e.m_strLabel + L"<>" + text).c_str(), NULL, 0);
			break;
		}
	}

	if (!bFound)
		::PostMessage(g_hMainWnd, MYWM_ITEMSEARCH, IDNO, (WPARAM)pSearch);

	pSearch->bRunning = FALSE;  // finish
	return 0;
}

// find next
BOOL MMainWnd::OnFindNext(HWND hwnd)
{
	m_search.bDownward = TRUE;
	if (m_search.strText.empty())
	{
		OnItemSearch(hwnd);
		return TRUE;
	}
	DoItemSearchBang(hwnd, NULL);
	return TRUE;
}

// find previous
BOOL MMainWnd::OnFindPrev(HWND hwnd)
{
	m_search.bDownward = FALSE;
	if (m_search.strText.empty())
	{
		OnItemSearch(hwnd);
		return TRUE;
	}
	DoItemSearchBang(hwnd, NULL);
	return TRUE;
}

// find the text
void MMainWnd::OnFind(HWND hwnd)
{
	m_search.bDownward = TRUE;
	OnItemSearch(hwnd);
}

// do item search
BOOL MMainWnd::DoItemSearch(ITEM_SEARCH& search)
{
	MWaitCursor wait;

	if (search.bIgnoreCases)
		CharUpperW(&search.strText[0]);

	search_proc(&search);

	return search.pFound != nullptr;
}

static void search_worker_thread(MMainWnd* pThis, HWND hwnd, MItemSearchDlg* pDialog)
{
	pThis->search_worker_thread_outer(hwnd, pDialog);
}

// show the item search dialog
void MMainWnd::OnItemSearch(HWND hwnd)
{
	// is there "item search" dialogs?
	if (MItemSearchDlg::Dialog())
	{
		HWND hDlg = *MItemSearchDlg::Dialog();
		if (::IsWindow(hDlg))
		{
			// bring it to the top
			SetForegroundWindow(hDlg);
			SetFocus(hDlg);
			return;
		}
		// erase hDlg from dialogs
		MItemSearchDlg::Dialog() = nullptr;
	}

	// create dialog
	auto pDialog = std::make_shared<MItemSearchDlg>(m_search);
	pDialog->CreateDialogDx(hwnd);
	MItemSearchDlg::Dialog() = pDialog;

	// set the window handles to m_search.res2text
	m_search.res2text.m_hwnd = hwnd;
	m_search.res2text.m_hwndDialog = *pDialog;

	// show it
	ShowWindow(*pDialog, SW_SHOWNORMAL);
	UpdateWindow(*pDialog);
}

// MYWM_ITEMSEARCH: do item search
LRESULT MMainWnd::OnItemSearchBang(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	UINT nID = (UINT)wParam;
	switch (nID)
	{
	case IDOK: // Start search
		DoItemSearchBang(hwnd, (MItemSearchDlg *)lParam);
		break;
	case IDCANCEL: // Canceled
	case IDYES: // Found
	case IDNO: // Not found
		DoEnableControls(TRUE);
		break;
	default:
		assert(0);
		break;
	}
	return 0;
}

BOOL MMainWnd::DoInnerSearch(HWND hwnd)
{
	DWORD ich, ichEnd;
	SendMessageW(m_hCodeEditor, EM_GETSEL, (WPARAM)&ich, (LPARAM)&ichEnd);

	MString strText = MWindowBase::GetWindowText(m_hCodeEditor);

	MString strTarget = m_search.strText;
	if (m_search.bIgnoreCases)
	{
		CharUpperW(&strText[0]);
		CharUpperW(&strTarget[0]);
	}

	size_t index = MString::npos;
	if (m_search.bDownward)
	{
		if (ich == ichEnd)
			index = strText.find(strTarget);
		else if (ich + 1 < strText.size())
			index = strText.find(strTarget, ich + 1);
	}
	else
	{
		if (ich == ichEnd)
			index = strText.rfind(strTarget);
		else if (ich > 0)
			index = strText.rfind(strTarget, ich - 1);
	}

	if (index != MString::npos)
	{
		SendMessageW(m_hCodeEditor, EM_SETSEL, INT(index), INT(index + strTarget.size()));
		SendMessageW(m_hCodeEditor, EM_SCROLLCARET, 0, 0);
		return TRUE;
	}

	return FALSE;
}

void MMainWnd::search_worker_thread_outer(HWND hwnd, MItemSearchDlg* pDialog)
{
	DoEnableControls(FALSE);

	search_worker_thread_inner(hwnd, pDialog);

	DoEnableControls(TRUE);
}

void MMainWnd::search_worker_thread_inner(HWND hwnd, MItemSearchDlg* pDialog)
{
	// get the selected entry
	auto entry = g_res.get_entry();

	if (m_search.bIgnoreCases)
		CharUpperW(&m_search.strText[0]);

	// initialize
	m_search.bCancelled = FALSE;
	m_search.pFound = nullptr;
	m_search.pCurrent = g_res.get_shared(entry);

	// start searching
	if (DoInnerSearch(hwnd))
	{
		m_search.bRunning = FALSE;

		if (pDialog)
			pDialog->Done();    // uninitialize

		return;
	}

	if (DoItemSearch(m_search) && m_search.pFound)
	{
		m_search.bRunning = FALSE;

		if (pDialog)
			pDialog->Done();    // uninitialize

		// select the found one
		TreeView_SelectItem(m_hwndTV, m_search.pFound->m_hItem);
		TreeView_EnsureVisible(m_hwndTV, m_search.pFound->m_hItem);

		DoInnerSearch(hwnd);
		return;
	}

	m_search.bRunning = FALSE;

	if (pDialog)
		pDialog->Done();    // uninitialize

	// is it not cancelled?
	if (!m_search.bCancelled)
	{
		// "no more item" message
		if (pDialog)
			EnableWindow(*pDialog, FALSE);
		MsgBoxDx(IDS_NOMOREITEM, MB_ICONINFORMATION);
		if (pDialog)
			EnableWindow(*pDialog, TRUE);
	}
}

BOOL MMainWnd::DoItemSearchBang(HWND hwnd, MItemSearchDlg *pDialog)
{
	// is it visible?
	if (pDialog == NULL || !IsWindowVisible(pDialog->m_hwnd))
	{
		pDialog = NULL;
	}

	std::thread t1(::search_worker_thread, this, hwnd, pDialog);
	t1.detach();

	return FALSE;
}
