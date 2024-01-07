#ifndef TCA9539_H_
#define TCA9539_H_

void TCA9539_Init();
int TCA9539_Set_Pin(int device, int port, int pin, int value);
int TCA9539_Get_Pin(int device, int port, int pin);
void TCA9539_Clear_Pins();

#endif
