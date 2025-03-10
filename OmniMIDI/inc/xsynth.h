/*

	OmniMIDI v15+ (Rewrite) for Windows NT

	This file contains the required code to run the driver under Windows 7 SP1 and later.
	This file is useful only if you want to compile the driver under Windows, it's not needed for Linux/macOS porting.

*/

#ifndef _XSYNTH_H
#define _XSYNTH_H

/* The XSynth library is licensed under the LGPL 3.0. */

#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#ifdef WIN32
#include <windows.h>
#define XSYNTH_IMP WINAPI
#else
#define XSYNTH_IMP
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define XSYNTH_VERSION 0x100

#define XMIDI_EVENT_NOTEON 0

#define XMIDI_EVENT_NOTEOFF 1

#define XMIDI_EVENT_ALLNOTESOFF 2

#define XMIDI_EVENT_ALLNOTESKILLED 3

#define XMIDI_EVENT_RESETCONTROL 4

#define XMIDI_EVENT_CONTROL 5

#define XMIDI_EVENT_PROGRAMCHANGE 6

#define XMIDI_EVENT_PITCH 7

#define XMIDI_EVENT_FINETUNE 8

#define XMIDI_EVENT_COARSETUNE 9

#define INTERPOLATION_NEAREST 100

#define INTERPOLATION_LINEAR 101

#define AUDIO_CHANNELS_MONO 1

#define AUDIO_CHANNELS_STEREO 2

/**
 * Parameters of the output audio
 * - sample_rate: Audio sample rate
 * - audio_channels: Number of audio channels
 *         Supported: AUDIO_CHANNELS_MONO (mono), AUDIO_CHANNELS_STEREO (stereo)
 */
typedef struct XSynth_StreamParams {
	uint32_t sample_rate;
	uint16_t audio_channels;
} XSynth_StreamParams;

/**
 * Options for initializing a ChannelGroup
 * - stream_params: Output parameters (see XSynth_StreamParams)
 * - channels: Number of MIDI channels
 * - drum_channels: Array with the IDs of channels that should only be used for drums
 * - drum_channels_count: Length of the above array
 * - use_threadpool: Whether or not to use XSynth's threadpool feature
 * - fade_out_killing: Whether of not to fade out notes when killed because of the voice limit
 */
typedef struct XSynth_GroupOptions {
	struct XSynth_StreamParams stream_params;
	uint32_t channels;
	const uint32_t* drum_channels;
	uint32_t drum_channels_count;
	bool use_threadpool;
	bool fade_out_killing;
} XSynth_GroupOptions;

/**
 * Options for initializing the XSynth Realtime module
 * - channels: Number of MIDI channels
 * - drum_channels: Array with the IDs of channels that should only be used for drums
 * - drum_channels_count: Length of the above array
 * - use_threadpool: Whether or not to use XSynth's threadpool feature
 * - fade_out_killing: Whether of not to fade out notes when killed because of the voice limit
 * - render_window_ms: The length of the buffer reader in ms
 * - ignore_range: A range of velocities that will not be played
 *         LOBYTE = start (0-127), HIBYTE = end (start-127)
 */
typedef struct XSynth_RealtimeConfig {
	uint32_t channels;
	const uint32_t* drum_channels;
	uint32_t drum_channels_count;
	bool use_threadpool;
	bool fade_out_killing;
	double render_window_ms;
	uint16_t ignore_range;
} XSynth_RealtimeConfig;

/**
 * A struct that holds all the statistics the realtime module can
 * provide.
 * - voice_count: The amount of active voices
 * - buffer: Number of samples requested in the last read
 * - render_time: Percentage of the renderer load
 */
typedef struct XSynth_RealtimeStats {
	uint64_t voice_count;
	int64_t buffer;
	double render_time;
} XSynth_RealtimeStats;

/**
 * Options for loading a new XSynth sample soundfont.
 * - stream_params: Output parameters (see XSynth_StreamParams)
 * - bank: The bank number (0-128) to extract and use from the soundfont
 *         A value of -1 means to use all available banks (bank 0 for SFZ)
 * - preset: The preset number (0-127) to extract and use from the soundfont
 *         A value of -1 means to use all available presets (preset 0 for SFZ)
 * - linear_release: Whether or not to use a linear release envelope
 * - use_effects: Whether or not to apply audio effects to the soundfont. Currently
 *         only affecting the use of the low pass filter. Setting to false may
 *         improve performance slightly.
 * - interpolator: The type of interpolator to use for the new soundfont
 *         Available values: INTERPOLATION_NEAREST (Nearest Neighbor interpolation),
 *         INTERPOLATION_LINEAR (Linear interpolation)
 */
