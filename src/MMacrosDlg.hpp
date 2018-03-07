// MMacrosDlg.hpp --- Dialogs for Predefined Macros
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2018 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////

#ifndef MZC4_MMACROSDLG_HPP_
#define MZC4_MMACROSDLG_HPP_

#include "MWindowBase.hpp"
#include "RisohSettings.hpp"
#include "MResizable.hpp"
#include "MComboBoxAutoComplete.hpp"
#include "resource.h"

struct MACRO_ENTRY;
class MAddMacroDlg;
class MEditMacroDlg;
class MMacrosDlg;

//////////////////////////////////////////////////////////////////////////////

struct MACRO_ENTRY
{
    TCHAR szKey[128];
    TCHAR szValue[256];
};

//////////////////////////////////////////////////////////////////////////////

class MAddMacroDlg : public MDialogBase
{
public:
    macro_map_type& m_map;
    MACRO_ENTRY& m_entry;
    MComboBoxAutoComplete m_cmb1;
    MComboBoxAutoComplete m_cmb2;

    MAddMacroDlg(macro_map_type& map, MACRO_ENTRY& entry) :
        MDialogBase(IDD_ADDMACRO), m_map(map), m_entry(entry)
    {
    }

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
        CheckDlgButton(hwnd, chx1, BST_CHECKED);

        HWND hCmb1 = GetDlgItem(hwnd, cmb1);
        SubclassChildDx(m_cmb1, cmb1);

        HWND hCmb2 = GetDlgItem(hwnd, cmb2);
        SubclassChildDx(m_cmb2, cmb2);

        CenterWindowDx();
        return TRUE;
    }

    void OnOK(HWND hwnd)
    {
        HWND hCmb1 = GetDlgItem(hwnd, cmb1);
        HWND hCmb2 = GetDlgItem(hwnd, cmb2);

        MACRO_ENTRY entry;

        ::GetWindowText(hCmb1, entry.szKey, _countof(entry.szKey));
        ::GetWindowText(hCmb2, entry.szValue, _countof(entry.szValue));

        mstr_trim(entry.szKey);
        mstr_trim(entry.szValue);

        if (entry.szKey[0] == 0)
        {
            ComboBox_SetEditSel(hCmb1, 0, -1);
            SetFocus(hCmb1);
            ErrorBoxDx(IDS_EMPTYSTR);
            return;
        }

        if (m_map.find(entry.szKey) != m_map.end())
        {
            ComboBox_SetEditSel(hCmb1, 0, -1);
            SetFocus(hCmb1);
            ErrorBoxDx(IDS_ALREADYEXISTS);
            return;
        }

        m_entry = entry;

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
        case cmb1:
            if (codeNotify == CBN_EDITCHANGE)
            {
                m_cmb1.OnEditChange();
            }
            break;
        case cmb2:
            if (codeNotify == CBN_EDITCHANGE)
            {
                m_cmb2.OnEditChange();
            }
            break;
        }
    }

    virtual INT_PTR CALLBACK
    DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
            HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        }
        return DefaultProcDx();
    }
};

//////////////////////////////////////////////////////////////////////////////

class MEditMacroDlg : public MDialogBase
{
public:
    MACRO_ENTRY& m_entry;
    MComboBoxAutoComplete m_cmb1;
    MComboBoxAutoComplete m_cmb2;

    MEditMacroDlg(MACRO_ENTRY& entry) :
        MDialogBase(IDD_EDITMACRO), m_entry(entry)
    {
    }

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
        CheckDlgButton(hwnd, chx1, BST_CHECKED);

        HWND hCmb1 = GetDlgItem(hwnd, cmb1);
        SubclassChildDx(m_cmb1, cmb1);

        HWND hCmb2 = GetDlgItem(hwnd, cmb2);
        SubclassChildDx(m_cmb2, cmb2);

        ::SetWindowText(m_cmb1, m_entry.szKey);
        ::SetWindowText(m_cmb2, m_entry.szValue);

