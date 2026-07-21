// Utils.h --- RisohEditor
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2025 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include <windows.h>
#include <unordered_map>
#include <vector>
#include "Common.hpp"
#include "MString.hpp"
#include "ConstantsDB.hpp"
#include "Res.hpp"
#include "StringRes.hpp"
#include "MessageRes.hpp"

// structure for language information
struct LANG_ENTRY
{
	LANGID LangID;    // language ID
	MStringW str;   // string

	// for sorting
	bool operator<(const LANG_ENTRY& ent) const
	{
		return str < ent.str;
	}
};
extern std::vector<LANG_ENTRY> g_langs;

void ReplaceFullWithHalf(wchar_t* pszText);
void ReplaceFullWithHalf(std::wstring& strText);

BOOL IsFileWritable(LPCWSTR pszFileName);
BOOL WaitForVirusScan(LPCWSTR pszFileName, DWORD dwTimeout = 15000);

bool create_directories_recursive_win32(const std::wstring& path);

std::wstring DumpBinaryAsText(const std::vector<BYTE>& data);
BOOL WriteBinaryFileDx(const WCHAR *filename, LPCVOID pv, size_t size);

struct AutoDeleteFileW
{
	std::wstring m_file;
	AutoDeleteFileW(const std::wstring& file) : m_file(file) { }
	~AutoDeleteFileW() { ::DeleteFileW(m_file.c_str()); }
};

WORD GetMachineOfBinary(LPCWSTR pszExeFile);
BOOL IsFileLockedDx(LPCTSTR pszFileName);
BOOL DeleteDirectoryDx(LPCTSTR pszDir);
BOOL IsEmptyDirectoryDx(LPCTSTR pszPath);
BOOL GetPathOfShortcutDx(HWND hwnd, LPCWSTR pszLnkFile, LPWSTR pszPath);
INT LogMessageBoxW(HWND hwnd, LPCWSTR text, LPCWSTR title, UINT uType);
HRESULT FileSystemAutoComplete(HWND hwnd);
void MyChangeNotify(LPCWSTR pszFileName);
LANGID GetDefaultResLanguage(VOID);
void GetStyleSelect(HWND hLst, std::vector<BYTE>& sel);
void GetStyleSelect(std::vector<BYTE>& sel, const ConstantsDB::TableType& table, DWORD dwValue);
DWORD AnalyseStyleDiff(
	DWORD dwValue, ConstantsDB::TableType& table,
	std::vector<BYTE>& old_sel, std::vector<BYTE>& new_sel);
