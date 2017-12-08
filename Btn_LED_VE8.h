#ifndef _BTN_LED_VE8_H
#define _BTN_LED_VE8_H

#include "mdr32f8_port.h"

typedef enum {VD7 = PORT_Pin_16, VD8 = PORT_Pin_17, VD9 = PORT_Pin_18}    VE8_LED;
typedef enum {Btn1 = PORT_Pin_18, Btn2 = PORT_Pin_21, Btn3 = PORT_Pin_22} VE8_Btn;

void LEDs_Init(void);
void LED_Set(VE8_LED led, uint32_t isOn);

void Btns_Init(void);
uint32_t Is_BntDown(VE8_Btn bnt);

void Delay(uint32_t delay);

#endif // _BTN_LED_VE8_H