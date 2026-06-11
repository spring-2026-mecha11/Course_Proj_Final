/*
 * Stepper_Motion.h
 *
 *  Created on: Jun 10, 2026
 *      Author: dreed
 */

#ifndef INC_STEPPER_MOTION_H_
#define INC_STEPPER_MOTION_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	STEPPER_OK = 0,
	STEPPER_ERR_NOT_HOMED,
	STEPPER_ERR_INVALID_RANGE,
	STEPPER_ERR_TIMEOUT,
	STEPPER_ERR_HOME_SWITCH_NOT_FOUND,
} StepperStatus_t;

void Stepper_Init(void);

StepperStatus_t Stepper_Home(void);

StepperStatus_t Stepper_MoveToAbsSteps(int32_t target_steps);
StepperStatus_t Stepper_MoveToRelSteps(int32_t delta_steps);
StepperStatus_t Stepper_MoveToPercent(float percent);

int32_t Stepper_GetPositionSteps(void);
float Stepper_GetPositionPercent(void);

bool Stepper_IsHomed(void);
void Stepper_ForceHomedForTesting(bool homed);

void Stepper_ApplySafeProfile(void);
void Stepper_ApplyNormalProfile(void);

void Stepper_Stop(void);


#endif /* INC_STEPPER_MOTION_H_ */




