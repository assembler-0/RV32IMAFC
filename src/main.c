#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/cpu.h"
#include "core/memory.h"

void print_result(cpu_t* cpu, uint32_t instruction, memory_t* memory) {
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t opcode = instruction & 0x7f;

    printf("--------------------------------\n");
    // Check if it's a store instruction to show memory state
    if (opcode == OPCODE_STORE) {
        uint32_t rs1 = (instruction >> 15) & 0x1f;
        uint32_t rs2 = (instruction >> 20) & 0x1f;
        uint32_t imm = 0;
        // S-type immediate
        uint32_t imm11_5 = (instruction >> 25) & 0x7F;
        uint32_t imm4_0 = (instruction >> 7) & 0x1F;
        imm = (int32_t)((imm11_5 << 5) | imm4_0);
        if (imm11_5 & 0x40) imm |= 0xFFFFF000; // Sign extend

        uint32_t addr = cpu->regs[rs1] + imm;
        printf("Memory at 0x%08x: %u\n", addr, memory_read_word(memory, addr));
    }
    else if (rd != 0) {
        printf("Result: x%d = %u (0x%x)\n", rd, cpu->regs[rd], cpu->regs[rd]);
    } else {
        printf("Result: No register change\n");
    }
    printf("PC: 0x%08x\n", cpu->pc);
    printf("Privilege Level: %u\n", cpu->privilege);
    printf("--------------------------------\n\n");
}

int main() {
    cpu_t cpu;
    memory_t memory;
    FILE* file;
    char line[1024];
    uint32_t instruction;
    int test_num = 1;

    cpu_init(&cpu);
    memory_init(&memory);

    file = fopen("instruction.bin", "r");
    if (!file) {
        printf("Error: Cannot open instruction.bin\n");
        return 1;
    }

    printf("=== RISC-V Emulator Test ===\n\n");

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        char* comment = strchr(line, '#');
        if (sscanf(line, "%x", &instruction) == 1) {
            printf("Test %d: 0x%08x%s", test_num++, instruction, comment ? comment : "\n");
            
            cpu_execute(&cpu, &memory, instruction);
            print_result(&cpu, instruction, &memory);
        }
    }

    fclose(file);
    return 0;
}
