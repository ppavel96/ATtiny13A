.include "tn13def.inc"

	ldi	r16, 0x00	; do not affect PORTB
	out	DDRB, r16	
	out	PORTB, r16
	ldi	r18, 0x00	; initial counter value
	
EEPROM_Write:
	sbic	EECR, EEPE	           ; wait for ready
	rjmp	EEPROM_Write
	ldi	r16, (0<<EEPM1)|(0<<EEPM0) ; erase and write
	out	EECR, r16	               ; set write mode
	ldi	r16, 0x00
	out	EEARL, r18	               ; set address
	out	EEDR, r18	               ; write the same value
	sbi	EECR, EEMPE	               ; reset state
	sbi	EECR, EEPE
	ldi	r17, 1		
	adc	r18, r17	               ; increment r18 by 1 
	ldi	r17, 20		               ; stop when r18==20
	cpse	r18, r17
	rjmp	EEPROM_Write	
	sleep
	