cmake_minimum_required(VERSION 3.20)
project(riscv-emulator C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# RISC-V Architecture Option
option(RV64 "Build for RV64 (64-bit RISC-V)" OFF)

if(RV64)
    add_definitions(-DXLEN=64)
    message(STATUS "Building for RV64 (64-bit RISC-V)")
else()
    add_definitions(-DXLEN=32)
    message(STATUS "Building for RV32 (32-bit RISC-V)")
endif()

# Add compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3 -march=native -mtune=native")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -DDEBUG")

# Include directories
include_directories(src/core)

# Source files
set(SOURCES
    src/main.c
    src/core/cpu.c
    src/core/decode.c
    src/core/memory.c
    src/core/jump_table.c
)

# Create executable
add_executable(riscv ${SOURCES})

# Link math library
target_link_libraries(riscv m)

# Optional: Create install target
install(TARGETS riscv DESTINATION bin)