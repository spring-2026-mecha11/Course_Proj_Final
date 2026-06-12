/*
 * audio_system.h
 *
 *  Created on: Jun 11, 2026
 *      Author: dreed
 */

#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

void AudioSystem_Init(I2S_HandleTypeDef *hi2s);
void AudioSystem_Task(void);

bool AudioSystem_IsRunning(void);

bool AudioSystem_PitchErrorValid(void);
int32_t AudioSystem_GetPitchErrorHz(void);

uint32_t AudioSystem_GetTargetPitchHz(void);
uint32_t AudioSystem_GetMeasuredPitchHz(void);

uint8_t AudioSystem_TargetValid(void);
uint8_t AudioSystem_MeasuredValid(void);

int64_t AudioSystem_GetTargetEnergy(void);
int64_t AudioSystem_GetMeasuredEnergy(void);

#endif
