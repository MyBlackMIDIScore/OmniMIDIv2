/*

	OmniMIDI v15+ (Rewrite) for Windows NT

	This file contains the required code to run the driver under Windows 7 SP1 and later.
	This file is useful only if you want to compile the driver under Windows, it's not needed for Linux/macOS porting.

*/

#ifdef _WIN32

#include "StreamPlayer.hpp"

OmniMIDI::CookedPlayer::CookedPlayer(OmniMIDI::SynthModule* sptr, WinDriver::DriverCallback* dptr, ErrorSystem::Logger* perr) : StreamPlayer(sptr, dptr, perr) {
	synthModule = sptr;
	drvCallback = dptr;
	ErrLog = perr;
	
	_CooThread = std::jthread(&CookedPlayer::PlayerThread, this);
	if (!_CooThread.joinable()) {
		CookedPlayer::~CookedPlayer();
	}
}

OmniMIDI::CookedPlayer::~CookedPlayer() {
	goToBed = true;

	if (_CooThread.joinable())
		_CooThread.join();
}

void OmniMIDI::CookedPlayer::PlayerThread() {
	bool noMoreDelta = false;
	uint32_t deltaMicroseconds = 0;

	Message("PlayerThread is ready.");
	while (!goToBed) {
		while (paused || !mhdrQueue)
		{
			Utils.MicroSleep(SLEEPVAL(1));
			if (goToBed) break;
		}

		if (goToBed) break;

		MIDIHDR* hdr = mhdrQueue;
		if (hdr->dwFlags & MHDR_DONE)
		{
			Message("Moving onto next packet... %x >>> %x", mhdrQueue, hdr->lpNext);
			mhdrQueue = hdr->lpNext;
			continue;
		}

		while (!paused) {
			if (hdr->dwOffset >= hdr->dwBytesRecorded)
			{
				hdr->dwFlags |= MHDR_DONE;
				hdr->dwFlags &= ~MHDR_INQUEUE;
				MIDIHDR* nexthdr = hdr->lpNext;

				mhdrQueue = nexthdr;
				drvCallback->CallbackFunction(MOM_DONE, (DWORD_PTR)hdr, 0);

				hdr->dwOffset = 0;
				hdr = nexthdr;
				
				break;
			}

			MIDIEVENT* event = (MIDIEVENT*)(hdr->lpData + hdr->dwOffset);

			uint32_t deltaTicks = event->dwDeltaTime;
			tickAcc += deltaTicks;

			if (event->dwEvent & MEVT_F_CALLBACK) {
				drvCallback->CallbackFunction(MOM_POSITIONCB, (DWORD_PTR)hdr, 0);
				Message("MEVT_F_CALLBACK called! (MOM_POSITIONCB, ready to process addr: %x)", hdr);
			}

			if (!smpte && !noMoreDelta && event->dwDeltaTime) {
				deltaMicroseconds = (tempo * deltaTicks / ticksPerQN);
				timeAcc += deltaMicroseconds;

				Utils.MicroSleep(SLEEPVAL(((signed long)deltaMicroseconds) * 10));
				noMoreDelta = true;
				
				break;
			}
			else if (smpte) {
				uint32_t fpsToFrlen = (uint32_t)((1000000 / smpteFramerate) / smpteFrameTicks);
				timeAcc += fpsToFrlen;
				Utils.MicroSleep(SLEEPVAL(fpsToFrlen));
			}

			noMoreDelta = false;

			switch (MEVT_EVENTTYPE(event->dwEvent)) {
			case MEVT_SHORTMSG:
				synthModule->PlayShortEvent((event->dwEvent >> 16) & 0xFF, (event->dwEvent >> 8) & 0xFF, event->dwEvent & 0xFF);
				break;
			case MEVT_LONGMSG:
				synthModule->PlayLongEvent((uint8_t*)event->dwParms, event->dwEvent & 0xFFFFFF);
				break;
			case MEVT_TEMPO:
				if (!smpte)
					SetTempo(event->dwEvent & 0xFFFFFF);

				break;
			default:
				break;
			}

			if (event->dwEvent & MEVT_F_LONG)
			{
				DWORD accum = ((event->dwEvent & 0xFFFFFF) + 3) & ~3;
				byteAcc += accum;
				hdr->dwOffset += accum;
			}

			byteAcc += 0xC;
			hdr->dwOffset += 0xC;
		}
	}

	// LOG(StrmErr, "timeAcc: %d - tickAcc: %d - byteAcc %x", timeAcc, tickAcc, byteAcc);
}

