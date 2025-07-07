in C        ; input number in C (counter)
movr A 1    ; result starts at 1

loop_start:
mul C       ; A = A * C
dec C
loop loop_start  ; decrement C and jump if C != 0

out A
halt
