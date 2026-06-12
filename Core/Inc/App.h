/**
 * @file App.h
 * @brief Top-level application scheduler interface.
 *
 * @defgroup AppFramework Application framework
 * @brief Connects CubeMX peripheral initialization to the project subsystems.
 * @{
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "main.h"

/**
 * @brief Initializes all team-written firmware modules after CubeMX setup.
 */
void App_Init(void);

/**
 * @brief Runs one pass of the cooperative application task loop.
 */
void App_Task(void);


/** @} */
#endif /* INC_APP_H_ */
