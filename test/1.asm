.device ATtiny13A

loop:
	adc  r15, r16
	nop
	rjmp loop
	