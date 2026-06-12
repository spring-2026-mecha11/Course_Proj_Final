/*
 * TMC_Drivers.h
 *
 *  Created on: Jun 9, 2026
 *      Author: dreed
 */

#ifndef INC_TMC_DRIVERS_H_
#define INC_TMC_DRIVERS_H_

#include "main.h"
#include <stdint.h>

// Initialization Structure for Parameterizing Setup

typedef struct
{
    uint8_t global_scaler;

    uint8_t ihold;
    uint8_t irun;
    uint8_t ihold_delay;

    uint8_t tpowerdown;

    uint32_t chopconf;

    uint32_t vstart;
    uint32_t a1;
    uint32_t v1;
    uint32_t amax;
    uint32_t vmax;
    uint32_t dmax;
    uint32_t d1;
    uint32_t vstop;
    uint32_t tzerowait;
} TMC_Config_t;



void TMC_ENN(void);  // Enable Stepper Driver = Sets DRV_ENN Pin Low
void TMC_DIS(void);  // Disable Stepper Driver = Sets DRV_ENN Pin High

void TMC_Select(void); // Enable SPI = Sets CS Pin Low
void TMC_Deselect(void); // Disable SPI = Sets CS Pin High

uint8_t TMC_Transfer40(uint8_t addr, uint32_t tx_data, uint32_t *rx_data); //Raw 5-Byte SPI Transfer

void TMC_Write_Reg(uint8_t addr, uint32_t data);  //Writes to a Register on the TMC

uint32_t TMC_Read_Reg(uint8_t addr);  //Reads from a register on the TMC

TMC_Config_t TMC_Default_Config(void); //Sets up generic configuration for init
void TMC_Init_With_Config(const TMC_Config_t *config); //
void TMC_Basic_Init(void);

void TMC_Set_Current(uint8_t ihold, uint8_t irun, uint8_t ihold_delay);
void TMC_Set_GlobalScaler(uint8_t global_scaler);

#endif /* INC_TMC_DRIVERS_H_ */
