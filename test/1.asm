.device ATtiny13A

    ldi  r16, 10
    ldi  r17, 10
    rjmp  loop

routine:
    mov r16, r17

loop:
    dec r16
    breq routine
    rjmp loop

.ESEG
.DB 0x01
.DB 0x02
.DB 0x03
