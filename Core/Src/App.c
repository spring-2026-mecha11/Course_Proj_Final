/**
 * @file App.c
 * @brief Application-level initialization and cooperative task scheduling.
 *
 * CubeMX owns clocks, GPIO, DMA, ADC, timers, SPI, I2S, and USB setup. This
 * file starts the project-specific modules after that hardware setup has
 * completed, then calls their task functions from the main loop.
 */

#include "App.h"

#include "Mode_Select.h"
#include "State_Machine.h"
#include "pressure_system.h"
#include "Stepper_Motion.h"
#include "servo_system.h"
#include "audio_system.h"
#include "live_harmonizer.h"

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
     * Hardware handles are passed into modules that need direct peripheral
     * access. Drivers keep those handles internally after initialization.
     */

    Mode_Select_Init();
    PressureSystem_Init(&hadc1, &htim3, TIM_CHANNEL_1);
    ServoSystem_Init(&htim2, TIM_CHANNEL_2);
    State_Machine_Init();
    AudioSystem_Init(&hi2s2);
    LiveHarmonizer_Reset();

}

void App_Task(void)
{
    uint32_t now_ms = HAL_GetTick();

    Mode_Select_Task(now_ms);
    AudioSystem_Task();
    PressureSystem_Update();
    State_Machine_Task(now_ms);
}
