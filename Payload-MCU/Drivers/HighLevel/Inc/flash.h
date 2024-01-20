/*
 * flash.h
 *
 *  Created on: Jan 17, 2024
 *      Author: Jascha Petersen
 */

#ifndef HIGHLEVEL_INC_FLASH_H_
#define HIGHLEVEL_INC_FLASH_H_

bool Flash_Write_LED_Status(int id, Power status);
Power Flash_Read_LED_Status(int id);
bool Flash_Write_Well_Temperature(int id, uint8_t temp);
int8_t Flash_Read_Well_Temperature(int id);

#endif /* HIGHLEVEL_INC_FLASH_H_ */
