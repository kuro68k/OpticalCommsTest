/*
 * xmega.S
 *
 * Various NVM / EEPROM related functions
 */

#include <avr\io.h>

#define NVM_CMD_NO_OPERATION_gc (0x00<<0)				// Noop/Ordinary LPM
#define NVM_CMD_READ_USER_SIG_ROW_gc (0x01<<0)			// Read user signature row
#define NVM_CMD_READ_CALIB_ROW_gc (0x02<<0)				// Read calibration row
#define NVM_CMD_READ_EEPROM_gc (0x06<<0)				// Read EEPROM
#define NVM_CMD_READ_FUSES_gc (0x07<<0)					// Read fuse byte
#define NVM_CMD_WRITE_LOCK_BITS_gc (0x08<<0)			// Write lock bits
#define NVM_CMD_ERASE_USER_SIG_ROW_gc (0x18<<0)			// Erase user signature row
#define NVM_CMD_WRITE_USER_SIG_ROW_gc (0x1A<<0)			// Write user signature row
#define NVM_CMD_ERASE_APP_gc (0x20<<0)					// Erase Application Section
#define NVM_CMD_ERASE_APP_PAGE_gc (0x22<<0)				// Erase Application Section page
#define NVM_CMD_LOAD_FLASH_BUFFER_gc (0x23<<0)			// Load Flash page buffer
#define NVM_CMD_WRITE_APP_PAGE_gc (0x24<<0)				// Write Application Section page
#define NVM_CMD_ERASE_WRITE_APP_PAGE_gc (0x25<<0)		// Erase-and-write Application Section page
#define NVM_CMD_ERASE_FLASH_BUFFER_gc (0x26<<0)			// Erase/flush Flash page buffer
#define NVM_CMD_ERASE_BOOT_PAGE_gc (0x2A<<0)			// Erase Boot Section page
#define NVM_CMD_WRITE_BOOT_PAGE_gc (0x2C<<0)			// Write Boot Section page
#define NVM_CMD_ERASE_WRITE_BOOT_PAGE_gc (0x2D<<0)		// Erase-and-write Boot Section page
#define NVM_CMD_ERASE_EEPROM_gc (0x30<<0)				// Erase EEPROM
#define NVM_CMD_ERASE_EEPROM_PAGE_gc (0x32<<0)			// Erase EEPROM page
#define NVM_CMD_LOAD_EEPROM_BUFFER_gc (0x33<<0)			// Load EEPROM page buffer
#define NVM_CMD_WRITE_EEPROM_PAGE_gc (0x34<<0)			// Write EEPROM page
#define NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc (0x35<<0)	// Erase-and-write EEPROM page
#define NVM_CMD_ERASE_EEPROM_BUFFER_gc (0x36<<0)		// Erase/flush EEPROM page buffer
#define NVM_CMD_APP_CRC_gc (0x38<<0)					// Generate Application section CRC
#define NVM_CMD_BOOT_CRC_gc (0x39<<0)					// Generate Boot Section CRC
#define NVM_CMD_FLASH_RANGE_CRC_gc (0x3A<<0)			// Generate Flash Range CRC
#define CCP_SPM_gc (0x9D<<0)							// SPM Instruction Protection
#define CCP_IOREG_gc (0xD8<<0)							// IO Register Protection


.global CCPWrite
.global NVM_read_fuse_byte
.global NVM_read_production_signature_byte
.global NVM_read_user_signature_byte
.global NVM_application_crc
.global NVM_boot_crc

.global NVM_eeprom_erase_all
.global NVM_eeprom_atomic_write_page
.global NVM_eeprom_load_buffer
.global NVM_eeprom_erase_write_buffer

.global NVM_wait
.global NVM_execute_command


/**************************************************************************************************
* Write a register protected by the CCP register
* *register
* uint8 value
*/
.section .ccpwrite,"ax",@progbits
CCPWrite:
	out		RAMPZ, r1						; clear RAMPZ
	movw	Z, r24							; address
	ldi		r18, CCP_IOREG_gc
	out		CCP, r18
	st		Z, r22
	out		SREG, r18
	ret


