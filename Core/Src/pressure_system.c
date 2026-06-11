/**
 * @file pressure_system.c
 * @brief Pressure-system state machine and ADC interrupt callbacks.
 *
 * The module waits for sensor settling, measures a zero-pressure baseline,
 * selects a filter strength from baseline noise, and then runs pressure
 * acquisition and PI fan control without blocking.
 */

#include "pressure_system.h"
#include "main.h"

#define FAN_PWM_MAX_DUTY          100U
#define FAN_PWM_START_DUTY        0U

#define PRESSURE_ADC_MAX_COUNTS   4095.0f
#define PRESSURE_SENSOR_VS        3.3f
#define PRESSURE_SENSOR_OFFSET    0.2f
#define PRESSURE_SENSOR_SLOPE     0.2f

#define PRESSURE_SAMPLE_PERIOD_MS 20U
#define PRESSURE_ZERO_SAMPLES     50U
#define PRESSURE_ZERO_DELAY_MS    10U
#define PRESSURE_ZERO_SETTLE_MS   3000U
#define PRESSURE_ADC_TIMEOUT_MS   5U

typedef enum
{
  /** Waiting for the pressure sensor output to settle after power-on. */
  PRESSURE_STATE_SETTLING = 0,

  /** Collecting samples used to determine zero offset and baseline noise. */
  PRESSURE_STATE_CALIBRATING,

  /** Sampling pressure and optionally running the PI controller. */
  PRESSURE_STATE_RUNNING
} PressureState;

volatile uint32_t fan_duty_percent = FAN_PWM_START_DUTY;
volatile uint32_t fan_pwm_inverted = 1U;

volatile uint32_t pressure_adc_raw = 0U;
volatile uint32_t pressure_adc_busy = 0U;
volatile uint32_t pressure_adc_error_count = 0U;

volatile float pressure_voltage = 0.0f;
volatile float pressure_kpa = 0.0f;
volatile float pressure_uncalibrated_kpa = 0.0f;
volatile uint32_t pressure_millivolts = 0U;
volatile int32_t pressure_milli_kpa = 0;

volatile float pressure_filtered_kpa = 0.0f;
volatile int32_t pressure_filtered_milli_kpa = 0;

volatile float pressure_zero_kpa = 0.0f;
volatile int32_t pressure_zero_milli_kpa = 0;
volatile uint32_t pressure_zero_adc_raw = 0U;
volatile uint32_t pressure_zero_done = 0U;
volatile uint32_t pressure_zero_settle_ms = PRESSURE_ZERO_SETTLE_MS;

volatile float pressure_noise_min_kpa = 0.0f;
volatile float pressure_noise_max_kpa = 0.0f;
volatile float pressure_noise_peak_to_peak_kpa = 0.0f;
volatile int32_t pressure_noise_peak_to_peak_milli_kpa = 0;

volatile uint32_t pressure_control_enabled = 0U;
volatile float pressure_target_kpa = 0.0f;
volatile float pressure_control_kp = 600.0f;
volatile float pressure_control_ki = 300.0f;
volatile float pressure_control_bias_percent = 0.0f;
volatile float pressure_control_integral = 0.0f;
volatile float pressure_control_output_percent = FAN_PWM_START_DUTY;
volatile float pressure_control_min_percent = 0.0f;
volatile float pressure_control_max_percent = 100.0f;
volatile float pressure_filter_alpha = 0.20f;
volatile int32_t pressure_control_direction = 1;
volatile float pressure_error_kpa = 0.0f;
volatile int32_t pressure_error_milli_kpa = 0;

static ADC_HandleTypeDef *pressure_adc;
static TIM_HandleTypeDef *fan_pwm_timer;
static uint32_t fan_pwm_channel;

static PressureState pressure_state;
volatile uint8_t debug_pressure_state = 0;

static uint32_t pressure_next_sample_ms;
static uint32_t pressure_adc_start_ms;

