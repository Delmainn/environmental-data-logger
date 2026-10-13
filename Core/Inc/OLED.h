/*
 * OLED I2C Driver
 *
 *
 * Author: Gabriel M
 * Created on: 12 October 2026
 *
 */

#ifndef OLED_I2C_DRIVER_H
#define OLED_I2C_DRIVER_H

#include "stm32f4xx_hal.h" /* Needed for STM32 HAL I2C types */

/*
 * OLED I2C address
 *
 * The OLED was discovered at 7-bit address 0x3C during an I2C bus scan.
 * STM32 HAL expects the address left-shifted by one before transmission.
 */

#define OLED_I2C_ADDR			(0x3C << 1)  /* Scanned the I2C bus to get this */

/* OLED Driver return status */
typedef enum
{
	OLED_OK = 0,
	OLED_ERROR_INVALID_ARG,
	OLED_ERROR_I2C
}OLED_Status;

/* Records which initialization command failed */
typedef enum
{
	OLED_INIT_STEP_NONE = 0,
	OLED_INIT_STEP_DISPLAY_OFF,
	OLED_INIT_STEP_DISPLAY_CLOCK,
	OLED_INIT_STEP_MULTIPLEX,
	OLED_INIT_STEP_DISPLAY_OFFSET,
	OLED_INIT_STEP_START_LINE,
	OLED_INIT_STEP_SEGMENT_REMAP,
	OLED_INIT_STEP_COM_SCAN,
	OLED_INIT_STEP_COM_PINS,
	OLED_INIT_STEP_CONTRAST,
	OLED_INIT_STEP_PRECHARGE,
	OLED_INIT_STEP_VCOMH,
	OLED_INIT_STEP_MEMORY_MODE,
	OLED_INIT_STEP_DISPLAY_RAM,
	OLED_INIT_STEP_NORMAL_DISPLAY,
	OLED_INIT_STEP_CHARGE_PUMP,
	OLED_INIT_STEP_DISPLAY_ON
} OLED_InitStep;

/* OLED device context and diagnostic state */
typedef struct
{
	I2C_HandleTypeDef *i2cHandle;

	OLED_InitStep failed_step; /* Initialization stage that failed */
	uint32_t i2c_error;		   /* HAL I2C error captured at failure */
}OLED;

/* Initialize the OLED controller on the selected I2C peripheral */
OLED_Status OLED_Initialise(OLED *dev, I2C_HandleTypeDef *i2cHandle);

#endif /* OLED_I2C_DRIVER_H */