void InitStyleListBox(HWND hLst, ConstantsDB::TableType& table);
void InitFontComboBox(HWND hCmb);
void InitCharSetComboBox(HWND hCmb, BYTE CharSet);
BYTE GetCharSetFromComboBox(HWND hCmb);
void InitCaptionComboBox(HWND hCmb, LPCTSTR pszCaption);
void InitClassComboBox(HWND hCmb, LPCTSTR pszClass);
void InitWndClassComboBox(HWND hCmb, LPCTSTR pszWndClass);
void InitCtrlIDComboBox(HWND hCmb);
void Res_ReplaceResTypeString(MString& str, bool bRevert = false);
MIdOrString ResourceTypeFromIDType(INT nIDTYPE_);
MString GetAssoc(const MString& name);
void InitComboBoxPlaceholder(HWND hCmb, UINT nStringID);
void InitResTypeComboBox(HWND hCmb1, const MIdOrString& type);	
void InitResNameComboBoxDword(HWND hCmb, const DWORD& id, IDTYPE_ nIDTYPE_);
void InitResNameComboBox(HWND hCmb, const MIdOrString& id, IDTYPE_ nIDTYPE_);
void InitResNameComboBox(HWND hCmb, const MIdOrString& id, IDTYPE_ nIDTYPE_1, IDTYPE_ nIDTYPE_2);
BOOL CheckCommand(MString strCommand);
void InitConstantComboBox(HWND hCmb);
void InitStringComboBox(HWND hCmb, const MString& strString);
void InitMessageComboBox(HWND hCmb, const MString& strString);
BOOL IsValidUILang(LANGID langid);
void InitLangComboBox(HWND hCmb3, LANGID langid, BOOL bUILanguage);
void InitLangComboBox(HWND hCmb3, LANGID langid);
void InitLangListView(HWND hLst1, LPCTSTR pszText);
LANGID LangFromText(LPWSTR pszLang);
BOOL CheckLangComboBox(HWND hCmb3, LANGID& lang, LANG_TYPE type);
BOOL CheckLangComboBox(HWND hCmb3, LANGID& lang);
MStringW TextFromLang(LANGID lang);
BOOL CheckTypeComboBox(HWND hCmb1, MIdOrString& type);
BOOL CheckNameComboBox(HWND hCmb2, const MIdOrString& type, MIdOrString& name);
BOOL Edt1_CheckFile(HWND hEdt1, MStringW& file);
MStringW GetKeyID(UINT wId);
void Cmb1_InitVirtualKeys(HWND hCmb1);
BOOL Cmb1_CheckKey(HWND hwnd, HWND hCmb1, BOOL bVirtKey, MStringW& str);
MString GetLanguageStatement(LANGID langid, BOOL bOldStyle);
void DoSetFileModified(BOOL bModified);
MString GetLanguageStatement(LANGID langid, BOOL bOldStyle);
BOOL StrDlg_GetEntry(HWND hwnd, STRING_ENTRY& entry);
void StrDlg_SetEntry(HWND hwnd, STRING_ENTRY& entry);
BOOL MsgDlg_GetEntry(HWND hwnd, MESSAGE_ENTRY& entry);
void MsgDlg_SetEntry(HWND hwnd, MESSAGE_ENTRY& entry);
MStringW GetRisohTemplate(const MIdOrString& type, const MIdOrString& name, LANGID wLang);
BOOL PlayMP3(LPCVOID ptr, size_t size);
void StopMP3(void);
BOOL PlayAvi(HWND hwnd, LPCVOID ptr, size_t size);
void StopAvi(void);

extern std::vector<LANG_ENTRY> g_langs;
extern TCHAR g_szMP3TempFile[MAX_PATH];

BOOL CALLBACK
EnumResLangProc(HMODULE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LPARAM lParam);
BOOL CALLBACK EnumLocalesProc(LPWSTR lpLocaleString);
BOOL CALLBACK EnumEngLocalesProc(LPWSTR lpLocaleString);
BOOL IsCodePageReallyUsable(UINT cp);
MStringW GetResTypeEncoding(const MIdOrString& type);
BOOL DoCheckFile(std::wstring& file, LPCWSTR psz);

extern std::vector<MString> *g_pNames;
extern std::vector<MString> *g_pTypes;

BOOL InitTypes(void);
BOOL InitNames(void);
BOOL InitLangListBox(HWND hwnd);
BOOL ChooseTypeListBoxType(HWND hwnd, const MIdOrString& type);
BOOL ChooseNameListBoxName(HWND hwnd, const MIdOrString& type, const MIdOrString& name);
BOOL ChooseLangListBoxLang(HWND hwnd, LANGID wLangId);
MStringW GetTextInclude1HeaderFile(const EntrySet& res, LPCWSTR szRCPath);
BOOL IsExeOrDll(LPCWSTR pszFileName);
BOOL IsDotExe(LPCWSTR pszFileName);
BOOL DumpTinyExeOrDll(HINSTANCE hInst, LPCWSTR pszFileName, INT nID);
BOOL DoResetCheckSum(LPCWSTR pszExeFile);
std::wstring generated_from(INT n);
void WriteMacroLine(MFile& file, const MStringA& name, const MStringA& definition);
MIdOrString GetTypeFromText(const WCHAR *pszText);
MIdOrString GetNameFromText(const WCHAR *pszText);
BOOL IsThereWndClass(const WCHAR *pszName);
void FreeWCLib();
