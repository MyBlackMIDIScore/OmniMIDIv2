/*

	OmniMIDI v15+ (Rewrite) for Win32/Linux

	This file contains the required code to run the driver under both Windows and Linux

*/

#ifndef _M_ARM

#include "XSynthM.hpp"

void OmniMIDI::XSynth::XSynthThread() {
	while (!IsSynthInitialized())
		MiscFuncs.MicroSleep(-1);

	while (IsSynthInitialized()) {
		auto data = XSynth_Realtime_GetStats();

		RenderingTime = data.render_time * 100.0f;
		ActiveVoices = data.voice_count;

		MiscFuncs.MicroSleep(-10000);
	}
}

bool OmniMIDI::XSynth::IsSynthInitialized() {
	if (!XLib)
		return false;

	return XSynth_Realtime_IsActive();
}

bool OmniMIDI::XSynth::LoadSynthModule() {
	auto ptr = (LibImport*)xLibImp;

	if (!Settings) {
		Settings = new XSynthSettings(ErrLog);
		Settings->LoadSynthConfig();
	}

	if (!XLib)
		XLib = new Lib("xsynth", nullptr, ErrLog, &ptr, xLibImpLen);

	if (XLib->IsOnline())
		return true;

	if (!XLib->LoadLib())
		return false;

	return true;
}

bool OmniMIDI::XSynth::UnloadSynthModule() {
	if (!XLib)
		return true;

	if (!XLib->UnloadLib())
		return false;

	return true;
}

bool OmniMIDI::XSynth::StartSynthModule() {
	if (XSynth_Realtime_IsActive())
		return true;

	if (!Settings)
		return false;

	realtimeConf = XSynth_GenDefault_RealtimeConfig();

	realtimeConf.fade_out_killing = Settings->FadeOutKilling;
	realtimeConf.render_window_ms = Settings->RenderWindow;
	realtimeConf.use_threadpool = Settings->ThreadPool;

	XSynth_Realtime_Init(realtimeConf);
	XSynth_Realtime_SetLayerCount(Settings->LayerCount);
	realtimeParams = XSynth_Realtime_GetStreamParams();

	LoadSoundFonts();
	SFSystem.RegisterCallback(this);

	_XSyThread = std::jthread(&XSynth::XSynthThread, this);
	if (!_XSyThread.joinable()) {
		NERROR("_XSyThread failed. (ID: %x)", true, _XSyThread.get_id());
		return false;
	}

	if (Settings->IsDebugMode()) {
		_LogThread = std::jthread(&SynthModule::LogFunc, this);
		if (!_LogThread.joinable()) {
			NERROR("_LogThread failed. (ID: %x)", true, _LogThread.get_id());
			return false;
		}

		Settings->OpenConsole();
	}

	return true;
}

bool OmniMIDI::XSynth::StopSynthModule() {
	SFSystem.RegisterCallback();

	if (XSynth_Realtime_IsActive()) {
		XSynth_Soundfont_RemoveAll();
		XSynth_Realtime_Drop();
	}

	if (_XSyThread.joinable())
		_XSyThread.join();

	if (_LogThread.joinable())
		_LogThread.join();

	if (Settings->IsDebugMode() && Settings->IsOwnConsole())
		Settings->CloseConsole();

	return true;
}

void OmniMIDI::XSynth::LoadSoundFonts() {
	if (SoundFonts.size() > 0)
	{
		XSynth_Soundfont_RemoveAll();
		SoundFonts.clear();
	}

	if (SFSystem.ClearList()) {
		while (SoundFontsVector == nullptr)
			SoundFontsVector = SFSystem.LoadList();

		if (SoundFontsVector != nullptr) {
			auto& dSFv = *SoundFontsVector;

			if (dSFv.size() > 0) {
				for (int i = 0; i < dSFv.size(); i++) {
					if (!dSFv[i].enabled)
						continue;

					auto sf = XSynth_GenDefault_SoundfontOptions();

					sf.stream_params.audio_channels = realtimeParams.audio_channels;
					sf.stream_params.sample_rate = realtimeParams.sample_rate;
					sf.preset = dSFv[i].spreset;
					sf.bank = dSFv[i].sbank;
					sf.interpolator = INTERPOLATION_LINEAR;
					sf.use_effects = dSFv[i].minfx;
					sf.linear_release = dSFv[i].linattmod;

					SoundFonts.push_back(XSynth_Soundfont_LoadNew((const char*)dSFv[i].path.c_str(), sf));
				}
			}		
		}
	}

	if (SoundFonts.size() > 0)
		XSynth_Realtime_SetSoundfonts(&SoundFonts[0], SoundFonts.size());
}

void OmniMIDI::XSynth::PlayShortEvent(unsigned int ev) {
	PlayShortEvent(ev & 0xFF, (ev >> 8) & 0xFF, (ev >> 16) & 0xFF);
}

void OmniMIDI::XSynth::UPlayShortEvent(unsigned int ev) {
	UPlayShortEvent(ev & 0xFF, (ev >> 8) & 0xFF, (ev >> 16) & 0xFF);
}

void OmniMIDI::XSynth::PlayShortEvent(unsigned char status, unsigned char param1, unsigned char param2) {
	if (!XLib->IsOnline() && !XSynth_Realtime_IsActive())
		return;

	UPlayShortEvent(status, param1, param2);
}

void OmniMIDI::XSynth::UPlayShortEvent(unsigned char status, unsigned char param1, unsigned char param2) {
	uint16_t evt = XMIDI_EVENT_NOTEON;
	uint16_t ev = 0;

	switch (status & 0xF0) {
	case NoteOn:
		ev = param2 << 8 | param1;
		break;
	case NoteOff:
		evt = XMIDI_EVENT_NOTEOFF;
		ev = param1;
		break;
	case PatchChange:
		evt = XMIDI_EVENT_PROGRAMCHANGE;
		ev = param1;
		break;
	case CC:
		evt = XMIDI_EVENT_CONTROL;
		ev = param2 << 8 | param1;
		break;
	case PitchBend:
		evt = XMIDI_EVENT_PITCH;
		ev = param2 << 7 | param1;
		break;
	default:
		switch (status) {
		case SystemReset:
			evt = XMIDI_EVENT_RESETCONTROL;
			break;
		default:
			break;
		}
	}

	XSynth_Realtime_SendEvent(status & 0xF, evt, ev);
}

#endif