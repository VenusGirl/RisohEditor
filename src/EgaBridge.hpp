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
#define WM_EGA_DO_PRINT     (WM_APP + 101)  // notification only; UI thread calls EgaBridge::TakePendingPrintText()

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
	void StopInteractive(bool wait = true);
	bool IsStopRequested();
	void* GetStopEventHandle();
	void RequestFileInput(const std::string& filename);
	bool TryTakeFileInputRequest(std::string& filename);
	bool RunOnUIThread(std::function<void(void*)> fn, void* param = nullptr);
	void ExecuteUITask(void* param);

	// Enter/input handshake between the UI thread and the EGA worker thread.
	// All of this state is (re-)created in Initialize() and destroyed in
	// Uninitialize(), so every session starts from a clean state -- this is
	// what guarantees that closing and re-opening the EGA dialog reconnects
	// correctly instead of misfiring on stale state left over from a
	// previous session.
	void NotifyEnterPressed();                          // UI thread: user pressed Enter / clicked OK
	bool IsEnterPressed();                              // EGA thread: non-destructive poll
	void ClearEnterPressed();                           // EGA thread: call once the wait is over

	void PrepareForInput();                             // EGA thread: call before requesting input
	void SubmitInputText(const std::wstring& text);      // UI thread: call after reading the edit control
	bool WaitAndTakeInputText(std::wstring& outText);    // EGA thread: wait for SubmitInputText or stop

	// Coalesced print output between the EGA worker thread and the UI thread.
	//
	// A naive "PostMessage once per EGA_do_print call" scheme lets a fast,
	// tight EGA loop (e.g. for(i, 0, 1000, println(i))) flood the UI
	// thread's message queue with hundreds/thousands of WM_EGA_DO_PRINT
	// messages within milliseconds, far faster than the UI thread can
	// drain them. While that backlog is being processed the window
	// effectively stops repainting/tracking the cursor, which looks and
	// feels like a hang even though no thread is actually deadlocked.
	//
	// QueuePrintText() instead appends to a shared buffer and posts
	// WM_EGA_DO_PRINT (defined above) only if a flush isn't already
	// pending, so any number of prints that happen between two UI-thread
	// pumps get coalesced into a single message / single edit-control
	// update.
	void QueuePrintText(const std::wstring& text);        // any thread (typically EGA worker thread)
	bool TakePendingPrintText(std::wstring& outText);      // UI thread: called on WM_EGA_DO_PRINT
}
