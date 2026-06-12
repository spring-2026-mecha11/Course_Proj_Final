/**
 * @file Mode_Select.c
 * @brief Debounced single-button user interface for mode selection.
 *
 * When no mode is active, one click selects live harmonizer after the double
 * click window expires, and two clicks select song player immediately. When a
 * mode is active, one press becomes an exit request consumed by the state
 * machine.
 */

#include "Mode_Select.h"
#include "main.h"

#define MODE_BUTTON_DEBOUNCE_MS       30u
#define MODE_BUTTON_DOUBLE_WINDOW_MS  450u

volatile AppMode_t selected_mode = APP_MODE_NONE;
volatile bool mode_selected = false;

/*
 * Simple debug/watch variables.
 */
volatile uint8_t debug_mode_button_raw = 1;
volatile uint8_t debug_mode_button_pressed = 0;
volatile uint8_t debug_mode_click_count = 0;
volatile uint8_t debug_mode_exit_requested = 0;
volatile uint8_t debug_mode_active = 0;

/*
 * Internal state.
 */
static bool mode_active = false;
static bool exit_requested = false;

static bool button_stable_pressed = false;
static bool button_last_raw_pressed = false;

static uint32_t button_last_change_ms = 0;
static uint32_t first_click_ms = 0;
static uint8_t pending_click_count = 0;


static bool Mode_Select_ReadButtonPressed(void)
{
    GPIO_PinState pin_state;

    pin_state = HAL_GPIO_ReadPin(MODE_BTN_GPIO_Port, MODE_BTN_Pin);

    /*
     * Pull-up input:
     *      SET   = not pressed
     *      RESET = pressed
     */
    debug_mode_button_raw = (pin_state == GPIO_PIN_SET) ? 1u : 0u;

    return (pin_state == GPIO_PIN_RESET);
}


void Mode_Select_Init(void)
{
    selected_mode = APP_MODE_NONE;
    mode_selected = false;

    mode_active = false;
    exit_requested = false;

    button_stable_pressed = false;
    button_last_raw_pressed = false;

    button_last_change_ms = 0;
    first_click_ms = 0;
    pending_click_count = 0;

    debug_mode_button_raw = 1;
    debug_mode_button_pressed = 0;
    debug_mode_click_count = 0;
    debug_mode_exit_requested = 0;
    debug_mode_active = 0;
}


void Mode_Select_SetModeActive(bool active)
{
    mode_active = active;
    debug_mode_active = active ? 1u : 0u;
}


bool Mode_Select_ConsumeExitRequest(void)
{
    if (exit_requested)
    {
        exit_requested = false;
        debug_mode_exit_requested = 0;

        return true;
    }

    return false;
}


void Mode_Select_Task(uint32_t now_ms)
{
    bool raw_pressed;
    bool press_event = false;

    raw_pressed = Mode_Select_ReadButtonPressed();

    /*
     * Debounce raw button state.
     */
    if (raw_pressed != button_last_raw_pressed)
    {
        button_last_raw_pressed = raw_pressed;
        button_last_change_ms = now_ms;
    }

    if ((now_ms - button_last_change_ms) >= MODE_BUTTON_DEBOUNCE_MS)
    {
        if (raw_pressed != button_stable_pressed)
        {
            button_stable_pressed = raw_pressed;
            debug_mode_button_pressed = button_stable_pressed ? 1u : 0u;

            if (button_stable_pressed)
            {
                press_event = true;
            }
        }
    }

    /*
     * If already inside live/song mode:
     * one button press means exit mode immediately.
     */
    if (mode_active)
    {
        if (press_event)
        {
            exit_requested = true;
            debug_mode_exit_requested = 1;

            pending_click_count = 0;
            debug_mode_click_count = 0;

            selected_mode = APP_MODE_NONE;
            mode_selected = false;
        }

        return;
    }

    /*
     * If already selected, wait for state machine to consume/reset.
     */
    if (mode_selected)
    {
        return;
    }

    /*
     * Mode select behavior:
     *      one click  -> live harmonizer after window expires
     *      two clicks -> song player immediately
     */
    if (press_event)
    {
        if (pending_click_count == 0u)
        {
            first_click_ms = now_ms;
            pending_click_count = 1u;
        }
        else
        {
            pending_click_count++;
        }

        debug_mode_click_count = pending_click_count;

        if (pending_click_count >= 2u)
        {
            selected_mode = APP_MODE_SONG_PLAYER;
            mode_selected = true;

            pending_click_count = 0u;
            debug_mode_click_count = 0u;
        }
    }

    if (pending_click_count == 1u)
    {
        if ((now_ms - first_click_ms) >= MODE_BUTTON_DOUBLE_WINDOW_MS)
        {
            selected_mode = APP_MODE_LIVE_HARMONIZER;
            mode_selected = true;

            pending_click_count = 0u;
            debug_mode_click_count = 0u;
        }
    }
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

    pending_click_count = 0u;
    debug_mode_click_count = 0u;
}
