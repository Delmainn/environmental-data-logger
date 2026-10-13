/*
 * OLED.c
 *
 * OLED I2C driver implementation
 *
 *
 *
 */

#include "OLED.h"

#define OLED_CMD_DISPLAY_OFF        	0xAE
#define OLED_CMD_SET_MULTIPLEX      	0xA8
#define OLED_MUX_RATIO_64           	0x3F
#define OLED_CMD_SET_DISPLAY_OFFSET 	0xD3
#define OLED_DISPLAY_OFFSET_ZERO		0x00
#define OLED_CMD_SET_START_LINE_0		0x40
#define OLED_CMD_SEGMENT_REMAP			0xA1
#define OLED_CMD_COM_SCAN_REMAP			0xC8
#define OLED_CMD_SET_COM_PINS			0xDA
#define OLED_COM_PINS_CONFIG			0x12

/*
#define OLED_CMD_SET_IREF				0xAD
#define OLED_IREF_INTERNAL				0x10 */

#define OLED_CMD_CHARGE_PUMP			0x8D
#define OLED_CHARGE_PUMP_ENABLE			0x14
#define OLED_CMD_SET_DISPLAY_CLOCK		0xD5
#define OLED_DISPLAY_CLOCK_DEFAULT		0x80

#define OLED_CMD_SET_PRECHARGE		 	0xD9
#define OLED_PRECHARGE_VENDOR			0xF1
#define OLED_CMD_SET_VCOMH				0xDB
#define OLED_VCOMH_VENDOR				0x30

#define OLED_CMD_SET_CONTRAST			0x81
#define OLED_CONTRAST_DEFAULT			0x7F
#define OLED_CMD_SET_MEMORY_MODE		0x20
#define OLED_MEMORY_MODE_HORIZONTAL		0x00
#define OLED_CMD_NORMAL_DISPLAY			0xA6
#define OLED_CMD_DISPLAY_RAM			0xA4
#define OLED_CMD_DISPLAY_ON				0xAF


#define OLED_CMD_ENTIRE_DISPLAY_ON		0xA5

/*
 * Sends a single command byte to the OLED controller over I2C
 *
 * The first byte is the OLED control byte:
 * 0x00 tells the controller that the following byte is a command.
 *
 * The second byte is the actual SSD1315 command
 */
static HAL_StatusTypeDef OLED_SendCommand(OLED *dev, uint8_t command)
{
	uint8_t packet[2];

	packet[0] = 0x00;
	packet[1] = command;

	return HAL_I2C_Master_Transmit(
		dev->i2cHandle,
		OLED_I2C_ADDR,
		packet,
		2,
		100
	);

}

/*
 * Sends a command byte followed by one parameter byte in the same I2C transaction.
 *
 * Some SSD1315 configuration commands are multi-byte commands, might be wise to have the parameter
 * follow immediately after the command within the command stream
 */
static HAL_StatusTypeDef OLED_SendCommandWithParam(OLED *dev,
													uint8_t command,
													uint8_t parameter)
{
	uint8_t packet[3];

	packet[0] = 0x00;  /* Control byte: following bytes are commands */
	packet[1] = command;
	packet[2]= parameter;

	return HAL_I2C_Master_Transmit(
		dev->i2cHandle,
		OLED_I2C_ADDR,
		packet,
		3,
		100
	);
}



/*
 * Initializes the OLED controller and associate it with an STM32 I2C peripheral
 */