static volatile uint32_t pressure_adc_result_ready;
static volatile uint32_t pressure_adc_result;

static uint32_t calibration_sample_count;
static uint32_t calibration_raw_sum;
static float calibration_pressure_sum;
static float calibration_min_kpa;
static float calibration_max_kpa;

static uint32_t pressure_filter_initialized;
static uint32_t pressure_last_control_ms;
static uint32_t pressure_sample_updated;

static uint32_t fan_applied_duty = UINT32_MAX;
static uint32_t fan_applied_inverted = UINT32_MAX;

static void Fan_SetDutyPercent(uint32_t duty_percent);
static void Pressure_ProcessSample(uint32_t raw);
static void Pressure_ProcessCalibrationSample(uint32_t raw);
static void Pressure_FinishCalibration(void);
static void PressureAdc_Update(uint32_t now_ms);
static void Pressure_RequestSample(uint32_t now_ms, uint32_t period_ms);
static void PressureControl_Update(uint32_t now_ms);

void PressureSystem_Init(ADC_HandleTypeDef *adc,
                         TIM_HandleTypeDef *fan_timer,
                         uint32_t fan_channel)
{
  pressure_adc = adc;
  fan_pwm_timer = fan_timer;
  fan_pwm_channel = fan_channel;

  fan_duty_percent = FAN_PWM_START_DUTY;

  pressure_control_enabled = 0U;
  pressure_control_integral = 0.0f;
  pressure_control_output_percent = 0.0f;

  pressure_adc_busy = 0U;
  pressure_adc_result_ready = 0U;
  pressure_adc_result = 0U;
  pressure_sample_updated = 0U;

  calibration_sample_count = 0U;
  calibration_raw_sum = 0U;
  calibration_pressure_sum = 0.0f;
  calibration_min_kpa = 0.0f;
  calibration_max_kpa = 0.0f;

  pressure_filter_initialized = 0U;
  pressure_last_control_ms = 0U;

  fan_applied_duty = UINT32_MAX;
  fan_applied_inverted = UINT32_MAX;

  pressure_zero_done = 0U;
  pressure_state = PRESSURE_STATE_SETTLING;
  pressure_next_sample_ms = HAL_GetTick() + pressure_zero_settle_ms;

  Fan_SetDutyPercent(fan_duty_percent);

  if (HAL_TIM_PWM_Start(fan_pwm_timer, fan_pwm_channel) != HAL_OK)
  {
    Error_Handler();
  }
}

void PressureSystem_SetTarget(float target_kpa)
{
  pressure_target_kpa = target_kpa;
  pressure_control_integral = 0.0f;

  pressure_error_kpa = pressure_target_kpa - pressure_filtered_kpa;
  pressure_error_milli_kpa = (int32_t)(pressure_error_kpa * 1000.0f);

  pressure_control_enabled = 1U;
}

void PressureSystem_Stop(void)
{
  pressure_control_enabled = 0U;
  pressure_control_integral = 0.0f;
  pressure_control_output_percent = 0.0f;

  fan_duty_percent = 0U;
  Fan_SetDutyPercent(fan_duty_percent);
}

void PressureSystem_Update(void)
{
  uint32_t now_ms = HAL_GetTick();

  PressureAdc_Update(now_ms);

  debug_pressure_state = (uint8_t)pressure_state;

  if (pressure_adc_result_ready != 0U)
  {
    uint32_t raw = pressure_adc_result;
    pressure_adc_result_ready = 0U;

    if (pressure_state == PRESSURE_STATE_CALIBRATING)
    {
      Pressure_ProcessCalibrationSample(raw);
    }
    else if (pressure_state == PRESSURE_STATE_RUNNING)
    {
      Pressure_ProcessSample(raw);
      pressure_sample_updated = 1U;
    }
  }

  if ((pressure_state == PRESSURE_STATE_SETTLING) &&
      ((int32_t)(now_ms - pressure_next_sample_ms) >= 0))
  {
    pressure_state = PRESSURE_STATE_CALIBRATING;
    pressure_next_sample_ms = now_ms;
  }

  if (pressure_state == PRESSURE_STATE_CALIBRATING)
  {
    Pressure_RequestSample(now_ms, PRESSURE_ZERO_DELAY_MS);
  }
  else if (pressure_state == PRESSURE_STATE_RUNNING)
  {
    Pressure_RequestSample(now_ms, PRESSURE_SAMPLE_PERIOD_MS);

    if (pressure_sample_updated != 0U)
    {
      pressure_sample_updated = 0U;
      PressureControl_Update(now_ms);
    }
  }

  Fan_SetDutyPercent(fan_duty_percent);
}

