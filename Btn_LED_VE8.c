#include "MDR1986VE8T.h"

#include "mdr32f8_config.h"
#include "mdr32f8_port.h"
#include "mdr32f8_clkctrl.h"

#include "VE8_Defs.h"
#include "Btn_LED_VE8.h"



void Btns_Init(void)
{
  PORT_InitTypeDef PORT_InitStructure;  
  
  CLKCTRL_PER0_CLKcmd(CLKCTRL_PER0_CLK_MDR_PORTE_EN, ENABLE);
  PORTE->KEY	= UNLOCK_KEY;
  
  PORT_StructInit(&PORT_InitStructure);
  
	/* Configure PORTC pins [18,21,22] for input */	
	PORT_InitStructure.PORT_Pin   = (Btn1 | Btn2 | Btn3);
	
  PORT_InitStructure.PORT_SOE      = PORT_SOE_IN;
  PORT_InitStructure.PORT_SANALOG  = PORT_SANALOG_DIGITAL;
	PORT_InitStructure.PORT_SPD  = PORT_SPD_OFF;
	PORT_InitStructure.PORT_SPWR = PORT_SPWR_10;
//	/* irq enable and set active irq level */	
//	PORT_InitStructure.PORT_SIE = PORT_SIE_ON;
//	PORT_InitStructure.PORT_SIT = PORT_SIT_LOW;

  PORT_Init(PORTE, &PORT_InitStructure);
	
//	/* PORTE IRQ enable */	
//	NVIC_EnableIRQ(PORTE_IRQn);
}  

//void BTN_OnClick_Handler(void)
//{

//}  

//void IRQ_PORTE_Handler()
//{  
//	PORTE->CIR=0xFFFFFFFF;

//  BTN_OnClick_Handler();
//}

uint32_t Is_BntDown(VE8_Btn bnt)
{
	return !PORT_ReadInputDataBit (PORTE, bnt);
}

//------------------    LEDs  -----------------------
void LEDs_Init(void)
{
  PORT_InitTypeDef PORT_InitStructure;  
  
  CLKCTRL_PER0_CLKcmd(CLKCTRL_PER0_CLK_MDR_PORTC_EN, ENABLE);
  PORTC->KEY	= UNLOCK_KEY;
  
  PORT_StructInit(&PORT_InitStructure);
  
	PORT_InitStructure.PORT_Pin   = (VD7 | VD8 | VD9);
	
  PORT_InitStructure.PORT_SOE    = PORT_SOE_OUT;
  PORT_InitStructure.PORT_SANALOG  = PORT_SANALOG_DIGITAL;
	PORT_InitStructure.PORT_SPD = PORT_SPD_OFF;
	PORT_InitStructure.PORT_SPWR = PORT_SPWR_10;

  PORT_Init(PORTC, &PORT_InitStructure);  
}  

void LED_Set(VE8_LED led, uint32_t isOn)
{
  if (isOn)
    PORT_SetBits(PORTC, led);
  else
    PORT_ResetBits(PORTC, led);
}  


void Delay(uint32_t delay)
{
	uint32_t i;
	for (i = 0; i < delay; i++)
	{
		__NOP();
	}
}