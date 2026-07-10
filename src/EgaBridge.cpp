// EgaBridge.cpp --- Bridge for EGA Programming Language integration
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2020 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#include "EgaBridge.hpp"
#include <windows.h>
#include <queue>
#include <utility>
#include <atomic>

#include "../EGA/ega.hpp"

using namespace EGA;

extern HWND s_hwndEga;

namespace
{
	static CRITICAL_SECTION s_cs;
	static bool     s_bCsReady    = false;
	static HANDLE   s_hThread     = NULL;
	static HANDLE   s_hStopEvent  = NULL;   // manual-reset
	static bool     s_bRunning    = false;
	static bool     s_bInitialized = false;
	static CRITICAL_SECTION s_fileCs;
	static bool     s_fileCsReady = false;
	static std::queue<std::string> s_fileQueue;
	static std::queue<std::pair<std::function<void(void*)>, void*>> s_uiQueue;
	static CRITICAL_SECTION s_uiCs;
	static HANDLE s_hUIDone = NULL;

	// Enter/input handshake state (owned by EgaBridge so that it is
	// re-created every session -- see Initialize()/Uninitialize()).
	static std::atomic<bool> s_bEnterPressed(false);
	static CRITICAL_SECTION s_inputCs;
	static bool     s_inputCsReady = false;
	static HANDLE   s_hInputDone   = NULL;   // auto-reset event
	static std::wstring s_inputBuffer;       // protected by s_inputCs

	// Coalesced print-output state. Protected by s_printCs. s_bPrintPosted
	// tracks whether a WM_EGA_DO_PRINT flush is already sitting in the UI
	// thread's queue, so that bursts of prints between two UI-thread pumps
	// get merged into a single message instead of one message each.
	static CRITICAL_SECTION s_printCs;
	static bool     s_printCsReady = false;
	static std::atomic<bool> s_bPrintPosted(false);
	static std::wstring s_printBuffer;       // protected by s_printCs
}

static DWORD WINAPI EgaBridgeThreadProc(LPVOID args)
{
#ifndef NDEBUG
	OutputDebugStringW(L"EgaBridgeThreadProc: enter\n");
#endif

	try
	{
		EGA_interactive(NULL, true);
	}
	catch (...)
	{
		;
	}

	EnterCriticalSection(&s_cs);
	s_bRunning = false;
	LeaveCriticalSection(&s_cs);

#ifndef NDEBUG
	OutputDebugStringW(L"EgaBridgeThreadProc: leave\n");
#endif

	// Do not close s_hThread here. Owner thread handles it.
	return 0;
}

namespace EgaBridge
{
	bool Initialize()
	{
		if (s_bInitialized)
			return true;

		InitializeCriticalSection(&s_cs);
		InitializeCriticalSection(&s_fileCs);
		InitializeCriticalSection(&s_uiCs);
		InitializeCriticalSection(&s_inputCs);
		InitializeCriticalSection(&s_printCs);
		s_fileCsReady = true;
		s_inputCsReady = true;
		s_printCsReady = true;

		s_hStopEvent = ::CreateEventW(NULL, TRUE, FALSE, NULL); // Manual-reset
		s_hUIDone    = ::CreateEventW(NULL, FALSE, FALSE, NULL); // auto-reset
		s_hInputDone = ::CreateEventW(NULL, FALSE, FALSE, NULL); // auto-reset
		if (!s_hStopEvent || !s_hUIDone || !s_hInputDone)
		{
			if (s_hInputDone) { ::CloseHandle(s_hInputDone); s_hInputDone = NULL; }
			if (s_hUIDone) { ::CloseHandle(s_hUIDone); s_hUIDone = NULL; }
			if (s_hStopEvent) { ::CloseHandle(s_hStopEvent); s_hStopEvent = NULL; }
			DeleteCriticalSection(&s_printCs);
			DeleteCriticalSection(&s_inputCs);
			DeleteCriticalSection(&s_uiCs);
			DeleteCriticalSection(&s_fileCs);
			DeleteCriticalSection(&s_cs);
			s_printCsReady = false;
			s_inputCsReady = false;
			s_fileCsReady = false;
			return false;
		}

		s_bCsReady = true;

		// Fresh session: make sure no state leaks in from a previous
		// EGA dialog session.
		s_bEnterPressed = false;
		s_inputBuffer.clear();
		s_bPrintPosted = false;
		s_printBuffer.clear();

		if (!EGA_init())
		{
			s_bCsReady = false;
			::CloseHandle(s_hInputDone); s_hInputDone = NULL;
			::CloseHandle(s_hUIDone); s_hUIDone = NULL;
			::CloseHandle(s_hStopEvent); s_hStopEvent = NULL;
			DeleteCriticalSection(&s_printCs);
			DeleteCriticalSection(&s_inputCs);
			DeleteCriticalSection(&s_uiCs);
			DeleteCriticalSection(&s_fileCs);
			s_printCsReady = false;
			s_inputCsReady = false;
			s_fileCsReady = false;
			DeleteCriticalSection(&s_cs);
			return false;
		}

		s_bInitialized = true;
		return true;
	}

