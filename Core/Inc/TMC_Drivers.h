/**
 * @file TMC_Drivers.h
 * @brief Low-level TMC5240 SPI register driver.
 *
 * @defgroup TMCDrivers TMC motor driver
 * @brief Provides chip-select control, 40-bit transfers, and setup helpers.
 * @{
 */

#ifndef INC_TMC_DRIVERS_H_
#define INC_TMC_DRIVERS_H_

#include "main.h"
#include <stdint.h>

/**
 * @brief Register values used to initialize the TMC5240.
 */
typedef struct
{
    uint8_t global_scaler;  /**< Global motor-current scale. */

    uint8_t ihold;          /**< Standstill hold current setting. */
    uint8_t irun;           /**< Run current setting. */
    uint8_t ihold_delay;    /**< Delay before reducing to hold current. */

    uint8_t tpowerdown;     /**< Driver power-down delay. */

    uint32_t chopconf;      /**< Chopper configuration register value. */

    uint32_t vstart;        /**< Starting velocity. */
    uint32_t a1;            /**< First acceleration. */
    uint32_t v1;            /**< Velocity threshold for second ramp segment. */
    uint32_t amax;          /**< Maximum acceleration. */
    uint32_t vmax;          /**< Maximum velocity. */
    uint32_t dmax;          /**< Maximum deceleration. */
    uint32_t d1;            /**< Final deceleration. */
    uint32_t vstop;         /**< Stop velocity. */
    uint32_t tzerowait;     /**< Delay after reaching zero velocity. */
} TMC_Config_t;


/** @brief Enables the TMC5240 driver output stage. */
void TMC_ENN(void);  // Enable Stepper Driver = Sets DRV_ENN Pin Low

/** @brief Disables the TMC5240 driver output stage. */
void TMC_DIS(void);  // Disable Stepper Driver = Sets DRV_ENN Pin High

/** @brief Pulls chip select low before an SPI transaction. */
void TMC_Select(void); // Enable SPI = Sets CS Pin Low

/** @brief Releases chip select after an SPI transaction. */
void TMC_Deselect(void); // Disable SPI = Sets CS Pin High

/**
 * @brief Performs one raw 5-byte TMC5240 SPI transaction.
 * @param addr Register address byte, including the write bit when needed.
 * @param tx_data Data payload shifted out after the address byte.
 * @param rx_data Optional location for the returned 32-bit payload.
 * @return TMC5240 status byte, or 0xFF if the HAL transfer fails.
 */
uint8_t TMC_Transfer40(uint8_t addr, uint32_t tx_data, uint32_t *rx_data); //Raw 5-Byte SPI Transfer

/**
 * @brief Writes one 32-bit TMC5240 register.
 * @param addr Register address without the write bit.
 * @param data Value to write.
 */
void TMC_Write_Reg(uint8_t addr, uint32_t data);  //Writes to a Register on the TMC

/**
 * @brief Reads one 32-bit TMC5240 register using the chip's two-step read.
 * @param addr Register address to read.
 * @return Register value returned by the second SPI transaction.
 */
uint32_t TMC_Read_Reg(uint8_t addr);  //Reads from a register on the TMC

/** @brief Returns the default startup configuration used by the project. */
TMC_Config_t TMC_Default_Config(void); //Sets up generic configuration for init

/**
 * @brief Applies a complete TMC5240 configuration.
 * @param config Register values to write during driver initialization.
 */
void TMC_Init_With_Config(const TMC_Config_t *config); //

/** @brief Initializes the driver with the default configuration. */
void TMC_Basic_Init(void);

/**
 * @brief Writes the TMC5240 run and hold current settings.
 * @param ihold Hold current setting, clamped to the TMC5240 5-bit range.
 * @param irun Run current setting, clamped to the TMC5240 5-bit range.
 * @param ihold_delay Delay setting, clamped to the TMC5240 4-bit range.
 */
void TMC_Set_Current(uint8_t ihold, uint8_t irun, uint8_t ihold_delay);

/**
 * @brief Writes the global current scaler, clamping zero to one.
 * @param global_scaler Global current scale value.
 */
void TMC_Set_GlobalScaler(uint8_t global_scaler);

/** @} */
#endif /* INC_TMC_DRIVERS_H_ */
