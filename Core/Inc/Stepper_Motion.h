/**
 * @file Stepper_Motion.h
 * @brief Position-control interface for the slide-whistle stepper motor.
 *
 * @defgroup StepperMotion Stepper motion
 * @brief Homes the slide, applies travel limits, and commands TMC5240 motion.
 * @{
 */

#ifndef INC_STEPPER_MOTION_H_
#define INC_STEPPER_MOTION_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	STEPPER_OK = 0,                         /**< Command accepted or complete. */
	STEPPER_ERR_NOT_HOMED,                  /**< Motion requested before homing. */
	STEPPER_ERR_INVALID_RANGE,              /**< Travel range is not usable. */
	STEPPER_ERR_TIMEOUT,                    /**< Reserved timeout status. */
	STEPPER_ERR_HOME_SWITCH_NOT_FOUND,      /**< Homing switch was not reached. */
} StepperStatus_t;

/** @brief Initializes the TMC driver and clears the homed state. */
void Stepper_Init(void);

/** @brief Homes the slide against the limit switch. */
StepperStatus_t Stepper_Home(void);

/** @brief Commands an absolute motor position in steps. */
StepperStatus_t Stepper_MoveToAbsSteps(int32_t target_steps);

/** @brief Commands motion relative to the current motor position. */
StepperStatus_t Stepper_MoveToRelSteps(int32_t delta_steps);

/** @brief Commands a position as a percentage of calibrated slide travel. */
StepperStatus_t Stepper_MoveToPercent(float percent);

/** @brief Reads the current TMC5240 position counter in steps. */
int32_t Stepper_GetPositionSteps(void);

/** @brief Reads the current slide position as percent of usable travel. */
float Stepper_GetPositionPercent(void);

/** @brief Returns true once the homing routine has completed. */
bool Stepper_IsHomed(void);

/** @brief Overrides homing status for bench testing without motion. */
void Stepper_ForceHomedForTesting(bool homed);

/** @brief Applies the slow, conservative motion profile used during homing. */
void Stepper_ApplySafeProfile(void);

/** @brief Applies the faster profile used for normal percent-position moves. */
void Stepper_ApplyNormalProfile(void);

/** @brief Stops by commanding the current position as the target. */
void Stepper_Stop(void);

/** @} */
#endif /* INC_STEPPER_MOTION_H_ */



