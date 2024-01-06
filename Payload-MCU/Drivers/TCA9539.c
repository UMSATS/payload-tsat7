/*
 * TCA9539.c
 *
 *  Created on: Dec 18, 2023
 *      Author: Jacob Petersen
 *
 *  Purpose: this is the driver file for the TCA9539 IO expander IC.
 */

#include "i2c.h"

#define IO_EX_ADDR_1 0x74 	// I2C address of expander 1 (wells 0-7)
#define IO_EX_ADDR_2 0x75	// I2C address of expander 2 (wells 8-15)

#define OUTPUT_PORT_0_ADDR 		0x02
#define OUTPUT_PORT_1_ADDR 		0x03
#define CONFIG_PORT_0_ADDR		0x06
#define CONFIG_PORT_1_ADDR		0x07

/*
 * IO expander initialization function.
 * This resets all of the outputs to 0.
 * The following needs to be done when initializing:
 *  - All pins need to be configured as outputs by setting the configuration registers
 *  - All output pins need to be set to LOW (off). They are HIGH by default.
 *  - This needs to be done for both ICs.
 */
void TCA9539_Init() {

	uint8_t message_buffer[2];

	// 0s are written to make all pins 0utput, and then to clear the output of those pins
	message_buffer[1] = 0;

	// transmit to device 1 config port 0
	message_buffer[0] = CONFIG_PORT_0_ADDR;

	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_1, message_buffer, 2, 100);

	// transmit to device 1 config port 1 (the message buffer address is changed but the 0 is unchanged)
	message_buffer[0] = CONFIG_PORT_1_ADDR;
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_1, message_buffer, 2, 100);

	// transmit to device 2 config port 0
	message_buffer[0] = CONFIG_PORT_0_ADDR;
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_2, message_buffer, 2, 100);

	// transmit to device 2 config port 1
	message_buffer[0] = CONFIG_PORT_1_ADDR;
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_2, message_buffer, 2, 100);

	// transmit to device 1 output port 0
	message_buffer[0] = OUTPUT_PORT_0_ADDR;
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_1, message_buffer, 2, 100);

	// transmit to device 1 output port 1
	message_buffer[0] = OUTPUT_PORT_1_ADDR;
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_1, message_buffer, 2, 100);

	// transmit to device 2 output port 0
	message_buffer[0] = OUTPUT_PORT_0_ADDR;
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_2, message_buffer, 2, 100);

	// transmit to device 2 output port 1
	message_buffer[0] = OUTPUT_PORT_1_ADDR;
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_2, message_buffer, 2, 100);

}

/*
 * Sets a specific pin "pin" in port "port" on device "device" to "value".
 */
int TCA9539_Set_Pin(int device, int port, int pin, int value) {
	// device 1 = wells 0-7, device 2 = wells 8-15
	if ((device == 1 || device == 2) && (port == 0 || port == 1)
			&& (pin >= 0 && pin < 8) && (value == 0 || value == 1)) {

		uint8_t message_buffer[2];
		uint8_t output_register;

		uint8_t device_addr;

		if (device == 1) {
			device_addr = IO_EX_ADDR_1;
		} else if (device == 2) {
			device_addr = IO_EX_ADDR_2;
		}

		if (port == 0) {
			message_buffer[0] = OUTPUT_PORT_0_ADDR;
		} else if (port == 1) {
			message_buffer[0] = OUTPUT_PORT_1_ADDR;
		}

		// indicate to the device that we want to get the status of the output port and then receive it
		HAL_I2C_Master_Transmit(&hi2c1, device_addr, message_buffer, 1, 100);
		HAL_I2C_Master_Receive(&hi2c1, device_addr, &output_register, 1, 100);

		// modify the output register
		message_buffer[1] = output_register;

		if (value == 0) {
			message_buffer[1] = (1 << pin) | message_buffer[1]; // set position "pin" to 1
		} else if (value == 1) {
			message_buffer[1] = ~(1 << pin) & message_buffer[1]; // set position "pin" to 0
		}

		// transmit modified output register to the device
		HAL_I2C_Master_Transmit(&hi2c1, device_addr, message_buffer, 2, 100);

		return 1;
	} else {
		return -1;
	}

}

/*
 * Gets the value of pin "pin" on port "port" on device "device" and returns it.
 */
int TCA9539_Get_Pin(int device, int port, int pin) {

	if ((device == 1 || device == 2) && (port == 0 || port == 1) && (pin >= 0 && pin < 8)) {

		uint8_t message_buffer;
		uint8_t device_addr;
		uint8_t output_register;

		if (device == 1) {
			device_addr = IO_EX_ADDR_1;
		} else if (device == 2) {
			device_addr = IO_EX_ADDR_2;
		}

		if (port == 0) {
			message_buffer = OUTPUT_PORT_0_ADDR;
		} else if (port == 1) {
			message_buffer = OUTPUT_PORT_1_ADDR;
		}

		// indicate to the device that we want to get the status of the output port and then receive it
		HAL_I2C_Master_Transmit(&hi2c1, device_addr, &message_buffer, 1, 100);
		HAL_I2C_Master_Receive(&hi2c1, device_addr, &output_register, 1, 100);

		if ((output_register & (1 << pin)) == 0) {
			return 0;
		} else {
			return 1;
		}

	} else {
		return -1;
	}
}

/*
 * Clears all the pins on every IO expander.
 */
void TCA9539_Clear_Pins() {

	uint8_t message_buffer[2];
	message_buffer[1] = 0;

	// clear port 0 on both devices
	message_buffer[0] = OUTPUT_PORT_0_ADDR;
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_1, message_buffer, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_2, message_buffer, 2, 100);

	// clear port 1 on both devices
	message_buffer[0] = OUTPUT_PORT_1_ADDR;
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_1, message_buffer, 2, 100);
	HAL_I2C_Master_Transmit(&hi2c1, IO_EX_ADDR_2, message_buffer, 2, 100);
}
