.device ATtiny13A

    ldi  r16, $18
    ldi  r17, $2

    ldi  r25, $10
    ldi  r24, $FF

routine:


loop:
    sub r16, r17
    adiw r25:r24, $1
    rjmp loop
    