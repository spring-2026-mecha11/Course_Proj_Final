/*
 * App_types.h
 *
 *  Created on: Jun 10, 2026
 *      Author: dreed
 */

#ifndef INC_APP_TYPES_H_
#define INC_APP_TYPES_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	APP_MODE_NONE = 0,
	APP_MODE_LIVE_HARMONIZER = 1,
	APP_MODE_SONG_PLAYER = 2,
} AppMode_t;

typedef enum
{
	SYS_STATE_BOOT = 0,
	SYS_STATE_INIT_HARDWARE,
	SYS_STATE_HOME_STEPPER,
	SYS_STATE_MODE_SELECT,

	SYS_STATE_LIVE_HARMONIZER,
	SYS_STATE_SONG_PLAYER,

	SYS_STATE_ERROR
} SystemState_t;

typedef enum
{
    SYS_ERROR_NONE = 0,
    SYS_ERROR_STEPPER_HOME_FAILED,
    SYS_ERROR_INVALID_MODE,
    SYS_ERROR_SONG_NOT_LOADED,
    SYS_ERROR_HARDWARE_FAULT
} SystemError_t;

typedef struct
{
	bool hardware_initialized;
	bool stepper_homed;

	bool mode_selected;
	AppMode_t requested_mode;
	AppMode_t active_mode;

	bool live_harmonizer_enabled;
	bool song_player_enabled;

	bool fan_enabled;
	bool fan_pressure_stable;

	bool mute_closed;

	bool audio_data_valid;
	bool user_audio_active;
	bool pitch_error_valid;

	bool usb_song_loaded;
	bool song_playing;
	bool song_done;

	bool fault_active;
	SystemError_t error_code;


} SystemFlags_t;

#endif /* INC_APP_TYPES_H_ */
