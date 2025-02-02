/*

	OmniMIDI v15+ (Rewrite) for Windows NT

	This file contains the required code to run the driver under Windows 7 SP1 and later.
	This file is useful only if you want to compile the driver under Windows, it's not needed for Linux/macOS porting.

*/

#ifndef _OFLUIDSYNTH_H
#define _OFLUIDSYNTH_H

#ifdef _WIN32
#include <Windows.h>
#endif

#include "inc/fluidsynth.h"
#include <thread>
#include <atomic>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <codecvt>
#include <locale>
#include <future>
#include "EvBuf_t.hpp"
#include "SynthMain.hpp"

#define FLUIDSYNTH_STR	"FluidSynth"

namespace OmniMIDI {
	class FluidSettings : public OMSettings {
	public:
		// Global settings
		unsigned int EvBufSize = 32768;
		unsigned int PeriodSize = 64;
		unsigned int Periods = 2;
		unsigned int ThreadsCount = 1;
		unsigned int MinimumNoteLength = 10;
		bool ExperimentalMultiThreaded = false;
		double OverflowVolume = 10000.0;
		double OverflowPercussion = 10000.0;
		double OverflowReleased = -10000.0;
		double OverflowImportant = 0.0;
		std::string Driver = "wasapi";
		std::string SampleFormat = "float";

		FluidSettings(ErrorSystem::Logger* PErr) : OMSettings(PErr) { }

		void RewriteSynthConfig() {
			nlohmann::json DefConfig = {
					ConfGetVal(SampleRate),
					ConfGetVal(EvBufSize),
					ConfGetVal(VoiceLimit),
					ConfGetVal(PeriodSize),
					ConfGetVal(Periods),
					ConfGetVal(ThreadsCount),
					ConfGetVal(MinimumNoteLength),
					ConfGetVal(OverflowVolume),
					ConfGetVal(OverflowPercussion),
					ConfGetVal(OverflowReleased),
					ConfGetVal(OverflowImportant),
					ConfGetVal(Driver),
					ConfGetVal(SampleFormat),
					ConfGetVal(ExperimentalMultiThreaded)
			};

			if (AppendToConfig(DefConfig))
				WriteConfig();

			CloseConfig();
			InitConfig(false, FLUIDSYNTH_STR, sizeof(FLUIDSYNTH_STR));
		}

		// Here you can load your own JSON, it will be tied to ChangeSetting()
		void LoadSynthConfig() {
			if (InitConfig(false, FLUIDSYNTH_STR)) {
				SynthSetVal(unsigned int, SampleRate);
				SynthSetVal(unsigned int, EvBufSize);
				SynthSetVal(unsigned int, VoiceLimit);
				SynthSetVal(unsigned int, PeriodSize);
				SynthSetVal(unsigned int, Periods);
				SynthSetVal(unsigned int, ThreadsCount);
				SynthSetVal(unsigned int, MinimumNoteLength);
				SynthSetVal(bool, ExperimentalMultiThreaded);
				SynthSetVal(double, OverflowVolume);
				SynthSetVal(double, OverflowPercussion);
				SynthSetVal(double, OverflowReleased);
				SynthSetVal(double, OverflowImportant);
				SynthSetVal(std::string, Driver);
				SynthSetVal(std::string, SampleFormat);
				return;
			}

			if (IsConfigOpen() && !IsSynthConfigValid()) {
				RewriteSynthConfig();
			}
		}
	};

	class FluidSynth : public SynthModule {
	private:
		OMShared::Funcs MiscFuncs;

		Lib* FluiLib = nullptr;

		LibImport fLibImp[25] = {
			// BASS
			ImpFunc(new_fluid_synth),
			ImpFunc(new_fluid_settings),
			ImpFunc(delete_fluid_synth),
			ImpFunc(delete_fluid_settings),
			ImpFunc(fluid_synth_all_notes_off),
			ImpFunc(fluid_synth_all_sounds_off),
			ImpFunc(fluid_synth_noteoff),
			ImpFunc(fluid_synth_noteon),
			ImpFunc(fluid_synth_cc),
			ImpFunc(fluid_synth_channel_pressure),
			ImpFunc(fluid_synth_key_pressure),
			ImpFunc(fluid_synth_pitch_bend),
			ImpFunc(fluid_synth_pitch_wheel_sens),
			ImpFunc(fluid_synth_program_change),
			ImpFunc(fluid_synth_program_reset),
			ImpFunc(fluid_synth_program_select),
			ImpFunc(fluid_synth_sysex),
			ImpFunc(fluid_synth_system_reset),
			ImpFunc(fluid_synth_sfunload),
			ImpFunc(fluid_synth_sfload),
			ImpFunc(fluid_settings_setint),
			ImpFunc(fluid_settings_setnum),
			ImpFunc(fluid_settings_setstr),
			ImpFunc(new_fluid_audio_driver),
			ImpFunc(delete_fluid_audio_driver)
		};
		size_t fLibImpLen = sizeof(fLibImp) / sizeof(fLibImp[0]);

		SoundFontSystem SFSystem;
		FluidSettings* Settings = nullptr;
		fluid_settings_t* fSet = nullptr;

		std::vector<int> SoundFontIDs;
		fluid_synth_t** AudioStreams = new fluid_synth_t*[16] {0};
		fluid_audio_driver_t** AudioDrivers = new fluid_audio_driver_t*[16] {0};
		size_t AudioStreamSize = 16;

		std::vector<int> SoundFonts;

		void EventsThread();
		bool ProcessEvBuf();

	public:
		FluidSynth(ErrorSystem::Logger* PErr) : SynthModule(PErr) {}
		bool LoadSynthModule() override;
		bool UnloadSynthModule() override;
		bool StartSynthModule() override;
		bool StopSynthModule() override;
		void LoadSoundFonts() override;
		bool SettingsManager(unsigned int setting, bool get, void* var, size_t size) override { return false; }
		unsigned int GetSampleRate() override  { return Settings->SampleRate; }
		bool IsSynthInitialized() override { return (AudioDrivers[0] != nullptr); }
		int SynthID() override { return 0x6F704EC6; }

		unsigned int PlayLongEvent(char* ev, unsigned int size) override;
		unsigned int UPlayLongEvent(char* ev, unsigned int size) override;

		// Not supported in FluidSynth
		SynthResult TalkToSynthDirectly(unsigned int evt, unsigned int chan, unsigned int param) override { return Ok; }
	};
}

#endif