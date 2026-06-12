/**
 * @file audio_processing.c
 * @brief YIN-style pitch detection for slide-whistle microphone data.
 *
 * Each channel accumulates a 512-sample window. When a window is full, the
 * detector removes DC offset, computes signal energy, and estimates the
 * fundamental frequency from a cumulative mean normalized difference function.
 */

#include "audio_processing.h"
#include <limits.h>
#include <stdio.h>
#include <stddef.h>

#define MAX_LAG_BUFFER 128U

void AudioProcessing_InitDetector(PitchDetector_t *detector)
{
    detector->index = 0;

    for (uint32_t i = 0; i < PITCH_WINDOW_SIZE; i++)
    {
        detector->window[i] = 0;
    }
}

/**
 * @brief Removes DC offset and returns mean squared signal energy.
 * @param samples Input sample window.
 * @param length Number of samples in the window.
 * @return Average centered-signal energy.
 */
static int64_t remove_dc_and_get_energy(const int32_t *samples,
                                        uint32_t length)
{
    int64_t sum = 0;

    for (uint32_t i = 0; i < length; i++)
    {
        sum += samples[i];
    }

    int32_t mean = (int32_t)(sum / (int64_t)length);

    int64_t energy = 0;

    for (uint32_t i = 0; i < length; i++)
    {
        int32_t centered = samples[i] - mean;
        energy += (int64_t)centered * (int64_t)centered;
    }

    return energy / (int64_t)length;
}

/**
 * @brief Estimates pitch from a complete sample window using a YIN-style test.
 * @param samples Sample window to analyze.
 * @param length Number of samples in the window.
 * @return Estimated pitch in hertz, or 0 if no pitch can be estimated.
 */
static uint32_t detect_pitch_yin(int32_t *samples, uint32_t length)
{
    uint32_t min_lag = AUDIO_SAMPLE_RATE_HZ / PITCH_MAX_HZ;
    uint32_t max_lag = AUDIO_SAMPLE_RATE_HZ / PITCH_MIN_HZ;

    if (min_lag < 2)
    {
        min_lag = 2;
    }

    if (max_lag >= length)
    {
        max_lag = length - 1;
    }

    if (max_lag >= MAX_LAG_BUFFER)
    {
        max_lag = MAX_LAG_BUFFER - 1;
    }

    uint64_t diff_values[MAX_LAG_BUFFER];
    uint32_t cmndf_q15[MAX_LAG_BUFFER];

    for (uint32_t i = 0; i < MAX_LAG_BUFFER; i++)
    {
        diff_values[i] = 0;
        cmndf_q15[i] = 32768U;
    }

    /*
     * Difference function.
     */
    for (uint32_t lag = 1; lag <= max_lag; lag++)
    {
        uint64_t diff = 0;

        for (uint32_t i = 0; i < length - lag; i++)
        {
            int64_t delta = (int64_t)samples[i] -
                            (int64_t)samples[i + lag];

            diff += (uint64_t)(delta * delta);
        }

        diff_values[lag] = diff;
    }

    /*
     * Cumulative mean normalized difference function.
     *
     * cmndf[lag] = diff[lag] /
     *              ((diff[1] + diff[2] + ... + diff[lag]) / lag)
     *
     * Stored as Q15:
     * 1.0 = 32768
     */
    uint64_t running_sum = 0;

    for (uint32_t lag = 1; lag <= max_lag; lag++)
    {
        running_sum += diff_values[lag];

        if (running_sum == 0)
        {
            cmndf_q15[lag] = 32768U;
        }
        else
        {
            cmndf_q15[lag] =
                (uint32_t)((diff_values[lag] *
                            (uint64_t)lag *
                            32768ULL) / running_sum);
        }
    }

    /*
     * YIN threshold.
     *
     * 0.15 is a good starting point.
     * Q15 threshold = 0.15 * 32768 = 4915.
     */
    const uint32_t threshold_q15 = 4915U;

    uint32_t best_lag = 0;

    for (uint32_t lag = min_lag; lag <= max_lag; lag++)
    {
        if (cmndf_q15[lag] < threshold_q15)
        {
            /*
             * Move forward to the local minimum around this dip.
             */
            while ((lag + 1 <= max_lag) &&
                   (cmndf_q15[lag + 1] < cmndf_q15[lag]))
            {
                lag++;
            }

            best_lag = lag;
            break;
        }
    }

    /*
     * Fallback: choose lowest CMNDF value in range.
     */
    if (best_lag == 0)
    {
        uint32_t best_value = UINT32_MAX;

        for (uint32_t lag = min_lag; lag <= max_lag; lag++)
        {
            if (cmndf_q15[lag] < best_value)
            {
                best_value = cmndf_q15[lag];
                best_lag = lag;
            }
        }
    }

    if (best_lag == 0)
    {
        return 0;
    }

    /*
     * Parabolic interpolation using CMNDF values.
     */
    if ((best_lag > min_lag) && (best_lag < max_lag))
    {
        int64_t y0 = (int64_t)cmndf_q15[best_lag - 1];
        int64_t y1 = (int64_t)cmndf_q15[best_lag];
        int64_t y2 = (int64_t)cmndf_q15[best_lag + 1];

        int64_t denom = y0 - 2 * y1 + y2;
        int64_t delta_q10 = 0;

        if (denom != 0)
        {
            delta_q10 = ((y0 - y2) * 512) / denom;
        }

        int64_t lag_q10 =
            ((int64_t)best_lag * 1024) + delta_q10;

        if (lag_q10 <= 0)
        {
            return 0;
        }

        return (uint32_t)(((int64_t)AUDIO_SAMPLE_RATE_HZ * 1024 +
                           lag_q10 / 2) / lag_q10);
    }

    return (AUDIO_SAMPLE_RATE_HZ + best_lag / 2) / best_lag;
}


int AudioProcessing_AddBlock(PitchDetector_t *detector,
                             const int32_t *samples,
                             uint32_t length,
                             PitchResult_t *result)
{
    for (uint32_t i = 0; i < length; i++)
    {
        int32_t s = (int32_t)((int16_t)samples[i]);

        detector->window[detector->index] = s;
        detector->index++;

        if (detector->index >= PITCH_WINDOW_SIZE)
        {
            detector->index = 0;

            result->energy =
                remove_dc_and_get_energy(detector->window,
                                         PITCH_WINDOW_SIZE);

            result->pitch_hz =
                detect_pitch_yin(detector->window,
                                 PITCH_WINDOW_SIZE);

            if ((result->pitch_hz >= PITCH_MIN_HZ) &&
                (result->pitch_hz <= PITCH_MAX_HZ))
            {
                result->valid = 1;
            }
            else
            {
                result->pitch_hz = 0;
                result->valid = 0;
            }

            return 1;
        }
    }

    return 0;
}
