/*
 * xmega.h
 *
 */


#ifndef XMEGA_H_
#define XMEGA_H_


#define NOP()	__asm__ __volatile__("nop")
#define	WDR()	__asm__ __volatile__("wdr")

// port settings
#define DISABLE_DIGITAL_INPUT(A)	A = (A & ~PORT_ISC_gm) | PORT_ISC_INPUT_DISABLE_gc
#define ENABLE_PULLUP(A)			A = (A & ~PORT_OPC_gm) | PORT_OPC_PULLUP_gc
#define ENABLE_PULLDOWN(A)			A = (A & ~PORT_OPC_gm) | PORT_OPC_PULLDOWN_gc
#define ENABLE_TOTEM(A)				A = (A & ~PORT_OPC_gm) | PORT_OPC_TOTEM_gc

// Watchdog timer
#define WATCHDOG_ON					do {	WDR();																	\
											CCPWrite(&WDT.CTRL, WDT_PER_8KCLK_gc | WDT_ENABLE_bm | WDT_CEN_bm);	\
									} while(0);
#define WATCHDOG_OFF				do {	while (WDT.STATUS & WDT_SYNCBUSY_bm);									\
											CCPWrite(&WDT.CTRL, WDT_CEN_bm);										\
									} while(0);

// EEPROM
#define	EEP_MAPPED_ADDR(page, byte)	(MAPPED_EEPROM_START + (EEPROM_PAGE_SIZE * page) + byte)
#define EEP_EnablePowerReduction()	( NVM.CTRLB |= NVM_EPRM_bm )
#define EEP_DisablePowerReduction() ( NVM.CTRLB &= ~NVM_EPRM_bm )
#define EEP_EnableMapping()			( NVM.CTRLB |= NVM_EEMAPEN_bm )		// not on E5
#define EEP_DisableMapping()		( NVM.CTRLB &= ~NVM_EEMAPEN_bm )	// not on E5
extern void		NVM_eeprom_erase_all(void);
extern void		NVM_eeprom_atomic_write_page(uint8_t page);
extern void		NVM_eeprom_load_buffer(const void *data, uint8_t size);
extern void		NVM_eeprom_erase_write_buffer(void);


// NVM functions in xmega.S
extern void		CCPWrite(volatile uint8_t *address, uint8_t value);
extern uint8_t	NVM_read_fuse_byte(uint8_t index);
extern uint8_t	NVM_read_production_signature_byte(uint8_t index);
extern uint8_t	NVM_read_user_signature_byte(uint16_t index);
extern uint32_t	NVM_application_crc(void);
extern uint32_t	NVM_boot_crc(void);
extern void		NVM_wait(void);
extern void		NVM_execute_command(uint8_t command);


/****************************************************************************************
* Baud rate setting calculator for XMEGA devices
* Author: omegacs
* https://web.archive.org/web/20130601050533/http://blog.omegacs.net/2010/08/18/xmega-fractional-baud-rate-source-code/
*/
#define _BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,bscale) (					\
((bscale) < 0) ?														\
  (int)((((float)(f_cpu)/(8*(float)(baud)))-1)*(1<<-(bscale)))			\
: (int)((float)(f_cpu)/((1<<(bscale))*8*(float)(baud)))-1 )

#define _BSCALE(f_cpu,baud) (											\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-7) < 4096) ? -7 :				\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-6) < 4096) ? -6 :				\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-5) < 4096) ? -5 :				\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-4) < 4096) ? -4 :				\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-3) < 4096) ? -3 :				\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-2) < 4096) ? -2 :				\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-1) < 4096) ? -1 :				\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,0) < 4096) ? 0 :					\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,1) < 4096) ? 1 :					\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,2) < 4096) ? 2 :					\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,3) < 4096) ? 3 :					\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,4) < 4096) ? 4 :					\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,5) < 4096) ? 5 :					\
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,6) < 4096) ? 6 :					\
7 )

#define BSEL(f_cpu,baud)												\
	_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,_BSCALE(f_cpu,baud))

#define BSCALE(f_cpu,baud) ((_BSCALE(f_cpu,baud)<0) ? (16+_BSCALE(f_cpu,baud)) : _BSCALE(f_cpu,baud))

#define CALC_BAUDCTRLA(baud)	BSEL(F_CPU, baud) & 0xff;
#define CALC_BAUDCTRLB(baud)	(BSCALE(F_CPU, baud) << USART_BSCALE0_bp) | (BSEL(F_CPU, baud) >> 8);


#endif /* XMEGA_H_ */
