movr C 15      ; Store a number into A
loop_start:
out C		; Output current value
dec C
loop loop_start ; Decrement C, jump to loop_start if C != 0
halt
