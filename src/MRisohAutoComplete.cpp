// MRisohAutoComplete.cpp --- input auto-completion
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2026 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#include "MRisohAutoComplete.hpp"
#include "Utils.h"

//////////////////////////////////////////////////////////////////////////////
// MRisohAutoComplete

MRisohAutoComplete::MRisohAutoComplete(INT type, BOOL bUILanguage)
{
	m_type = type;
	m_nCurrentElement = 0;
	m_nRefCount = 1;
	m_fBound = FALSE;
	m_pAC = NULL;

	if (type == 0) // Types
	{
		if (InitTypes() && g_pTypes)
		{
			for (auto& name : *g_pTypes)
			{
				push_back(name);
			}
		}
	}
	if (type == 1) // Names
	{
		if (InitNames() && g_pNames)
		{
			for (auto& name : *g_pNames)
			{
				push_back(name);
			}
		}
	}
	if (type == 2) // Languages
	{
		for (auto& lang : g_langs)
		{
			if (bUILanguage && !IsValidUILang(lang.LangID))
				continue;

			push_back(lang.str);
		}
	}
}

MRisohAutoComplete::~MRisohAutoComplete()
{
	unbind();
}

void MRisohAutoComplete::push_back(const std::wstring& text)
{
	auto it = std::find(m_list.begin(), m_list.end(), text);
	if (it == m_list.end())
		m_list.push_back(text);
}

void MRisohAutoComplete::erase(const std::wstring& text)
{
	auto it = std::find(m_list.begin(), m_list.end(), text);
	if (it != m_list.end())
		m_list.erase(it);
}

size_t MRisohAutoComplete::size() const
{
	return m_list.size();
}

bool MRisohAutoComplete::empty() const
{
	return size() == 0;
}

bool MRisohAutoComplete::bind(HWND hwndEdit)
{
	assert(::IsWindow(hwndEdit));

	if (m_fBound && m_pAC)
		return false;

	::CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pAC));
	if (m_pAC)
	{
		IAutoComplete2 *pAC2 = NULL;
		m_pAC->QueryInterface(IID_PPV_ARGS(&pAC2));
		if (pAC2)
		{
			pAC2->SetOptions(ACO_AUTOSUGGEST | ACO_UPDOWNKEYDROPSLIST);
			pAC2->Release();
		}

		m_pAC->Init(hwndEdit, this, NULL, NULL);
		m_fBound = TRUE;
		return true;
	}

	assert(0);
	return false;
}

void MRisohAutoComplete::unbind()
{
	if (!m_fBound)
		return;

	if (m_pAC)
	{
		m_pAC->Release();
		m_pAC = NULL;
		m_fBound = FALSE;
	}
}

// IUnknown interface
STDMETHODIMP_(ULONG) MRisohAutoComplete::AddRef()
{
	return ++m_nRefCount;
}

STDMETHODIMP_(ULONG) MRisohAutoComplete::Release()
{
	ULONG nCount = --m_nRefCount;
	if (nCount == 0)
		delete this;
	return nCount;
}

STDMETHODIMP MRisohAutoComplete::QueryInterface(REFIID riid, void** ppvObject)
{
	if (ppvObject == NULL)
		return E_POINTER;

	*ppvObject = NULL;

	IUnknown *punk = NULL;
	if (riid == IID_IUnknown)
		punk = static_cast<IUnknown *>(this);
	else if (riid == IID_IEnumString)
		punk = static_cast<IEnumString *>(this);

	if (punk == NULL)
		return E_NOINTERFACE;

	punk->AddRef();
	*ppvObject = punk;
	return S_OK;
}

// IEnumString interface
STDMETHODIMP MRisohAutoComplete::Next(ULONG celt, LPOLESTR* rgelt, ULONG* pceltFetched)
{
	if (!rgelt)
		return E_POINTER;

	if (!celt)
		celt = 1;

	ULONG fetched = 0;
	for (ULONG i = 0; i < celt; i++)
	{
		if (m_nCurrentElement == m_list.size())
			break;

		size_t cb = (m_list[m_nCurrentElement].size() + 1) * sizeof(WCHAR);
		rgelt[i] = reinterpret_cast<LPWSTR>(::CoTaskMemAlloc(cb));
		if (!rgelt[i])
		{
			for (ULONG j = 0; j < fetched; j++)
				::CoTaskMemFree(rgelt[j]);
			if (pceltFetched)
				*pceltFetched = 0;
			return E_OUTOFMEMORY;
		}
		memcpy(rgelt[i], m_list[m_nCurrentElement].c_str(), cb);

		fetched++;
		m_nCurrentElement++;
	}

	if (pceltFetched)
		*pceltFetched = fetched;

	return (fetched == celt) ? S_OK : S_FALSE;
}

STDMETHODIMP MRisohAutoComplete::Skip(ULONG celt)
{
	m_nCurrentElement += celt;
	if (m_nCurrentElement > m_list.size())
		m_nCurrentElement = 0;
	return S_OK;
}

STDMETHODIMP MRisohAutoComplete::Reset(void)
{
	m_nCurrentElement = 0;
	return S_OK;
}

STDMETHODIMP MRisohAutoComplete::Clone(IEnumString** ppenum)
{
	if (!ppenum)
		return E_POINTER;

	MRisohAutoComplete *cloned = new MRisohAutoComplete(m_type);

	cloned->AddRef();
	cloned->m_list = m_list;
	*ppenum = cloned;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// MTVAutoCompleteEdit

MTVAutoCompleteEdit::MTVAutoCompleteEdit()
	: m_hwndTV(NULL)
	, m_bHooked(FALSE)
	, m_bAdjustSize(FALSE)
{
}

void MTVAutoCompleteEdit::hook(HWND hwndEdit, HWND hwndTV)
{
	if (!m_bHooked)
	{
		SubclassDx(hwndEdit);
		m_hwndTV = hwndTV;
		m_bHooked = TRUE;
	}
}

void MTVAutoCompleteEdit::unhook()
{
	if (m_bHooked)
	{
		UnsubclassDx();
	}
	m_bHooked = FALSE;
	m_hwndTV = NULL;
}

void MTVAutoCompleteEdit::AdjustDropdown()
{
	if (!m_bAdjustSize || !m_bHooked || !::IsWindow(m_hwnd) || !::IsWindow(m_hwndTV))
		return;

	HWND hwndDropdown = FindWindowW(L"Auto-Suggest Dropdown", nullptr);
	if (!IsWindowVisible(hwndDropdown))
		return;

	RECT rcEdit;
	GetWindowRect(m_hwnd, &rcEdit);

	RECT rcTV;
	GetClientRect(m_hwndTV, &rcTV);
	MapWindowPoints(m_hwndTV, nullptr, (PPOINT)&rcTV, 2);

	RECT rcDrop;
	GetWindowRect(hwndDropdown, &rcDrop);

	INT x = rcEdit.left, y = rcEdit.bottom;
	INT width = __max(rcTV.right - x, 200);
	SetWindowPos(hwndDropdown, HWND_TOPMOST, x, y, width, rcDrop.bottom - rcDrop.top,
				 SWP_NOACTIVATE);
}

LRESULT CALLBACK
MTVAutoCompleteEdit::WindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = DefaultProcDx(hwnd, uMsg, wParam, lParam);
	switch (uMsg)
	{
	case WM_WINDOWPOSCHANGED:
	case WM_MOVE:
	case WM_SIZE:
	case WM_CHAR:
	case WM_KEYUP:
		AdjustDropdown();
		break;
	}
	return ret;
}
