/**
 * @file audio_system.h
 * @brief I2S audio capture and two-channel pitch comparison.
 *
 * @defgroup AudioSystem Audio capture
 * @brief Receives stereo microphone data and publishes target/measured pitch.
 * @{
 */

#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

/**
 * @brief Starts I2S DMA capture and initializes both pitch detectors.
 * @param hi2s I2S peripheral connected to the stereo audio ADC.
 */
void AudioSystem_Init(I2S_HandleTypeDef *hi2s);

/**
 * @brief Processes any half/full DMA buffers marked ready by HAL callbacks.
 */
void AudioSystem_Task(void);

/** @brief Returns true when I2S DMA reception started successfully. */
bool AudioSystem_IsRunning(void);

/** @brief Returns true when both channels have valid pitches. */
bool AudioSystem_PitchErrorValid(void);

/** @brief Returns target pitch minus measured pitch in hertz. */
int32_t AudioSystem_GetPitchErrorHz(void);

/** @brief Returns the latest user-whistle pitch in hertz. */
uint32_t AudioSystem_GetTargetPitchHz(void);

/** @brief Returns the latest automated-whistle pitch in hertz. */
uint32_t AudioSystem_GetMeasuredPitchHz(void);

/** @brief Returns nonzero when the user-whistle channel is valid. */
uint8_t AudioSystem_TargetValid(void);

/** @brief Returns nonzero when the automated-whistle channel is valid. */
uint8_t AudioSystem_MeasuredValid(void);

/** @brief Returns the latest user-whistle signal energy. */
int64_t AudioSystem_GetTargetEnergy(void);

/** @brief Returns the latest automated-whistle signal energy. */
int64_t AudioSystem_GetMeasuredEnergy(void);

/** @} */
#endif
