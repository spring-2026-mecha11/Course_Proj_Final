/*
 * Mode_Select.h
 *
 *  Created on: Jun 10, 2026
 *      Author: dreed
 */

#ifndef INC_MODE_SELECT_H_
#define INC_MODE_SELECT_H_

#include "main.h"
#include "App_types.h"
#include <stdbool.h>

extern volatile AppMode_t selected_mode;
extern volatile bool mode_selected;

void Mode_Select_Init(void);
void Mode_Select_Task(uint32_t now_ms);

bool Mode_Select_HasSelection(void);
AppMode_t Mode_Select_GetSelectedMode(void);

void Mode_Select_Reset(void);


#endif /* INC_MODE_SELECT_H_ */
