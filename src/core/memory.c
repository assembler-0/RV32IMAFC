#include "memory.h"
#include <stdio.h>

void memory_init(memory_t* memory) {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        memory->mem[i] = 0;
    }
}

uint32_t memory_read(memory_t* memory, uint32_t address) {
    if (address + 3 >= MEMORY_SIZE) {
        printf("Error: Memory read out of bounds at address 0x%08x\n", address);
        return 0;
    }
    return *(uint32_t*)(memory->mem + address);
}

void memory_write(memory_t* memory, uint32_t address, uint32_t value) {
    if (address + 3 >= MEMORY_SIZE) {
        printf("Error: Memory write out of bounds at address 0x%08x\n", address);
        return;
    }
    *(uint32_t*)(memory->mem + address) = value;
}

uint8_t memory_read_byte(memory_t* memory, uint32_t address) {
    if (address >= MEMORY_SIZE) {
        printf("Error: Memory read byte out of bounds at address 0x%08x\n", address);
        return 0;
    }
    return memory->mem[address];
}

uint16_t memory_read_halfword(memory_t* memory, uint32_t address) {
    if (address + 1 >= MEMORY_SIZE) {
        printf("Error: Memory read halfword out of bounds at address 0x%08x\n", address);
        return 0;
    }
    return *(uint16_t*)(memory->mem + address);
}

uint32_t memory_read_word(memory_t* memory, uint32_t address) {
    if (address + 3 >= MEMORY_SIZE) {
        printf("Error: Memory read word out of bounds at address 0x%08x\n", address);
        return 0;
    }
    return *(uint32_t*)(memory->mem + address);
}

void memory_write_byte(memory_t* memory, uint32_t address, uint8_t value) {
    if (address >= MEMORY_SIZE) {
        printf("Error: Memory write byte out of bounds at address 0x%08x\n", address);
        return;
    }
    memory->mem[address] = value;
}

void memory_write_halfword(memory_t* memory, uint32_t address, uint16_t value) {
    if (address + 1 >= MEMORY_SIZE) {
        printf("Error: Memory write halfword out of bounds at address 0x%08x\n", address);
        return;
    }
    *(uint16_t*)(memory->mem + address) = value;
}
void memory_write_word(memory_t* memory, uint32_t address, uint32_t value) {
    if (address + 1 >= MEMORY_SIZE) {
        printf("Error: Memory write halfword out of bounds at address 0x%08x\n", address);
        return;
    }
    *(uint32_t*)(memory->mem + address) = value;
}
