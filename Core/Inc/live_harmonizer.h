/**
 * @file live_harmonizer.h
 * @brief Outer pitch-control loop for live harmonizer mode.
 *
 * @defgroup LiveHarmonizer Live harmonizer
 * @brief Converts microphone pitch error into stepper slide-position changes.
 * @{
 */

#ifndef LIVE_HARMONIZER_H
#define LIVE_HARMONIZER_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Resets controller state and returns the target to the start position.
 */
void LiveHarmonizer_Reset(void);

/**
 * @brief Runs one timed pitch-control update when enough time has elapsed.
 * @param now_ms Current HAL tick in milliseconds.
 */
void LiveHarmonizer_Task(uint32_t now_ms);

/**
 * @brief Proportional gain in slide-percent change per hertz of pitch error.
 *
 * Positive pitch error means the user whistle is higher than the automated
 * whistle. The sign of the final motion is also set by
 * LIVE_PITCH_CONTROL_SIGN, so this value is intended to stay positive during
 * live tuning.
 */
extern volatile float live_harmonizer_kp;

/**
 * @brief Integral gain in slide-percent change per hertz-second of error.
 *
 * This value trims sustained pitch error. It should remain much smaller than
 * live_harmonizer_kp because the stepper target is clamped to the playable
 * travel range.
 */
extern volatile float live_harmonizer_ki;

/** @} */
#endif
