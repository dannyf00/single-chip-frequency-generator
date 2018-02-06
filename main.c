#include "config.h"						//configuration words
#include "gpio.h"
#include "delay.h"						//we use software delays
#include "refco.h"						//we use ref clock generator

//hardware configuration
#define REFCLKO_TO_PIN()	PPS_RPA4_TO_REFCLKO()	//REFCO on PA4 (output can also be routed to other pins)
#define F_REFCO				10001		//refco output frequency, in Hz

//for debugging only
#define LED_PORT			LATB
#define LED_DDR				TRISB
#define LED					(1<<7)
#define LED_DLY				100			//ms to be wasted -> to manage main loop update frequency
//end hardware configuration

//global defines

//global variables

int main(void) {
	uint32_t tmp_N, tmp_M;

	mcu_init();						//reset the mcu
	
	REFCLKO_TO_PIN();				//assign the refco output pin
	refco_init(REFCO_SYSCLK);		//refco module input is SYSCLK (default divider/trim: N=1, M=0)
	//Output clock = SystemCoreClock / (2 * (N + M / 512))
	tmp_M = refco_calc(SystemCoreClock, F_REFCO, &tmp_N);		//tmp = N..M, Q32.9 format (Q(16+9).9 format actually)
	refco_setdiv(tmp_N, tmp_M);	//tmp's lease significant 9 bits are M, most significant bits are N
	//refco output on pin specified by REFCLKO_TO_PIN()
	
	IO_OUT(LED_DDR, LED);			//led as output - for debugging
	while (1) {
		IO_FLP(LED_PORT, LED);		//blink the led - for debugging
		delay_ms(LED_DLY);			//waste some time
	}
}
