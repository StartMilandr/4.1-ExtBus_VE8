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
#define ERR_AREA_SIZE     20

//  1645RU5U has 512 Kb
#define	ECC_NUM_BYTES	0x30000
#define ECC_NUM_WORDS	(ECC_NUM_BYTES >> 2)

#define	NO_ECC_NUM_BYTES	0x40000
#define NO_ECC_NUM_WORDS	(NO_ECC_NUM_BYTES >> 2)

#define ADDR(x) (*((volatile uint32_t *)(x)))

uint32_t Err1_ValueSrc, Err2_ValueSrc;
uint32_t Err1_Value, Err2_Value;

volatile uint32_t regECC_ADDR;
volatile uint32_t regECC_DATA;
volatile uint32_t regECC_ECC;


void     FillData_NoECC(void);
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
  //  Ecc Off
  ExtBus_Init_RGN0_D8(DISABLE, RGN0_EccAddr);  
  
  //  Fill Data and ECC - NO_ECC_NUM_WORDS
  Fill_Data32_ByInd(RGN0_StartAddr, NO_ECC_NUM_WORDS); 

  //  Ecc On restore
  ExtBus_Init_RGN0_D8(ENABLE, RGN0_EccAddr);
}  

uint32_t TestRD_ECC(void)
{
  uint32_t* addr;
  uint32_t i = 0;
  uint32_t rdValue, errCnt;
  uint32_t eccErrStart = EXT_BUS_CNTR->RGN0_ECCS;

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
  //  ECC Off
  ExtBus_Init_RGN0_D8(DISABLE, RGN0_EccAddr);

  //  Single Error
  Err1_ValueSrc = ADDR(RGN0_Addr_Err1);
  Err1_Value = Err1_ValueSrc ^ 1; 
  ADDR(RGN0_Addr_Err1) = Err1_Value; 
  
  //  Double Error  
  Err2_ValueSrc = ADDR(RGN0_Addr_Err2);
  Err2_Value = Err2_ValueSrc ^ 3;
  ADDR(RGN0_Addr_Err2) = Err2_Value;
  
  //  Restore ECC ON
  ExtBus_Init_RGN0_D8(ENABLE, RGN0_EccAddr);    
}  

void LogEccRegs(void)
{  
  regECC_ADDR = EXT_BUS_CNTR->ECC_ADDR;
  regECC_DATA = EXT_BUS_CNTR->ECC_DATA;
  regECC_ECC  = EXT_BUS_CNTR->ECC_ECC;
}  

uint32_t TestRD_Err1(void)
{
  uint32_t* addr;
  uint32_t i = 0;
  uint32_t rdValue, errCnt;
  uint32_t eccErrStart;
  uint32_t Err1Cnt, Err2Cnt, eccLogOK;  
   
  eccErrStart = EXT_BUS_CNTR->RGN0_ECCS;     
  //  Read and Check Data
  addr = (uint32_t*)RGN0_StartAddr;  
  errCnt = 0;
  
  for (i = 0; i < ERR_AREA_SIZE; ++i)
	{
		rdValue = *addr++;
		if(rdValue != i) 
      errCnt++; 
	}

  //  Check Log ECC Regs
  LogEccRegs();
  eccLogOK = (regECC_ADDR == RGN0_Addr_Err1) && (regECC_DATA == Err1_Value) && (GetECC(regECC_ADDR, Err1_ValueSrc) == regECC_ECC);
  // Check Error Counters  
  Err1Cnt = (EXT_BUS_CNTR->RGN0_ECCS >> 16) - (eccErrStart >> 16);
  Err2Cnt = ((EXT_BUS_CNTR->RGN0_ECCS >> 8) & 0xFF) - ((eccErrStart >> 8) & 0xFF);
           
  //  Result
  return (errCnt == 0)  && (Err1Cnt < 2) && (Err2Cnt == 0) && eccLogOK;
}

uint32_t TestRD_Err2(void)
{
  uint32_t* addr;
  uint32_t i = 0;
  uint32_t rdValue, i_offs, errCnt;
  uint32_t eccErrStart;
  uint32_t Err1Cnt, Err2Cnt, eccLogOK; 
   
  eccErrStart = EXT_BUS_CNTR->RGN0_ECCS; 
  
  //  Read and Check Data
  addr = (uint32_t*)RGN0_Addr_Err2 - ERR_AREA_SIZE / 2;  
  i_offs = (RGN0_Addr_Err2 / 4 - ERR_AREA_SIZE / 2) & 0xFFFFF;
  errCnt = 0;
  
  for (i = 0; i < ERR_AREA_SIZE; ++i)
	{
		rdValue = *addr++;
		if(rdValue != (i + i_offs)) 
      errCnt++; 
	}

  //  Check Log ECC Regs
  LogEccRegs();
  eccLogOK = (regECC_ADDR == RGN0_Addr_Err2) && (regECC_DATA == Err2_Value) && (GetECC(regECC_ADDR, Err2_ValueSrc) == regECC_ECC);  
  // Check Error Counters
  Err1Cnt = (EXT_BUS_CNTR->RGN0_ECCS >> 16) - (eccErrStart >> 16);
  Err2Cnt = ((EXT_BUS_CNTR->RGN0_ECCS >> 8) & 0xFF) - ((eccErrStart >> 8) & 0xFF);
           
  //  Result
  return (errCnt == 1)  && (Err1Cnt < 2) && (Err2Cnt < 2) && eccLogOK;
}

uint32_t ErrorTest(void)
{
  uint32_t val;
  
  //  Write Errors
  Write_Errors();
   
  //  Check single error fixed
  if (!TestRD_Err1())
    return 0;
  
  //  Check double error found
  if (!TestRD_Err2())
    return 0;
  
  //  OverWrite Single Error Data - Double Write
  val = ADDR(RGN0_Addr_Err1);
  ADDR(RGN0_Addr_Err1) = val;
  ADDR(RGN0_Addr_Err1) = val;
  
  //  Restore Double Error
  ADDR(RGN0_Addr_Err2) = Err2_ValueSrc;
  ADDR(RGN0_Addr_Err2) = Err2_ValueSrc; 

  //  Test Read all Memory without RGN0_ECCS increment
  return TestRD_ECC();
}  
