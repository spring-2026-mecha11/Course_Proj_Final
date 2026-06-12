/*
 * App.c
 *
 *  Created on: Jun 10, 2026
 *      Author: dreed
 */

#include "App.h"

#include "Mode_Select.h"
#include "State_Machine.h"
#include "pressure_system.h"
#include "Stepper_Motion.h"
#include "servo_system.h"
#include "audio_system.h"

// Fan Externs
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

//Servo Extern
extern TIM_HandleTypeDef htim2;

// Audio Extern
extern I2S_HandleTypeDef hi2s2;

void App_Init(void)
{
    /*
     * Initialize high-level software modules here.
     *
     * CubeMX already initializes GPIO, SPI, I2C, timers, clocks, etc.
     * Do not reconfigure hardware here.
     */

    Mode_Select_Init();
    PressureSystem_Init(&hadc1, &htim3, TIM_CHANNEL_1);
    ServoSystem_Init(&htim2, TIM_CHANNEL_2);
    State_Machine_Init();
    AudioSystem_Init(&hi2s2);

    /*
     * Future module init calls:
     *
     * Servo_Mute_Init();
     * Fan_Control_Init();
     * Audio_Input_Init();
     * Live_Harmonizer_Init();
     * Song_Player_Init();
     */
}

void App_Task(void)
{
    uint32_t now_ms = HAL_GetTick();

    /*
     * These should all be non-blocking task functions.
     */

    Mode_Select_Task(now_ms);

    /*
     * Future subsystem task calls:
     *
     * Audio_Input_Task(now_ms);
     * Fan_Control_Task(now_ms);
     * Servo_Mute_Task(now_ms);
     * Live_Harmonizer_Task(now_ms);
     * Song_Player_Task(now_ms);
     */
    AudioSystem_Task();
    PressureSystem_Update();
    State_Machine_Task(now_ms);
}
