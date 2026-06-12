/**
 * @file System_Config.h
 * @brief Tunable constants shared by application modules.
 *
 * Constants in this file define user-interface timing, harmonizer limits, and
 * pressure targets. Keeping them in one header makes the final tuning values
 * visible in the generated documentation.
 */

#ifndef INC_SYSTEM_CONFIG_H_
#define INC_SYSTEM_CONFIG_H_

#include "App_types.h"

/** @name Mode input values */
/** @{ */
#define MODE_INPUT_NONE              0u
#define MODE_INPUT_LIVE_HARMONIZER   1u
#define MODE_INPUT_SONG_PLAYER       2u
/** @} */

/** @name Mode select behavior */
/** @{ */
#define MODE_SELECT_TIMEOUT_MS       10000u
#define MODE_SELECT_DEFAULT_MODE     APP_MODE_LIVE_HARMONIZER
/** @} */

/** @name Reserved button timing constants */
/** @{ */
#define BUTTON_DEBOUNCE_MS           30u
#define BUTTON_MULTI_PRESS_GAP_MS    600u
#define BUTTON_MODE_SELECT_WINDOW_MS 2000u
/** @} */

/** @name Live harmonizer timing and travel limits */
/** @{ */
#define LIVE_CONTROL_PERIOD_MS       10u
#define LIVE_STEPPER_DEADBAND_PCT    0.25f

#define LIVE_PITCH_CONTROL_PERIOD_MS        200u
#define LIVE_PITCH_CONTROL_SIGN             -1.0f
#define LIVE_PITCH_ERROR_DEADBAND_HZ        2

#define LIVE_STEPPER_MIN_PERCENT            25.0f
#define LIVE_STEPPER_START_PERCENT          50.0f
#define LIVE_STEPPER_MAX_PERCENT            75.0f
/** @} */

/** @name Song player timing */
/** @{ */
#define SONG_PLAYER_TASK_PERIOD_MS   5u
/** @} */


/** @name Pressure targets */
/** @{ */
#define LIVE_PRESSURE_TARGET_KPA   0.15f
#define SONG_PRESSURE_TARGET_KPA   0.15f
/** @} */

#endif /* INC_SYSTEM_CONFIG_H_ */
