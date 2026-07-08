// EgaBridge.cpp --- Bridge for EGA Programming Language integration
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2020 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#include "EgaBridge.hpp"
#include <windows.h>

#include "../EGA/ega.hpp"

using namespace EGA;

namespace
{
	static CRITICAL_SECTION s_cs;
	static bool     s_bCsReady    = false;
	static HANDLE   s_hThread     = NULL;
	static HANDLE   s_hStopEvent  = NULL;   // manual-reset
	static bool     s_bRunning    = false;
	static bool     s_bInitialized = false;
}

static DWORD WINAPI EgaBridgeThreadProc(LPVOID args)
{
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

	return 0;
}

namespace EgaBridge
{
	bool Initialize()
	{
		if (s_bInitialized)
			return true;

		InitializeCriticalSection(&s_cs);
		s_hStopEvent = ::CreateEventW(NULL, TRUE, FALSE, NULL); // Manual-reset
		if (!s_hStopEvent)
		{
			DeleteCriticalSection(&s_cs);
			return false;
		}

		s_bCsReady = true;

		if (!EGA_init())
		{
			s_bCsReady = false;
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

		::SetEvent(s_hStopEvent); // Stop now!
		hThread = s_hThread;
		LeaveCriticalSection(&s_cs);

		if (hThread)
		{
			if (::WaitForSingleObject(hThread, 3000) == WAIT_TIMEOUT)
			{
				::TerminateThread(hThread, 1);
			}
			::CloseHandle(hThread);
			s_hThread = NULL;
		}
	}

	bool IsStopRequested()
	{
		return s_hStopEvent != NULL &&
		       ::WaitForSingleObject(s_hStopEvent, 0) == WAIT_OBJECT_0;
	}
}
