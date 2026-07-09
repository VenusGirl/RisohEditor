// EgaBridge.hpp --- Bridge for EGA Programming Language integration
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2020 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include <cstdarg>
#include <cstddef>
#include <string>
#include <functional>
#include <windows.h>

#define WM_EGA_DO_RUN_ON_UI (WM_APP + 102)

// Function pointer types matching EGA API
typedef bool (*EgaInputFn)(char* buf, size_t buflen);
typedef void (*EgaPrintFn)(const char* fmt, va_list va);

namespace EgaBridge
{
	bool Initialize();
	void Uninitialize();
	void SetInputFn(EgaInputFn fn);
	void SetPrintFn(EgaPrintFn fn);
	bool StartInteractive();
	void StopInteractive();
	bool IsStopRequested();
	void* GetStopEventHandle();
	void RequestFileInput(const std::string& filename);
	bool TryTakeFileInputRequest(std::string& filename);
	bool RunOnUIThread(std::function<void(void*)> fn, void* param = nullptr);
	void ExecuteUITask(void* param);
}
