/**
 * @file audio_processing.h
 * @brief Block-based pitch detector used by the audio subsystem.
 *
 * @defgroup AudioProcessing Audio pitch processing
 * @brief Estimates whistle pitch from microphone samples using a YIN-style detector.
 * @{
 */

#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stdint.h>

#define AUDIO_SAMPLE_RATE_HZ     15836U
#define PITCH_WINDOW_SIZE        512
#define PITCH_MIN_HZ             290U
#define PITCH_MAX_HZ             1000U
#define LEFT_ENERGY_THRESHOLD    20000000LL
#define RIGHT_ENERGY_THRESHOLD   20000000LL


typedef struct
{
    uint32_t pitch_hz;  /**< Estimated fundamental frequency in hertz. */
    int valid;          /**< Nonzero when the pitch lies in the accepted range. */
    int64_t energy;     /**< Mean squared signal energy after DC removal. */
} PitchResult_t;

/**
 * @brief Rolling pitch-detection window for one audio channel.
 */
typedef struct
{
    int32_t window[PITCH_WINDOW_SIZE];  /**< Most recent centered samples. */
    uint32_t index;                     /**< Next write index in the window. */
} PitchDetector_t;

/** @brief Clears a detector's sample window and write index. */
void AudioProcessing_InitDetector(PitchDetector_t *detector);

/**
 * @brief Adds samples and runs pitch detection when the window fills.
 * @return Nonzero when result was updated.
 */
int AudioProcessing_AddBlock(PitchDetector_t *detector,
                             const int32_t *samples,
                             uint32_t length,
                             PitchResult_t *result);

/** @} */
#endif
