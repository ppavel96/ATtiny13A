.device ATtiny13A

    ldi  r16, $1
    ldi  r17, $2

loop:
    add  r16, r17
    rjmp loop
    