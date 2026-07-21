// MRisohAutoComplete.hpp --- input auto-completion
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2026 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include <windows.h>
#include <shldisp.h>
#include <shlguid.h>
#include <string>
#include <vector>
#include "MEditCtrl.hpp"

//////////////////////////////////////////////////////////////////////////////
// MRisohAutoComplete

class MRisohAutoComplete : public IEnumString
{
public:
	INT m_type = 2;

	MRisohAutoComplete(INT type, BOOL bUILanguage = FALSE);
	virtual ~MRisohAutoComplete();

	void push_back(const std::wstring& text);
	void erase(const std::wstring& text);
	size_t size() const;
	bool empty() const;

	bool bind(HWND hwndEdit);
	void unbind();

	// IUnknown interface
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;

	// IEnumString interface
	STDMETHODIMP Next(ULONG celt, LPOLESTR* rgelt, ULONG* pceltFetched) override;
	STDMETHODIMP Skip(ULONG celt) override;
	STDMETHODIMP Reset(void) override;
	STDMETHODIMP Clone(IEnumString** ppenum) override;

protected:
	IAutoComplete *m_pAC;
	std::vector<std::wstring> m_list;
	ULONG m_nCurrentElement;
	ULONG m_nRefCount;
	BOOL m_fBound;
};

//////////////////////////////////////////////////////////////////////////////
// MTVAutoCompleteEdit

class MTVAutoCompleteEdit : public MEditCtrl
{
public:
	HWND m_hwndTV;
	BOOL m_bHooked;
	BOOL m_bAdjustSize;

	MTVAutoCompleteEdit();

	void hook(HWND hwndEdit, HWND hwndTV = NULL);
	void unhook();

	void AdjustDropdown();

protected:
	LRESULT CALLBACK WindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};
