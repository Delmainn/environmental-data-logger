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
#define OLED_PRECHARGE_VENDOR			0xF1

#define OLED_CMD_SET_VCOMH				0xDB
#define OLED_VCOMH_VENDOR				0x30

#define OLED_CMD_SET_CONTRAST			0x81
#define OLED_CONTRAST_VENDOR			0x66

#define OLED_CMD_SET_MEMORY_MODE		0x20
#define OLED_MEMORY_MODE_HORIZONTAL		0x00
#define OLED_CMD_NORMAL_DISPLAY			0xA6
#define OLED_CMD_DISPLAY_RAM			0xA4
#define OLED_CMD_DISPLAY_ON				0xAF


#define OLED_CMD_ENTIRE_DISPLAY_ON		0xA5

#define OLED_DATA_CHUNK_SIZE			16

#define OLED_CMD_SET_COLUMN_ADDRESS		0x21
#define OLED_CMD_SET_PAGE_ADDRESS		0x22

#define OLED_COLUMN_START				0x00
#define OLED_COLUMN_END					0x7F
#define OLED_PAGE_START					0x00
#define	OLED_PAGE_END					0x07

#define OLED_GDDRAM_SIZE				1024
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
 * Sends display data bytes to the OLED GDDRAM
 */
static HAL_StatusTypeDef OLED_SendData(OLED *dev,
										uint8_t *data,
										uint16_t size)
{
	uint8_t packet[OLED_DATA_CHUNK_SIZE + 1];

	while (size > 0)
	{
	uint16_t chunk_size;

	if (size > OLED_DATA_CHUNK_SIZE)
	{
		chunk_size = OLED_DATA_CHUNK_SIZE;
	} else{
		chunk_size = size;
	}

	/* GDDRAM data control byte */
	packet[0] = 0x40;

	/* Copy display bytes after the control byte */
	for (uint16_t i = 0; i < chunk_size; i ++)
	{
		packet[i + 1] = data[i];
	}

	if (HAL_I2C_Master_Transmit(
		dev->i2cHandle,
		OLED_I2C_ADDR,
		packet,
		chunk_size + 1,
		100) != HAL_OK)
	{
		return HAL_ERROR;
	}

	data += chunk_size;
	size -= chunk_size;

	}
	return HAL_OK;

}

/*
 * Sends a command byte followed by two parameter bytes in the same I2C transaction
 */
static HAL_StatusTypeDef OLED_SendCommandWithTwoParams(
	OLED *dev,
	uint8_t command,
	uint8_t param1,
	uint8_t param2)
{
	uint8_t packet[4];
	packet[0] = 0x00;
	packet[1] = command;
	packet[2] = param1;
	packet[3] = param2;

	return HAL_I2C_Master_Transmit(
		dev->i2cHandle,
		OLED_I2C_ADDR,
		packet,
		4,
		100
	);
}

/*
 * Clears the OLED display by writing zeros to the full GDDRAM
 */

OLED_Status OLED_Clear(OLED *dev)
{
	return OLED_Fill(dev, 0x00);
}

/*
 * Fills the OLED GDDRAM with a repeated byte pattern
 */
OLED_Status OLED_Fill(OLED *dev, uint8_t pattern)
{
	/* Set the active GDDRAM column range to the full display width */
	if (OLED_SendCommandWithTwoParams(
			dev,
			OLED_CMD_SET_COLUMN_ADDRESS,
			OLED_COLUMN_START,
			OLED_COLUMN_END) != HAL_OK)
	{
		dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
		return OLED_ERROR_I2C;
	}

	/* Set the active GDDRAM page range to the full display height */
	if (OLED_SendCommandWithTwoParams(
			dev,
			OLED_CMD_SET_PAGE_ADDRESS,
			OLED_PAGE_START,
			OLED_PAGE_END) != HAL_OK)
	{
		dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
		return OLED_ERROR_I2C;
	}
	uint8_t fill_data[OLED_DATA_CHUNK_SIZE];

	/* Fill the transmit buffer with the requested GDDRAM pattern */
	for (uint16_t i = 0; i < OLED_DATA_CHUNK_SIZE; i++)
	{
		fill_data[i] = pattern;
	}

	/* Fill the full GDDRAM in 16-byte blocks */
	for (uint16_t i = 0; i < OLED_GDDRAM_SIZE; i += OLED_DATA_CHUNK_SIZE)
	{
		if (OLED_SendData(dev, fill_data, OLED_DATA_CHUNK_SIZE) != HAL_OK)
		{
			dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
			return OLED_ERROR_I2C;
		}
	}

	dev->i2c_error = HAL_I2C_ERROR_NONE;
	return OLED_OK;
}

/*
 * Initializes the OLED controller and associate it with an STM32 I2C peripheral
 */
