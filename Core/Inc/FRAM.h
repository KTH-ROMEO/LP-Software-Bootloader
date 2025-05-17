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

#define FRAM_BOOT_CNT 0x0000
#define FRAM_FFU_ID 0x0002
#define FRAM_UNIT_ID 0x0003
#define FRAM_GS_ID_FPGA 0x0004
#define FRAM_GS_ID_UC 0x0005
#define FRAM_VHF_TIME_SLOT 0x0006

#define FRAM_SWEEP_TABLE_SECTION_START 0x0FC0
#define FRAM_SWEEP_TABLE_FOOTER_SIZE    8 // bytes
#define FRAM_SWEEP_TABLE_SIZE   512 + FRAM_SWEEP_TABLE_FOOTER_SIZE  // bytes


#define FRAM_BOOT_METADATA      FRAM_SWEEP_TABLE_SECTION_START
#define FRAM_SWEEP_TABLE_0      FRAM_SWEEP_TABLE_SECTION_START + (FRAM_SWEEP_TABLE_SIZE * 1)
#define FRAM_SWEEP_TABLE_1      FRAM_SWEEP_TABLE_SECTION_START + (FRAM_SWEEP_TABLE_SIZE * 2)
#define FRAM_SWEEP_TABLE_2      FRAM_SWEEP_TABLE_SECTION_START + (FRAM_SWEEP_TABLE_SIZE * 3)
#define FRAM_SWEEP_TABLE_3      FRAM_SWEEP_TABLE_SECTION_START + (FRAM_SWEEP_TABLE_SIZE * 4)
#define FRAM_SWEEP_TABLE_4      FRAM_SWEEP_TABLE_SECTION_START + (FRAM_SWEEP_TABLE_SIZE * 5)
#define FRAM_SWEEP_TABLE_5      FRAM_SWEEP_TABLE_SECTION_START + (FRAM_SWEEP_TABLE_SIZE * 6)
#define FRAM_SWEEP_TABLE_6      FRAM_SWEEP_TABLE_SECTION_START + (FRAM_SWEEP_TABLE_SIZE * 7)


#define FRAM_FINAL_ADDRESS 0x1FFF

extern I2C_HandleTypeDef hi2c4;

//HAL_StatusTypeDef writeFRAM_DMA(uint16_t addr, uint8_t* data, uint32_t size);
//HAL_StatusTypeDef readFRAM_DMA(uint16_t addr, uint8_t* buf, uint32_t size);
HAL_StatusTypeDef writeFRAM(uint16_t addr, uint8_t* data, uint32_t size);
HAL_StatusTypeDef readFRAM(uint16_t addr, uint8_t* buf, uint32_t size);

uint16_t read_sweep_table_value_FRAM(uint8_t table_id, uint8_t step_id);



#endif /* INC_FRAM_H_ */
