#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PTR_L 0
#define PTR_R 1
#define INCR 2
#define DECR 3
#define OUTPUT 4
#define INPUT 5
#define JMP_IZ 6
#define JMP_NZ 7
#define END 8

uint8_t into_token(char* character) {
    switch (*character) {
    case '>':
        return PTR_R;
    case '<':
        return PTR_L;
    case '+':
        return INCR;
    case '-':
        return DECR;
    case '.':
        return OUTPUT;
    case ',':
        return INPUT;
    case '[':
        return JMP_IZ;
    case ']':
        return JMP_NZ;
    default:
        return END;
    }
}

// Memory impl details
#define MEM_SIZE 5192
uint8_t* memory_begin;
uint8_t* mem_ptr;

void allocate_memory() {
    memory_begin = malloc(MEM_SIZE);
    mem_ptr = memory_begin;
}

// Instruction Data
#define CALL_STACK 1024
#define INSTRUCTION_IDX uint64_t
uint8_t* instructions;
INSTRUCTION_IDX* call_stack;

void allocate_call_stack() {
    call_stack = malloc(CALL_STACK * sizeof(INSTRUCTION_IDX));
}

void mark_new_call() {
    call_stack += sizeof(INSTRUCTION_IDX);
    *call_stack = instructions;
}

void finish_call() {
    call_stack -= sizeof(INSTRUCTION_IDX);
}

void jump_to_call_start() {
    instructions = *call_stack;
}

/// Returns 1 of error.
uint8_t load_from_file(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return 1;
    }
    fseek(file, 0, SEEK_END);
    INSTRUCTION_IDX size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // This allocation is an overestimate.
    instructions = malloc(size); // Do not overwrite the start.
    uint8_t* instructions_cpy = instructions;
    // This will probably be slow.
    char token;
    for (INSTRUCTION_IDX i = 0; i < size; i++) {
        fread(&token, 1, 1, file);
        uint8_t tok = into_token(&token);
        if (tok != END) {
            *instructions_cpy = tok;
            instructions_cpy++;
        }
    }
    *instructions_cpy = END;
    return 0;
}

// Logic
void ptr_l() {
    mem_ptr++;
}
void ptr_r() {
    mem_ptr--;
}
void incr() {
    (*mem_ptr)++;
}
void decr() {
    (*mem_ptr)--;
}
void output() {
    printf("%c", *mem_ptr);
}
void input() {
    scanf("%c", mem_ptr);
}

// Execution
void execute() {
    while (*instructions != END) {
        switch (*instructions) {
        case PTR_L:
            ptr_l();
            break;
        case PTR_R:
            ptr_r();
            break;
        case INCR:
            incr();
            break;
        case DECR:
            decr();
            break;
        case OUTPUT:
            output();
            break;
        case INPUT:
            input();
            break;
        
        // Fancy control flow stuff
        case JMP_IZ:
            if (*mem_ptr == 0) {
                // Find the next matching ]
                uint16_t depth = 1;
                do {
                    instructions++;
                    if (*instructions == JMP_IZ)
                        depth++;
                    else if (*instructions == JMP_NZ)
                        depth--;
                } while (depth);
            }
            else
                mark_new_call();
            break;
        case JMP_NZ:
            if (*mem_ptr != 0)
                jump_to_call_start();
            break;
        }

        instructions++;
    }
}

void configure() {
    allocate_call_stack();
    allocate_memory();
}

// Entry Point
int32_t main(int32_t argc, char* argv[]) {
    printf("BrainC 1.0.0 - The BrainF interpreter\n");
    if (argc > 1) {
        char* filename = argv[1];
        if (load_from_file(filename)) {
            printf("Could not read from file!\n");
            return 1;
        }
        configure();
        execute();
        printf("\n");
        return 0;
    }
    else {
        printf("No file supplied! Doing nothing.\n");
        return 1;
    }
}