HAL_StatusTypeDef OLED_Initialise(OLED *dev, I2C_HandleTypeDef *i2cHandle)
{
	dev->i2cHandle = i2cHandle;
	dev->fail_step = 0;
	dev->i2c_error = HAL_I2C_ERROR_NONE;

	/* Step 1: Keep display off while controller settings are configured */
	dev->fail_step = 1;

	if (OLED_SendCommand(dev, OLED_CMD_DISPLAY_OFF) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return HAL_ERROR;
	}

	/* Step 1 succeeded */
	dev->fail_step = 0;

	/* Configure panel for 64 active rows */
	if (OLED_SendCommandWithParam(dev,
								  OLED_CMD_SET_MULTIPLEX,
							      OLED_MUX_RATIO_64)!= HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Align display RAM with the top physical row */
	if (OLED_SendCommandWithParam(dev,
								  OLED_CMD_SET_DISPLAY_OFFSET,
								  OLED_DISPLAY_OFFSET_ZERO)!= HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use display RAM line 0 as the top visible row */
	if (OLED_SendCommand(dev, OLED_CMD_SET_START_LINE_0) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Map display RAM horizontally to match the panel's physical orientation */
	if (OLED_SendCommand(dev, OLED_CMD_SEGMENT_REMAP) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Scan COM lines in reverse order to match the panel's vertical orientation */
	if (OLED_SendCommand(dev, OLED_CMD_COM_SCAN_REMAP) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Configure COM pin hardware for the 128x64 panel layout */
	if (OLED_SendCommandWithParam(dev,
			                      OLED_CMD_SET_COM_PINS,
								  OLED_COM_PINS_CONFIG)!= HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use the SSD1315 internal current reference to drive OLED segment current
	if (OLED_SendCommandWithParam(dev,
			                      OLED_CMD_SET_IREF,
								  OLED_IREF_INTERNAL)!= HAL_OK)
	{
		return HAL_ERROR;
	}*/

	/*space*/

	/* Use the controller's reset/default display clock settings */
	if (OLED_SendCommandWithParam(dev,
			                      OLED_CMD_SET_DISPLAY_CLOCK,
								  OLED_DISPLAY_CLOCK_DEFAULT) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use the controller's default OLED pre-charge timing */
	if (OLED_SendCommandWithParam(dev,
			                      OLED_CMD_SET_PRECHARGE,
								  OLED_PRECHARGE_VENDOR) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use the controller's default VCOMH de-select level */
	if (OLED_SendCommandWithParam(dev,
			                      OLED_CMD_SET_VCOMH,
								  OLED_VCOMH_VENDOR) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use the controller's default display contrast level */
	if (OLED_SendCommandWithParam(dev,
			                      OLED_CMD_SET_CONTRAST,
								  OLED_CONTRAST_DEFAULT) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use horizontal addressing so framebuffer data advances across columns, then pages */
	if (OLED_SendCommandWithParam(dev,
			                      OLED_CMD_SET_MEMORY_MODE,
								  OLED_MEMORY_MODE_HORIZONTAL) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use normal pixel polarity: RAM 0 = pixel off, RAM 1 = pixel on */
	if (OLED_SendCommand(dev, OLED_CMD_NORMAL_DISPLAY) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Display pixel data from GDDRAM */
	if (OLED_SendCommand(dev, OLED_CMD_DISPLAY_RAM) != HAL_OK)
	{
	    return HAL_ERROR;
	}

	/* Enable the OLED controller's internal charge pump */
	if (OLED_SendCommandWithParam(dev,
			                      OLED_CMD_CHARGE_PUMP,
								  OLED_CHARGE_PUMP_ENABLE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Enable the OLED panel after all controller settings are configured */
	if (OLED_SendCommand(dev, OLED_CMD_DISPLAY_ON) != HAL_OK)
	{
		return HAL_ERROR;
	}
		HAL_Delay(100);  /* vendor p.13: let the panel supply stabilize */

	/* Temporary bring-up test: force all OLED pixels on, ignoring GDDRAM */
	if (OLED_SendCommand(dev, OLED_CMD_ENTIRE_DISPLAY_ON) != HAL_OK)
	{
		return HAL_ERROR;
	}
		dev->fail_step = 0;
		dev->i2c_error = HAL_I2C_ERROR_NONE;

		return HAL_OK;

}
