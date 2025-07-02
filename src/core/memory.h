#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 0x100000 // 1MB of memory

typedef struct {
    uint8_t mem[MEMORY_SIZE];
} memory_t;

void memory_init(memory_t* memory);
uint32_t memory_read(memory_t* memory, uint32_t address);
void memory_write(memory_t* memory, uint32_t address, uint32_t value);
uint8_t memory_read_byte(memory_t* memory, uint32_t address);
uint16_t memory_read_halfword(memory_t* memory, uint32_t address);
uint32_t memory_read_word(memory_t* memory, uint32_t address);
void memory_write_byte(memory_t* memory, uint32_t address, uint8_t value);
void memory_write_halfword(memory_t* memory, uint32_t address, uint16_t value);
void memory_write_word(memory_t* memory, uint32_t address, uint32_t value);
uint64_t memory_read_doubleword(memory_t* memory, uint32_t address);
void memory_write_doubleword(memory_t* memory, uint32_t address, uint64_t value);

#endif // MEMORY_H
