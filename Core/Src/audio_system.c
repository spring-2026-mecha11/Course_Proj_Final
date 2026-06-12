/**
 * @file audio_system.c
 * @brief Stereo I2S DMA receiver and pitch-error publisher.
 *
 * The PCM1802 provides interleaved left/right microphone samples. DMA callbacks
 * mark half-buffers ready, and the foreground task reconstructs each channel,
 * runs pitch detection, applies energy thresholds, and publishes the pitch
 * error used by the live harmonizer.
 */

#include "audio_system.h"
#include "audio_processing.h"
#include <string.h>

#define AUDIO_DMA_HALFWORDS      4096U
#define I2S_DMA_TRANSFER_SIZE    (AUDIO_DMA_HALFWORDS / 2U)
#define HALF_DMA_HALFWORDS       (AUDIO_DMA_HALFWORDS / 2U)
#define FRAMES_PER_HALF_BUFFER   (HALF_DMA_HALFWORDS / 4U)

static I2S_HandleTypeDef *audio_i2s_handle = NULL;

static volatile uint8_t audio_half_ready = 0;
static volatile uint8_t audio_full_ready = 0;
static volatile uint8_t audio_running = 0;

static volatile uint32_t target_pitch_hz = 0;
static volatile uint32_t measured_pitch_hz = 0;
static volatile uint8_t target_valid = 0;
static volatile uint8_t measured_valid = 0;

static volatile int32_t pitch_error_hz = 0;
static volatile uint8_t pitch_error_valid = 0;

static volatile int64_t target_energy = 0;
static volatile int64_t measured_energy = 0;

/** @cond INTERNAL_BUFFERS */
static uint16_t audio_rx_buffer[AUDIO_DMA_HALFWORDS];

static int32_t left_samples[FRAMES_PER_HALF_BUFFER] __attribute__((aligned(32)));
static int32_t right_samples[FRAMES_PER_HALF_BUFFER] __attribute__((aligned(32)));

static PitchDetector_t left_detector __attribute__((aligned(32)));
static PitchDetector_t right_detector __attribute__((aligned(32)));
/** @endcond */

/*
 * Live watch variables for debugger inspection.
 */
volatile uint32_t debug_audio_target_pitch_hz = 0;
volatile uint32_t debug_audio_measured_pitch_hz = 0;
volatile int32_t debug_audio_pitch_error_hz = 0;
volatile uint8_t debug_audio_pitch_error_valid = 0;
volatile uint8_t debug_audio_target_valid = 0;
volatile uint8_t debug_audio_measured_valid = 0;
volatile int64_t debug_audio_target_energy = 0;
volatile int64_t debug_audio_measured_energy = 0;
volatile uint8_t debug_audio_running = 0;
volatile uint32_t debug_audio_blocks_processed = 0;
volatile uint32_t debug_audio_dma_errors = 0;

volatile uint16_t debug_audio_dma_0 = 0;
volatile uint16_t debug_audio_dma_1 = 0;
volatile uint16_t debug_audio_dma_2 = 0;
volatile uint16_t debug_audio_dma_3 = 0;

volatile int16_t debug_audio_left_raw = 0;
volatile int16_t debug_audio_right_raw = 0;

/**
 * @brief Reconstructs one DMA half-buffer into left and right sample arrays.
 * @param offset_halfwords Start index of the DMA half-buffer to process.
 */
static void AudioSystem_ReconstructStereoSamples(uint32_t offset_halfwords)
{
    uint32_t in = offset_halfwords;

    debug_audio_dma_0 = audio_rx_buffer[offset_halfwords + 0];
    debug_audio_dma_1 = audio_rx_buffer[offset_halfwords + 1];
    debug_audio_dma_2 = audio_rx_buffer[offset_halfwords + 2];
    debug_audio_dma_3 = audio_rx_buffer[offset_halfwords + 3];

    for (uint32_t n = 0; n < FRAMES_PER_HALF_BUFFER; n++)
    {
        int16_t l_msb = (int16_t)audio_rx_buffer[in];
        int16_t r_msb = (int16_t)audio_rx_buffer[in + 2];

        debug_audio_left_raw = l_msb;
        debug_audio_right_raw = r_msb;

        left_samples[n] = (int32_t)l_msb;
        right_samples[n] = (int32_t)r_msb;

        in += 4U;
    }
}


/**
 * @brief Runs pitch detection on one reconstructed stereo audio block.
 * @param offset_halfwords Start index of the DMA half-buffer to process.
 */
