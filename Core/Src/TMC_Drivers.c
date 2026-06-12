/*
 * TMC_Drivers.c
 *
 *  Created on: Jun 9, 2026
 *      Author: dreed
 */


#include "TMC_Drivers.h"
#include "TMC_Registers.h"

extern SPI_HandleTypeDef hspi3;
#define TMC_SPI_HANDLE hspi3

//Enables Stepper Motor Driver by Setting DRV_ENN Pin Low

void TMC_ENN(void)
{
    HAL_GPIO_WritePin(DRV_ENN_GPIO_Port, DRV_ENN_Pin, GPIO_PIN_RESET);
}

//Disables Stepper Motor Driver by Setting DRV_ENN Pin High

void TMC_DIS(void)
{
    HAL_GPIO_WritePin(DRV_ENN_GPIO_Port, DRV_ENN_Pin, GPIO_PIN_SET);
}

// Enables SPI Transactions by setting CS Pin Low

void TMC_Select(void)
{
    HAL_GPIO_WritePin(CHIP_SEL_GPIO_Port, CHIP_SEL_Pin, GPIO_PIN_RESET);
}

// Disables SPI Transactions by setting CS Pin High

void TMC_Deselect(void)
{
    HAL_GPIO_WritePin(CHIP_SEL_GPIO_Port, CHIP_SEL_Pin, GPIO_PIN_SET);
}

uint8_t TMC_Transfer40(uint8_t addr, uint32_t tx_data, uint32_t *rx_data)
{
	uint8_t tx[5];
	uint8_t rx[5];

	tx[0] = addr;
	tx[1] = (uint8_t)(tx_data >> 24);
	tx[2] = (uint8_t)(tx_data >> 16);
	tx[3] = (uint8_t)(tx_data >> 8);
	tx[4] = (uint8_t)(tx_data);

	TMC_Select();


	HAL_StatusTypeDef hal_status =
			HAL_SPI_TransmitReceive(&TMC_SPI_HANDLE, tx, rx, 5, HAL_MAX_DELAY);

	TMC_Deselect();


	if (hal_status != HAL_OK)
	{
		if(rx_data != NULL)
		{
			*rx_data = 0;
		}

		return 0XFF;

	}

	if(rx_data != NULL)
	{
		*rx_data = ((uint32_t)rx[1] << 24) |
				   ((uint32_t)rx[2] << 16) |
				   ((uint32_t)rx[3] << 8)  |
				   ((uint32_t)rx[4]);

	}

	return rx[0];

}

void TMC_Write_Reg(uint8_t addr, uint32_t data)
{
	uint32_t echo;
	TMC_Transfer40(addr | 0x80, data, &echo);
}


uint32_t TMC_Read_Reg(uint8_t addr)
{
	uint32_t echo;
	uint32_t value;

	// First Transaction sends read address
	TMC_Transfer40(addr & 0x7F, 0x00000000, &echo);

	// Second Transaction clocks out the requested register value
	TMC_Transfer40(addr & 0x7F, 0x00000000, &value);

	return value;

}

TMC_Config_t TMC_Default_Config(void)
{
    TMC_Config_t config;

    config.global_scaler = 128;

    config.ihold = 20;
    config.irun = 20;
    config.ihold_delay = 6;

    config.tpowerdown = 255;

    config.chopconf = 0x10410153;

    config.vstart = 1;
    config.a1 = 1000;
    config.v1 = 5000;
    config.amax = 1000;
    config.vmax = 20000;
    config.dmax = 1000;
    config.d1 = 1000;
    config.vstop = 10;
    config.tzerowait = 0;

    return config;
}

void TMC_Init_With_Config(const TMC_Config_t *config)
{
    TMC_DIS();
    TMC_Deselect();

    HAL_Delay(10);

    TMC_Write_Reg(TMC5240_GSTAT, 0x00000007);

    TMC_Write_Reg(TMC5240_GLOBALSCALER, config->global_scaler);

    uint32_t ihold_irun =
        ((uint32_t)(config->ihold & 0x1F)) |
        ((uint32_t)(config->irun & 0x1F) << 8) |
        ((uint32_t)(config->ihold_delay & 0x0F) << 16);

    TMC_Write_Reg(TMC5240_IHOLD_IRUN, ihold_irun);

    TMC_Write_Reg(TMC5240_TPOWERDOWN, config->tpowerdown);

    TMC_Write_Reg(TMC5240_CHOPCONF, config->chopconf);

    TMC_Write_Reg(TMC5240_RAMPMODE, 0);
    TMC_Write_Reg(TMC5240_XACTUAL, 0);

    TMC_Write_Reg(TMC5240_VSTART, config->vstart);
    TMC_Write_Reg(TMC5240_A1, config->a1);
    TMC_Write_Reg(TMC5240_V1, config->v1);
    TMC_Write_Reg(TMC5240_AMAX, config->amax);
    TMC_Write_Reg(TMC5240_VMAX, config->vmax);
    TMC_Write_Reg(TMC5240_DMAX, config->dmax);
    TMC_Write_Reg(TMC5240_D1, config->d1);
    TMC_Write_Reg(TMC5240_VSTOP, config->vstop);
    TMC_Write_Reg(TMC5240_TZEROWAIT, config->tzerowait);
}


void TMC_Basic_Init(void)
{
    TMC_Config_t config = TMC_Default_Config();
    TMC_Init_With_Config(&config);
}

void TMC_Set_GlobalScaler(uint8_t global_scaler)
{
    if (global_scaler == 0)
    {
        global_scaler = 1;
    }

    TMC_Write_Reg(TMC5240_GLOBALSCALER, global_scaler);
}


void TMC_Set_Current(uint8_t ihold, uint8_t irun, uint8_t ihold_delay)
{
    uint32_t ihold_irun;

    if (ihold > 31)
    {
        ihold = 31;
    }

    if (irun > 31)
    {
        irun = 31;
    }

    if (ihold_delay > 15)
    {
        ihold_delay = 15;
    }

    ihold_irun =
        ((uint32_t)(ihold & 0x1F)) |
        ((uint32_t)(irun & 0x1F) << 8) |
        ((uint32_t)(ihold_delay & 0x0F) << 16);

    TMC_Write_Reg(TMC5240_IHOLD_IRUN, ihold_irun);
}