/**
 * @brief Starts one interrupt-driven pressure ADC conversion when due.
 * @param now_ms Current HAL tick value.
 * @param period_ms Delay before the next conversion request.
 */
static void Pressure_RequestSample(uint32_t now_ms, uint32_t period_ms)
{
  ADC_ChannelConfTypeDef config = {0};

  if ((int32_t)(now_ms - pressure_next_sample_ms) < 0)
  {
    return;
  }

  if ((pressure_adc_busy != 0U) || (pressure_adc_result_ready != 0U))
  {
    return;
  }

  config.Channel = ADC_CHANNEL_4;
  config.Rank = 1;
  config.SamplingTime = ADC_SAMPLETIME_84CYCLES;

  if (HAL_ADC_ConfigChannel(pressure_adc, &config) != HAL_OK)
  {
    pressure_adc_error_count++;
    return;
  }

  pressure_adc_busy = 1U;
  pressure_adc_start_ms = now_ms;

  if (HAL_ADC_Start_IT(pressure_adc) == HAL_OK)
  {
    pressure_next_sample_ms = now_ms + period_ms;
  }
  else
  {
    pressure_adc_busy = 0U;
    pressure_adc_error_count++;
  }
}

/**
 * @brief Recovers from an ADC conversion whose interrupt never completed.
 * @param now_ms Current HAL tick value.
 */
static void PressureAdc_Update(uint32_t now_ms)
{
  if ((pressure_adc_busy != 0U) &&
      ((now_ms - pressure_adc_start_ms) >= PRESSURE_ADC_TIMEOUT_MS))
  {
    (void)HAL_ADC_Stop_IT(pressure_adc);
    pressure_adc_busy = 0U;
    pressure_adc_error_count++;
  }
}

/**
 * @brief Accumulates one baseline sample during startup calibration.
 * @param raw Raw 12-bit ADC conversion.
 */
static void Pressure_ProcessCalibrationSample(uint32_t raw)
{
  float volts = (PRESSURE_SENSOR_VS * (float)raw) / PRESSURE_ADC_MAX_COUNTS;

  float sample_kpa =
      ((volts / PRESSURE_SENSOR_VS) - PRESSURE_SENSOR_OFFSET) /
      PRESSURE_SENSOR_SLOPE;

  if (calibration_sample_count == 0U)
  {
    calibration_min_kpa = sample_kpa;
    calibration_max_kpa = sample_kpa;
  }
  else
  {
    if (sample_kpa < calibration_min_kpa)
    {
      calibration_min_kpa = sample_kpa;
    }

    if (sample_kpa > calibration_max_kpa)
    {
      calibration_max_kpa = sample_kpa;
    }
  }

  calibration_pressure_sum += sample_kpa;
  calibration_raw_sum += raw;
  calibration_sample_count++;

  if (calibration_sample_count >= PRESSURE_ZERO_SAMPLES)
  {
    Pressure_FinishCalibration();
  }
}

/**
 * @brief Computes zero offset and chooses filtering from measured noise.
 */
