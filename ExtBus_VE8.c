#include "mdr32f8_config.h"
#include "mdr32f8_port.h"
#include "mdr32f8_clkctrl.h"
#include "mdr32f8_ebc.h"

#include "VE8_Defs.h"
#include "ExtBus_VE8.h"


void ExtBus_InitPins_A19_D8(void)
{	
  PORT_InitTypeDef PORT_InitStructure;
  
	CLKCTRL_PER0_CLKcmd(CLKCTRL_PER0_CLK_MDR_PORTC_EN, ENABLE);
  CLKCTRL_PER0_CLKcmd(CLKCTRL_PER0_CLK_MDR_PORTD_EN, ENABLE);
  CLKCTRL_PER0_CLKcmd(CLKCTRL_PER0_CLK_MDR_PORTE_EN, ENABLE);  
  
	PORTC->KEY		= UNLOCK_KEY;
	PORTD->KEY		= UNLOCK_KEY;
	PORTE->KEY		= UNLOCK_KEY;  

  PORT_StructInit(&PORT_InitStructure);
  
  //  DATA BUS
  //    Data[0..1]
	PORT_InitStructure.PORT_Pin   = (PORT_Pin_30|PORT_Pin_31);
	PORT_InitStructure.PORT_SFUNC  = PORT_SFUNC_2;
  PORT_InitStructure.PORT_SANALOG  = PORT_SANALOG_DIGITAL;
	PORT_InitStructure.PORT_SPWR = PORT_SPWR_10;
  PORT_Init(PORTD, &PORT_InitStructure);
	//    Data[2..7]
	PORT_InitStructure.PORT_Pin   = (PORT_Pin_0|PORT_Pin_1|PORT_Pin_2|PORT_Pin_3|PORT_Pin_4|PORT_Pin_5);
	PORT_InitStructure.PORT_SFUNC  = PORT_SFUNC_2;
  PORT_InitStructure.PORT_SANALOG  = PORT_SANALOG_DIGITAL;
	PORT_InitStructure.PORT_SPWR = PORT_SPWR_10;
  PORT_Init(PORTE, &PORT_InitStructure);  

  //  ADDR BUS
  //    Addr[0..1]
	PORT_InitStructure.PORT_Pin   = (PORT_Pin_30|PORT_Pin_31);
	PORT_InitStructure.PORT_SFUNC  = PORT_SFUNC_2;
  PORT_InitStructure.PORT_SANALOG  = PORT_SANALOG_DIGITAL;
	PORT_InitStructure.PORT_SPWR = PORT_SPWR_10;
  PORT_Init(PORTC, &PORT_InitStructure);

  //    Addr[2..18]
	PORT_InitStructure.PORT_Pin   = ( PORT_Pin_0|PORT_Pin_1|PORT_Pin_2|PORT_Pin_3|
																		PORT_Pin_4|PORT_Pin_5|PORT_Pin_6|PORT_Pin_7|
																		PORT_Pin_8|PORT_Pin_9|PORT_Pin_10|PORT_Pin_11|
																		PORT_Pin_12|PORT_Pin_13|PORT_Pin_14|PORT_Pin_15|PORT_Pin_16 );
	PORT_InitStructure.PORT_SFUNC  = PORT_SFUNC_2;
  PORT_InitStructure.PORT_SANALOG  = PORT_SANALOG_DIGITAL;
	PORT_InitStructure.PORT_SPWR = PORT_SPWR_10;
  PORT_Init(PORTD, &PORT_InitStructure); 

  //  CTRL BUS
  //    PD19 = nCS, PD23 = nOE, PD24 = nWE
	PORT_InitStructure.PORT_Pin   = (PORT_Pin_19|PORT_Pin_23|PORT_Pin_24);
	PORT_InitStructure.PORT_SFUNC  = PORT_SFUNC_2;
  PORT_InitStructure.PORT_SANALOG  = PORT_SANALOG_DIGITAL;
	PORT_InitStructure.PORT_SPWR = PORT_SPWR_10;
  PORT_Init(PORTD, &PORT_InitStructure);  
};

#define RGN_WS_TIME  4

void ExtBus_Init_RGN0_D8(FunctionalState Ecc_EN, uint32_t baseECC)
{
  EBC_RGN_InitTypeDef EBC_RGNx_IS;  
  
	CLKCTRL_PER0_CLKcmd(CLKCTRL_PER0_CLK_MDR_EBC_EN, ENABLE);	
	EXT_BUS_CNTR->KEY = UNLOCK_KEY;
	
  EXT_BUS_CNTR->RGN0_ECCBASE = baseECC; //  0x10030000;
  EXT_BUS_CNTR->RGN0_ECCS |= (3 << 4);  //  set FIX_SECC and FIX_DECC bit
  
  EBC_RGNx_StructInit(&EBC_RGNx_IS);
   
	//EBC_RGNx_IS.RGN_DIVOCLK = RGN_WS_TIME;
	EBC_RGNx_IS.RGN_WS_HOLD = RGN_WS_TIME;
	EBC_RGNx_IS.RGN_WS_SETUP  = RGN_WS_TIME;
	EBC_RGNx_IS.RGN_WS_ACTIVE = RGN_WS_TIME;
	EBC_RGNx_IS.RGN_MODE = 2; //EBC_MODE_8X;
	EBC_RGNx_IS.RGN_ECCEN = Ecc_EN;
  
  if (Ecc_EN)
  {
    EBC_RGNx_IS.RGN_ECCMODE = ENABLE;
    EBC_RGNx_IS.RGN_READ32 = ENABLE;  
  }  
	
	EBC_RGNx_Init(RGN0, &EBC_RGNx_IS);
	EBC_RGNx_Cmd(RGN0, ENABLE);
}  

void Fill_Data32_ByInd(uint32_t starAddr, uint32_t count)
{
  uint32_t* addr;
  uint32_t i = 0;  
   
	addr = (uint32_t*)starAddr;  
	for (i = 0; i < count; ++i)
	{
		*addr++ = i;
	}	  
}  

const unsigned long long H[8] = {
(unsigned long long) 0x0738C808099264FF,	
(unsigned long long) 0x38C808099264FF07,
(unsigned long long) 0xC808099264FF0738,
(unsigned long long) 0x08099264FF0738C8,
(unsigned long long) 0x099264FF0738C808,
(unsigned long long) 0x9264FF0738C80809,
(unsigned long long) 0x64FF0738C8080992,
(unsigned long long) 0xFF0738C808099264
};

//модифицированная программа вычисления ecc 
unsigned int GetECC(unsigned int data,  unsigned int adr)
{
  unsigned int* ptr_H;
  int i, j;	
  unsigned int res;
  unsigned int ecc;
  unsigned int datai;
  unsigned int adri;

  ecc =0;
  ptr_H = (unsigned int*)(&H);
  for (i=0; i<8; i++)
  {
    datai = *ptr_H;
    ptr_H++;
    adri = *ptr_H;
    ptr_H++;
    datai &= data;
    adri &= adr;
    res = 0;
    
    for (j=0; j < 32; j++)
    {
      res ^= adri >> j;
      res ^= datai >> j;
    }
    res &= 0x1;
    res <<= i;
    ecc |= res;
  }
  
  return ecc;
}

