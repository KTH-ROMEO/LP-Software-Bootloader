#include "FRAM.h"

// CRC16-CCITT
static uint16_t CRC16_byte(uint16_t crcValue, uint8_t newByte) {
	uint8_t i;

	for (i = 0; i < 8; i++) {
		if (((crcValue & 0x8000) >> 8) ^ (newByte & 0x80)){
			crcValue = (crcValue << 1)  ^ 0x1021;
		}else{
			crcValue = (crcValue << 1);
		}
		newByte <<= 1;
	}

	return crcValue;
}

uint16_t Calc_CRC16(uint8_t* data, uint16_t length) {
    uint16_t CRCvalue = 0xFFFF;
    for(uint16_t i = 0; i < length; i++) {
        CRCvalue = CRC16_byte(CRCvalue, data[i]);
    }

    return CRCvalue;
}

void unpack_metadata(Metadata_Struct* metadata, uint8_t* metadata_raw)
{
	metadata->crc = ((uint16_t)metadata_raw[0] << 8) | metadata_raw[1];
	metadata->new_metadata = metadata_raw[2];
	metadata->boot_feedback = metadata_raw[3];
	metadata->image_index = metadata_raw[4];
	metadata->boot_counter = metadata_raw[5];
	metadata->error_code = metadata_raw[6];
}

void pack_metadata(Metadata_Struct* metadata, uint8_t* metadata_raw)
{
    metadata_raw[2] = metadata->new_metadata;
    metadata_raw[3] = metadata->boot_feedback;
    metadata_raw[4] = metadata->image_index;
    metadata_raw[5] = metadata->boot_counter;
    metadata_raw[6] = metadata->error_code;
}

HAL_StatusTypeDef writeFRAM(uint16_t addr, uint8_t* data, uint32_t size) {
	return HAL_I2C_Mem_Write(&hi2c4, FRAM_I2C_ADDR, addr, 2, data, size, 50);
}

HAL_StatusTypeDef readFRAM(uint16_t addr, uint8_t* buf, uint32_t size) {
	return HAL_I2C_Mem_Read(&hi2c4, FRAM_I2C_ADDR_READ, addr, 2, buf, size, 50);
}

