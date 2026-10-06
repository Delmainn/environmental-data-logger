/*
 *
 * BME280 Environmental Sensor, Temperature, Humidity, Barometric Pressure I2C Driver
 *
 * Author:	Gabriel M
 * Created:	6 October 2026
 *
 */

#ifndef BME280_I2C_DRIVER_H
#define BME280_I2C_DRIVER_H

#include "stm32f4xx_hal.h" /* Needed for I2C */

/*
 * DEFINES
 */
#define BME280_I2C_ADDR					(0x77 << 1)	/* Voltage high(SDO to VDDIO OR ADDR to HIGH)  --> 0x77
										                         Voltage low(SDO to GND OR ADDR to LOW) --> 0x76
										WAVESHARE WIKI & PG.32(BOSCH datasheet) */

#define BME280_CHIP_ID 					0x60
/*
 * REGISTERS (p. 25)
 */
/* Calibration registers: block 1 */
#define BME280_REG_CALIB00   			0x88
#define BME280_REG_CALIB01   			0x89
#define BME280_REG_CALIB02  			0x8A
#define BME280_REG_CALIB03   			0x8B
#define BME280_REG_CALIB04   			0x8C
#define BME280_REG_CALIB05   			0x8D
#define BME280_REG_CALIB06   			0x8E
#define BME280_REG_CALIB07   			0x8F
#define BME280_REG_CALIB08   			0x90
#define BME280_REG_CALIB09   			0x91
#define BME280_REG_CALIB10   			0x92
#define BME280_REG_CALIB11   			0x93
#define BME280_REG_CALIB12   			0x94
#define BME280_REG_CALIB13   			0x95
#define BME280_REG_CALIB14  	 		0x96
#define BME280_REG_CALIB15   			0x97
#define BME280_REG_CALIB16   			0x98
#define BME280_REG_CALIB17   			0x99
#define BME280_REG_CALIB18   			0x9A
#define BME280_REG_CALIB19   			0x9B
#define BME280_REG_CALIB20   			0x9C
#define BME280_REG_CALIB21   			0x9D
#define BME280_REG_CALIB22   			0x9E
#define BME280_REG_CALIB23   			0x9F
#define BME280_REG_CALIB24   			0xA0
#define BME280_REG_CALIB25   			0xA1

/* Identification and reset */
#define BME280_REG_ID 					0xD0
#define BME280_REG_RESET 				0xE0

/* Calibration registers: block 2*/

#define BME280_REG_CALIB26   			0xE1
#define BME280_REG_CALIB27   			0xE2
#define BME280_REG_CALIB28   			0xE3
#define BME280_REG_CALIB29   			0xE4
#define BME280_REG_CALIB30   			0xE5
#define BME280_REG_CALIB31   			0xE6
#define BME280_REG_CALIB32   			0xE7
#define BME280_REG_CALIB33   			0xE8
#define BME280_REG_CALIB34   			0xE9
#define BME280_REG_CALIB35   			0xEA
#define BME280_REG_CALIB36   			0xEB
#define BME280_REG_CALIB37   			0xEC
#define BME280_REG_CALIB38   			0xED
#define BME280_REG_CALIB39   			0xEE
#define BME280_REG_CALIB40   			0xEF
#define BME280_REG_CALIB41   			0xF0

/* Control and status registers */
#define BME280_REG_CTRL_HUM 			0xF2
#define BME280_REG_STATUS 				0xF3
#define BME280_REG_CTRL_MEAS 			0xF4
#define BME280_REG_CONFIG				0xF5

/* Pressure data registers */
#define BME280_REG_PRESS_MSB 			0xF7
#define BME280_REG_PRESS_LSB 			0xF8
#define BME280_REG_PRESS_XLSB			0xF9

/* Temperature data registers */
#define BME280_REG_TEMP_MSB 			0xFA
#define BME280_REG_TEMP_LSB 			0xFB
#define BME280_REG_TEMP_XLSB 			0xFC

/* Humidity data registers */
#define BME280_REG_HUM_MSB 				0xFD
#define BME280_REG_HUM_LSB				0xFE

/*
 * ERROR STATUS
 */
typedef enum {
	BME280_OK = 0,
	BME280_ERROR_I2C,
	BME280_ERROR_CHIP_ID,
	BME280_ERROR_CONFIG,
	BME280_ERROR_CALIBRATION,

} BME280_Status;
/*
 * SENSOR STRUCT
 */

typedef struct {

	/* I2c Handle */
	I2C_HandleTypeDef *i2cHandle;

	/* Factory calibration coefficients */
	uint16_t dig_T1;
	int16_t  dig_T2;
	int16_t  dig_T3;

	uint16_t dig_P1;
	int16_t  dig_P2;
	int16_t  dig_P3;
	int16_t  dig_P4;
	int16_t  dig_P5;
	int16_t  dig_P6;
	int16_t  dig_P7;
	int16_t  dig_P8;
	int16_t  dig_P9;

	uint8_t  dig_H1;
	int16_t  dig_H2;
	uint8_t  dig_H3;
	int16_t  dig_H4;
	int16_t  dig_H5;
	int8_t   dig_H6;

	/* Temperature */
	float temp_C;

	/* Humidity */
	float humidity_RH;

	/* Barometric Pressure */
	float pressure_Pa;
} BME280;

/*
 * INITIALISATION
 */
BME280_Status BME280_Initialise(
	BME280 *dev,
	I2C_HandleTypeDef *i2cHandle
);

/*
 * DATA ACQUISITION
 */

HAL_StatusTypeDef BME280_ReadMeasurements(BME280 *dev);

/*
 * LOW-LEVEL FUNCTIONS
 */
HAL_StatusTypeDef BME280_ReadRegister(
	BME280 *dev,
	uint8_t reg,
	uint8_t *data
);

HAL_StatusTypeDef BME280_ReadRegisters(
	BME280 *dev,
	uint8_t reg,
	uint8_t *data,
	uint8_t length
);

HAL_StatusTypeDef BME280_WriteRegister(
	BME280 *dev,
	uint8_t reg,
	uint8_t *data
);


#endif /* BME280_I2C_DRIVER_H */

