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

#include "stm32f4xx_hal.h" /* Needed for I2C */

/*
 * DEFINES
 */

#define OLED_I2C_ADDR			(0x3C << 1)  /* Scanned the I2C bus to get this */

/* STRUCT */
typedef struct
{
	I2C_HandleTypeDef *i2cHandle;
    uint8_t fail_step;      /* 0 = no failure */
    uint32_t i2c_error;     /* HAL I2C error code at failure */
}OLED;

/* Initialisation */
HAL_StatusTypeDef OLED_Initialise(OLED *dev, I2C_HandleTypeDef *i2cHandle);

#endif /* OLED_I2C_DRIVER_H */