OLED_Status OLED_Initialise(OLED *dev, I2C_HandleTypeDef *i2cHandle)
{
	if ((dev == NULL) || (i2cHandle == NULL))
	{
		return OLED_ERROR_INVALID_ARG;
	}
	dev->i2cHandle = i2cHandle;
	dev->failed_step= OLED_INIT_STEP_NONE;
	dev->i2c_error = HAL_I2C_ERROR_NONE;

	/* Keep display off while controller settings are configured */
	dev->failed_step = OLED_INIT_STEP_DISPLAY_OFF;

	if (OLED_SendCommand(dev, OLED_CMD_DISPLAY_OFF) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Use the controller's reset/default display clock settings */
	dev->failed_step = OLED_INIT_STEP_DISPLAY_CLOCK;

	if (OLED_SendCommandWithParam(dev,
			                      OLED_CMD_SET_DISPLAY_CLOCK,
								  OLED_DISPLAY_CLOCK_DEFAULT) != HAL_OK)
	{
		dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
		return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Configure panel for 64 active rows */
	dev->failed_step = OLED_INIT_STEP_MULTIPLEX;

	if (OLED_SendCommandWithParam(dev,
	                              OLED_CMD_SET_MULTIPLEX,
	                              OLED_MUX_RATIO_64) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Align display RAM with the top physical row */
	dev->failed_step = OLED_INIT_STEP_DISPLAY_OFFSET;

	if (OLED_SendCommandWithParam(dev,
	                              OLED_CMD_SET_DISPLAY_OFFSET,
	                              OLED_DISPLAY_OFFSET_ZERO) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Use display RAM line 0 as the top visible row */
	dev->failed_step = OLED_INIT_STEP_START_LINE;

	if (OLED_SendCommand(dev, OLED_CMD_SET_START_LINE_0) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Map display RAM horizontally to match the panel's physical orientation */
	dev->failed_step = OLED_INIT_STEP_SEGMENT_REMAP;

	if (OLED_SendCommand(dev, OLED_CMD_SEGMENT_REMAP) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Scan COM lines in reverse order to match the panel's vertical orientation */
	dev->failed_step = OLED_INIT_STEP_COM_SCAN;

	if (OLED_SendCommand(dev, OLED_CMD_COM_SCAN_REMAP) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Configure COM pin hardware for the 128x64 panel layout */
	dev->failed_step = OLED_INIT_STEP_COM_PINS;

	if (OLED_SendCommandWithParam(dev,
	                              OLED_CMD_SET_COM_PINS,
	                              OLED_COM_PINS_CONFIG) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Use the module vendor's recommended display contrast level */
	dev->failed_step = OLED_INIT_STEP_CONTRAST;

	if (OLED_SendCommandWithParam(dev,
	                              OLED_CMD_SET_CONTRAST,
	                              OLED_CONTRAST_VENDOR) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Use the module vendor's recommended OLED pre-charge timing */
	dev->failed_step = OLED_INIT_STEP_PRECHARGE;

	if (OLED_SendCommandWithParam(dev,
	                              OLED_CMD_SET_PRECHARGE,
	                              OLED_PRECHARGE_VENDOR) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Use the module vendor's recommended VCOMH de-select level */
	dev->failed_step = OLED_INIT_STEP_VCOMH;

	if (OLED_SendCommandWithParam(dev,
	                              OLED_CMD_SET_VCOMH,
	                              OLED_VCOMH_VENDOR) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Use horizontal addressing so framebuffer data advances across columns, then pages */
	dev->failed_step = OLED_INIT_STEP_MEMORY_MODE;

	if (OLED_SendCommandWithParam(dev,
	                              OLED_CMD_SET_MEMORY_MODE,
	                              OLED_MEMORY_MODE_HORIZONTAL) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Display pixel data from GDDRAM */
	dev->failed_step = OLED_INIT_STEP_DISPLAY_RAM;

	if (OLED_SendCommand(dev, OLED_CMD_DISPLAY_RAM) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Use normal pixel polarity: RAM 0 = pixel off, RAM 1 = pixel on */
	dev->failed_step = OLED_INIT_STEP_NORMAL_DISPLAY;

	if (OLED_SendCommand(dev, OLED_CMD_NORMAL_DISPLAY) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Enable the OLED controller's internal charge pump */
	dev->failed_step = OLED_INIT_STEP_CHARGE_PUMP;

	if (OLED_SendCommandWithParam(dev,
	                              OLED_CMD_CHARGE_PUMP,
	                              OLED_CHARGE_PUMP_ENABLE) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	/* Enable the OLED panel after all controller settings are configured */
	dev->failed_step = OLED_INIT_STEP_DISPLAY_ON;

	if (OLED_SendCommand(dev, OLED_CMD_DISPLAY_ON) != HAL_OK)
	{
	    dev->i2c_error = HAL_I2C_GetError(dev->i2cHandle);
	    return OLED_ERROR_I2C;
	}

	dev->failed_step = OLED_INIT_STEP_NONE;

	HAL_Delay(100);  /* vendor p.13: let the panel supply stabilize */

	dev->i2c_error = HAL_I2C_ERROR_NONE;

	return OLED_OK;

}