static void AudioSystem_ProcessAudio(uint32_t offset_halfwords)
{
    PitchResult_t target_result = {0};
    PitchResult_t measured_result = {0};

    AudioSystem_ReconstructStereoSamples(offset_halfwords);

    int target_updated =
        AudioProcessing_AddBlock(&left_detector,
                                 left_samples,
                                 FRAMES_PER_HALF_BUFFER,
                                 &target_result);

    int measured_updated =
        AudioProcessing_AddBlock(&right_detector,
                                 right_samples,
                                 FRAMES_PER_HALF_BUFFER,
                                 &measured_result);

    if (target_updated)
    {
        target_energy = target_result.energy;

        if ((target_result.valid) &&
            (target_result.energy >= LEFT_ENERGY_THRESHOLD))
        {
            target_pitch_hz = target_result.pitch_hz;
            target_valid = 1;
        }
        else
        {
            target_pitch_hz = 0;
            target_valid = 0;
        }
    }

    if (measured_updated)
    {
        measured_energy = measured_result.energy;

        if ((measured_result.valid) &&
            (measured_result.energy >= RIGHT_ENERGY_THRESHOLD))
        {
            measured_pitch_hz = measured_result.pitch_hz;
            measured_valid = 1;
        }
        else
        {
            measured_pitch_hz = 0;
            measured_valid = 0;
        }
    }

    if (target_valid && measured_valid)
    {
        pitch_error_hz =
            (int32_t)target_pitch_hz -
            (int32_t)measured_pitch_hz;

        pitch_error_valid = 1;
    }
    else
    {
        pitch_error_hz = 0;
        pitch_error_valid = 0;
    }

    debug_audio_target_pitch_hz = target_pitch_hz;
    debug_audio_measured_pitch_hz = measured_pitch_hz;
    debug_audio_pitch_error_hz = pitch_error_hz;
    debug_audio_pitch_error_valid = pitch_error_valid;
    debug_audio_target_valid = target_valid;
    debug_audio_measured_valid = measured_valid;
    debug_audio_target_energy = target_energy;
    debug_audio_measured_energy = measured_energy;

    debug_audio_blocks_processed++;
}


void AudioSystem_Init(I2S_HandleTypeDef *hi2s)
{
    audio_i2s_handle = hi2s;

    AudioProcessing_InitDetector(&left_detector);
    AudioProcessing_InitDetector(&right_detector);

    memset((void *)audio_rx_buffer, 0, sizeof(audio_rx_buffer));

    audio_half_ready = 0;
    audio_full_ready = 0;

    target_pitch_hz = 0;
    measured_pitch_hz = 0;
    target_valid = 0;
    measured_valid = 0;
    pitch_error_hz = 0;
    pitch_error_valid = 0;

    if (HAL_I2S_Receive_DMA(audio_i2s_handle,
                            (uint16_t *)audio_rx_buffer,
                            I2S_DMA_TRANSFER_SIZE) == HAL_OK)
    {
        audio_running = 1;
    }
    else
    {
        audio_running = 0;
        debug_audio_dma_errors++;
    }

    debug_audio_running = audio_running;
}


void AudioSystem_Task(void)
{
    if (!audio_running)
    {
        return;
    }

    if (audio_half_ready)
    {
        audio_half_ready = 0;
        AudioSystem_ProcessAudio(0);
    }

    if (audio_full_ready)
    {
        audio_full_ready = 0;
        AudioSystem_ProcessAudio(HALF_DMA_HALFWORDS);
    }
}


bool AudioSystem_IsRunning(void)
{
    return (audio_running != 0);
}


bool AudioSystem_PitchErrorValid(void)
{
    return (pitch_error_valid != 0);
}


int32_t AudioSystem_GetPitchErrorHz(void)
{
    return pitch_error_hz;
}


uint32_t AudioSystem_GetTargetPitchHz(void)
{
    return target_pitch_hz;
}


uint32_t AudioSystem_GetMeasuredPitchHz(void)
{
    return measured_pitch_hz;
}


uint8_t AudioSystem_TargetValid(void)
{
    return target_valid;
}


uint8_t AudioSystem_MeasuredValid(void)
{
    return measured_valid;
}


int64_t AudioSystem_GetTargetEnergy(void)
{
    return target_energy;
}


int64_t AudioSystem_GetMeasuredEnergy(void)
{
    return measured_energy;
}


/*
 * HAL I2S callbacks are defined here so only this module owns audio DMA
 * completion events.
 */
/**
 * @brief Marks the first half of the I2S DMA buffer ready for processing.
 * @param hi2s I2S handle supplied by HAL.
 */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s == audio_i2s_handle)
    {
        audio_half_ready = 1;
    }
}


/**
 * @brief Marks the second half of the I2S DMA buffer ready for processing.
 * @param hi2s I2S handle supplied by HAL.
 */
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s == audio_i2s_handle)
    {
        audio_full_ready = 1;
    }
}
