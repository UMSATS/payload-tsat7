/*
 * flash.h
 *
 *  Created on: Jan 17, 2024
 *      Author: Jascha Petersen
 */

#ifndef HIGHLEVEL_INC_FLASH_H_
#define HIGHLEVEL_INC_FLASH_H_

#include <stdint.h>
#include <stddef.h>

/**
 * @brief   Writes data to a specified location in the current memory block in use.
 *
 * @param memory_offset Byte offset in the current block.
 * @param data          The data to write.
 * @param data_size     The number of bytes to write.
 * @return              true on success. false on error.
 */
bool Flash_Write(int memory_offset, uint8_t *data, size_t data_size);

/**
 * @brief   Reads data from a specified location in the current memory block in use.
 *
 * @param memory_offset Byte offset in the current block.
 * @param data_size     The number of bytes to read.
 * @param data_out      The data to read.
 * @return              true on success. false on error.
 */
bool Flash_Read(int memory_offset, size_t data_size, uint8_t *data_out);

//bool Flash_Write_LED_Status(int id, Power status);
//Power Flash_Read_LED_Status(int id);
//bool Flash_Write_Well_Temperature(int id, uint8_t temp);
//int8_t Flash_Read_Well_Temperature(int id);

#endif /* HIGHLEVEL_INC_FLASH_H_ */
