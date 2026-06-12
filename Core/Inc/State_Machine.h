/**
 * @file State_Machine.h
 * @brief High-level operating-state controller.
 *
 * @defgroup StateMachine State machine
 * @brief Coordinates startup, mode selection, active modes, and safe faults.
 * @{
 */

#ifndef INC_STATE_MACHINE_H_
#define INC_STATE_MACHINE_H_

#include "main.h"
#include "App_types.h"

/**
 * @brief Resets the state machine to the boot state and clears flags.
 */
void State_Machine_Init(void);

/**
 * @brief Advances the system state machine by one cooperative task step.
 * @param now_ms Current HAL tick in milliseconds.
 */
void State_Machine_Task(uint32_t now_ms);

/**
 * @brief Returns the current high-level system state.
 */
SystemState_t State_Machine_GetState(void);

/**
 * @brief Returns the state machine's live runtime flags.
 */
SystemFlags_t *State_Machine_GetFlags(void);

/** @} */
#endif /* INC_STATE_MACHINE_H_ */
