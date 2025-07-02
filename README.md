# RISC-V Emulator

A complete RISC-V RV32/64GC emulator implementation in C.

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
