/*
 * Mode_Select.c
 *
 *  Created on: Jun 10, 2026
 *      Author: dreed
 */

#include "Mode_Select.h"

volatile AppMode_t selected_mode = APP_MODE_NONE;
volatile bool mode_selected = false;

void Mode_Select_Init(void)
{
    selected_mode = APP_MODE_NONE;
    mode_selected = false;
}

void Mode_Select_Task(uint32_t now_ms)
{
    (void)now_ms;

    /*
     * For now:
     *      change selected_mode in the debugger
     *      then set mode_selected = true
     *
     * Later:
     *      button code can set these same two variables
     *
     * Example future button behavior:
     *
     * if (single_press_detected)
     * {
     *     selected_mode = APP_MODE_LIVE_HARMONIZER;
     *     mode_selected = true;
     * }
     *
     * if (double_press_detected)
     * {
     *     selected_mode = APP_MODE_SONG_PLAYER;
     *     mode_selected = true;
     * }
     */
}

bool Mode_Select_HasSelection(void)
{
    return mode_selected;
}

AppMode_t Mode_Select_GetSelectedMode(void)
{
    return selected_mode;
}

void Mode_Select_Reset(void)
{
    selected_mode = APP_MODE_NONE;
    mode_selected = false;
}
