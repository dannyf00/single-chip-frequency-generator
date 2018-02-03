#include "refco.h"						//we use reference clock output on pb2

//hardware configuration
//end hardware configuration

//global defines

//global variables

//initialize ref clock
//1. select the source;
//2. disable the output / module
//3. enable the output / module
//4. output ***disabled***
uint32_t refco_init(uint32_t clk_source) {
	while (REFOCON & (1<<8)) continue;	//wait for REFOCON.ACTIVE bit to clear
	REFOCON &=~(1<<15);					//1->enable REFCO, 0->disable it
	REFOCON &=~(1<<12);					//'1'=output enabled, '0'=output disabled
	//set the REFIN bits
	REFOCON = (REFOCON &~0x0f) | (clk_source & 0x0f);		//lowest 4 bits (ROSEL) effective
	//REFOCON = (REFOCON &~0x7fff0000ul) | (((uint32_t) 1 << 16) & 0x7fff0000ul);	//set REFODIV bits to a minimum of 1
	REFOCONbits.RODIV = 1;				//divider is a minimum of 1
	REFOTRIMbits.ROTRIM= 0;
	REFOCON |= (1<<9);					//start switch divider
	while (REFOCON & (1<<8)) continue;	//wait for REFOCON.ACTIVE bit to clear
	REFOCON |= (1<<12);					//'1'=output enable, '0'=output disabled
	REFOCON |= (1<<15);					//1->enable REFCO, 0->disable it
	return REFOCON & 0x0f;				//return the clock source
}

//set ref clock divider N (0..32767)
//F_REFCO = F_CLK / (2 * (N + M / 512))
//return REFOCON.RODIV bits
uint32_t refco_setdiv(uint32_t n, uint32_t m) {
	REFOCONbits.ON = 0;					// |= (1<<15);					//1->enable REFCO, 0->disable it
	//set divider
	while (REFOCON & (1<<8)) continue;	//wait for REFOCON.ACTIVE bit to clear
	//REFOCON = (REFOCON &~0x7fff0000ul) | (((uint32_t) n << 16) & 0x7fff0000ul);	//set REFODIV bits
	//REFOCON |= (1<<9);					//start switch divider

	//set trim
	//while (REFOCON & (1<<8)) continue;	//wait for REFOCON.ACTIVE bit to clear
	//REFOTRIM = (REFOTRIM &~0xff800000ul) | (((uint32_t) m << 23) & 0xff800000ul);	//set REFODIV bits
	REFOCONbits.RODIV = (n==0)?1:n;		//divider is a minimum of 1
	REFOTRIMbits.ROTRIM= m;
	REFOCONbits.DIVSWEN = 1;			// |= (1<<9);					//start switch divider
	while (REFOCON & (1<<8)) continue;	//wait for REFOCON.ACTIVE bit to clear
	
	//enable refco	
	REFOCONbits.ON = 1;					// |= (1<<15);					//1->enable REFCO, 0->disable it
	return REFOCON & 0x0f;				//return the clock source
}

//set ref clock output to desired frequency
//fref_out <=freq_in / (2 * (N + M / 512))
//output: N..M
uint32_t refco_setfreq(uint32_t fref_in, uint32_t fref_out) {
	uint32_t n, m;
	if (fref_in <= (fref_out * 2)) {		//invalid input
		n = 1;
		m = 0;
	} else {								//valid input
		//fref_in = fref_in / 2;
		n = fref_in / fref_out / 2;
		//m = fref_in - n * fref_out;			//find the residual
		m = 256 * fref_in / fref_out - 512 * n;
	}
	if (n==0) n=1;							//minimum value of n is 1
	refco_setdiv(n, m);
	//refco_settrim(m);
	return (n << 8) | m;
}

//enable refco output
//assume that dividers are set
void refco_en(void) {
	REFOCON |= (1<<15);					//'1'->enable refco; '0'->disable refco
	RPB2R = 0x07;						//'0b0111' -> RPB2 = REFCLKO
	REFOCON |= (1<<12);					//'1'=output enable, '0'=output disabled
}

//disable refco output
void refco_dis(void) {
	REFOCON &=~(1<<15);					//'1'->enable refco; '0'->disable refco
	RPB2R = 0x00;						//'0b0000' -> no channel = PORTB2
	REFOCON &=~(1<<12);					//'1'=output enable, '0'=output disabled
}

