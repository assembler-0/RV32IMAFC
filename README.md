# RISC-V Emulator

A complete RISC-V RV32/64GC emulator implementation in C.

## Features

- **RV32I** - Base integer instruction set
- **RV32M** - Integer multiplication and division
- **RV32A** - Atomic instructions
- **RV32F** - Single-precision floating-point
- **RV32D** - Double-precision floating-point  
- **RV32C** - Compressed instructions
- **Zicsr** - Control and status register instructions
- **Privilege levels** - Machine, Supervisor, User modes
- **Memory management** - Load/store operations
- **Exception handling** - ECALL, EBREAK, MRET, SRET

## Building

### Using CMake
```bash
mkdir build
cd build
cmake .. -XLEN=64 -DCMAKE_BUILD_TYPE=Release
make
```

### Using Make
```bash
make
```

## Usage

```bash
./riscv
```

The emulator will execute test instructions from `instruction.bin` or run built-in tests.

## Architecture

- `src/core/cpu.c` - CPU execution engine
- `src/core/decode.c` - Instruction decoding
- `src/core/memory.c` - Memory subsystem
- `src/main.c` - Main program and test harness

## License

MIT License - see LICENSE file for details.
