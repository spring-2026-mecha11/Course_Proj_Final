/**
 * @file servo_system.c
 * @brief Open-loop RC servo PWM position driver.
 *
 * Servo angles from -60 to +60 degrees are converted to PWM pulses from
 * 900 to 2100 microseconds. The state machine uses the named up/down helpers
 * to open or cover the slide whistle outlet.
 */

#include "servo_system.h"
#include "main.h"

#define SERVO_MIN_DEGREES    (-60.0f)
#define SERVO_MAX_DEGREES    60.0f
#define SERVO_CENTER_PULSE   1500.0f
#define SERVO_US_PER_DEGREE  10.0f

/** Timer and channel used to generate the servo PWM signal. */
static TIM_HandleTypeDef *servo_timer_handle;
static uint32_t servo_timer_channel;

/** Mechanical positions used by ServoSystem_Up() and ServoSystem_Down(). */
volatile float servo_up_position_degrees = 0.0f;
volatile float servo_down_position_degrees = -44.0f;

/**
 * @brief Initializes the PWM interface and centers the servo.
 */
void ServoSystem_Init(TIM_HandleTypeDef *timer_handle,
                      uint32_t timer_channel)
{
  servo_timer_handle = timer_handle;
  servo_timer_channel = timer_channel;

  ServoSystem_Down();

  if (HAL_TIM_PWM_Start(servo_timer_handle, servo_timer_channel) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief Moves the servo to its configured up position.
 */
void ServoSystem_Up(void)
{
  ServoSystem_SetPosition(servo_up_position_degrees);
}

/**
 * @brief Moves the servo to its configured down position.
 */
void ServoSystem_Down(void)
{
  ServoSystem_SetPosition(servo_down_position_degrees);
}

/**
 * @brief Converts a requested angle to a PWM pulse and applies it.
 */
void ServoSystem_SetPosition(float angle_degrees)
{
  if (angle_degrees < SERVO_MIN_DEGREES)
  {
    angle_degrees = SERVO_MIN_DEGREES;
  }
  else if (angle_degrees > SERVO_MAX_DEGREES)
  {
    angle_degrees = SERVO_MAX_DEGREES;
  }

  uint32_t pulse_us =
      (uint32_t)(SERVO_CENTER_PULSE +
                 (angle_degrees * SERVO_US_PER_DEGREE) + 0.5f);

  __HAL_TIM_SET_COMPARE(servo_timer_handle, servo_timer_channel, pulse_us);
}

/**
 * @brief Stops PWM generation for the servo signal.
 */
void ServoSystem_Disable(void)
{
  if (servo_timer_handle != NULL)
  {
    (void)HAL_TIM_PWM_Stop(servo_timer_handle, servo_timer_channel);
  }
}
