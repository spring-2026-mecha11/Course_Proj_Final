/**
 * @file pressure_system.h
 * @brief Pressure sensing, fan PWM, and PI pressure control.
 *
 * The pressure system starts in a calibration sequence, then provides filtered
 * pressure readings and a closed-loop fan command for the rest of the
 * application.
 */

#ifndef PRESSURE_SYSTEM_H
#define PRESSURE_SYSTEM_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>
/**
 * @defgroup PressureSystem Pressure system
 * @brief Non-blocking pressure acquisition and fan pressure control.
 * @{
 */

/**
 * @brief Initializes pressure acquisition and starts the fan PWM peripheral.
 *
 * The fan starts at 0%. Pressure calibration begins after
 * pressure_zero_settle_ms and completes asynchronously through
 * PressureSystem_Update().
 *
 * @param adc ADC handle configured for the pressure sensor input.
 * @param fan_timer Timer handle configured for fan PWM generation.
 * @param fan_channel HAL timer channel used for fan PWM.
 */
void PressureSystem_Init(ADC_HandleTypeDef *adc,
                         TIM_HandleTypeDef *fan_timer,
                         uint32_t fan_channel);

/**
 * @brief Enables pressure control and changes its target.
 * @param target_kpa Desired gauge pressure in kPa.
 */
void PressureSystem_SetTarget(float target_kpa);

/**
 * @brief Disables pressure control and commands the fan to 0%.
 */
void PressureSystem_Stop(void);

/**
 * @brief Advances calibration, sampling, and control state.
 *
 * Call continuously from the main loop. The function does not block.
 */
void PressureSystem_Update(void);

/** @brief Returns true when startup zero calibration has completed. */
bool PressureSystem_IsReady(void);

/** @brief Returns true when filtered pressure is close to the target. */
bool PressureSystem_IsStable(void);

/** @brief Returns true when PI pressure control is enabled. */
bool PressureSystem_IsEnabled(void);

/** @brief Returns the filtered gauge pressure in kPa. */
float PressureSystem_GetPressureKpa(void);

/** @brief Returns the active pressure target in kPa. */
float PressureSystem_GetTargetKpa(void);

/** @brief Returns the logical fan duty command from 0 to 100%. */
uint32_t PressureSystem_GetFanDutyPercent(void);


/** @name Live Expressions: fan output */
/** @{ */
extern volatile uint32_t fan_duty_percent;
extern volatile uint32_t fan_pwm_inverted;
/** @} */

/** @name Live Expressions: pressure measurement and control */
/** @{ */
extern volatile uint32_t pressure_adc_raw;
extern volatile uint32_t pressure_adc_busy;
extern volatile uint32_t pressure_adc_error_count;
extern volatile float pressure_voltage;
extern volatile float pressure_kpa;
extern volatile float pressure_uncalibrated_kpa;
extern volatile uint32_t pressure_millivolts;
extern volatile int32_t pressure_milli_kpa;
extern volatile float pressure_filtered_kpa;
extern volatile int32_t pressure_filtered_milli_kpa;
extern volatile float pressure_zero_kpa;
extern volatile int32_t pressure_zero_milli_kpa;
extern volatile uint32_t pressure_zero_adc_raw;
extern volatile uint32_t pressure_zero_done;
extern volatile uint32_t pressure_zero_settle_ms;
extern volatile float pressure_noise_min_kpa;
extern volatile float pressure_noise_max_kpa;
extern volatile float pressure_noise_peak_to_peak_kpa;
extern volatile int32_t pressure_noise_peak_to_peak_milli_kpa;
extern volatile uint32_t pressure_control_enabled;
extern volatile float pressure_target_kpa;
extern volatile float pressure_control_kp;
extern volatile float pressure_control_ki;
extern volatile float pressure_control_bias_percent;
extern volatile float pressure_control_integral;
extern volatile float pressure_control_output_percent;
extern volatile float pressure_control_min_percent;
extern volatile float pressure_control_max_percent;
extern volatile float pressure_filter_alpha;
extern volatile int32_t pressure_control_direction;
extern volatile float pressure_error_kpa;
extern volatile int32_t pressure_error_milli_kpa;

/** @} */

/** @} */

#endif
