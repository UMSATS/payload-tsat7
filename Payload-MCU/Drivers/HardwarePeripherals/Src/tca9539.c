/*
 * tca9539.c
 *
 *  Created on: Dec 18, 2023
 *      Author: Jacob Petersen
 *
 *  Purpose: this is the driver file for the TCA9539 IO expander IC.
 */

#include "tca9539.h"
#include "assert.h"

#include "i2c.h"

const uint32_t TIMEOUT = 100;

// I2C addresses of each expander.
const uint8_t IO_EXPANDER_I2C_ADDRESSES[] = {
		0x74, 	// IO Expander 1 (Wells 1-8)
		0x75	// IO Expander 2 (Wells 9-16)
};

typedef enum {
	CONFIG_PORT_0 = 0,
	CONFIG_PORT_1,
	OUTPUT_PORT_0,
	OUTPUT_PORT_1
} PortID;

// internal addresses for port registers.
const uint8_t PORT_ADDRESSES[] = {
		0x06,   // CONFIG PORT 0
		0x07,   // CONFIG PORT 1
		0x02,   // OUTPUT PORT 0
		0x03,   // OUTPUT PORT 1
};

static void get_port(IOExpanderID device, PortID port);
static void set_port(IOExpanderID device, PortID port, uint8_t bitmap);
static bool check_params(IOExpanderID device, int port, int pin);

void TCA9539_Init()
{
	// configure all pins to be outputs.
	set_port(IO_EXPANDER_1, CONFIG_PORT_0, 0x00);
	set_port(IO_EXPANDER_1, CONFIG_PORT_1, 0x00);
	set_port(IO_EXPANDER_2, CONFIG_PORT_0, 0x00);
	set_port(IO_EXPANDER_2, CONFIG_PORT_1, 0x00);

	// clear outputs.
	TCA9539_Clear_Pins();
}

bool TCA9539_Set_Pin(IOExpanderID device, int port, int pin, bool set) {
	if (!check_params(device, port, pin))
		return false;

	// get the register for the corresponding port.
	PortID output_port = (port == 0) ? OUTPUT_PORT_0 : OUTPUT_PORT_1;
	uint8_t output_register = get_port(device, output_port);

	// modify the register.
	if (set)
		output_register &= ~(1 << pin); // set position "pin" to 0.
	else
		output_register |=  (1 << pin); // set position "pin" to 1.

	// transmit modified output register to the device.
	set_port(device, output_port, output_register);

	return true;
}

bool TCA9539_Get_Pin(IOExpanderID device, int port, int pin) {
	if (!check_params(device, port, pin))
		return -1;

	// get the register for the corresponding port.
	PortID output_port = (port == 0) ? OUTPUT_PORT_0 : OUTPUT_PORT_1;
	uint8_t output_register = get_port(device, output_port);

	// extract the bit corresponding to the given pin.
	uint8_t mask = output_register & (1 << pin);

	return mask ? true : false;
}

void TCA9539_Clear_Pins()
{
	// clear all outputs for both devices.
	set_port(IO_EXPANDER_1, OUTPUT_PORT_0, 0x00);
	set_port(IO_EXPANDER_1, OUTPUT_PORT_1, 0x00);
	set_port(IO_EXPANDER_2, OUTPUT_PORT_0, 0x00);
	set_port(IO_EXPANDER_2, OUTPUT_PORT_1, 0x00);
}

static uint8_t get_port(IOExpanderID device, PortID port)
{
	uint8_t port_register;

	uint8_t i2c_address = IO_EXPANDER_I2C_ADDRESSES[device];
	uint8_t msg = PORT_ADDRESSES[port];

	// indicate to the device which port we want.
	HAL_I2C_Master_Transmit(&hi2c1, i2c_address, &msg, sizeof(msg), TIMEOUT);

	// now receive the current state of the register.
	HAL_I2C_Master_Receive(&hi2c1, i2c_address, &port_register, sizeof(port_register), TIMEOUT);

	return port_register;
}

static void set_port(IOExpanderID device, PortID port, uint8_t bitmap)
{
	uint8_t i2c_address = IO_EXPANDER_I2C_ADDRESSES[device];
	uint8_t msg[] = { PORT_ADDRESSES[port], bitmap };

	HAL_I2C_Master_Transmit(&hi2c1, i2c_address, msg, sizeof(msg), TIMEOUT);
}

static bool check_params(IOExpanderID device, int port, int pin)
{
	ASSERT(device == IO_EXPANDER_1 || device == IO_EXPANDER_2, "invalid device: %d", device);
	ASSERT(port == PORT_1 || port == PORT_2, "invalid port: %d", port);
	ASSERT(pin >= 0 && pin <= 7, "invalid pin: %d", pin);

	if (device != IO_EXPANDER_1 && device != IO_EXPANDER_2)
	{
		LOG_ERRROR("invalid device: %d", device);
		return false;
	}

	if (port < 0 || port > 1)
	{
		LOG_ERRROR("invalid port: %d", port);
		return false;
	}

	if (pin < 0 || pin > 7)
	{
		LOG_ERRROR("invalid pin: %d", pin);
		return false;
	}

	// all checks passed.
	return true;
}