	void Uninitialize()
	{
		if (!s_bInitialized)
			return;

		StopInteractive(true);
		EGA_uninit();
		s_bInitialized = false;

		if (s_hStopEvent) { ::CloseHandle(s_hStopEvent); s_hStopEvent = NULL; }
		if (s_bCsReady)   { DeleteCriticalSection(&s_cs); s_bCsReady = false; }
		if (s_fileCsReady) { DeleteCriticalSection(&s_fileCs); s_fileCsReady = false; }
		if (s_hUIDone) { ::CloseHandle(s_hUIDone); s_hUIDone = nullptr; }
		::DeleteCriticalSection(&s_uiCs);

		if (s_hInputDone) { ::CloseHandle(s_hInputDone); s_hInputDone = NULL; }
		if (s_inputCsReady) { DeleteCriticalSection(&s_inputCs); s_inputCsReady = false; }
		s_bEnterPressed = false;
		s_inputBuffer.clear();

		if (s_printCsReady) { DeleteCriticalSection(&s_printCs); s_printCsReady = false; }
		s_bPrintPosted = false;
		s_printBuffer.clear();
	}

	void SetInputFn(EgaInputFn fn)
	{
		EGA_set_input_fn(fn);
	}

	void SetPrintFn(EgaPrintFn fn)
	{
		EGA_set_print_fn(fn);
	}

	bool StartInteractive()
	{
		EnterCriticalSection(&s_cs);

		OutputDebugStringA("StartInteractive\n");

		if (s_bRunning)
		{
			OutputDebugStringA("already running\n");
			LeaveCriticalSection(&s_cs);
			return true;
		}

		::ResetEvent(s_hStopEvent);
		s_bRunning = true;

		HANDLE hThread = ::CreateThread(NULL, 0, EgaBridgeThreadProc, NULL, 0, NULL);
		if (!hThread)
		{
			OutputDebugStringA("CreateThread failed\n");
			s_bRunning = false;
			LeaveCriticalSection(&s_cs);
			return false;
		}

		if (s_hThread)
			::CloseHandle(s_hThread);
		s_hThread = hThread;

		LeaveCriticalSection(&s_cs);
		return true;
	}

	void StopInteractive(bool wait)
	{
		HANDLE hThread = NULL;

		EnterCriticalSection(&s_cs);

		if (!s_bRunning)
		{
			if (s_hThread)
			{
				::CloseHandle(s_hThread);
				s_hThread = NULL;
			}

			LeaveCriticalSection(&s_cs);
			return;
		}

		EGA_stop();
		::SetEvent(s_hStopEvent);
		hThread = s_hThread;

		LeaveCriticalSection(&s_cs);

		if (hThread && wait)
		{
			DWORD result = ::WaitForSingleObject(hThread, 10 * 1000);

			if (result == WAIT_TIMEOUT)
				::DebugBreak();

			::CloseHandle(hThread);

			EnterCriticalSection(&s_cs);

			if (s_hThread == hThread)
				s_hThread = NULL;

			LeaveCriticalSection(&s_cs);
		}

		EnterCriticalSection(&s_fileCs);

		while (!s_fileQueue.empty())
			s_fileQueue.pop();

		LeaveCriticalSection(&s_fileCs);
	}

	bool IsStopRequested()
	{
		return EGA_is_stopping() || 
		       (s_hStopEvent != NULL &&
		        ::WaitForSingleObject(s_hStopEvent, 0) == WAIT_OBJECT_0);
	}

	void* GetStopEventHandle()
	{
		return s_hStopEvent;
	}

	void RequestFileInput(const std::string& filename)
	{
		if (!s_fileCsReady)
			return;
		EnterCriticalSection(&s_fileCs);
		s_fileQueue.push(filename);
		LeaveCriticalSection(&s_fileCs);
	}

	bool TryTakeFileInputRequest(std::string& filename)
	{
		if (!s_fileCsReady)
			return false;
		bool ret = false;
		EnterCriticalSection(&s_fileCs);
		if (!s_fileQueue.empty())
		{
			filename = s_fileQueue.front();
			s_fileQueue.pop();
			ret = true;
		}
		LeaveCriticalSection(&s_fileCs);
		return ret;
	}

