#include "cpu.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

// ------ instruction functions
static int add_reg(struct cpu *cpu);
static int sub_reg(struct cpu *cpu);
static int mul_reg(struct cpu *cpu);
static int div_reg(struct cpu *cpu);
static int inc_reg(struct cpu *cpu);
static int dec_reg(struct cpu *cpu);
static int loop_ind(struct cpu *cpu);
static int movr(struct cpu *cpu);
static int load_reg(struct cpu *cpu);
static int store_reg(struct cpu *cpu);
static int in_reg(struct cpu *cpu);
static int get_reg(struct cpu *cpu);
static int out_reg(struct cpu *cpu);
static int put_reg(struct cpu *cpu);
static int swap_reg(struct cpu *cpu);
static int push_reg(struct cpu *cpu);
static int pop_reg(struct cpu *cpu);

// ------ tool function
static int validate_register(struct cpu *cpu, enum cpu_register reg);

struct cpu {
    int32_t reg_a;
    int32_t reg_b;
    int32_t reg_c;
    int32_t reg_d;
    int32_t next_instr;
    int32_t *memory_point;
    int32_t *stack_start;
    int32_t *stack_end;
    int32_t stack_amount;
    int32_t stack_last_val;
    int32_t stack_first_index;
    enum cpu_status status;
};

int32_t* cpu_create_memory(FILE *program, size_t stack_capacity, int32_t **stack_bottom)
{
    // check if the parameters are NULL
    assert(program != NULL);
    assert(stack_bottom != NULL);

    int32_t input = 0;
    int32_t cell = 0;
    int32_t byte_counter = 0;

    // allocate initial 4 KiB
    size_t memory_length = 1024;
    int32_t memory_index = 0;
    int32_t *p_memory = calloc(1024, sizeof(int32_t));
    if (p_memory == NULL) {
        return NULL;
    }

    // read the program 4 bytes at a time
    while ((input = fgetc(program)) != EOF) {
        input = input << (8 * byte_counter);
        cell += input;
        byte_counter++;

        // save cell to the memory
        if (byte_counter == 4) {

            // check if we have enough allocated memory
            if (memory_index + stack_capacity >= memory_length) {
                int32_t *new_memory = NULL;
                new_memory = (int32_t*)realloc(p_memory, (memory_length + 1024) * sizeof(int32_t));
                if (new_memory == NULL) {
                    free(p_memory);
                    return NULL;
                }
                p_memory = new_memory;

                // clear the unset cells
                memory_length += 1024;
                memset(p_memory + memory_index, 0, (memory_length - memory_index) * sizeof(int32_t));
            }

            p_memory[memory_index] = cell;
            memory_index++;
            byte_counter = 0;
            cell = 0;
        }
    }

    // check if the input was correct
    if (byte_counter == 0) {

        // loading was correct
        *stack_bottom = &p_memory[memory_length - 1];
        return p_memory;
    }

    // loading was incorrect
    free(p_memory);
    return NULL;
}

struct cpu *cpu_create(int32_t *memory, int32_t *stack_bottom, size_t stack_capacity)
{
    // check if the parameters are NULL
    assert(memory != NULL);
    assert(stack_bottom != NULL);

    // initialize cpu
    struct cpu *cpu = malloc(sizeof(struct cpu));
    if (cpu == NULL) {
        return NULL;
    }

    // setup all the attributes
    cpu->reg_a = 0;
    cpu->reg_b = 0;
    cpu->reg_c = 0;
    cpu->reg_d = 0;
    cpu->stack_amount = 0;
    cpu->next_instr = 0;
    cpu->stack_start = stack_bottom;
    cpu->stack_end = stack_bottom - stack_capacity + 1;
    cpu->memory_point = memory;
    cpu->stack_last_val = cpu->stack_start - cpu->memory_point;
    cpu->stack_first_index = cpu->stack_start - cpu->memory_point;
    cpu->status = CPU_OK;
    return cpu;
}