static void Pressure_FinishCalibration(void)
{
  pressure_zero_kpa =
      calibration_pressure_sum / (float)calibration_sample_count;

  pressure_zero_milli_kpa = (int32_t)(pressure_zero_kpa * 1000.0f);
  pressure_zero_adc_raw = calibration_raw_sum / calibration_sample_count;

  pressure_noise_min_kpa = calibration_min_kpa - pressure_zero_kpa;
  pressure_noise_max_kpa = calibration_max_kpa - pressure_zero_kpa;

  pressure_noise_peak_to_peak_kpa =
      calibration_max_kpa - calibration_min_kpa;

  pressure_noise_peak_to_peak_milli_kpa =
      (int32_t)(pressure_noise_peak_to_peak_kpa * 1000.0f);

  if (pressure_noise_peak_to_peak_kpa < 0.010f)
  {
    pressure_filter_alpha = 0.35f;
  }
  else if (pressure_noise_peak_to_peak_kpa < 0.030f)
  {
    pressure_filter_alpha = 0.20f;
  }
  else
  {
    pressure_filter_alpha = 0.10f;
  }

  pressure_zero_done = 1U;
  pressure_filtered_kpa = 0.0f;
  pressure_filtered_milli_kpa = 0;
  pressure_filter_initialized = 0U;

  pressure_state = PRESSURE_STATE_RUNNING;
  pressure_next_sample_ms = HAL_GetTick();
}

/**
 * @brief Converts and filters one pressure sample.
 * @param raw Raw 12-bit ADC conversion.
 */
static void Pressure_ProcessSample(uint32_t raw)
{
  float alpha;

  pressure_adc_raw = raw;

  pressure_voltage =
      (PRESSURE_SENSOR_VS * (float)raw) / PRESSURE_ADC_MAX_COUNTS;

  pressure_uncalibrated_kpa =
      ((pressure_voltage / PRESSURE_SENSOR_VS) - PRESSURE_SENSOR_OFFSET) /
      PRESSURE_SENSOR_SLOPE;

  pressure_kpa = pressure_uncalibrated_kpa - pressure_zero_kpa;

  pressure_millivolts = ((raw * 3300U) + 2047U) / 4095U;
  pressure_milli_kpa = (int32_t)(pressure_kpa * 1000.0f);

  alpha = pressure_filter_alpha;

  if (alpha < 0.0f)
  {
    alpha = 0.0f;
  }
  else if (alpha > 1.0f)
  {
    alpha = 1.0f;
  }

  if (pressure_filter_initialized == 0U)
  {
    pressure_filtered_kpa = pressure_kpa;
    pressure_filter_initialized = 1U;
  }
  else
  {
    pressure_filtered_kpa += alpha * (pressure_kpa - pressure_filtered_kpa);
  }

  pressure_filtered_milli_kpa =
      (int32_t)(pressure_filtered_kpa * 1000.0f);

  pressure_error_kpa = pressure_target_kpa - pressure_filtered_kpa;
  pressure_error_milli_kpa = (int32_t)(pressure_error_kpa * 1000.0f);
}

/**
 * @brief Executes one PI-control update with conditional anti-windup.
 * @param now_ms Current HAL tick value.
 */
