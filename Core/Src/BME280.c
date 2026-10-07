/*
 * BME280.c
 *
 *  BME280 I2C driver implementation.
 *
 *  Implements device initialization, factory calibration loading,
 *  measurement acquisition, and low-level register access using
 *  the STM32 HAL I2C interface.
 */

#include "BME280.h"

/* Private driver constants --------------------------------------------------*/

#define BME280_STATUS_IM_UPDATE    							0x01
#define BME280_NVM_TIMEOUT_MS								100
#define BME280_I2C_TIMEOUT_MS 								100

/* BME280 initialization -----------------------------------------------------*/

/*
 * Initializes a BME280 device instance.
 *
 * Associates the device with an STM32 I2C peripheral, verifies the
 * sensor identity, waits for factory calibration data to become
 * available, loads the calibration coefficients, and configures
 * the sensor for measurement.
 */
BME280_Status BME280_Initialise(
	BME280 *dev,
	I2C_HandleTypeDef *i2cHandle
)
{
	uint8_t sensor_status;
	uint8_t chip_id;

	uint8_t calib1[26];
	uint8_t calib2[7];

	uint16_t raw_H4;
	uint16_t raw_H5;

	uint32_t start_tick;

	uint8_t ctrl_hum  = 0x01;
	uint8_t ctrl_meas = 0x24;
	uint8_t config    = 0x00;

	HAL_StatusTypeDef status;

	/* Associate this BME280 instance with its I2C peripheral. */
	dev->i2cHandle		= i2cHandle;

	/* Verify that the responding device identifies as a BME280. */
	status = BME280_ReadRegister(
		dev,
		BME280_REG_ID,
		&chip_id
);
	if(status != HAL_OK)
	{
		return BME280_ERROR_I2C;
	}

	if(chip_id != BME280_CHIP_ID)
	{
		return BME280_ERROR_CHIP_ID;
	}

	/*
	 * Wait for the sensor to finish copying factory calibration
	 * data from NVM into the readable image registers.
	 */

	start_tick = HAL_GetTick();

	do
	{
		/*
		         * Read sensor status to determine whether the NVM-to-image
		         * register copy is still in progress.
		*/

		status = BME280_ReadRegister(
				dev,
				BME280_REG_STATUS,
				&sensor_status
	);
		if (status != HAL_OK)
		{
			return BME280_ERROR_I2C;
		}
		if ((HAL_GetTick()- start_tick) > BME280_NVM_TIMEOUT_MS)
		{
			return BME280_ERROR_CALIBRATION;
		}

	} while (sensor_status & BME280_STATUS_IM_UPDATE);

	 /*
	  * Read the factory calibration register blocks after the
	  * NVM-to-image-register copy has completed.
	  */
	status = BME280_ReadRegisters(
		dev,
		BME280_REG_CALIB00,
		calib1,
		26
);
	if(status != HAL_OK)
	{
		return BME280_ERROR_CALIBRATION;
	}

	status = BME280_ReadRegisters(
		dev,
		BME280_REG_CALIB26,
		calib2,7
);
	if(status != HAL_OK)
	{
		return BME280_ERROR_CALIBRATION;
	}


	/* Reconstruct temperature calibration coefficients. */

	dev->dig_T1 =
		((uint16_t)calib1[1] << 8) |
		 (uint16_t)calib1[0];

	dev->dig_T2 = (int16_t)(
		((uint16_t)calib1[3] << 8) |
		 (uint16_t)calib1[2]
);
	dev->dig_T3 = (int16_t)(
		((uint16_t)calib1[5] << 8) |
		 (uint16_t)calib1[4]
);
	/* Reconstruct pressure calibration coefficients. */

	dev->dig_P1 =
		((uint16_t)calib1[7] << 8)  |
		 (uint16_t)calib1[6];

	dev->dig_P2 = (int16_t)(
		((uint16_t)calib1[9] << 8) |
		 (uint16_t)calib1[8]
);

	dev->dig_P3 = (int16_t)(
		((uint16_t)calib1[11] << 8) |
		 (uint16_t)calib1[10]
);

	dev->dig_P4 = (int16_t)(
		((uint16_t)calib1[13] << 8) |
		 (uint16_t)calib1[12]

);
	dev->dig_P5 = (int16_t)(
		((uint16_t)calib1[15] << 8) |
		 (uint16_t)calib1[14]

);
	dev->dig_P6 = (int16_t)(
		((uint16_t)calib1[17] << 8) |
		 (uint16_t)calib1[16]

);
	dev->dig_P7 = (int16_t)(
		((uint16_t)calib1[19] << 8) |
		 (uint16_t)calib1[18]

);
	dev->dig_P8 = (int16_t)(
		((uint16_t)calib1[21] << 8) |
		 (uint16_t)calib1[20]

);
	dev->dig_P9 = (int16_t)(
		((uint16_t)calib1[23] << 8) |
		 (uint16_t)calib1[22]
);
	/*
	 * Reconstruct humidity calibration coefficients.
	 * H4 and H5 require additional bit manipulation because they
	 * share portions of register 0xE5.
	 */
	dev->dig_H1 = calib1[25];

	dev->dig_H2 = (int16_t)(
	    ((uint16_t)calib2[1] << 8) |
	     (uint16_t)calib2[0]
	);

	dev->dig_H3 = calib2[2];

	raw_H4 =
	    ((uint16_t)calib2[3] << 4) |
	    (calib2[4] & 0x0F);

	raw_H5 =
	    ((uint16_t)calib2[5] << 4) |
	    (calib2[4] >> 4);

	/* Sign-extend the 12-bit H4 and H5 coefficients to 16 bits. */
	if (raw_H4 & 0x0800)
	{
	    raw_H4 |= 0xF000;
	}

	if (raw_H5 & 0x0800)
	{
	    raw_H5 |= 0xF000;
	}

	dev->dig_H4 = (int16_t)raw_H4;
	dev->dig_H5 = (int16_t)raw_H5;

	dev->dig_H6 = (int8_t)calib2[6];

	/*
	 * Configure measurement behavior.
	 * Humidity must be written before ctrl_meas for the setting
	 * to become effective
	 */
	status = BME280_WriteRegister(dev, BME280_REG_CTRL_HUM, &ctrl_hum);

	if (status != HAL_OK)
	{
		return BME280_ERROR_CONFIG;
	}

	status = BME280_WriteRegister(dev, BME280_REG_CONFIG, &config);

	if (status != HAL_OK)
	{
		return BME280_ERROR_CONFIG;
	}

	status = BME280_WriteRegister(dev,BME280_REG_CTRL_MEAS, &ctrl_meas);

	if (status != HAL_OK)
	{
	    return BME280_ERROR_CONFIG;
	}

	dev->temp_C      = 0.0f;
	dev->humidity_RH = 0.0f;
	dev->pressure_Pa = 0.0f;

    /* Initialization complete. */
	return BME280_OK;


}







/* Low-level register access -------------------------------------------------*/

/*
 * Reads one byte from a BME280 register.
 */

HAL_StatusTypeDef BME280_ReadRegister(
	BME280 *dev,
	uint8_t reg,
	uint8_t *data
)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, BME280_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, BME280_I2C_TIMEOUT_MS);
}

/*
 * Reads multiple consecutive bytes beginning at the specified register.
 * Used for burst reads such as calibration and measurement data.
 */

HAL_StatusTypeDef BME280_ReadRegisters(
	BME280 *dev,
	uint8_t reg,
	uint8_t *data,
	uint8_t length
)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, BME280_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, BME280_I2C_TIMEOUT_MS);
}

/*
 * Writes one byte to a BME280 register.
 */

HAL_StatusTypeDef BME280_WriteRegister(
	BME280 *dev,
	uint8_t reg,
	uint8_t *data
)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, BME280_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, BME280_I2C_TIMEOUT_MS);
}