        CenterWindowDx();
        return TRUE;
    }

    void OnOK(HWND hwnd)
    {
        HWND hCmb1 = GetDlgItem(hwnd, cmb1);
        HWND hCmb2 = GetDlgItem(hwnd, cmb2);

        MACRO_ENTRY entry;

        ::GetWindowText(hCmb1, entry.szKey, _countof(entry.szKey));
        ::GetWindowText(hCmb2, entry.szValue, _countof(entry.szValue));

        mstr_trim(entry.szKey);
        mstr_trim(entry.szValue);

        if (entry.szKey[0] == 0)
        {
            ComboBox_SetEditSel(hCmb1, 0, -1);
            SetFocus(hCmb1);
            ErrorBoxDx(IDS_EMPTYSTR);
            return;
        }

        m_entry = entry;

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
        case cmb1:
            if (codeNotify == CBN_EDITCHANGE)
            {
                m_cmb1.OnEditChange();
            }
            break;
        case cmb2:
            if (codeNotify == CBN_EDITCHANGE)
            {
                m_cmb2.OnEditChange();
            }
            break;
        }
    }

    virtual INT_PTR CALLBACK
    DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
            HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        }
        return DefaultProcDx();
    }
};

//////////////////////////////////////////////////////////////////////////////

class MMacrosDlg : public MDialogBase
{
public:
    RisohSettings& m_settings;
    macro_map_type& m_map;
    MResizable m_resizable;
    HWND m_hLst1;
    HICON m_hIcon;
    HICON m_hIconSm;
    MString m_strTemp;

    MMacrosDlg(RisohSettings& settings)
        : MDialogBase(IDD_MACROS), m_settings(settings),
          m_map(settings.macros)
    {
        m_hIcon = LoadIconDx(IDI_SMILY);
        m_hIconSm = LoadSmallIconDx(IDI_SMILY);
    }

    ~MMacrosDlg()
    {
        DestroyIcon(m_hIcon);
        DestroyIcon(m_hIconSm);
    }

    void OnDelete(HWND hwnd)
    {
        INT iItem = ListView_GetNextItem(m_hLst1, -1, LVNI_ALL | LVNI_SELECTED);
        if (iItem < 0)
            return;

        ListView_DeleteItem(m_hLst1, iItem);

        MACRO_ENTRY entry;
        ListView_GetItemText(m_hLst1, iItem, 0, entry.szKey, _countof(entry.szKey));

        m_map.erase(entry.szKey);
    }

    void OnAdd(HWND hwnd)
    {
        INT iItem = ListView_GetNextItem(m_hLst1, -1, LVNI_ALL | LVNI_SELECTED);
        if (iItem < 0)
        {
            return;
        }

        MACRO_ENTRY entry;
        MAddMacroDlg dialog(m_map, entry);
        if (dialog.DialogBoxDx(hwnd) == IDOK)
        {
            m_map[dialog.m_entry.szKey] = dialog.m_entry.szValue;

            INT iItem = ListView_GetItemCount(m_hLst1);
            LV_ITEM item;

            ZeroMemory(&item, sizeof(item));
            item.iItem = iItem;
            item.mask = LVIF_TEXT;
            item.iSubItem = 0;
            item.pszText = entry.szKey;
            ListView_InsertItem(m_hLst1, &item);

            ZeroMemory(&item, sizeof(item));
            item.iItem = iItem;
            item.mask = LVIF_TEXT;
            item.iSubItem = 1;
            item.pszText = entry.szValue;
            ListView_SetItem(m_hLst1, &item);

            UINT state = LVIS_SELECTED | LVIS_FOCUSED;
            ListView_SetItemState(m_hLst1, iItem, state, state);
            ListView_EnsureVisible(m_hLst1, iItem, FALSE);
        }
    }

    void OnModify(HWND hwnd)
    {
        INT iItem = ListView_GetNextItem(m_hLst1, -1, LVNI_ALL | LVNI_SELECTED);
        if (iItem < 0)
        {
            return;
        }

        MACRO_ENTRY entry;
        ListView_GetItemText(m_hLst1, iItem, 0, entry.szKey, _countof(entry.szKey));
        ListView_GetItemText(m_hLst1, iItem, 1, entry.szValue, _countof(entry.szValue));

        mstr_trim(entry.szKey);
        mstr_trim(entry.szValue);

        MEditMacroDlg dialog(entry);
        if (dialog.DialogBoxDx(hwnd) == IDOK)
        {
            m_map[dialog.m_entry.szKey] = dialog.m_entry.szValue;

            LV_ITEM item;

            ZeroMemory(&item, sizeof(item));
            item.iItem = iItem;
            item.mask = LVIF_TEXT;
            item.iSubItem = 0;
            item.pszText = entry.szKey;
            ListView_SetItem(m_hLst1, &item);

            ZeroMemory(&item, sizeof(item));
            item.iItem = iItem;
            item.mask = LVIF_TEXT;
            item.iSubItem = 1;
            item.pszText = entry.szValue;
            ListView_SetItem(m_hLst1, &item);
        }
    }