int32_t cpu_get_register(struct cpu *cpu, enum cpu_register reg)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // check if the given register is valid
    assert(reg >= REGISTER_A && reg <= REGISTER_D);

    // get the value from register
    switch (reg) {
        case REGISTER_A:
            return cpu->reg_a;
        case REGISTER_B:
            return cpu->reg_b;
        case REGISTER_C:
            return cpu->reg_c;
        default:
            return cpu->reg_d;
    }
}

void cpu_set_register(struct cpu *cpu, enum cpu_register reg, int32_t value)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // check if the given register is valid
    assert(reg >= REGISTER_A && reg <= REGISTER_D);

    // set the register to value
    if (reg == REGISTER_A) {
        cpu->reg_a = value;
    }
    else if (reg == REGISTER_B) {
        cpu->reg_b = value;
    }
    else if (reg == REGISTER_C) {
        cpu->reg_c = value;
    }
    else {
        cpu->reg_d = value;
    }
    return;
}

enum cpu_status cpu_get_status(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);
    return cpu->status;
}

int32_t cpu_get_stack_size(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);
    return cpu->stack_amount;
}

void cpu_destroy(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // dealocate the memory and reset all the attributes
    free(cpu->memory_point);
    cpu->reg_a = 0;
    cpu->reg_b = 0;
    cpu->reg_c = 0;
    cpu->reg_d = 0;
    cpu->next_instr = 0;
    cpu->memory_point = NULL;
    cpu->stack_amount = 0;
    cpu->stack_last_val = 0;
    cpu->stack_first_index = 0;
    cpu->stack_start = NULL;
    cpu->stack_end = NULL;
    cpu->status = CPU_OK;
    return;
}

void cpu_reset(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // reset stack and the necessary registers
    memset(cpu->stack_end, 0, (cpu->stack_start - cpu->stack_end + 1) * sizeof(int32_t));
    cpu->reg_a = 0;
    cpu->reg_b = 0;
    cpu->reg_c = 0;
    cpu->reg_d = 0;
    cpu->stack_amount = 0;
    cpu->stack_last_val = 0;
    cpu->stack_first_index = 0;
    cpu->next_instr = 0;
    cpu->status = CPU_OK;
    return;
}

int cpu_step(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // check if the status is CPU_OK
    if (cpu->status != CPU_OK) {
 
        // check if the status is unknown
        if (cpu->status < CPU_OK || cpu->status > CPU_IO_ERROR) {
            cpu->status = CPU_ILLEGAL_INSTRUCTION;
        }
        return 0;
    }

    // check if we are accessing outside memory or stack
    if (cpu->next_instr < 0 || cpu->next_instr >= (cpu->stack_end - cpu->memory_point)) {
        cpu->status = CPU_INVALID_ADDRESS;
        return 0;
    }

    // check if the instruction is correct
    if (cpu->memory_point[cpu->next_instr] < 0 || cpu->memory_point[cpu->next_instr] > 18) {
        cpu->status = CPU_ILLEGAL_INSTRUCTION;
        return 0;
    }

    // execute one instruction
    switch (cpu->memory_point[cpu->next_instr]) {
        // don't do anything
        case 0:
            cpu->next_instr += 1;
            return 1;
        // halted process
        case 1:
            cpu->status = CPU_HALTED;
            return 0;
        case 2:
            return add_reg(cpu);
        case 3:
            return sub_reg(cpu);
        case 4:
            return mul_reg(cpu);
        case 5:
            return div_reg(cpu);
        case 6:
            return inc_reg(cpu);
        case 7:
            return dec_reg(cpu);
        case 8:
            return loop_ind(cpu);
        case 9:
            return movr(cpu);
        case 10:
            return load_reg(cpu);
        case 11:
            return store_reg(cpu);
        case 12:
            return in_reg(cpu);
        case 13:
            return get_reg(cpu);
        case 14:
            return out_reg(cpu);
        case 15:
            return put_reg(cpu);
        case 16:
            return swap_reg(cpu);
        case 17:
            return push_reg(cpu);
        default:
            return pop_reg(cpu);
    }
}