void OmniMIDI::CookedPlayer::SetTempo(uint32_t ntempo) {
	tempo = ntempo;
	bpm = 60000000 / tempo;
	Message("Received new tempo. (tempo: %d, ticksPerQN : %d, BPM: %d)", tempo, ticksPerQN, bpm);
}

void OmniMIDI::CookedPlayer::SetTicksPerQN(uint16_t nTicksPerQN) {
	char smptePortion = (nTicksPerQN >> 8) & 0xFF;

	bool qSmpte = smptePortion & 0x80;
	char qSmpteFramerate = smptePortion & 0x7F;
	char qSmpteFrameTicks = nTicksPerQN & 0xFF;

	if (qSmpte) {
		smpte = true;
		smpteFramerate = smptePortion;
		smpteFrameTicks = qSmpteFrameTicks;
		Message("Received new SMPTE setting. (framerate: %dFPS, %d ticks per frame)", smpteFramerate, qSmpteFrameTicks);
		return;
	}

	smpte = false;
	ticksPerQN = nTicksPerQN;
	Message("Received new TPQ. (tempo: %d, ticksPerQN : %d, BPM: %d)", tempo, ticksPerQN, bpm);
}

bool OmniMIDI::CookedPlayer::AddToQueue(MIDIHDR* mhdr) {
	MIDIHDR* pmhdrQueue = mhdrQueue;

	if (!mhdrQueue) {
		mhdrQueue = mhdr;
	}
	else {
		if (pmhdrQueue == mhdr) {
			return false;
		}

		while (pmhdrQueue->lpNext != nullptr)
		{
			pmhdrQueue = pmhdrQueue->lpNext;
			if (pmhdrQueue == mhdr)
				return false;
		}

		pmhdrQueue->lpNext = mhdr;
	}

	return true;
}

bool OmniMIDI::CookedPlayer::ResetQueue() {
	for (MIDIHDR* hdr = mhdrQueue; hdr; hdr = hdr->lpNext)
	{
		hdr->dwFlags &= ~MHDR_INQUEUE;
		hdr->dwFlags |= MHDR_DONE;
		drvCallback->CallbackFunction(MOM_DONE, (DWORD_PTR)hdr, 0);
	}

	return true;
}

bool OmniMIDI::CookedPlayer::EmptyQueue() {
	if (IsQueueEmpty())
		return false;

	paused = true;

	for (MIDIHDR* hdr = mhdrQueue; hdr; hdr = hdr->lpNext)
	{
		hdr->dwFlags &= ~MHDR_INQUEUE;
		hdr->dwFlags |= MHDR_DONE;
	}

	tickAcc = 0;
	timeAcc = 0;
	byteAcc = 0;
	smpteFramerate = 30;
	smpteFrameTicks = 4;
	tempo = 500000;
	ticksPerQN = 0x18;

	return true;
}

void OmniMIDI::CookedPlayer::GetPosition(MMTIME* mmtime) {
	signed long totalSeconds = timeAcc / 1000000;
	uint32_t playedSamples = (uint32_t)(timeAcc / (1000000 / synthModule->GetSampleRate()));

	mmtime->u.cb = byteAcc;

	mmtime->u.ms = timeAcc / 10000;

	mmtime->u.smpte.fps = smpteFramerate;
	mmtime->u.smpte.frame = (uint8_t)(totalSeconds / smpteFrameTicks) % smpteFramerate;
	mmtime->u.smpte.sec = totalSeconds % 60;
	mmtime->u.smpte.min = (totalSeconds / 60) % 60;
	mmtime->u.smpte.hour = (uint8_t)(totalSeconds / 3600);

	mmtime->u.sample = playedSamples;

	mmtime->u.midi.songptrpos = (tickAcc + ticksPerQN / 8) / (ticksPerQN / 4);

	mmtime->u.ticks = tickAcc;
}

#endif