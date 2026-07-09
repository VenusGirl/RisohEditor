// EgaBridge.cpp --- Bridge for EGA Programming Language integration
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2020 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#include "EgaBridge.hpp"
#include <windows.h>
#include <queue>

#include "../EGA/ega.hpp"

using namespace EGA;

extern HWND s_hwndEga;

namespace
{
	static CRITICAL_SECTION s_cs;
	static volatile bool     s_bCsReady    = false;
	static HANDLE   s_hThread     = NULL;
	static HANDLE   s_hStopEvent  = NULL;   // manual-reset
	static volatile bool     s_bRunning    = false;
	static volatile bool     s_bInitialized = false;
	static CRITICAL_SECTION s_fileCs;
	static volatile bool             s_fileCsReady = false;
	static std::queue<std::string> s_fileQueue;
	static std::function<void(void*)> s_uiTask;
	static CRITICAL_SECTION s_uiCs;
	static HANDLE s_hUIDone = NULL;
	static void* s_uiParam = nullptr;
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
		s_fileCsReady = true;

		s_hStopEvent = ::CreateEventW(NULL, TRUE, FALSE, NULL); // Manual-reset
		s_hUIDone    = ::CreateEventW(NULL, FALSE, FALSE, NULL); // auto-reset
		if (!s_hStopEvent)
		{
			if (s_hUIDone) { ::CloseHandle(s_hUIDone); s_hUIDone = NULL; }
			if (s_hStopEvent) { ::CloseHandle(s_hStopEvent); s_hStopEvent = NULL; }
			DeleteCriticalSection(&s_fileCs);
			DeleteCriticalSection(&s_cs);
			s_fileCsReady = false;
			DeleteCriticalSection(&s_cs);
			return false;
		}

		s_bCsReady = true;

		if (!EGA_init())
		{
			s_bCsReady = false;
			::CloseHandle(s_hUIDone); s_hUIDone = NULL;
			::CloseHandle(s_hStopEvent); s_hStopEvent = NULL;
			DeleteCriticalSection(&s_uiCs);
			DeleteCriticalSection(&s_fileCs);
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

		StopInteractive();
		EGA_uninit();
		s_bInitialized = false;

		if (s_hStopEvent) { ::CloseHandle(s_hStopEvent); s_hStopEvent = NULL; }
		if (s_bCsReady)   { DeleteCriticalSection(&s_cs); s_bCsReady = false; }
		if (s_fileCsReady) { DeleteCriticalSection(&s_fileCs); s_fileCsReady = false; }
		if (s_hUIDone) { ::CloseHandle(s_hUIDone); s_hUIDone = nullptr; }
		::DeleteCriticalSection(&s_uiCs);
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
		if (s_bRunning)
		{
			LeaveCriticalSection(&s_cs);
			return true;
		}

		::ResetEvent(s_hStopEvent);
		s_bRunning = true;

		HANDLE hThread = ::CreateThread(NULL, 0, EgaBridgeThreadProc, NULL, 0, NULL);
		if (!hThread)
		{
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

	void StopInteractive()
	{
		HANDLE hThread = NULL;

		EnterCriticalSection(&s_cs);
		if (!s_bRunning)
		{
			LeaveCriticalSection(&s_cs);
			return;
		}

		EGA_stop();
		::SetEvent(s_hStopEvent); // Stop now!
		hThread = s_hThread;
		LeaveCriticalSection(&s_cs);

		if (hThread)
		{
			if (::WaitForSingleObject(hThread, 10 * 1000) == WAIT_TIMEOUT)
			{
				::TerminateThread(hThread, 1);
			}
			::CloseHandle(hThread);
			s_hThread = NULL;
		}
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
		if (IsStopRequested())
			return false;

		EnterCriticalSection(&s_uiCs);
		s_uiTask = fn;
		s_uiParam = param;
		LeaveCriticalSection(&s_uiCs);

		::ResetEvent(s_hUIDone);

		if (!::IsWindow(s_hwndEga))
			return false;

		::PostMessageW(s_hwndEga, WM_EGA_DO_RUN_ON_UI, 0, (LPARAM)param);

		HANDLE waitHandles[2] = { s_hUIDone, s_hStopEvent };
		DWORD wait = ::WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
		return wait == WAIT_OBJECT_0;
	}

	void ExecuteUITask(void* param)
	{
		std::function<void(void*)> task;
		EnterCriticalSection(&s_uiCs);
		task = s_uiTask;
		LeaveCriticalSection(&s_uiCs);

		if (task)
			task(param);

		::SetEvent(s_hUIDone);
	}
}
