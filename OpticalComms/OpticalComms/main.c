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
	
	PORTC.DIRSET = PIN7_bm;	// TX
	STDIO_init();
	puts("\r\nOptical Comms Test");
	
	PORTC.DIRCLR = PIN4_bm;
	ENABLE_PULLUP(PORTC.PIN4CTRL);
	
	// timer for measuring pulse width
	PORTC.DIRCLR = PIN4_bm;
	PORTC.PIN4CTRL = (PORTC.PIN4CTRL & ~PORT_ISC_gm) | PORT_ISC_BOTHEDGES_gc;	// manual says must be both edges
	//PORTC.PIN4CTRL |= PORT_INVEN_bm;
	EVSYS.CH0CTRL = EVSYS_DIGFILT_8SAMPLES_gc;
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTC_PIN4_gc;

	TCC1.CTRLA = 0;
	TCC1.CTRLB = TC1_CCAEN_bm;
	TCC1.CTRLC = 0;
	TCC1.CTRLD = TC_EVACT_PW_gc | TC_EVSEL_CH0_gc;
	//TCC1.CTRLD = TC_EVACT_FRQ_gc | TC_EVSEL_CH0_gc;
	TCC1.CTRLE = 0;
	TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc;
	TCC1.INTCTRLB = TC_CCAINTLVL_LO_gc;
	TCC1.PER = 8000;
	TCC1.CNT = 0;
	TCC1.CTRLA = TC_CLKSEL_DIV1024_gc;
	
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
	sei();

	uint8_t ptr_copy = 0;
	for(;;)
	{
		uint8_t ptr = capture_ptr;
		if ((ptr > 0) && (ptr == ptr_copy))	// data in buffer and not still reading
		{
			cli();
			capture_ptr = 0;

			// find average
			uint32_t ave = 0;
			for (uint8_t i = 1; i < ptr; i++)	// discard sample 0
				ave += capture_buffer[i];
			ave /= ptr;
			printf("ave: %lu\r\n", ave);

			// compare pulse width with average
			for (uint8_t i = 1; i < ptr; i++)	// discard sample 0
			{
				//printf("%u\r\n", capture_buffer[i]);
				if ((i & 7) == 0)
					putchar(' ');
				putchar(capture_buffer[i] > ave ?  '0' : '1');
			}
			sei();
			ptr = 0;
			putchar('\r');
			putchar('\n');
		}
		ptr_copy = ptr;
		_delay_ms(500);
	}
}

/**************************************************************************************************
** Timer capture handler, triggered when pulse width is measured
*/
ISR(TCC1_CCA_vect)
{
	//TCC1.INTFLAGS = TC1_CCAIF_bm;
	capture_buffer[capture_ptr++] = TCC1.CCA;
	//putchar('.');
}

ISR(TCC1_OVF_vect)
{
	TCC1.INTFLAGS = TC1_OVFIF_bm;
	//putchar('#');
}
