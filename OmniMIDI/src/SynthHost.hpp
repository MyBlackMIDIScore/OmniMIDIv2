/*

	OmniMIDI v15+ (Rewrite) for Win32/Linux

	This file contains the required code to run the driver under both Windows and Linux

*/

#ifndef _SYNTHHOST_H
#define _SYNTHHOST_H

#include "ErrSys.hpp"
#include "HostSettings.hpp"

#ifdef _WIN32
// Cooked player, let it cook...
#include "StreamPlayer.hpp"
#endif

// Synthesizers
#include "XSynthM.hpp"
#include "BASSSynth.hpp"
#include "FluidSynth.hpp"
#include "ShakraPipe.hpp"
#include "PluginSynth.hpp"

typedef OmniMIDI::SynthModule* (*rInitModule)();
typedef void(*rStopModule)();

namespace OmniMIDI {
	class SynthHost
	{
	private:
		// Ours
		OMShared::Funcs Utils;
		ErrorSystem::Logger* ErrLog = nullptr;
		SynthModule* Synth = nullptr;
		
		std::jthread _HealthThread;
		HostSettings* _SHSettings = nullptr;
		std::mutex _hostMutex;

#ifdef _WIN32
		// From driver lib
		HMODULE hwndMod = nullptr;
		StreamPlayer* StreamPlayer = nullptr;
		WinDriver::DriverCallback* DrvCallback = nullptr;
#endif

	public:
		SynthHost(ErrorSystem::Logger* PErr);
#ifdef _WIN32
		SynthHost(WinDriver::DriverCallback* dcasrc, HMODULE mod, ErrorSystem::Logger* PErr);
#endif
		~SynthHost();

		void RefreshSettings();
		bool Start(bool StreamPlayer = false);
		bool Stop(bool restart = false);
		OmniMIDI::SynthModule* GetSynth();
		bool IsKDMAPIAvailable() { return _SHSettings->IsKDMAPIEnabled(); }

#ifdef _WIN32 
		bool IsStreamPlayerAvailable() { return !StreamPlayer->IsDummy(); }
		bool IsBlacklistedProcess() { return _SHSettings->IsBlacklistedProcess(); }
		
		// Cooked player system
		bool SpInit(SynthModule* synthModule = nullptr);
		bool SpFree();
		void SpStart() { StreamPlayer->Start(); }
		void SpStop() { StreamPlayer->Stop(); }
		bool SpEmptyQueue() { return StreamPlayer->EmptyQueue(); }
		bool SpResetQueue() { return StreamPlayer->ResetQueue(); }
		bool SpAddToQueue(MIDIHDR* mhdr) { return StreamPlayer->AddToQueue(mhdr); }
		void SpSetTempo(uint32_t ntempo) { StreamPlayer->SetTempo(ntempo); }
		uint32_t SpGetTempo() { return StreamPlayer->GetTempo(); }
		void SpSetTicksPerQN(uint32_t nTimeDiv) { StreamPlayer->SetTicksPerQN(nTimeDiv); }
		uint32_t SpGetTicksPerQN() { return StreamPlayer->GetTicksPerQN(); }
		void SpGetPosition(MMTIME* mmtime) { StreamPlayer->GetPosition(mmtime); }
#endif

		void HostHealthCheck();

		// Event handling system
		void PlayShortEvent(uint32_t ev);
		void PlayShortEvent(uint8_t status, uint8_t param1, uint8_t param2);
		float GetRenderingTime();
		uint64_t GetActiveVoices();
		SynthResult PlayLongEvent(char* ev, uint32_t size);
		SynthResult Reset() { return Synth->Reset(); }
		SynthResult TalkToSynthDirectly(uint32_t evt, uint32_t chan, uint32_t param) { return Synth->TalkToSynthDirectly(evt, chan, param); }
		bool SettingsManager(uint32_t setting, bool get, void* var, size_t size) { return Synth->SettingsManager(setting, get, var, size); }
		bool IsSynthInitialized() { return Synth->IsSynthInitialized(); }
	};
}

#endif