/**************************************************************************************************
* Read signature bytes
* uint8 index
*/
.section .nvm_production_signature_byte,"ax",@progbits
NVM_read_production_signature_byte:
	ldi		r20, NVM_CMD_READ_CALIB_ROW_gc	; Prepare NVM command in R20
	clr		r25
	rjmp	common_lpm

.section .nvm_read_user_signature_byte,"ax",@progbits
NVM_read_user_signature_byte:
	ldi		r20, NVM_CMD_READ_USER_SIG_ROW_gc	; Prepare NVM command in R20
	rjmp	common_lpm

.section .common_lpm,"ax",@progbits
common_lpm:
	movw	ZL, r24							; Load index into Z
	sts		NVM_CMD, r20					; Load prepared command into NVM Command register
	lpm		r24, Z
	sts		NVM_CMD, r1						; clean up
	ret



/**************************************************************************************************
* Read fuse byte via NVM
* uint8_t index
*/
.section .nvm_read_fuse_byte,"ax",@progbits
NVM_read_fuse_byte:
	sts		NVM_ADDR0, r24					; Load fuse byte index into NVM Address Register 0
	sts		NVM_ADDR1, r1					; Load zero into NVM Address Register 1
	sts		NVM_ADDR2, r1					; Load zero into NVM Address Register 2
	ldi		r24, NVM_CMD_READ_FUSES_gc		; Prepare NVM command in R20
	rcall	NVM_execute_command				; Jump to common NVM Action code
	movw	r24, r22						; Move low byte to 1 byte return address
	ret

/**************************************************************************************************
* Calculate flash CRCs with NVM
* Returns a uint32 CRC
*/
.section .nvm_application_crc,"ax",@progbits
NVM_application_crc:
	ldi		r24, NVM_CMD_APP_CRC_gc			; Prepare NVM command in R20
	rjmp	NVM_execute_command				; Jump to common NVM Action code

.section .nvm_boot_crc,"ax",@progbits
NVM_boot_crc:
	ldi		r24, NVM_CMD_BOOT_CRC_gc		; Prepare NVM command in R20
	rjmp	NVM_execute_command				; Jump to common NVM Action code



/**************************************************************************************************
* Erase entire EEPROM
*/
.section .nvm_eeprom_erase_all,"ax",@progbits
NVM_eeprom_erase_all:
// most XMEGA devices
#if !defined(__AVR_ATxmega8E5__) && !defined(__AVR_ATxmega16E5__) && !defined(__AVR_ATxmega32E5__)
	// disable memory mapping
	rcall	NVM_wait
	lds		r18, NVM_CTRLB
	push	r18
	andi	r18, ~NVM_EEMAPEN_bm
	sts		NVM_CTRLB, r18

	; clear EEPROM write buffer
	sts		NVM_ADDR1, r1
	sts		NVM_ADDR2, r1
	ldi		r19, NVM_CMD_LOAD_EEPROM_BUFFER_gc
	sts		NVM_CMD, r19

	ldi		r19, 31							; buffer address
	ldi		r20, 0xFF
erase_all_loop:
	sts		NVM_ADDR0, r19
	sts		NVM_DATA0, r20
	dec		r19
	brge	erase_all_loop

	; execute erase command
	ldi		r24, NVM_CMD_ERASE_EEPROM_gc
	rcall	NVM_execute_command
	// restore memory mapping
	pop		r18
	sts		NVM_CTRLB, r18
	ret
#endif

// E5 range
#if defined(__AVR_ATxmega8E5__) || defined(__AVR_ATxmega16E5__) || defined(__AVR_ATxmega32E5__)
	; erase mapped EEPROM
	ldi		ZL, MAPPED_EEPROM_START & 0xFF
	ldi		ZH, MAPPED_EEPROM_START >> 8
	ldi		r24, EEPROM_SIZE & 0xFF
	ldi		r25, EEPROM_SIZE >> 8
