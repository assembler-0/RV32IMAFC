# RV64GC ISA Test Suite
# Base Integer (RV64I)
addi x1, x0, 42         # Load immediate
addi x2, x0, 10         # Load immediate
add x3, x1, x2          # Add
sub x4, x1, x2          # Subtract
sll x5, x1, x2          # Shift left logical
slt x6, x1, x2          # Set less than
sltu x7, x1, x2         # Set less than unsigned
xor x8, x1, x2          # XOR
srl x9, x1, x2          # Shift right logical
sra x10, x1, x2         # Shift right arithmetic
or x11, x1, x2          # OR
and x12, x1, x2         # AND

# M extension (Multiplication and Division)
mul x13, x1, x2         # Multiply
mulh x14, x1, x2        # Multiply high signed-signed
mulhsu x15, x1, x2      # Multiply high signed-unsigned
mulhu x16, x1, x2       # Multiply high unsigned-unsigned
div x17, x1, x2         # Divide signed
divu x18, x1, x2        # Divide unsigned
rem x19, x1, x2         # Remainder signed
remu x20, x1, x2        # Remainder unsigned

# Immediate operations
slti x13, x1, 50        # Set less than immediate
sltiu x14, x1, 50       # Set less than immediate unsigned
xori x15, x1, 15        # XOR immediate
ori x16, x1, 15         # OR immediate
andi x17, x1, 15        # AND immediate
slli x18, x1, 2         # Shift left logical immediate
srli x19, x1, 2         # Shift right logical immediate
srai x20, x1, 2         # Shift right arithmetic immediate

# Upper immediate
lui x21, 0x12345        # Load upper immediate
auipc x22, 0x1000       # Add upper immediate to PC

# Memory operations
sw x1, 0(x0)            # Store word
lw x23, 0(x0)           # Load word
sh x2, 4(x0)            # Store halfword
lh x24, 4(x0)           # Load halfword
lhu x25, 4(x0)          # Load halfword unsigned
sb x1, 8(x0)            # Store byte
lb x26, 8(x0)           # Load byte
lbu x27, 8(x0)          # Load byte unsigned

# Branch operations
beq x1, x1, 4           # Branch if equal (taken)
nop                     # Should be skipped
bne x1, x2, 4           # Branch if not equal (taken)
nop                     # Should be skipped
blt x2, x1, 4           # Branch if less than (taken)
nop                     # Should be skipped
bge x1, x2, 4           # Branch if greater equal (taken)
nop                     # Should be skipped
bltu x2, x1, 4          # Branch if less than unsigned (taken)
nop                     # Should be skipped
bgeu x1, x2, 4          # Branch if greater equal unsigned (taken)
nop                     # Should be skipped

# Jump operations
jal x28, 8              # Jump and link
nop                     # Should be skipped
jalr x29, x28, 0        # Jump and link register

# RV64I (64-bit specific)
addiw x30, x1, 10       # Add immediate word
slliw x31, x1, 2        # Shift left logical immediate word
srliw x1, x31, 1        # Shift right logical immediate word
sraiw x2, x31, 1        # Shift right arithmetic immediate word
addw x3, x30, x31       # Add word
subw x4, x30, x31       # Subtract word
sllw x5, x30, x31       # Shift left logical word
srlw x6, x30, x31       # Shift right logical word
sraw x7, x30, x31       # Shift right arithmetic word

# System instructions
ecall                   # Environment call
ebreak                  # Environment break
mret                   # Machine return
sret                   # Supervisor return
wfi                    # Wait for interrupt
nop                     # No operation

# Compressed instructions (RVC) - basic ones
# c.addi x1, 5          # Compressed add immediate
# c.li x2, 10           # Compressed load immediate
# c.mv x3, x1           # Compressed move
# c.add x1, x2          # Compressed add

# Floating Point (RVF/RVD) - Single precision
# fadd.s f1, f2, f3     # Float add single
# fsub.s f1, f2, f3     # Float subtract single
# fmul.s f1, f2, f3     # Float multiply single
# fdiv.s f1, f2, f3     # Float divide single
# fsqrt.s f1, f2        # Float square root single
# fmin.s f1, f2, f3     # Float minimum single
# fmax.s f1, f2, f3     # Float maximum single
# feq.s x1, f1, f2      # Float equal single
# flt.s x1, f1, f2      # Float less than single
# fle.s x1, f1, f2      # Float less equal single
# fcvt.w.s x1, f1       # Convert float to int
# fcvt.s.w f1, x1       # Convert int to float
# flw f1, 0(x1)         # Float load word
# fsw f1, 0(x1)         # Float store word

# Double precision (RVD)
# fadd.d f1, f2, f3     # Float add double
# fsub.d f1, f2, f3     # Float subtract double
# fmul.d f1, f2, f3     # Float multiply double
# fdiv.d f1, f2, f3     # Float divide double
# fsqrt.d f1, f2        # Float square root double
# fcvt.d.s f1, f2       # Convert single to double
# fcvt.s.d f1, f2       # Convert double to single
# fld f1, 0(x1)         # Float load double
# fsd f1, 0(x1)         # Float store double

# Atomic operations (RVA)
# lr.w x1, (x2)         # Load reserved word
# sc.w x1, x3, (x2)     # Store conditional word
# amoswap.w x1, x3, (x2) # Atomic swap word
# amoadd.w x1, x3, (x2)  # Atomic add word
# amoxor.w x1, x3, (x2)  # Atomic XOR word
# amoand.w x1, x3, (x2)  # Atomic AND word
# amoor.w x1, x3, (x2)   # Atomic OR word
# amomin.w x1, x3, (x2)  # Atomic minimum word
# amomax.w x1, x3, (x2)  # Atomic maximum word

# CSR operations
# csrrw x1, mstatus, x2  # CSR read/write
# csrrs x1, mstatus, x2  # CSR read/set
# csrrc x1, mstatus, x2  # CSR read/clear
# csrrwi x1, mstatus, 5  # CSR read/write immediate
# csrrsi x1, mstatus, 5  # CSR read/set immediate
# csrrci x1, mstatus, 5  # CSR read/clear immediate

# Privileged instructions
# sfence.vma x1, x2      # Supervisor fence

# Final test
addi x31, x0, 0xDEAD    # End marker