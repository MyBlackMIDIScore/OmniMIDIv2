/*

	OmniMIDI v15+ (Rewrite) for Windows NT

	This file contains the required code to run the driver under Windows 7 SP1 and later.
	This file is useful only if you want to compile the driver under Windows, it's not needed for Linux/macOS porting.

*/

#ifdef _WIN32

#ifndef _WDMDRV_H
#define _WDMDRV_H

#pragma once

#define MIDI_IO_PACKED	0x00000000L			// Legacy mode, used by most MIDI apps
#define MIDI_IO_COOKED	0x00000002L			// Stream mode, used by some old MIDI apps (Such as GZDoom)

// Always first
#include "Common.hpp"

// The rest
#include <math.h>
#include <mmeapi.h>
#include <assert.h>
#include "cmmddk.h"
#include "ErrSys.hpp"
#include "Utils.hpp"

using namespace OMShared;

namespace WinDriver {
	typedef void(CALLBACK* midiOutProc)(HMIDIOUT hmOut, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR Param1, DWORD_PTR Param2);
	typedef LRESULT(WINAPI* drvDefDriverProc)(DWORD dwDI, HDRVR hdrvr, UINT uMsg, LPARAM lP1, LPARAM lP2);

	struct Callback {
		HMIDIOUT Handle = nullptr;
		DWORD Mode = 0;
		midiOutProc funcPtr = nullptr;
		DWORD_PTR Instance = 0;
	};

	class DriverMask {
	private:
		unsigned short ManufacturerID = 0xFFFF;
		unsigned short ProductID = 0xFFFF;
		unsigned short Technology = MOD_SWSYNTH;
		unsigned short Support = MIDICAPS_VOLUME | MIDICAPS_STREAM;

		ErrorSystem::Logger* ErrLog = nullptr;

	public:
		// Change settings
		DriverMask(ErrorSystem::Logger* PErr) { ErrLog = PErr; }
		bool ChangeSettings(short, short, short, short);
		unsigned long GiveCaps(UINT, PVOID, DWORD);
	};

	class DriverCallback {

	private:
		Callback* pCallback = nullptr;
		drvDefDriverProc pDrvProc = nullptr;
		ErrorSystem::Logger* ErrLog;

	public:
		// Callbacks
		DriverCallback(ErrorSystem::Logger* PErr);
		bool IsCallbackReady();
		bool PrepareCallbackFunction(MIDIOPENDESC*, DWORD);
		bool ClearCallbackFunction();
		void CallbackFunction(DWORD, DWORD_PTR, DWORD_PTR);
		LRESULT ProcessMessage(DWORD_PTR, HDRVR, UINT, LPARAM, LPARAM);
	};

	class DriverComponent {

	private:
		HDRVR DrvHandle = nullptr;
		HMODULE LibHandle = nullptr;

		ErrorSystem::Logger* ErrLog;

	public:
		DriverComponent(ErrorSystem::Logger* PErr) { ErrLog = PErr; }

		// Opening and closing the driver
		bool SetDriverHandle(HDRVR drv = nullptr);

		// Loading and unloading the library
		bool SetLibraryHandle(HMODULE mod = nullptr);

		// Setting the driver's pointer for the app
		bool OpenDriver(MIDIOPENDESC*, DWORD, DWORD_PTR);
		bool CloseDriver();
	};

	class Blacklist {
	private:

	};
}

#endif
#endif