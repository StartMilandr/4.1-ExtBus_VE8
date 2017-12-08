#include "mdr32f8_config.h"
#include "mdr32f8_port.h"
#include "mdr32f8_clkctrl.h"

void Clock_HSE0_Init(uint32_t pll_Q, uint32_t pll_N)
{  
/* Set CLKCTRL to default */
	CLKCTRL_DeInit();
	
	/* Enable HSE0 clock */
	CLKCTRL_HSEconfig(CLKCTRL_HSE0_CLK_ON);
	
	/* Check HSE success, enable PLL0, check PLL0 success, select MAX_CLK src */
	while(CLKCTRL_HSEstatus(CLKCTRL_HSEn_STAT_HSE0_RDY) != SUCCESS){}
	
	/* Select PLL0 clk src, PLL0_N, PLL0_Q to get FINT = FIN*(PLLn_N)/(PLLn_Q+1) */
	CLKCTRL_CPU_PLLconfig(PLL0, CLKCTRL_PLLn_CLK_SELECT_HSE0div1, pll_Q, pll_N);  //  PLLn, SRC, Q, N
	while(CLKCTRL_CPU_PLLstatus(0) != SUCCESS){};
	
	CLKCTRL_MAX_CLKSelection(CLKCTRL_MAX_CLK_PLL0);	     
}

void Clock_HSE0_Init_60MHz(void)
{
  Clock_HSE0_Init(0, 0); //4);
}  
