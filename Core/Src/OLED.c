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
#define OLED_CMD_CHARGE_PUMP			0x8D
#define OLED_CHARGE_PUMP_ENABLE			0x14
#define OLED_CMD_SET_DISPLAY_CLOCK		0xD5
#define OLED_DISPLAY_CLOCK_DEFAULT		0x80
#define OLED_CMD_SET_PRECHARGE		 	0xD9
#define OLED_PRECHARGE_DEFAULT			0x22
#define OLED_CMD_SET_VCOMH				0xDB
#define OLED_VCOMH_DEFAULT				0x20
#define OLED_CMD_SET_CONTRAST			0x81
#define OLED_CONTRAST_DEFAULT			0x7F
#define OLED_CMD_SET_MEMORY_MODE		0x20
#define OLED_MEMORY_MODE_HORIZONTAL		0x00
#define OLED_CMD_NORMAL_DISPLAY			0xA6
#define OLED_CMD_DISPLAY_RAM			0xA4
#define OLED_CMD_DISPLAY_ON				0xAF
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
 * Initializes the OLED controller and associate it with an STM32 I2C peripheral
 */
HAL_StatusTypeDef OLED_Initialise(OLED *dev, I2C_HandleTypeDef *i2cHandle)
{
	dev->i2cHandle = i2cHandle;

	/* Keep display off while controller settings are configured */
	if (OLED_SendCommand(dev, OLED_CMD_DISPLAY_OFF)!= HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Configure panel for 64 active rows */
	if (OLED_SendCommand(dev, OLED_CMD_SET_MULTIPLEX) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (OLED_SendCommand(dev, OLED_MUX_RATIO_64) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Align display RAM with the top physical row */
	if (OLED_SendCommand(dev, OLED_CMD_SET_DISPLAY_OFFSET) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (OLED_SendCommand(dev, OLED_DISPLAY_OFFSET_ZERO) != HAL_OK)
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
	if (OLED_SendCommand(dev, OLED_CMD_SET_COM_PINS) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (OLED_SendCommand(dev, OLED_COM_PINS_CONFIG) != HAL_OK)
	{
		return HAL_ERROR;
	}
	/* Enable the OLED controller's internal charge pump */
	if (OLED_SendCommand(dev, OLED_CMD_CHARGE_PUMP) != HAL_OK)
	{
		return HAL_ERROR;
	}
	if (OLED_SendCommand(dev, OLED_CHARGE_PUMP_ENABLE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use the controller's reset/default display clock settings */
	if (OLED_SendCommand(dev, OLED_CMD_SET_DISPLAY_CLOCK) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (OLED_SendCommand(dev, OLED_DISPLAY_CLOCK_DEFAULT) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use the controller's default OLED pre-charge timing */
	if (OLED_SendCommand(dev, OLED_CMD_SET_PRECHARGE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (OLED_SendCommand(dev, OLED_PRECHARGE_DEFAULT) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use the controller's default VCOMH de-select level */
	if (OLED_SendCommand(dev, OLED_CMD_SET_VCOMH) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (OLED_SendCommand(dev, OLED_VCOMH_DEFAULT) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use the controller's default display contrast level */
	if (OLED_SendCommand(dev, OLED_CMD_SET_CONTRAST) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (OLED_SendCommand(dev, OLED_CONTRAST_DEFAULT) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use horizontal addressing so framebuffer data advances across columns, then pages */
	if (OLED_SendCommand(dev, OLED_CMD_SET_MEMORY_MODE) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (OLED_SendCommand(dev, OLED_MEMORY_MODE_HORIZONTAL) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Use normal pixel polarity: RAM 0 = pixel off, RAM 1 = pixel on */
	if (OLED_SendCommand(dev, OLED_CMD_NORMAL_DISPLAY) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Display pixel data from GDDRAM instead of forcing all pixels on */
	if (OLED_SendCommand(dev, OLED_CMD_DISPLAY_RAM) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* Enable the OLED panel after all controller settings are configured */
	if (OLED_SendCommand(dev, OLED_CMD_DISPLAY_ON) != HAL_OK)
	{
		return HAL_ERROR;
	}

		return HAL_OK;

}
