/*
 * FRAM.h
 *
 *  Created on: May 17, 2025
 *      Author: sergi
 */

#ifndef INC_FRAM_H_
#define INC_FRAM_H_

#include "main.h"

#define FRAM_I2C_ADDR 0xA0
#define FRAM_I2C_ADDR_READ 0xA1

#define FRAM_SWEEP_TABLE_SECTION_START 0x0FC0
#define FRAM_FINAL_ADDRESS 0x1FFF

extern I2C_HandleTypeDef hi2c4;

typedef enum {
	BOOT_NEW_IMAGE = 0,
	BOOTED_OK = 1
} Boot_Feedback;

typedef enum {
	NO = 0,
	YES = 1
} New_Metadata;

typedef enum {
	NO_BOOT_ERROR = 0,
	CORRUPTED_METADATA_ERROR = 1,
	WRONG_METADATA_ERROR = 2,
	HARDWARE_RESET_ERROR = 3,
	BOOT_FAILURE_PREVIOUS_IMAGE_ERROR = 4,
	BOOT_FAILURE_CURRENT_IMAGE_ERROR = 5,
}Error_Code;

// The metadata structre that is save into FRAM. 7 bytes long in total
typedef struct{
	uint16_t crc;
	uint8_t new_metadata;
	uint8_t boot_feedback;
	uint8_t image_index;
	uint8_t boot_counter;
	uint8_t error_code;
} Metadata_Struct;

HAL_StatusTypeDef writeFRAM(uint16_t addr, uint8_t* data, uint32_t size);
HAL_StatusTypeDef readFRAM(uint16_t addr, uint8_t* buf, uint32_t size);
uint16_t Calc_CRC16(uint8_t* data, uint16_t length);
void unpack_metadata(Metadata_Struct* metadata, uint8_t* metadataraw);
void pack_metadata(Metadata_Struct* metadata, uint8_t* metadata_raw);

#endif /* INC_FRAM_H_ */