static void PressureControl_Update(uint32_t now_ms)
{
  float dt_s;
  float error_kpa;
  float proportional;
  float output;
  float unclamped_output;
  float integral_candidate;
  float min_output;
  float max_output;

  if (pressure_control_enabled == 0U)
  {
    pressure_control_integral = 0.0f;
    pressure_control_output_percent = (float)fan_duty_percent;
    pressure_last_control_ms = now_ms;
    return;
  }

  if (pressure_last_control_ms == 0U)
  {
    pressure_last_control_ms = now_ms;
    return;
  }

  dt_s = (float)(now_ms - pressure_last_control_ms) / 1000.0f;
  pressure_last_control_ms = now_ms;

  if (dt_s <= 0.0f)
  {
    return;
  }

  min_output = pressure_control_min_percent;
  max_output = pressure_control_max_percent;

  if (min_output < 0.0f)
  {
    min_output = 0.0f;
  }

  if (max_output > 100.0f)
  {
    max_output = 100.0f;
  }

  if (max_output < min_output)
  {
    max_output = min_output;
  }

  error_kpa = pressure_error_kpa;

  if (pressure_control_direction < 0)
  {
    error_kpa = -error_kpa;
  }

  proportional = pressure_control_kp * error_kpa;

  unclamped_output =
      pressure_control_bias_percent + proportional + pressure_control_integral;

  integral_candidate =
      pressure_control_integral + (pressure_control_ki * error_kpa * dt_s);

  if (((unclamped_output < max_output) && (unclamped_output > min_output)) ||
      ((unclamped_output >= max_output) && (error_kpa < 0.0f)) ||
      ((unclamped_output <= min_output) && (error_kpa > 0.0f)))
  {
    pressure_control_integral = integral_candidate;
  }

  if (pressure_control_integral > max_output)
  {
    pressure_control_integral = max_output;
  }
  else if (pressure_control_integral < min_output - max_output)
  {
    pressure_control_integral = min_output - max_output;
  }

  output =
      pressure_control_bias_percent + proportional + pressure_control_integral;

  if (output > max_output)
  {
    output = max_output;
  }
  else if (output < min_output)
  {
    output = min_output;
  }

  pressure_control_output_percent = output;
  fan_duty_percent = (uint32_t)(output + 0.5f);
}

/**
 * @brief Applies logical fan duty to the timer compare register.
 * @param duty_percent Requested logical fan duty from 0 to 100%.
 *
 * fan_pwm_inverted translates logical duty for the external transistor stage.
 */
static void Fan_SetDutyPercent(uint32_t duty_percent)
{
  uint32_t pulse;
  uint32_t period_counts;

  if (duty_percent > FAN_PWM_MAX_DUTY)
  {
    duty_percent = FAN_PWM_MAX_DUTY;
  }

  if ((duty_percent == fan_applied_duty) &&
      (fan_pwm_inverted == fan_applied_inverted))
  {
    return;
  }

  period_counts = __HAL_TIM_GET_AUTORELOAD(fan_pwm_timer) + 1U;

  pulse = (period_counts * duty_percent) / FAN_PWM_MAX_DUTY;

  if (fan_pwm_inverted != 0U)
  {
    pulse = period_counts - pulse;
  }

  __HAL_TIM_SET_COMPARE(fan_pwm_timer, fan_pwm_channel, pulse);

  fan_applied_duty = duty_percent;
  fan_applied_inverted = fan_pwm_inverted;
}

/**
 * @brief HAL callback for a completed pressure ADC conversion.
 * @param adc ADC handle supplied by HAL.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *adc)
{
  if ((adc == pressure_adc) && (pressure_adc_busy != 0U))
  {
    pressure_adc_result = HAL_ADC_GetValue(adc);
    pressure_adc_result_ready = 1U;
    pressure_adc_busy = 0U;
  }
}

/**
 * @brief HAL callback for a pressure ADC conversion error.
 * @param adc ADC handle supplied by HAL.
 */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *adc)
{
  if (adc == pressure_adc)
  {
    pressure_adc_busy = 0U;
    pressure_adc_error_count++;
  }
}


bool PressureSystem_IsReady(void)
{
    return (pressure_zero_done != 0U);
}

bool PressureSystem_IsEnabled(void)
{
    return (pressure_control_enabled != 0U);
}

bool PressureSystem_IsStable(void)
{
    float error = pressure_error_kpa;

    if (pressure_zero_done == 0U)
    {
        return false;
    }

    if (error < 0.0f)
    {
        error = -error;
    }

    return (error <= 0.02f);
}

float PressureSystem_GetPressureKpa(void)
{
    return pressure_filtered_kpa;
}

float PressureSystem_GetTargetKpa(void)
{
    return pressure_target_kpa;
}

uint32_t PressureSystem_GetFanDutyPercent(void)
{
    return fan_duty_percent;
}
