#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    PTR_L,
    PTR_R,
    INCR,
    DECR,
    OUTPUT,
    INPUT,
    JMP_IZ,
    JMP_NZ,
    END
};

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
typedef uintptr_t instruction_idx_t;

uint8_t* instructions_ptr;
uint8_t* instructions_base;
instruction_idx_t* call_stack;

void allocate_call_stack() {
    call_stack = malloc(CALL_STACK * sizeof(instruction_idx_t));
}

void mark_new_call() {
    call_stack += sizeof(instruction_idx_t);
    *call_stack = (instruction_idx_t)instructions_ptr;
}

void finish_call() {
    call_stack -= sizeof(instruction_idx_t);
}

void jump_to_call_start() {
    instructions_ptr = (uint8_t*)call_stack;
}

/// Returns 1 of error.
uint8_t load_from_file(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return 1;
    }
    fseek(file, 0, SEEK_END);
    instruction_idx_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // This allocation is an overestimate.
    instructions_base = malloc(size); // Do not overwrite the start.
    instructions_ptr = instructions_base;
    uint8_t* instructions_cpy = instructions_base;
    // This will probably be slow.
    char token;
    for (instruction_idx_t i = 0; i < size; i++) {
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

void load_input(char* source, instruction_idx_t size) {
    instructions_base = malloc(size);
    instructions_ptr = instructions_base;
    uint8_t* instructions_cpy = instructions_base;
    for (instruction_idx_t i = 0; i < size; i++) {
        uint8_t tok = into_token(source + i);
        if (tok != END) {
            *instructions_cpy = tok;
            instructions_cpy++;
        }
    }
    *instructions_cpy = END;
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
    while (*instructions_ptr != END) {
        switch (*instructions_ptr) {
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
                    instructions_ptr++;
                    if (*instructions_ptr == JMP_IZ)
                        depth++;
                    else if (*instructions_ptr == JMP_NZ)
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

        instructions_ptr++;
    }

    free(instructions_base);
}

void configure() {
    allocate_call_stack();
    allocate_memory();
}

void repl_loop() {
    char* input = malloc(1024);
    while (1) {
        printf(">>> ");
        scanf("%1024s", input);
        load_input(input, 1024);
        execute();
    }
}

// Entry Point
int main(int argc, char* argv[]) {
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
        printf("Starting REPL...\n");
        configure();
        repl_loop();
        return 0;
    }
}