erase_all_loop:
	st		Z+, r1
	sbiw	r24, 1
	brne	erase_all_loop
	; execute erase command
	rcall	NVM_wait
	ldi		r24, NVM_CMD_ERASE_EEPROM_gc
	rjmp	NVM_execute_command
#endif



/**************************************************************************************************
* Write a page to EEPROM
* uint8 page
*/
.section .nvm_eeprom_atomic_write_page,"ax",@progbits
NVM_eeprom_atomic_write_page:
	rcall	NVM_wait
#if !defined(__AVR_ATxmega8E5__) && !defined(__AVR_ATxmega16E5__) && !defined(__AVR_ATxmega32E5__)
	// disable memory mapping
	lds		r18, NVM_CTRLB
	push	r18
	andi	r18, ~NVM_EEMAPEN_bm
;	sts		NVM_CTRLB, r18
#endif
	; calculate page address
	ldi		r18, EEPROM_PAGE_SIZE
	mul		r24, r18
	; execute page write
	sts		NVM_ADDR0, r0					; page address
	sts		NVM_ADDR1, r1
	clr		r1
	sts		NVM_ADDR2, r1
	ldi		r24, NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc
#if !defined(__AVR_ATxmega8E5__) && !defined(__AVR_ATxmega16E5__) && !defined(__AVR_ATxmega32E5__)
	rcall	NVM_execute_command
	// restore memory mapping
	pop		r18
	sts		NVM_CTRLB, r18
	ret
#else
	rjmp	NVM_execute_command
#endif



/**************************************************************************************************
* Load EEPROM write buffer (not used on E5)
* *data
* uint8 size in bytes to load, max EEPROM_PAGE_SIZE
*/
.section .nvm_eeprom_load_buffer,"ax",@progbits
NVM_eeprom_load_buffer:
	rcall	NVM_wait

	lds		r18, NVM_CTRLB					; save memory mapping state
	push	r18
	andi	r18, ~NVM_EEMAPEN_bm			; disable memory mapping
	sts		NVM_CTRLB, r18

	ldi		r19, NVM_CMD_LOAD_EEPROM_BUFFER_gc
	sts		NVM_CMD, r19

	sts		NVM_ADDR1, r1
	sts		NVM_ADDR2, r1

	movw	Z, r24							; data pointer
	add		ZL, r22
	adc		ZH, r1
	dec		r22
eeprom_buffer_load_loop:
	sts		NVM_ADDR0, r22
	ld		r20, -Z
	sts		NVM_DATA0, r22
	dec		r22								; size
	brpl	eeprom_buffer_load_loop

	pop		r18
	sts		NVM_CTRLB, r18					; restore memory mapping state
	ret


/**************************************************************************************************
* Erase EEPROM write buffer (not used on E5)
*/
.section .nvm_boot_crc,"ax",@progbits
NVM_eeprom_erase_write_buffer:
	rcall	NVM_wait
	ldi		r24, NVM_CMD_ERASE_EEPROM_BUFFER_gc
	rjmp	NVM_execute_command



/**************************************************************************************************
* NVM utility functions
*/

; wait for NVM to finish
.section .nvm_wait,"ax",@progbits
NVM_wait:
	lds		r18, NVM_STATUS
	sbrc	r18, NVM_NVMBUSY_bp
	rjmp	NVM_wait
	ret

; execute command in r20
.section .nvm_execute_command,"ax",@progbits
NVM_execute_command:
	sts		NVM_CMD, r24					; load command
	ldi		r18, CCP_IOREG_gc				; prepare Protect IO-register signature
	ldi		r19, NVM_CMDEX_bm
	sts		CCP, r18
	sts		NVM_CTRLA, r19					; execute command

	lds		r22, NVM_DATA0					; return value
	lds		r23, NVM_DATA1
	lds		r24, NVM_DATA2
	clr		r25								; clear R25 in order to return a clean 32-bit value for C
	sts		NVM_CMD, r1
	ret
