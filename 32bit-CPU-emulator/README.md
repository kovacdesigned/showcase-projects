This is an emulator for a simple 32-bit processor, written in C.  

## Features
- General-purpose registers
- Basic instruction set (arithmetic, memory access, control flow)
- 32-bit architecture
- Support for binary input files


## Project Structure
- cpu.c # Emulator core
- cpu.h # CPU definitions and register structure
- main.c # Entry point for the emulator
- CMakeLists.txt # Build configuration


## Assembly file syntax
- Only one instruction per line.
- Operands are separated by a single space.
- Leading/trailing spaces are allowed.
- Comments start with ;.
- Labels are alphanumeric identifiers ending with : (e.g., loop_start:).
- Labels can be used where an INDEX is expected.


## Running the Emulator
After building, run the emulator with the compiled binary.
The emulator accepts two or three arguments:

    ./cpu <mode> [stack_capacity] <program.bin>

- mode:
run — Executes the entire program and prints the final CPU state.
trace — Shows the CPU state after each instruction and waits for Enter before continuing.

- stack_capacity (optional)
Specifies the stack size. If omitted, a default value is used.

- program.bin:
Path to the binary program file.


## CPU Overview
The CPU uses:
- Registers: A, B, C, D
- A status register (CPU state codes)
- A stack pointer
- A program counter (current instruction)
- A program memory buffer
- Stack memory region at the end of program memory (grows downwards)

Programs begin at the start of memory. The stack grows from the end toward the beginning.


## Instruction Format
Each instruction is 32 bits. Operands (registers, numbers, instruction indices) are also 32-bit and use little-endian format.

Instructions may use:
REG: Register index (0 = A, 1 = B, 2 = C, 3 = D)
INDEX: Instruction index (for jumps and loops)
NUM: Literal number


## Status Codes
The CPU sets a status code:

- CPU_OK
- CPU_HALTED
- CPU_ILLEGAL_INSTRUCTION
- CPU_ILLEGAL_OPERAND
- CPU_INVALID_ADDRESS
- CPU_INVALID_STACK_OPERATION
- CPU_DIV_BY_ZERO
- CPU_IO_ERROR


## Instruction Set
-   halt — Stops execution and sets status to CPU_HALTED.

-   add REG — Adds value of REG to A.
-   sub REG — Subtracts value of REG from A.
-   mul REG — Multiplies A by REG.
-   div REG — Divides A by REG. Sets CPU_DIV_BY_ZERO if REG is 0.
-   inc REG — Increments REG.
-   dec REG — Decrements REG.

-   loop INDEX — If C is not zero, jump to instruction INDEX.
-   movr REG NUM — Sets REG to NUM.

-   push REG — Pushes REG value onto stack. Fails if full.
-   pop REG — Pops top value from stack into REG. Fails if empty.
-   load REG NUM — Loads value from stack at offset D + NUM into REG.
-   store REG NUM — Stores REG value at offset D + NUM in stack.

-   in REG — Reads a number from input and stores in REG. On EOF, stores -1 in REG and sets C to 0.
-   get REG — Reads one byte from input and stores in REG. On EOF, acts like in.
-   out REG — Prints the number in REG to output.
-   put REG — Prints the ASCII character (0–255) in REG. Sets CPU_ILLEGAL_OPERAND if out of range.

-   swap REG REG — Swaps two registers.
