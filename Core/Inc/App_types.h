/**
 * @file App_types.h
 * @brief Shared application modes, states, errors, and runtime flags.
 */

#ifndef INC_APP_TYPES_H_
#define INC_APP_TYPES_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief User-selectable operating modes.
 */
typedef enum
{
	APP_MODE_NONE = 0,             /**< No active user mode. */
	APP_MODE_LIVE_HARMONIZER = 1,  /**< Match or harmonize with the user. */
	APP_MODE_SONG_PLAYER = 2,      /**< Reserved stored-song playback mode. */
} AppMode_t;

/**
 * @brief High-level system states enforced by the main state machine.
 */
typedef enum
{
	SYS_STATE_BOOT = 0,        /**< Initial software state after reset. */
	SYS_STATE_INIT_HARDWARE,   /**< Calibrate sensors and place outputs safe. */
	SYS_STATE_HOME_STEPPER,    /**< Establish the slide position reference. */
	SYS_STATE_MODE_SELECT,     /**< Wait for button-selected operating mode. */

	SYS_STATE_LIVE_HARMONIZER, /**< Run microphone-driven pitch matching. */
	SYS_STATE_SONG_PLAYER,     /**< Reserved stored-song playback state. */

	SYS_STATE_ERROR            /**< Safe fault state. */
} SystemState_t;

/**
 * @brief Fault reasons reported by the state machine.
 */
typedef enum
{
    SYS_ERROR_NONE = 0,                 /**< No fault is active. */
    SYS_ERROR_STEPPER_HOME_FAILED,      /**< Homing switch was not reached. */
    SYS_ERROR_INVALID_MODE,             /**< Mode selector produced bad data. */
    SYS_ERROR_SONG_NOT_LOADED,          /**< Song mode requested without data. */
    SYS_ERROR_HARDWARE_FAULT            /**< Generic hardware fault. */
} SystemError_t;

/**
 * @brief Runtime status flags shared for coordination and debugging.
 *
 * The state machine owns these flags. They summarize subsystem state for
 * decisions, watch windows, and Doxygen documentation of the control flow.
 */
typedef struct
{
	bool hardware_initialized;      /**< Stepper and safe startup commands ran. */
	bool stepper_homed;             /**< Slide position zero has been found. */

	bool mode_selected;             /**< Button UI has selected a mode. */
	AppMode_t requested_mode;       /**< Mode requested by the button UI. */
	AppMode_t active_mode;          /**< Mode currently being executed. */

	bool live_harmonizer_enabled;   /**< Live harmonizer subsystem is active. */
	bool song_player_enabled;       /**< Song player subsystem is active. */

	bool fan_enabled;               /**< Pressure controller has a target. */
	bool fan_pressure_stable;       /**< Pressure error is within tolerance. */

	bool mute_closed;               /**< Servo is blocking the whistle outlet. */

	bool audio_data_valid;          /**< Reserved aggregate audio-valid flag. */
	bool user_audio_active;         /**< Reserved user-whistle activity flag. */
	bool pitch_error_valid;         /**< Both pitches are valid for control. */

	bool usb_song_loaded;           /**< Reserved USB song-load status. */
	bool song_playing;              /**< Reserved song playback in-progress. */
	bool song_done;                 /**< Reserved song playback completion. */

	bool fault_active;              /**< System has entered safe fault state. */
	SystemError_t error_code;       /**< Reason for the active fault. */


} SystemFlags_t;

#endif /* INC_APP_TYPES_H_ */
