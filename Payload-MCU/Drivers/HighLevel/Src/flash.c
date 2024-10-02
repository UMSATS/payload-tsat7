/*
 * flash.c
 *
 *  Created on: Jan 17, 2024
 *      Author: Jascha Petersen
 */

#include "main.h"
#include "power.h"

#include <stdint.h>
#include <stdbool.h>

#define BLOCK_COUNTER_ADDRESS 0x08000000
#define BLOCK_START_ADDRESS   0x08000001
#define BLOCK_END_ADDRESS     0x08100000
#define BLOCK_SIZE 32

size_t current_block = 0;
uint8_t block_buffer[BLOCK_SIZE];

// output ports: 2 bytes
// temperatures: 16 bytes
// active states: 16 bytes

// Boundaries of the available flash memory
#define FLASH_BOUNDARY_MIN_ADDR 		0x08000000
#define FLASH_BOUNDARY_MAX_ADDR 		0x08080000

#define FLASH_WELL_TEMP_STRUCT_ADDR  	0x08040000
#define FLASH_LED_STATUS_STRUCT_ADDR	0x08040100

typedef struct WELL_TEMP_STRUCT {
	/*
	 * Array of 16 ints with temperature goal in celsius for each well.
	 * -1 means that well is OFF and no temperature regulation is happening.
	 * */
	int8_t temperatures[16];

} Flash_Well_Temperatures;

typedef struct LED_STATUS_STRUCT {
	/*
	 * Array of 16 uints. 1 is LED on, 0 is LED off.
	 * These are separate rather than 1 uint16 so they can be invidiually addressed.
	 */
	uint8_t leds[16];

} Flash_LED_Status;

/*
 * Takes the raw data from each port of a TCA9539 and writes it to flash
 */
bool Flash_Write_LED_Status(int id, uint8_t d1p1, uint8_t d1p2, uint8_t d2p1, uint8_t d2p2) {

	Flash_LED_Status leds;

	HAL_FLASH_Unlock();

//	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, FLASH_LED_STATUS_STRUCT_ADDR + (id * sizeof(uint8_t)), (uint8_t)status);

	HAL_FLASH_Lock();

	return true;
}

Power Flash_Read_LED_Status(int id) {

	Power* addr;
	addr = (Power*) (FLASH_LED_STATUS_STRUCT_ADDR + (id * sizeof(Power)));

	return *addr;
}

bool Flash_Write_Well_Temperature(int id, uint8_t temp) {

	HAL_FLASH_Unlock();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, FLASH_WELL_TEMP_STRUCT_ADDR + (id * sizeof(int8_t)), temp);
	HAL_FLASH_Lock();

	if (HAL_FLASH_GetError() != HAL_FLASH_ERROR_NONE) {
	    // Handle error
	}

	return true;
}

int8_t Flash_Read_Well_Temperature(int id) {

	uint8_t* addr;
	addr = (uint8_t*) (FLASH_WELL_TEMP_STRUCT_ADDR + (id * sizeof(int8_t)));

	return *addr;

}
