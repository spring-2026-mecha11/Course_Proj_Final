#ifndef MODE_SELECT_H
#define MODE_SELECT_H

#include <stdint.h>
#include <stdbool.h>
#include "app_types.h"

void Mode_Select_Init(void);
void Mode_Select_Task(uint32_t now_ms);

bool Mode_Select_HasSelection(void);
AppMode_t Mode_Select_GetSelectedMode(void);
void Mode_Select_Reset(void);

/*
 * New button-mode functions.
 */
void Mode_Select_SetModeActive(bool active);
bool Mode_Select_ConsumeExitRequest(void);

#endif