    void OnOK(HWND hwnd)
    {
        INT nCount = ListView_GetItemCount(m_hLst1);

        MACRO_ENTRY entry;

        m_map.clear();
        for (INT i = 0; i < nCount; ++i)
        {
            ListView_GetItemText(m_hLst1, i, 0, entry.szKey, _countof(entry.szKey));
            ListView_GetItemText(m_hLst1, i, 1, entry.szValue, _countof(entry.szValue));
            m_map[entry.szKey] = entry.szValue;
        }

        EndDialog(IDOK);
    }

    void OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
    {
        if (hwndContext == m_hLst1)
        {
            HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(2));
            HMENU hSubMenu = GetSubMenu(hMenu, 4);

            SetForegroundWindow(hwnd);
            TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                xPos, yPos, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);
            DestroyMenu(hMenu);
        }
    }

    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
        switch (id)
        {
        case psh1:
        case CMDID_ADD:
            OnAdd(hwnd);
            break;
        case psh2:
        case CMDID_MODIFY:
            OnModify(hwnd);
            break;
        case psh3:
        case CMDID_DELETE:
            OnDelete(hwnd);
            break;
        case CMDID_RENAME:
            OnRename(hwnd);
            break;
        case IDOK:
            if (codeNotify == 0 || codeNotify == BN_CLICKED)
                OnOK(hwnd);
            break;
        case IDCANCEL:
            if (codeNotify == 0 || codeNotify == BN_CLICKED)
                EndDialog(IDCANCEL);
            break;
        case psh6:
            if (codeNotify == 0 || codeNotify == BN_CLICKED)
            {
                m_settings.ResetMacros();
                EndDialog(IDOK);
            }
            break;
        case psh7:
            ListView_DeleteAllItems(m_hLst1);
            break;
        }
    }

    void OnRename(HWND hwnd)
    {
        INT iItem = ListView_GetNextItem(m_hLst1, -1, LVNI_ALL | LVNI_SELECTED);
        if (iItem < 0)
        {
            return;
        }

        ListView_EditLabel(m_hLst1, iItem);
    }

    LRESULT OnNotify(HWND hwnd, int idFrom, NMHDR *pnmhdr)
    {
        if (idFrom == lst1)
        {
            if (pnmhdr->code == LVN_KEYDOWN)
            {
                LV_KEYDOWN *KeyDown = (LV_KEYDOWN *)pnmhdr;
                if (KeyDown->wVKey == VK_DELETE)
                {
                    OnDelete(hwnd);
                    return 0;
                }
                if (KeyDown->wVKey == VK_F2)
                {
                    OnRename(hwnd);
                    return 0;
                }
            }
            if (pnmhdr->code == NM_DBLCLK)
            {
                OnModify(hwnd);
                return 0;
            }
            if (pnmhdr->code == LVN_BEGINLABELEDIT)
            {
                INT iItem = ListView_GetNextItem(m_hLst1, -1, LVNI_ALL | LVNI_SELECTED);
                if (iItem == -1)
                    return TRUE;
                LV_DISPINFO *pInfo = (LV_DISPINFO *)pnmhdr;
                pInfo->item.iItem = iItem;
                if (pInfo->item.pszText)
                    m_strTemp = pInfo->item.pszText;
                else
                    m_strTemp.clear();
                return 0;
            }
            if (pnmhdr->code == LVN_ENDLABELEDIT)
            {
                INT iItem = ListView_GetNextItem(m_hLst1, -1, LVNI_ALL | LVNI_SELECTED);
                if (iItem < 0)
                    return TRUE;

                LV_DISPINFO *pInfo = (LV_DISPINFO *)pnmhdr;

                if (pInfo->item.pszText == NULL)
                    return TRUE;

                macro_map_type::const_iterator it;
                it = m_map.find(pInfo->item.pszText);
                if (it != m_map.end())
                {
                    return TRUE;
                }

                MString strValue = m_map[m_strTemp];
                m_map.erase(m_strTemp);
                m_map[pInfo->item.pszText] = strValue;

                LV_ITEM item;
                ZeroMemory(&item, sizeof(item));
                item.iItem = iItem;
                item.mask = LVIF_TEXT;
                item.iSubItem = 0;
                item.pszText = pInfo->item.pszText;
                ListView_SetItem(m_hLst1, &item);
            }
        }
        return 0;
    }

    void OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu)
    {
        INT iItem = ListView_GetNextItem(m_hLst1, -1, LVNI_ALL | LVNI_SELECTED);
        if (iItem >= 0)
        {
            EnableMenuItem(hMenu, CMDID_MODIFY, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, CMDID_DELETE, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, CMDID_RENAME, MF_BYCOMMAND | MF_ENABLED);
        }
        else
        {
            EnableMenuItem(hMenu, CMDID_MODIFY, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenu, CMDID_DELETE, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenu, CMDID_RENAME, MF_BYCOMMAND | MF_GRAYED);
        }
    }

    virtual INT_PTR CALLBACK
    DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
            HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
            HANDLE_MSG(hwnd, WM_NOTIFY, OnNotify);
            HANDLE_MSG(hwnd, WM_SIZE, OnSize);
            HANDLE_MSG(hwnd, WM_CONTEXTMENU, OnContextMenu);
            HANDLE_MSG(hwnd, WM_INITMENUPOPUP, OnInitMenuPopup);
        }
        return DefaultProcDx();
    }

    void OnSize(HWND hwnd, UINT state, int cx, int cy)
    {
        m_resizable.OnSize();
    }

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
        m_hLst1 = GetDlgItem(hwnd, lst1);
        ListView_SetExtendedListViewStyle(m_hLst1, LVS_EX_FULLROWSELECT);

        LV_COLUMN column;
        ZeroMemory(&column, sizeof(column));

        column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
        column.fmt = LVCFMT_LEFT;
        column.cx = 150;
        column.pszText = LoadStringDx(IDS_MACRONAME);
        column.iSubItem = 0;
        ListView_InsertColumn(m_hLst1, 0, &column);

        column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
        column.fmt = LVCFMT_LEFT;
        column.cx = 300;
        column.pszText = LoadStringDx(IDS_MACROVALUE);
        column.iSubItem = 1;
        ListView_InsertColumn(m_hLst1, 1, &column);

        LV_ITEM item;
        INT iItem = 0;

        macro_map_type::iterator it, end = m_map.end();
        for (it = m_map.begin(); it != end; ++it)
        {
            ZeroMemory(&item, sizeof(item));
            item.iItem = iItem;
            item.mask = LVIF_TEXT;
            item.iSubItem = 0;
            item.pszText = const_cast<LPTSTR>(it->first.c_str());
            ListView_InsertItem(m_hLst1, &item);

            ZeroMemory(&item, sizeof(item));
            item.iItem = iItem;
            item.mask = LVIF_TEXT;
            item.iSubItem = 1;
            item.pszText = const_cast<LPTSTR>(it->second.c_str());
            ListView_SetItem(m_hLst1, &item);

            ++iItem;
        }

        m_resizable.OnParentCreate(hwnd);

        m_resizable.SetLayoutAnchor(lst1, mzcLA_TOP_LEFT, mzcLA_BOTTOM_RIGHT);
        m_resizable.SetLayoutAnchor(psh1, mzcLA_TOP_RIGHT);
        m_resizable.SetLayoutAnchor(psh2, mzcLA_TOP_RIGHT);
        m_resizable.SetLayoutAnchor(psh3, mzcLA_TOP_RIGHT);
        m_resizable.SetLayoutAnchor(psh4, mzcLA_TOP_RIGHT);
        m_resizable.SetLayoutAnchor(psh5, mzcLA_TOP_RIGHT);
        m_resizable.SetLayoutAnchor(psh7, mzcLA_TOP_RIGHT);
        m_resizable.SetLayoutAnchor(IDOK, mzcLA_BOTTOM_RIGHT);
        m_resizable.SetLayoutAnchor(IDCANCEL, mzcLA_BOTTOM_RIGHT);
        m_resizable.SetLayoutAnchor(psh6, mzcLA_BOTTOM_LEFT);

        SendMessageDx(WM_SETICON, ICON_BIG, (LPARAM)m_hIcon);
        SendMessageDx(WM_SETICON, ICON_SMALL, (LPARAM)m_hIconSm);

        UINT state = LVIS_SELECTED | LVIS_FOCUSED;
        ListView_SetItemState(m_hLst1, 0, state, state);
        ListView_EnsureVisible(m_hLst1, 0, FALSE);

        SetFocus(m_hLst1);

        CenterWindowDx();
        return TRUE;
    }
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef MZC4_MMACROSDLG_HPP_