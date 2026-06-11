/**
 * @file servo_system.h
 * @brief PWM position control for an RC servo.
 *
 * The servo is controlled through a timer configured for 50 Hz PWM with
 * timer counts representing microseconds.
 */

#ifndef SERVO_SYSTEM_H
#define SERVO_SYSTEM_H

#include "stm32f4xx_hal.h"

/**
 * @brief Initializes the servo driver and starts PWM output.
 *
 * The servo is initially commanded to its centered position.
 *
 * @param timer_handle Pointer to the configured PWM timer.
 * @param timer_channel HAL timer channel connected to the servo signal.
 */
void ServoSystem_Init(TIM_HandleTypeDef *timer_handle,
                      uint32_t timer_channel);

/**
 * @brief Commands the servo to the configured up position.
 */
void ServoSystem_Up(void);

/**
 * @brief Commands the servo to the configured down position.
 */
void ServoSystem_Down(void);

/**
 * @brief Stops the servo PWM output.
 */
void ServoSystem_Disable(void);

#endif /* SERVO_SYSTEM_H */
