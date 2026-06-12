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
    uint32_t pitch_hz;
    int valid;
    int64_t energy;
} PitchResult_t;

typedef struct
{
    int32_t window[PITCH_WINDOW_SIZE];
    uint32_t index;
} PitchDetector_t;

void AudioProcessing_InitDetector(PitchDetector_t *detector);
int AudioProcessing_AddBlock(PitchDetector_t *detector,
                             const int32_t *samples,
                             uint32_t length,
                             PitchResult_t *result);

#endif
