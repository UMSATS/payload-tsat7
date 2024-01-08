#ifndef TCA9539_H_
#define TCA9539_H_

#include <stdbool.h>

typedef enum
{
	IO_EXPANDER_1 = 0, // LEDs & heaters 0-7.
	IO_EXPANDER_2 = 1  // LEDs & heaters 8-15.
} IOExpanderID;

/**
 * @brief Performs necessary initialisation of each pin.
 */
void TCA9539_Init();

/**
 * @brief Sets or unsets a pin on one of the expanders on one of the ports.
 *
 * @return true on success. false on error.
 */
bool TCA9539_Set_Pin(IOExpanderID device, int port, int pin, bool set);

/**
 * @brief Gets the state of a pin on one of the expanders on one of the ports.
 *
 * @return 1 if set, 0 if unset. -1 on error.
 */
int TCA9539_Get_Pin(IOExpanderID device, int port, int pin);

/**
 * @brief Sets all pins to low.
 */
void TCA9539_Clear_Pins();

#endif
