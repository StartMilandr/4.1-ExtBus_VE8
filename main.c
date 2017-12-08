#include "mdr32f8_port.h"

#include "Clock_VE8.h"
#include "ExtBus_VE8.h"
#include "Btn_LED_VE8.h"

#define LED_TEST_OK    VD7
#define LED_COMLETED   VD8
#define BTN_RUN        Btn1

#define RGN0_StartAddr  0x10000000
#define RGN0_EccAddr    0x10030000

//  1645RU5U has 512 Kb
#define	NUM_BYTES	0x30000  //0x80000
#define NUM_WORDS	(NUM_BYTES >> 2)

uint32_t TestProc(void);

int main(void)
{	
	POR_disable();
  
  //  Clock
  Clock_HSE0_Init_60MHz();
  
  //  Bus
  ExtBus_InitPins_A19_D8();
  ExtBus_Init_RGN0_D8(ENABLE, RGN0_EccAddr);
  
  //  Btns and Leds
  Btns_Init();
  LEDs_Init();
  
	while (1)
  {
    if (Is_BntDown(BTN_RUN))
    {      
      while (Is_BntDown(BTN_RUN));
      
      LED_Set(LED_TEST_OK | LED_COMLETED, DISABLE);
      
      if (TestProc())
      {  
        Delay(10000);
        LED_Set(LED_TEST_OK, ENABLE);
      }
      LED_Set(LED_COMLETED, ENABLE);
    }
  
  }  
}

volatile uint32_t eccData, eccAddr, eccEcc;

uint32_t TestProc(void)
{
  uint32_t* addr;
  uint32_t i = 0;
  uint32_t rdValue, errCnt;
  
  //  Fill RGN0 mem with Index
	addr = (uint32_t*)RGN0_StartAddr;  
	for (i = 0; i < NUM_WORDS; ++i)
	{
		*addr++ = i;
	}	  
 
  //  Read and Check Data
  addr = (uint32_t*)RGN0_StartAddr;  
  errCnt = 0;
  
  for (i = 0; i < NUM_WORDS; ++i)
	{
		rdValue = *addr++;
		if(rdValue != i) 
      errCnt++; 
	}
 
  return (errCnt == 0) && (EXT_BUS_CNTR->RGN0_ECCS == 0);    
}  