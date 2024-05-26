/*
 * tca9539.c
 *
 *  Created on: Dec 18, 2023
 *      Author: Jacob Petersen, Logan Furedi
 *
 *  Purpose: this is the driver file for the TCA9539 IO expander IC.
 */

#include "tca9539.h"
#include "power.h"
#include "assert.h"
#include "i2c.h"
#include "tuk/log.h"
#include "tuk/error_tracker.h"

static const uint32_t TIMEOUT = 100;

// I2C addresses of each IO expander.
static const uint8_t EXPANDER_I2C_ADDRESSES[] = {
		0x74 << 1, 	// IO Expander 1 (Wells 0-7)
		0x75 << 1 	// IO Expander 2 (Wells 8-15)
};

typedef enum {
	CONFIG_PORT_0 = 0,
	CONFIG_PORT_1,
	OUTPUT_PORT_0,
	OUTPUT_PORT_1
} PortID;

// internal addresses for port registers.
static const uint8_t PORT_ADDRESSES[] = {
		0x06,   // CONFIG PORT 0
		0x07,   // CONFIG PORT 1
		0x02,   // OUTPUT PORT 0
		0x03,   // OUTPUT PORT 1
};

static bool get_port(ExpanderID device, PortID port, uint8_t *out);
static bool set_port(ExpanderID device, PortID port, uint8_t bitmap);
static bool check_params(ExpanderID device, ExpanderPinID pin);

#define LOG_SUBJECT "TCA9539"

bool TCA9539_Init()
{
	// configure all pins to be outputs.
	if (!set_port(EXPANDER_1, CONFIG_PORT_0, 0x00)) return false;
	if (!set_port(EXPANDER_1, CONFIG_PORT_1, 0x00)) return false;
	if (!set_port(EXPANDER_2, CONFIG_PORT_0, 0x00)) return false;
	if (!set_port(EXPANDER_2, CONFIG_PORT_1, 0x00)) return false;

	// clear outputs.
	if (!TCA9539_Clear_Pins()) return false;

	return true;
}

int TCA9539_Get_Pin(ExpanderID device, ExpanderPinID pin)
{
	if (!check_params(device, pin))
		return -1;

	int port = pin / 8; // 0 or 1.

	// get the register for the corresponding port.
	uint8_t output_register;
	PortID port_id = (port == 0) ? OUTPUT_PORT_0 : OUTPUT_PORT_1;
	if (!get_port(device, port_id, &output_register))
		return -1;

	// bitmask of the bit we want.
	uint8_t mask = 1 << (pin - port*8);

	// extract the bit corresponding to the given pin.
	uint8_t bit = output_register & mask;

	return bit ? 1 : 0;
}

bool TCA9539_Set_Pin(ExpanderID device, ExpanderPinID pin, Power power)
{
	if (!check_params(device, pin))
		return false;

	int port = pin / 8; // 0 or 1.

	// get the register for the corresponding port.
	uint8_t output_register;
	PortID port_id = (port == 0) ? OUTPUT_PORT_0 : OUTPUT_PORT_1;
	if (!get_port(device, port_id, &output_register))
		return -1;

	// bitmask where the 1 is the bit we want to modify.
	uint8_t mask = 1 << (pin - port*8);

	// modify the register.
	if (power == ON)
		output_register |=  mask; // set the bit to 1.
	else
		output_register &= ~mask; // set the bit to 0.

	// transmit modified output register to the device.
	set_port(device, port_id, output_register);

	return true;
}

bool TCA9539_Clear_Pins()
{
	// clear all outputs for both devices.
	if (!set_port(EXPANDER_1, OUTPUT_PORT_0, 0x00)) return false;
	if (!set_port(EXPANDER_1, OUTPUT_PORT_1, 0x00)) return false;
	if (!set_port(EXPANDER_2, OUTPUT_PORT_0, 0x00)) return false;
	if (!set_port(EXPANDER_2, OUTPUT_PORT_1, 0x00)) return false;

	return true;
}

/**
 * @brief Gets the state of the register for the desired port.
 *
 * @param device	which device to retrieve a port from.
 * @param port		which port register to retrieve.
 * @param out		8-bit register.
 * @return 			true on success. false on error.
 */
#define OUTPUT_PORT_OFFSET 3
static bool get_port(ExpanderID device, PortID port, uint8_t *out)
{
	HAL_StatusTypeDef status;

	uint8_t i2c_address = EXPANDER_I2C_ADDRESSES[device];
	uint8_t msg = PORT_ADDRESSES[port];

	// indicate to the device which port we want.
	status = HAL_I2C_Master_Transmit(&hi2c1, i2c_address << 1, &msg, sizeof(msg), TIMEOUT);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to transmit port address 0x%02X to device %d. (I2C address: 0x%02X, HAL error code: %d)", msg, device, i2c_address, status);
		PUT_ERROR(ERROR_I2C_TRANSMIT, (uint8_t)port, (uint8_t)status); // TODO: inconsistent usage with rest of code. Do we want this?
		return false;
	}

	// now receive the current state of the register.
	uint8_t port_register;
	status = HAL_I2C_Master_Receive(&hi2c1, i2c_address << 1, &port_register, sizeof(port_register), TIMEOUT);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to get register for port %d from device %d. (I2C address: 0x%02X, HAL error code: %d)", port, device, i2c_address, status);
		PUT_ERROR(ERROR_I2C_RECEIVE, (uint8_t)port, (uint8_t)status); // TODO: inconsistent usage with rest of code. Do we want this?
		return false;
	}

	*out = port_register;

	return true;
}

/**
 * @brief Sets the state of the register for the desired port.
 *
 * @param device	which device to target.
 * @param port		which port register to modify.
 * @param bitmap	the new 8-bit register.
 * @return 			true on success. false on error.
 */
static bool set_port(ExpanderID device, PortID port, uint8_t bitmap)
{
	HAL_StatusTypeDef status;

	uint8_t i2c_address = EXPANDER_I2C_ADDRESSES[device];
	uint8_t msg[] = { PORT_ADDRESSES[port], bitmap };

	status = HAL_I2C_Master_Transmit(&hi2c1, i2c_address << 1, msg, sizeof(msg), TIMEOUT);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to transmit message { port address: 0x%02X, bitmap: 0x%02X } to device %d. (I2C address: 0x%02X, HAL error code: %d)", msg[0], msg[1], device, i2c_address, status);

		return false;
	}

//	Flash_Write(OUTPUT_PORT_OFFSET, &bitmap, 1);

	return true;
}

/**
 * @brief Ensures the device, port, and pin numbers are valid.
 *
 * @return true if valid. false otherwise.
 */
static bool check_params(ExpanderID device, ExpanderPinID pin)
{
	ASSERT(device == EXPANDER_1 || device == EXPANDER_2, "invalid device id: %d.", device);
	ASSERT(pin >= EXPANDER_PIN_0 && pin <= EXPANDER_PIN_17, "invalid pin id: %d.", pin);

	if (device != EXPANDER_1 && device != EXPANDER_2)
	{
		LOG_ERROR("invalid device: %d.", device);
		PUT_ERROR(ERROR_PLD_TCA9539_INVALID_EXPANDER_ID);
		return false;
	}

	if (pin < EXPANDER_PIN_0 || pin > EXPANDER_PIN_17)
	{
		LOG_ERROR("invalid pin: %d.", pin);
		PUT_ERROR(ERROR_PLD_TCA9539_INVALID_EXPANDER_PIN_ID);
		return false;
	}

	// all checks passed.
	return true;
}