	bool RunOnUIThread(std::function<void(void*)> fn, void* param)
	{
#ifndef NDEBUG
		OutputDebugStringA("RunOnUIThread\n");
#endif
		if (IsStopRequested())
			return false;

		HWND hwnd;
		EnterCriticalSection(&s_uiCs);
		s_uiQueue.push(std::make_pair(fn, param));
		hwnd = s_hwndEga;
		LeaveCriticalSection(&s_uiCs);

		if (!::IsWindow(hwnd))
			return false;

		if (!::PostMessageW(hwnd, WM_EGA_DO_RUN_ON_UI, 0, 0))
			return false;

		HANDLE waitHandles[2] = { s_hUIDone, s_hStopEvent };
		DWORD wait = ::WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
		return wait == WAIT_OBJECT_0;
	}

	void ExecuteUITask(void*)
	{
		std::function<void(void*)> task;
		void* param = nullptr;

		EnterCriticalSection(&s_uiCs);

		if (!s_uiQueue.empty())
		{
			task  = std::move(s_uiQueue.front().first);
			param = s_uiQueue.front().second;
			s_uiQueue.pop();
		}

		LeaveCriticalSection(&s_uiCs);

		if (task)
		{
			try
			{
				task(param);
			}
			catch (...)
			{
	#ifndef NDEBUG
				OutputDebugStringA(
					"EGA UI task exception\n");
	#endif
			}
		}

		::SetEvent(s_hUIDone);
	}

	void NotifyEnterPressed()
	{
		s_bEnterPressed = true;
	}

	bool IsEnterPressed()
	{
		return s_bEnterPressed;
	}

	void ClearEnterPressed()
	{
		s_bEnterPressed = false;
	}

	void PrepareForInput()
	{
		if (s_hInputDone)
			::ResetEvent(s_hInputDone);
	}

	void SubmitInputText(const std::wstring& text)
	{
		if (!s_inputCsReady)
			return;

		EnterCriticalSection(&s_inputCs);
		s_inputBuffer = text;
		LeaveCriticalSection(&s_inputCs);

		if (s_hInputDone)
			::SetEvent(s_hInputDone);
	}

	bool WaitAndTakeInputText(std::wstring& outText)
	{
		if (!s_hInputDone || !s_hStopEvent)
			return false;

		HANDLE waitHandles[2] = { s_hInputDone, s_hStopEvent };
		DWORD wait = ::WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
		if (wait != WAIT_OBJECT_0)
			return false; // stop request (or failure)

		if (s_inputCsReady)
		{
			EnterCriticalSection(&s_inputCs);
			outText = s_inputBuffer;
			LeaveCriticalSection(&s_inputCs);
		}
		return true;
	}

	void QueuePrintText(const std::wstring& text)
	{
		if (!s_printCsReady || text.empty())
			return;

		// Append to the shared buffer and remember whether *we* are the
		// one that flips s_bPrintPosted from false to true -- only that
		// caller actually posts a message. Any prints that arrive while a
		// flush is already pending just add to the buffer and return
		// immediately; they'll be picked up by the pending flush once the
		// UI thread gets around to it. This is what turns an arbitrarily
		// long burst of EGA_do_print calls (e.g. from a tight `for` loop)
		// into at most one message in flight at a time, instead of one
		// WM_EGA_DO_PRINT per call.
		bool needPost = false;
		EnterCriticalSection(&s_printCs);
		s_printBuffer += text;
		if (!s_bPrintPosted.exchange(true))
			needPost = true;
		LeaveCriticalSection(&s_printCs);

		if (!needPost)
			return;

		HWND hwnd = s_hwndEga;
		if (!::IsWindow(hwnd) || !::PostMessageW(hwnd, WM_EGA_DO_PRINT, 0, 0))
		{
			// Nobody to deliver to right now (dialog not up yet / already
			// closing). Clear the flag so the text isn't lost silently --
			// the next QueuePrintText() call (or a manual flush once the
			// window exists) will retry posting.
			s_bPrintPosted = false;
		}
	}

	bool TakePendingPrintText(std::wstring& outText)
	{
		outText.clear();

		if (!s_printCsReady)
			return false;

		EnterCriticalSection(&s_printCs);
		outText.swap(s_printBuffer);
		s_bPrintPosted = false;
		LeaveCriticalSection(&s_printCs);

		return !outText.empty();
	}
}
