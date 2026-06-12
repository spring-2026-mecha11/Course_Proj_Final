/**
 * @file Mode_Select.h
 * @brief Single-button operating-mode selection interface.
 *
 * @defgroup ModeSelect Mode selection
 * @brief Debounces the mode button, detects click patterns, and requests exits.
 * @{
 */

#ifndef MODE_SELECT_H
#define MODE_SELECT_H

#include <stdint.h>
#include <stdbool.h>
#include "app_types.h"

/** @brief Resets button state and clears any selected mode. */
void Mode_Select_Init(void);

/**
 * @brief Samples and debounces the mode button.
 * @param now_ms Current HAL tick in milliseconds.
 */
void Mode_Select_Task(uint32_t now_ms);

/** @brief Returns true after a click pattern has selected a mode. */
bool Mode_Select_HasSelection(void);

/** @brief Returns the mode selected by the most recent click pattern. */
AppMode_t Mode_Select_GetSelectedMode(void);

/** @brief Clears the current selection and pending click count. */
void Mode_Select_Reset(void);

/** @brief Tells the button handler whether a mode is currently running. */
void Mode_Select_SetModeActive(bool active);

/** @brief Returns and clears a pending request to exit the active mode. */
bool Mode_Select_ConsumeExitRequest(void);

/** @} */
#endif
