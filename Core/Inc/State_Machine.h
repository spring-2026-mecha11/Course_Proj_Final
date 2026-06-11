/*
 * State_Machine.h
 *
 *  Created on: Jun 10, 2026
 *      Author: dreed
 */

#ifndef INC_STATE_MACHINE_H_
#define INC_STATE_MACHINE_H_

#include "main.h"
#include "App_types.h"

void State_Machine_Init(void);
void State_Machine_Task(uint32_t now_ms);

SystemState_t State_Machine_GetState(void);
SystemFlags_t *State_Machine_GetFlags(void);

#endif /* INC_STATE_MACHINE_H_ */