long long cpu_run(struct cpu *cpu, size_t steps)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // if the processor is shut down from the beginning
    if (cpu->status != CPU_OK) {
 
        // check if the status is unknown
        if (cpu->status < CPU_OK || cpu->status > CPU_IO_ERROR) {
            cpu->status = CPU_ILLEGAL_INSTRUCTION;
        }
        return 0;
    }

    int32_t executed = 0;

    // execute steps
    for (size_t i = 1; i < steps + 1; i++) {
        executed = cpu_step(cpu);
        if (!executed) {

            // if the program was correctly halted
            if (cpu->status == CPU_HALTED) {
                return i;
            }

            // if there was an error
            return i * -1;
        }
    }

    return steps;
}

static int validate_register(struct cpu *cpu, enum cpu_register reg)
{
    // check validity of input register
    if (reg < REGISTER_A || reg > REGISTER_D) {
        cpu->status = CPU_ILLEGAL_OPERAND;
        return 0;
    }

    return 1;
}

// ----- instruction functions

static int add_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);
    
    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    cpu->reg_a += cpu_get_register(cpu, reg);
    cpu->next_instr++;
    return 1;
}

static int sub_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);
    
    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    cpu->reg_a -= cpu_get_register(cpu, reg);
    cpu->next_instr++;
    return 1;
}

static int mul_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);
    
    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    cpu->reg_a *= cpu_get_register(cpu, reg);
    cpu->next_instr++;
    return 1;
}

static int div_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);
    
    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    // check if register value is 0
    if (cpu_get_register(cpu, reg) == 0) {
        cpu->status = CPU_DIV_BY_ZERO;
        return 0;
    }

    cpu->reg_a /= cpu_get_register(cpu, reg);
    cpu->next_instr++;
    return 1;
}

static int inc_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    cpu_set_register(cpu, reg, cpu_get_register(cpu, reg) + 1);
    cpu->next_instr++;
    return 1;
}

static int dec_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);
    
    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    cpu_set_register(cpu, reg, cpu_get_register(cpu, reg) - 1);
    cpu->next_instr++;
    return 1;
}

static int loop_ind(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);
    
    // load index
    cpu->next_instr++;
    int32_t index = cpu->memory_point[cpu->next_instr];

    // check if register C is not equal to zero
    if (cpu_get_register(cpu, 2) == 0) {
        cpu->next_instr++;
        return 1;
    }

    cpu->next_instr = index;
    return 1;
}

static int movr(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    // load number
    cpu->next_instr++;
    int32_t number = cpu->memory_point[cpu->next_instr];

    cpu_set_register(cpu, reg, number);
    cpu->next_instr++;
    return 1;
}

static int load_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    // load number
    cpu->next_instr++;
    int32_t number = cpu->memory_point[cpu->next_instr];

    int32_t reg_d = cpu_get_register(cpu, REGISTER_D);

    // check if we are correctly accessing the stack
    if (cpu->stack_last_val + reg_d + number > cpu->stack_first_index) {
        cpu->next_instr -= 2;
        cpu->status = CPU_INVALID_STACK_OPERATION;
        return 0;
    }

    if (cpu->stack_last_val + reg_d + number < cpu->stack_last_val) {
        cpu->next_instr -= 2;
        cpu->status = CPU_INVALID_STACK_OPERATION;
        return 0;
    }

    // check if the stack is empty
    if (cpu->stack_amount == 0) {
        cpu->status = CPU_INVALID_STACK_OPERATION;
        return 0;
    }

    cpu_set_register(cpu, reg, cpu->memory_point[cpu->stack_last_val + reg_d + number]);
    cpu->next_instr++;
    return 1;
}