typedef struct XSynth_SoundfontOptions {
	struct XSynth_StreamParams stream_params;
	int16_t bank;
	int16_t preset;
	bool linear_release;
	bool use_effects;
	uint16_t interpolator;
} XSynth_SoundfontOptions;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/**
	 * Returns the version of XSynth
	 *
	 * --Returns--
	 * The XSynth version. For example, 0x010102 (hex), would be version 1.1.2
	 */
	extern uint32_t(XSYNTH_IMP* XSynth_GetVersion)(void);

	/**
	 * Generates the default values for the XSynth_StreamParams struct
	 * Default values are:
	 * - sample_rate = 44.1kHz
	 * - audio_channels = AUDIO_CHANNELS_STEREO
	 */
	extern struct XSynth_StreamParams(*XSynth_GenDefault_StreamParams)(void);

	/**
	 * Generates the default values for the XSynth_RealtimeConfig struct
	 * Default values are:
	 * - channels: 16
	 * - drum_channels: [9] (MIDI channel 10)
	 * - drum_channels_count: 1
	 * - use_threadpool: False
	 * - fade_out_killing: False
	 * - render_window_ms: 10.0ms
	 * - ignore_range: 0->0 (Nothing ignored)
	 */
	extern struct XSynth_RealtimeConfig(*XSynth_GenDefault_RealtimeConfig)(void);

	/**
	 * Initializes the XSynth Realtime module with the given configuration.
	 *
	 * --Parameters--
	 * - config: The initialization configuration (XSynth_RealtimeConfig struct)
	 */
	extern void(XSYNTH_IMP* XSynth_Realtime_Init)(struct XSynth_RealtimeConfig config);

	/**
	 * Sends a MIDI event to the realtime module.
	 *
	 * --Parameters--
	 * - channel: The number of the MIDI channel to send the event to (MIDI channel 1 is 0)
	 * - event: The type of MIDI event sent (see XSynth_ChannelGroup_SendEvent for available options)
	 * - params: Parameters for the event
	 */
	extern void(XSYNTH_IMP* XSynth_Realtime_SendEvent)(uint32_t channel, uint16_t event, uint16_t params);

	/**
	 * Checks if the XSynth Realtime module is loaded.
	 *
	 * --Returns--
	 * True if it is loaded, false if it is not.
	 */
	extern bool(XSYNTH_IMP* XSynth_Realtime_IsActive)(void);

	/**
	 * Returns the audio stream parameters of the realtime module as an
	 * XSynth_StreamParams struct. This may be useful when loading a new
	 * soundfont which is meant to be used here.
	 *
	 * --Returns--
	 * This function returns an XSynth_StreamParams struct.
	 *
	 * --Errors--
	 * This function will panic if the realtime module is not loaded.
	 */
	extern struct XSynth_StreamParams(XSYNTH_IMP* XSynth_Realtime_GetStreamParams)(void);

	/**
	 * Returns the statistics of the realtime module as an
	 * XSynth_RealtimeStats struct.
	 *
	 * --Returns--
	 * This function returns an XSynth_RealtimeStats struct.
	 *
	 * --Errors--
	 * This function will panic if the realtime module is not loaded.
	 */
	extern struct XSynth_RealtimeStats(XSYNTH_IMP* XSynth_Realtime_GetStats)(void);

	/**
	 * Sets the desired layer limit on the realtime module. One layer
	 * corresponds to one voice per key per channel.
	 *
	 * --Parameters--
	 * - layers: The layer limit (0 = no limit, 1-MAX = limit)
	 *         Where MAX is the maximum value of an unsigned 64bit integer
	 */
	extern void(XSYNTH_IMP* XSynth_Realtime_SetLayerCount)(uint64_t layers);

	/**
	 * Sets a list of soundfonts to be used in the realtime module. To load
	 * a new soundfont, see the XSynth_Soundfont_LoadNew function.
	 *
	 * --Parameters--
	 * - sf_ids: Pointer to an array of soundfont IDs
	 * - count: The length of the above array
	 *
	 * --Errors--
	 * This function will panic if any of the given soundfont IDs is invalid.
	 */
	extern void(XSYNTH_IMP* XSynth_Realtime_SetSoundfonts)(const uint64_t* sf_ids, uint64_t count);

	/**
	 * Resets the realtime module. Kills all active notes and resets
	 * all control change.
	 */
	extern void(XSYNTH_IMP* XSynth_Realtime_Reset)(void);

	/**
	 * Terminates the instance of the realtime module.
	 */
	extern void(XSYNTH_IMP* XSynth_Realtime_Drop)(void);

	/**
	 * Generates the default values for the XSynth_SoundfontOptions struct
	 * Default values are:
	 * - stream_params: Defaults for the XSynth_StreamParams struct
	 * - bank: -1
	 * - preset: -1
	 * - linear_release: False
	 * - use_effects: True
	 * - interpolator: INTERPOLATION_NEAREST
	 */
	extern struct XSynth_SoundfontOptions(XSYNTH_IMP* XSynth_GenDefault_SoundfontOptions)(void);

	/**
	 * Loads a new XSynth sample soundfont in memory.
	 *
	 * --Parameters--
	 * - path: The path of the soundfont to be loaded
	 * - options: The soundfont initialization options
	 *         (XSynth_SoundfontOptions struct)
	 *
	 * --Returns--
	 * This function returns the ID of the loaded soundfont, which can be used
	 * to send it to a channel group.
	 *
	 * --Errors--
	 * This function will error if XSynth has trouble parsing the soundfont or
	 * if the maximum number of active groups is reached.
	 * Max: 65.535 soundfonts.
	 */
	extern uint64_t(XSYNTH_IMP* XSynth_Soundfont_LoadNew)(const char* path, struct XSynth_SoundfontOptions options);

	/**
	 * Removes the desired soundfont from the ID list.
	 *
	 * Keep in mind that this does not clear the memory the soundfont is
	 * using. To free the used memory the soundfont has to be unloaded/
	 * replaced in the channel groups where it was sent. The function
	 * XSynth_ChannelGroup_ClearSoundfonts can be used for this purpose.
	 *
	 * To completely free the memory a soundfont is using it has to both
	 * be removed from the ID list and also from any channel groups using it.
	 *
	 * --Parameters--
	 * - id: The ID of the desired soundfont to be removed
	 */
	extern void(XSYNTH_IMP* XSynth_Soundfont_Remove)(uint64_t id);

	/**
	 * Removes all soundfonts from the ID list. See the documentation of the
	 * XSynth_Soundfont_Remove to find information about clearing the memory
	 * a soundfont is using.
	 */
	extern void(XSYNTH_IMP* XSynth_Soundfont_RemoveAll)(void);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


#endif