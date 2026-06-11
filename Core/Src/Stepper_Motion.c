/*
 * Stepper_Motion.c
 *
 *  Created on: Jun 10, 2026
 *      Author: dreed
 */

#include "Stepper_Motion.h"
#include "TMC_Drivers.h"
#include "TMC_Registers.h"

// Ramp mode Values
#define TMC_RAMPMODE_POSITION	   0
#define TMC_RAMPMODE_VELOCITY_POS  1
#define TMC_RAMPMODE_VELOCITY_NEG  2


//Ramp Stat Bits for Limit Switch Detection
#define RAMP_STAT_STOP_L (1UL << 0)
#define RAMP_STAT_STOP_R (1UL << 1)

#define HOME_SWITCH_USES_REFL	1


//Defining Total Travel Range (Must Be Calibrated!!)
#define STEPPER_RANGE_STEPS 512000 // 512000 = appx 10 rev

// Homing Behavior

#define HOME_BACKOFF_STEPS    3000
#define HOME_TIMEOUT_MS       15000

static bool stepper_homed = false;

static int32_t stepper_min_steps = 0;
static int32_t stepper_max_steps = STEPPER_RANGE_STEPS;


//Helpful Debug variables
volatile int32_t stepper_debug_position = 0;
volatile float stepper_debug_percent = 0.0f;
volatile uint32_t stepper_debug_ramp_stat = 0;
volatile uint8_t stepper_debug_homed = 0;


static bool Stepper_HomeSwitchTriggered(void)
{
	uint32_t ramp_stat = TMC_Read_Reg(TMC5240_RAMP_STAT);

	stepper_debug_ramp_stat = ramp_stat;

#if HOME_SWITCH_USES_REFL
	return((ramp_stat & RAMP_STAT_STOP_L) != 0);
#else
	return((ramp_stat & RAMP_STAT_STOP_R) != 0);
#endif
}


static int32_t Stepper_ClampSteps(int32_t target_steps)
{
	if (target_steps < stepper_min_steps)
	{
		target_steps = stepper_min_steps;
	}

	if (target_steps > stepper_max_steps)
	{
		target_steps = stepper_max_steps;
	}

	return target_steps;
}


void Stepper_Init(void)
{
	TMC_Basic_Init();

	stepper_homed = false;

	stepper_min_steps = 0;
	stepper_max_steps = STEPPER_RANGE_STEPS;

	stepper_debug_homed = 0;
	stepper_debug_position = 0;
	stepper_debug_percent = 0.0f;

	TMC_ENN();
}


void Stepper_ApplySafeProfile(void)
{
	TMC_Write_Reg(TMC5240_VSTART, 1);
	TMC_Write_Reg(TMC5240_A1, 1000);
	TMC_Write_Reg(TMC5240_V1, 5000);
	TMC_Write_Reg(TMC5240_AMAX, 1000);
	TMC_Write_Reg(TMC5240_VMAX, 20000);
	TMC_Write_Reg(TMC5240_DMAX, 1000);
	TMC_Write_Reg(TMC5240_D1, 1000);
	TMC_Write_Reg(TMC5240_VSTOP, 10);
	TMC_Write_Reg(TMC5240_TZEROWAIT, 0);
}


void Stepper_ApplyNormalProfile(void)
{
	TMC_Write_Reg(TMC5240_VSTART, 1);
	TMC_Write_Reg(TMC5240_A1, 2000);
	TMC_Write_Reg(TMC5240_V1, 10000);
	TMC_Write_Reg(TMC5240_AMAX, 2000);
	TMC_Write_Reg(TMC5240_VMAX, 50000);
	TMC_Write_Reg(TMC5240_DMAX, 2000);
	TMC_Write_Reg(TMC5240_D1, 2000);
	TMC_Write_Reg(TMC5240_VSTOP, 10);
	TMC_Write_Reg(TMC5240_TZEROWAIT, 0);
}

int32_t Stepper_GetPositionSteps(void)
{
    int32_t pos = (int32_t)TMC_Read_Reg(TMC5240_XACTUAL);

    stepper_debug_position = pos;

    return pos;
}



