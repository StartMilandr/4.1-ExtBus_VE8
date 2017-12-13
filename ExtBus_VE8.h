#ifndef _EXTBUS_VE8_H
#define _EXTBUS_VE8_H


void ExtBus_InitPins_A19_D8(void);
void ExtBus_Init_RGN0_D8(FunctionalState Ecc_EN, uint32_t baseECC);

void Fill_Data32_ByInd(uint32_t starAddr, uint32_t count);

unsigned int GetECC(unsigned int adr, unsigned int data);

#endif // _EXTBUS_VE8_H
