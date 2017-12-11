#include "mdr32f8_port.h"

#include "Clock_VE8.h"
#include "ExtBus_VE8.h"
#include "Btn_LED_VE8.h"

#define LED_COMLETED    VD7
#define LED_ECC         VD8
#define LED_NoECC       VD9

#define BTN_FILL_NoECC   Btn1
#define BTN_ECC_WR_RD    Btn2
#define BTN_ECC_FIX_ERR  Btn3


#define RGN0_StartAddr  0x10000000
#define RGN0_EccAddr    0x10030000

#define RGN0_Addr_Err1    0x10000010
#define RGN0_Addr_Err2    0x10010000

//  1645RU5U has 512 Kb
#define	ECC_NUM_BYTES	0x30000
#define ECC_NUM_WORDS	(ECC_NUM_BYTES >> 2)

#define	NO_ECC_NUM_BYTES	0x40000
#define NO_ECC_NUM_WORDS	(NO_ECC_NUM_BYTES >> 2)

#define ADDR(x) (*((volatile uint32_t *)(x)))

uint32_t Err2_Value;

void     FillData_NoECC(void);
uint32_t FillData_ECC(void);
uint32_t TestRD_ECC(void);

uint32_t ErrorTest(void);
void Write_Errors(void);

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
  // Fill NoECC    
    if (Is_BntDown(BTN_FILL_NoECC))
    {
      while (Is_BntDown(BTN_FILL_NoECC));
      
      //  Write Read Test by 32 bit without ECC      
      LED_Set(LED_COMLETED | LED_NoECC | LED_ECC, DISABLE);
      Delay(10000);            
      
      FillData_NoECC();
      LED_Set(LED_NoECC, ENABLE);
      
      LED_Set(LED_COMLETED , ENABLE);      
    }  
    
  //  Test ECC
    if (Is_BntDown(BTN_ECC_WR_RD))
    {      
      while (Is_BntDown(BTN_ECC_WR_RD));
      
      LED_Set(LED_COMLETED | LED_NoECC | LED_ECC, DISABLE);
     
      //  Double write memory to initialize
      Fill_Data32_ByInd(RGN0_StartAddr, ECC_NUM_WORDS);
      Fill_Data32_ByInd(RGN0_StartAddr, ECC_NUM_WORDS);      
      
      //  Test Read all Memory without RGN0_ECCS increment
      if (TestRD_ECC())
        LED_Set(LED_ECC, ENABLE);

      //  Completed
      LED_Set(LED_COMLETED, ENABLE);
    }

  //  Write errors and fix    
    if (Is_BntDown(BTN_ECC_FIX_ERR))
    {
      while (Is_BntDown(BTN_ECC_FIX_ERR));
      
      //  Write Read Test by 32 bit without ECC      
      LED_Set(LED_COMLETED | LED_NoECC | LED_ECC, DISABLE);
      Delay(10000);            

      if (ErrorTest())
        LED_Set(LED_ECC, ENABLE);

      LED_Set(LED_COMLETED , ENABLE);          
    } 
    
  }  
}

void FillData_NoECC()
{  
  uint32_t eccErrStart; 
  
  //  Ecc Off
  ExtBus_Init_RGN0_D8(DISABLE, RGN0_EccAddr);  
  
  //  Fill Data and ECC - NO_ECC_NUM_WORDS
  eccErrStart = EXT_BUS_CNTR->RGN0_ECCS;     
  Fill_Data32_ByInd(RGN0_StartAddr, NO_ECC_NUM_WORDS); 

  //  Ecc On restore
  ExtBus_Init_RGN0_D8(ENABLE, RGN0_EccAddr);
}  

uint32_t FillData_ECC()
{ 
  uint32_t eccErrStart;   
  
  eccErrStart = EXT_BUS_CNTR->RGN0_ECCS;   
  Fill_Data32_ByInd(RGN0_StartAddr, ECC_NUM_WORDS);  
  
  return (EXT_BUS_CNTR->RGN0_ECCS == eccErrStart);
}

uint32_t TestRD_ECC(void)
{
  uint32_t* addr;
  uint32_t i = 0;
  uint32_t rdValue, errCnt;
  uint32_t eccErrStart; 
   
  eccErrStart = EXT_BUS_CNTR->RGN0_ECCS;     
  //  Read and Check Data
  addr = (uint32_t*)RGN0_StartAddr;  
  errCnt = 0;
  
  for (i = 0; i < ECC_NUM_WORDS; ++i)
	{
		rdValue = *addr++;
		if(rdValue != i) 
      errCnt++; 
	}
 
  return (errCnt == 0) && (EXT_BUS_CNTR->RGN0_ECCS == eccErrStart);    
}  

//  ----------- Simulate Errors ------------

void Write_Errors(void)
{
  uint32_t val;
  
  //  ECC Off
  ExtBus_Init_RGN0_D8(DISABLE, RGN0_EccAddr);

  //  Single Error
  val = ADDR(RGN0_Addr_Err1);
  val = val ^ 1; 
  ADDR(RGN0_Addr_Err1) = val; 
  
  //  Double Error  
  Err2_Value = ADDR(RGN0_Addr_Err2);
  val = Err2_Value ^ 3;
  ADDR(RGN0_Addr_Err2) = val;    
  
  //  Restore ECC ON
  ExtBus_Init_RGN0_D8(ENABLE, RGN0_EccAddr);    
}  

uint32_t TestRD_Err1(void)
{
  uint32_t* addr;
  uint32_t i = 0;
  uint32_t rdValue, errCnt;
  uint32_t eccErrStart, eccCntrOK; 
   
  eccErrStart = EXT_BUS_CNTR->RGN0_ECCS;     
  //  Read and Check Data
  addr = (uint32_t*)RGN0_StartAddr;  
  errCnt = 0;
  
  for (i = 0; i < 10; ++i)
	{
		rdValue = *addr++;
		if(rdValue != i) 
      errCnt++; 
	}
    
  eccCntrOK = (EXT_BUS_CNTR->RGN0_ECCS == (eccErrStart + (1 << 16))) || (EXT_BUS_CNTR->RGN0_ECCS == (eccErrStart + (2 << 16)));
  return (errCnt == 0)  && eccCntrOK;
}

void Clear_Err1(void)
{
  uint32_t* addr;
  uint32_t i = 0;
  uint32_t rdValue, errCnt;
  uint32_t eccErrStart; 
   
  addr = (uint32_t*)RGN0_StartAddr;  
  errCnt = 0;
  
  for (i = 0; i < 10; ++i)
	{
		*addr++ = i;
	}
}

uint32_t ErrorTest(void)
{
  uint32_t val;
  
  //  Write Errors
  Write_Errors();
   
  //  Check single error fixed
  if (!TestRD_Err1())
    return 0;
  
  //  OverWrite Single Error Data - Double Write
  val = ADDR(RGN0_Addr_Err1);
  ADDR(RGN0_Addr_Err1) = val;
  ADDR(RGN0_Addr_Err1) = val;
  
  //  Restore Double Error
  ADDR(RGN0_Addr_Err2) = Err2_Value;
  ADDR(RGN0_Addr_Err2) = Err2_Value; 

  //  Test Read all Memory without RGN0_ECCS increment
  return TestRD_ECC();
}  
