/*
 * OpticalComms.c
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "stdio_fast.h"
#include "xmega.h"

volatile uint16_t capture_buffer[256];
volatile uint8_t capture_ptr = 0;

int main(void)
{
	// 16MHz external clock
	OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
	OSC.CTRL |= OSC_XOSCEN_bm;
	while(!(OSC.STATUS & OSC_XOSCRDY_bm));
	CCPWrite(&CLK.PSCTRL, CLK_PSADIV_1_gc | CLK_PSBCDIV_1_1_gc);
	CCPWrite(&CLK.CTRL, CLK_SCLKSEL_XOSC_gc);
	
	STDIO_init();
	puts("Optical Comms Test");
	
	PORTC.DIRCLR = PIN4_bm;
	ENABLE_PULLUP(PORTC.PIN4CTRL);
	
	// timer for measuring pulse width
	PORTC.PIN4CTRL = (PORTC.PIN4CTRL & ~PORT_ISC_gm) | PORT_ISC_BOTHEDGES_gc;	// manual says must be both edges
	EVSYS.CH0CTRL = 0;
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTC_PIN4_gc;

	TCC0.CTRLA = 0;
	TCC0.CTRLB = TC0_CCCEN_bm;
	TCC0.CTRLC = 0;
	TCC0.CTRLD = TC_EVACT_PW_gc | TC_EVSEL_CH0_gc;
	TCC0.CTRLE = 0;
	TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
	TCC0.INTCTRLB = TC_CCAINTLVL_LO_gc;
	TCC0.PER = 8000;
	TCC0.CNT = TC_CLKSEL_DIV1024_gc;

	uint8_t ptr_copy = 0;
	for(;;)
	{
		uint8_t ptr = capture_ptr;
		if ((ptr > 0) && (ptr == ptr_copy))	// data in buffer and not still reading
		{
			cli();
			capture_ptr = 0;
			for (uint8_t i = 0; i < ptr; i++)
				printf("%u\r\n", capture_buffer[i]);
			sei();
			ptr = 0;
		}
		ptr_copy = ptr;
		_delay_ms(500);
	}
}

/**************************************************************************************************
** Timer capture handler, triggered when pulse width is measured
*/
ISR(TCC0_CCA_vect)
{
	TCC0.INTFLAGS = TC0_CCAIF_bm;
	capture_buffer[capture_ptr++] = TCC0.CCA;
}