float Stepper_GetPositionPercent(void)
{
    int32_t pos = Stepper_GetPositionSteps();

    float percent =
        ((float)(pos - stepper_min_steps) * 100.0f) /
        (float)STEPPER_RANGE_STEPS;

    if (percent < 0.0f)
    {
        percent = 0.0f;
    }

    if (percent > 100.0f)
    {
        percent = 100.0f;
    }

    stepper_debug_percent = percent;

    return percent;
}

bool Stepper_IsHomed(void)
{
    return stepper_homed;
}


void Stepper_ForceHomedForTesting(bool homed)
{
    stepper_homed = homed;
    stepper_debug_homed = homed ? 1 : 0;
}


void Stepper_Stop(void)
{
    int32_t current_steps = Stepper_GetPositionSteps();

    TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_POSITION);
    TMC_Write_Reg(TMC5240_XTARGET, (uint32_t)current_steps);
}


StepperStatus_t Stepper_Home(void)
{
	uint32_t start_time;


	stepper_homed = false;
	stepper_debug_homed = 0;

	Stepper_ApplySafeProfile();

	// Set Limit Switch as Active Low
	TMC_Write_Reg(TMC5240_SWMODE, 0x00000005);
	// Move Negative at a slow speed towards limit switch
	TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_VELOCITY_NEG);

	start_time = HAL_GetTick();

	while (!Stepper_HomeSwitchTriggered())
	{
		if ((HAL_GetTick() - start_time) > HOME_TIMEOUT_MS)
		{
			Stepper_Stop();
			return STEPPER_ERR_HOME_SWITCH_NOT_FOUND;

		}

		HAL_Delay(2);
	}

	Stepper_Stop();
	HAL_Delay(100);

	//Rough zero at first switch contact

	TMC_Write_Reg(TMC5240_XACTUAL, 0);

	// BACK OFF FROM SWITCH

	TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_POSITION);
	TMC_Write_Reg(TMC5240_XTARGET, HOME_BACKOFF_STEPS);

	HAL_Delay(1500);

	//Re-approach zero again for repetition,
	TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_VELOCITY_NEG);

	start_time = HAL_GetTick();

	while (!Stepper_HomeSwitchTriggered())
	{
		if ((HAL_GetTick() - start_time) > HOME_TIMEOUT_MS)
		{
			Stepper_Stop();
			return STEPPER_ERR_HOME_SWITCH_NOT_FOUND;
		}

		HAL_Delay(2);
	}

	Stepper_Stop();
	HAL_Delay(100);

	//Setting Final Zero
	TMC_Write_Reg(TMC5240_XACTUAL,0);
	TMC_Write_Reg(TMC5240_XTARGET,0);
	TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_POSITION);

	stepper_min_steps = 0;
	stepper_max_steps = STEPPER_RANGE_STEPS;

	stepper_homed = true;
	stepper_debug_homed = 1;

	return STEPPER_OK;
}


StepperStatus_t Stepper_MoveToAbsSteps(int32_t target_steps)
{
    if (!stepper_homed)
    {
        return STEPPER_ERR_NOT_HOMED;
    }

    target_steps = Stepper_ClampSteps(target_steps);

    Stepper_ApplySafeProfile();

    TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_POSITION);
    TMC_Write_Reg(TMC5240_XTARGET, (uint32_t)target_steps);

    return STEPPER_OK;
}


StepperStatus_t Stepper_MoveToRelSteps(int32_t delta_steps)
{
    int32_t current_steps = Stepper_GetPositionSteps();
    int32_t target_steps = current_steps + delta_steps;

    return Stepper_MoveToAbsSteps(target_steps);
}

StepperStatus_t Stepper_MoveToPercent(float percent)
{
    int32_t target_steps;

    if (!stepper_homed)
    {
        return STEPPER_ERR_NOT_HOMED;
    }

    if (STEPPER_RANGE_STEPS <= 0)
    {
        return STEPPER_ERR_INVALID_RANGE;
    }

    if (percent < 0.0f)
    {
        percent = 0.0f;
    }

    if (percent > 100.0f)
    {
        percent = 100.0f;
    }

    target_steps =
        stepper_min_steps +
        (int32_t)((percent / 100.0f) * (float)STEPPER_RANGE_STEPS);

    target_steps = Stepper_ClampSteps(target_steps);

    Stepper_ApplyNormalProfile();

    TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_POSITION);
    TMC_Write_Reg(TMC5240_XTARGET, (uint32_t)target_steps);

    return STEPPER_OK;
}