static int store_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    // load number
    cpu->next_instr++;
    int32_t number = cpu->memory_point[cpu->next_instr];

    // load number from stack to the register
    int32_t reg_d = cpu_get_register(cpu, REGISTER_D);

    // check if we are correctly accessing the stack
    if (cpu->stack_last_val + reg_d + number > cpu->stack_first_index) {
        cpu->next_instr -= 2;
        cpu->status = CPU_INVALID_STACK_OPERATION;
        return 0;
    }

    if (cpu->stack_last_val + reg_d + number < cpu->stack_last_val) {
        cpu->next_instr -= 2;
        cpu->status = CPU_INVALID_STACK_OPERATION;
        return 0;
    }

    // check if the stack is empty
    if (cpu->stack_amount == 0) {
        cpu->status = CPU_INVALID_STACK_OPERATION;
        return 0;
    }

    cpu->memory_point[cpu->stack_last_val + reg_d + number] = cpu_get_register(cpu, reg);
    cpu->next_instr++;
    return 1;
}

static int in_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    long long int input = 0;
    int result = scanf("%lld", &input);

    if (result == EOF) {
        cpu_set_register(cpu, 2, 0);
        cpu_set_register(cpu, reg, -1);
        cpu->next_instr++;
        return 1;
    }
    else if (result != 1 || input < INT32_MIN || input > INT32_MAX) {
        cpu->status = CPU_IO_ERROR;
        return 0;
    }
    else {
        cpu_set_register(cpu, reg, input);
        cpu->next_instr++;
        return 1;
    }
}

static int get_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    // read input
    int32_t input = 0;

    // if there is nothing left and the file ends with EOF
    if ((input = getchar()) == EOF) {
        cpu_set_register(cpu, 2, 0);
        cpu_set_register(cpu, reg, -1);
        cpu->next_instr++;
        return 1;
    }

    // store the value to the given register
    cpu_set_register(cpu, reg, input);
    cpu->next_instr++;
    return 1;
}

static int out_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    printf("%d \n", cpu_get_register(cpu, reg));
    cpu->next_instr++;
    return 1;
}

static int put_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    // check the value from the register
    int32_t reg_value = cpu_get_register(cpu, reg);
    if (reg_value < 0 || reg_value > 255) {
        cpu->status = CPU_ILLEGAL_OPERAND;
        return 0;
    }

    // output the value as character
    printf("%c", reg_value);
    cpu->next_instr++;
    return 1;
}

static int swap_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load registers
    cpu->next_instr++;
    enum cpu_register reg_one = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg_one)) {
        cpu->next_instr--;
        return 0;
    }

    cpu->next_instr++;
    enum cpu_register reg_two = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg_two)) {
        cpu->next_instr -= 2;
        return 0;
    }

    // swap registers
    int32_t swap_helper = cpu_get_register(cpu, reg_one);
    cpu_set_register(cpu, reg_one, cpu_get_register(cpu, reg_two));
    cpu_set_register(cpu, reg_two, swap_helper);
    cpu->next_instr++;
    return 1;
}

static int push_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    // check if the stack is full
    int32_t stack_size = cpu->stack_start - cpu->stack_end + 1;
    if (cpu->stack_amount == stack_size) {
        cpu->status = CPU_INVALID_STACK_OPERATION;
        return 0;
    }

    // push value from the register on the stack
    if (cpu->stack_amount != 0) {
        cpu->stack_last_val--;
    }

    cpu->memory_point[cpu->stack_last_val] = cpu_get_register(cpu, reg);
    cpu->stack_amount++;
    cpu->next_instr++;
    return 1;
}

static int pop_reg(struct cpu *cpu)
{
    // check if the parameters are NULL
    assert(cpu != NULL);

    // load register
    cpu->next_instr++;
    enum cpu_register reg = cpu->memory_point[cpu->next_instr];

    if (!validate_register(cpu, reg)) {
        cpu->next_instr--;
        return 0;
    }

    // check if the stack is empty
    if (cpu->stack_amount == 0) {
        cpu->status = CPU_INVALID_STACK_OPERATION;
        return 0;    
    }

    // pop the value from the stack to the register
    cpu_set_register(cpu, reg, cpu->memory_point[cpu->stack_last_val]);
    cpu->memory_point[cpu->stack_last_val] = 0;
    cpu->stack_amount--;
    if (cpu->stack_amount != 0) {
        cpu->stack_last_val++;
    }
    cpu->next_instr++;
    return 1;
}
