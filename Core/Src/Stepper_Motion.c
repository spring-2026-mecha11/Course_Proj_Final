/**
 * @file Stepper_Motion.c
 * @brief Slide-position driver built on the TMC5240 register interface.
 *
 * The stepper subsystem creates a position abstraction for the rest of the
 * application. It homes against a TMC5240 limit-switch input, defines a fixed
 * step range for the whistle slide, and supports both raw step commands and
 * percentage-of-travel commands used by the harmonizer.
 */

#include "Stepper_Motion.h"
#include "TMC_Drivers.h"
#include "TMC_Registers.h"

/** @name TMC5240 ramp modes */
/** @{ */
#define TMC_RAMPMODE_POSITION	   0
#define TMC_RAMPMODE_VELOCITY_POS  1
#define TMC_RAMPMODE_VELOCITY_NEG  2
/** @} */


/** @name Limit-switch status bits in RAMP_STAT */
/** @{ */
#define RAMP_STAT_STOP_L (1UL << 0)
#define RAMP_STAT_STOP_R (1UL << 1)
/** @} */

#define HOME_SWITCH_USES_REFL	1


/** Total calibrated slide travel represented in TMC5240 microsteps. */
#define STEPPER_RANGE_STEPS 270000

/** @name Homing behavior */
/** @{ */
#define HOME_BACKOFF_STEPS    5000
#define HOME_TIMEOUT_MS       20000
/** @} */


/** @name Current settings for homing and normal motion */
/** @{ */
#define STEPPER_NORMAL_GLOBALSCALER   128
#define STEPPER_NORMAL_IHOLD          20
#define STEPPER_NORMAL_IRUN           20
#define STEPPER_NORMAL_IHOLD_DELAY    6

#define STEPPER_PERCENT_GLOBALSCALER  192
#define STEPPER_PERCENT_IHOLD         16
#define STEPPER_PERCENT_IRUN          26
#define STEPPER_PERCENT_IHOLD_DELAY   6
/** @} */

static bool stepper_homed = false;

static int32_t stepper_min_steps = 0;
static int32_t stepper_max_steps = STEPPER_RANGE_STEPS;


/** @name Live watch variables */
/** @{ */
volatile int32_t stepper_debug_position = 0;
volatile float stepper_debug_percent = 0.0f;
volatile uint32_t stepper_debug_ramp_stat = 0;
volatile uint8_t stepper_debug_homed = 0;
/** @} */


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


/**
 * @brief Clamps an absolute step target to the calibrated travel limits.
 * @param target_steps Requested target in TMC5240 position counts.
 * @return Clamped target in position counts.
 */
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

/**
 * @brief Applies conservative current settings used for setup and stopping.
 */
static void Stepper_ApplyNormalCurrent(void)
{
    TMC_Set_GlobalScaler(STEPPER_NORMAL_GLOBALSCALER);
    TMC_Set_Current(STEPPER_NORMAL_IHOLD,
                    STEPPER_NORMAL_IRUN,
                    STEPPER_NORMAL_IHOLD_DELAY);
}


/**
 * @brief Applies higher run current used for active percent-position moves.
 */
static void Stepper_ApplyPercentCurrent(void)
{
    TMC_Set_GlobalScaler(STEPPER_PERCENT_GLOBALSCALER);
    TMC_Set_Current(STEPPER_PERCENT_IHOLD,
                    STEPPER_PERCENT_IRUN,
                    STEPPER_PERCENT_IHOLD_DELAY);
}

void Stepper_Init(void)
{
	TMC_Basic_Init();
	Stepper_ApplyNormalCurrent();
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
    TMC_Write_Reg(TMC5240_A1, 12000);
    TMC_Write_Reg(TMC5240_V1, 60000);
    TMC_Write_Reg(TMC5240_AMAX, 12000);
    TMC_Write_Reg(TMC5240_VMAX, 300000);
    TMC_Write_Reg(TMC5240_DMAX, 12000);
    TMC_Write_Reg(TMC5240_D1, 12000);
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
	Stepper_ApplyNormalCurrent();
    int32_t current_steps = Stepper_GetPositionSteps();

    TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_POSITION);
    TMC_Write_Reg(TMC5240_XTARGET, (uint32_t)current_steps);
}


StepperStatus_t Stepper_Home(void)
{
	uint32_t start_time;


	stepper_homed = false;
	stepper_debug_homed = 0;
	Stepper_ApplyNormalCurrent();
	Stepper_ApplySafeProfile();

	/* Configure the TMC5240 to stop when the active-low home switch trips. */
	TMC_Write_Reg(TMC5240_SWMODE, 0x00000005);
	/* Move negative at a slow speed toward the home switch. */
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

	/* Roughly zero at first switch contact, then back off and re-approach. */

	TMC_Write_Reg(TMC5240_XACTUAL, 0);

	TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_POSITION);
	TMC_Write_Reg(TMC5240_XTARGET, HOME_BACKOFF_STEPS);

	HAL_Delay(1500);

	/* Re-approach the switch to reduce dependence on first-contact bounce. */
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

	/* Store the final zero reference used for all later position commands. */
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

    Stepper_ApplyNormalCurrent();
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

    Stepper_ApplyPercentCurrent();
    Stepper_ApplyNormalProfile();

    TMC_Write_Reg(TMC5240_RAMPMODE, TMC_RAMPMODE_POSITION);
    TMC_Write_Reg(TMC5240_XTARGET, (uint32_t)target_steps);

    return STEPPER_OK;
}
