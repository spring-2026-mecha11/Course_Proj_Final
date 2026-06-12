/**
 * @file live_harmonizer.c
 * @brief Pitch-error controller for the automated whistle slide.
 *
 * This module runs slower than the audio pipeline. When both microphone
 * channels have valid pitch estimates, it nudges the stepper target by a
 * proportional/integral term based on target pitch minus measured pitch. If
 * either channel is invalid, it stops motion and recenters control around the
 * current slide position.
 */

#include "live_harmonizer.h"

#include "System_Config.h"
#include "Stepper_Motion.h"
#include "audio_system.h"

static float target_percent;
static float integral_hz_s;
static uint32_t last_control_ms;
static bool timing_initialized;
static bool control_active;

volatile float live_harmonizer_kp = 0.05f;
volatile float live_harmonizer_ki = 0.0001f;

/** @name Live watch variables */
/** @{ */
volatile uint8_t debug_live_control_active = 0;
volatile uint8_t debug_live_stepper_status = 0;
volatile int32_t debug_live_pitch_error_hz = 0;
volatile float debug_live_pitch_integral = 0.0f;
volatile float debug_live_stepper_target_percent = 0.0f;
volatile float debug_live_nudge_percent = 0.0f;
/** @} */

static float LiveHarmonizer_ClampPercent(float percent)
{
    if (percent < LIVE_STEPPER_MIN_PERCENT)
    {
        return LIVE_STEPPER_MIN_PERCENT;
    }

    if (percent > LIVE_STEPPER_MAX_PERCENT)
    {
        return LIVE_STEPPER_MAX_PERCENT;
    }

    return percent;
}

void LiveHarmonizer_Reset(void)
{
    target_percent = LIVE_STEPPER_START_PERCENT;
    integral_hz_s = 0.0f;
    last_control_ms = 0;
    timing_initialized = false;
    control_active = false;

    debug_live_control_active = 0;
    debug_live_stepper_status = 0;
    debug_live_pitch_error_hz = 0;
    debug_live_pitch_integral = 0.0f;
    debug_live_stepper_target_percent = target_percent;
    debug_live_nudge_percent = 0.0f;
}

void LiveHarmonizer_Task(uint32_t now_ms)
{
    if (!timing_initialized)
    {
        last_control_ms = now_ms;
        timing_initialized = true;
        target_percent = LiveHarmonizer_ClampPercent(Stepper_GetPositionPercent());
        return;
    }

    if ((now_ms - last_control_ms) < LIVE_PITCH_CONTROL_PERIOD_MS)
    {
        return;
    }

    float dt_s = 0.001f * (float)(now_ms - last_control_ms);
    last_control_ms = now_ms;

    if (!AudioSystem_TargetValid() || !AudioSystem_PitchErrorValid())
    {
        Stepper_Stop();
        integral_hz_s = 0.0f;
        target_percent = LiveHarmonizer_ClampPercent(Stepper_GetPositionPercent());
        control_active = false;
        debug_live_control_active = 0;
        debug_live_pitch_integral = 0.0f;
        debug_live_stepper_target_percent = target_percent;
        debug_live_nudge_percent = 0.0f;
        return;
    }

    int32_t error_hz = AudioSystem_GetPitchErrorHz();

    if ((error_hz > -LIVE_PITCH_ERROR_DEADBAND_HZ) &&
        (error_hz < LIVE_PITCH_ERROR_DEADBAND_HZ))
    {
        error_hz = 0;
    }

    integral_hz_s += (float)error_hz * dt_s;

    float nudge_percent =
        LIVE_PITCH_CONTROL_SIGN *
        ((live_harmonizer_kp * (float)error_hz) +
         (live_harmonizer_ki * integral_hz_s));

    target_percent += nudge_percent;

    if ((target_percent < LIVE_STEPPER_MIN_PERCENT) ||
        (target_percent > LIVE_STEPPER_MAX_PERCENT))
    {
        target_percent = LiveHarmonizer_ClampPercent(target_percent);
        integral_hz_s = 0.0f;
    }

    StepperStatus_t status = Stepper_MoveToPercent(target_percent);

    control_active = true;
    debug_live_control_active = 1;
    debug_live_stepper_status = (uint8_t)status;
    debug_live_pitch_error_hz = error_hz;
    debug_live_pitch_integral = integral_hz_s;
    debug_live_stepper_target_percent = target_percent;
    debug_live_nudge_percent = nudge_percent;
}
