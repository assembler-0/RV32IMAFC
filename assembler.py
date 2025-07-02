#!/usr/bin/env python3

import re
import sys

# Register mappings
REGS = {f'x{i}': i for i in range(32)}
REGS.update({
    'zero': 0, 'ra': 1, 'sp': 2, 'gp': 3, 't0': 5, 't1': 6, 't2': 7,
    's0': 8, 'fp': 8, 's1': 9, 'a0': 10, 'a1': 11, 'a2': 12, 'a3': 13,
    'a4': 14, 'a5': 15, 'a6': 16, 'a7': 17, 's2': 18, 's3': 19, 's4': 20,
    's5': 21, 's6': 22, 's7': 23, 's8': 24, 's9': 25, 's10': 26, 's11': 27,
    't3': 28, 't4': 29, 't5': 30, 't6': 31
})

def parse_reg(r):
    return REGS.get(r.lower(), 0)

def parse_imm(s):
    if s.startswith('0x'): return int(s, 16)
    return int(s)

def encode_r(op, rd, rs1, rs2, f3, f7):
    return (f7 << 25) | (rs2 << 20) | (rs1 << 15) | (f3 << 12) | (rd << 7) | op

def encode_i(op, rd, rs1, imm, f3):
    return ((imm & 0xfff) << 20) | (rs1 << 15) | (f3 << 12) | (rd << 7) | op

def encode_s(op, rs1, rs2, imm, f3):
    return ((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (f3 << 12) | ((imm & 0x1f) << 7) | op

def encode_b(op, rs1, rs2, imm, f3):
    return (((imm >> 12) & 1) << 31) | (((imm >> 5) & 0x3f) << 25) | (rs2 << 20) | (rs1 << 15) | (f3 << 12) | (((imm >> 1) & 0xf) << 8) | (((imm >> 11) & 1) << 7) | op

def encode_u(op, rd, imm):
    return (imm & 0xfffff000) | (rd << 7) | op

def encode_j(op, rd, imm):
    return (((imm >> 20) & 1) << 31) | (((imm >> 1) & 0x3ff) << 21) | (((imm >> 11) & 1) << 20) | (((imm >> 12) & 0xff) << 12) | (rd << 7) | op

def assemble_line(line):
    line = re.sub(r'#.*', '', line).strip()
    if not line: return None
    
    parts = re.split(r'[,\s]+', line)
    inst = parts[0].lower()
    
    # R-type
    if inst == 'add': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 0, 0)
    elif inst == 'sub': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 0, 0x20)
    elif inst == 'sll': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 1, 0)
    elif inst == 'slt': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 2, 0)
    elif inst == 'sltu': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 3, 0)
    elif inst == 'xor': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 4, 0)
    elif inst == 'srl': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 5, 0)
    elif inst == 'sra': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 5, 0x20)
    elif inst == 'or': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 6, 0)
    elif inst == 'and': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 7, 0)
    # M-extension
    elif inst == 'mul': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 0, 0x01)
    elif inst == 'mulh': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 1, 0x01)
    elif inst == 'mulhsu': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 2, 0x01)
    elif inst == 'mulhu': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 3, 0x01)
    elif inst == 'div': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 4, 0x01)
    elif inst == 'divu': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 5, 0x01)
    elif inst == 'rem': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 6, 0x01)
    elif inst == 'remu': return encode_r(0x33, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 7, 0x01)
    
    # I-type
    elif inst == 'addi': return encode_i(0x13, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 0)
    elif inst == 'slti': return encode_i(0x13, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 2)
    elif inst == 'sltiu': return encode_i(0x13, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 3)
    elif inst == 'xori': return encode_i(0x13, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 4)
    elif inst == 'ori': return encode_i(0x13, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 6)
    elif inst == 'andi': return encode_i(0x13, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 7)
    elif inst == 'slli': return encode_i(0x13, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 1)
    elif inst == 'srli': return encode_i(0x13, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 5)
    elif inst == 'srai': return encode_i(0x13, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 5) | (0x20 << 25)
    elif inst == 'jalr': return encode_i(0x67, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 0)
    
    # RV64I word operations
    elif inst == 'addiw': return encode_i(0x1b, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 0)
    elif inst == 'slliw': return encode_i(0x1b, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 1)
    elif inst == 'srliw': return encode_i(0x1b, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 5)
    elif inst == 'sraiw': return encode_i(0x1b, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), 5) | (0x20 << 25)
    elif inst == 'addw': return encode_r(0x3b, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 0, 0)
    elif inst == 'subw': return encode_r(0x3b, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 0, 0x20)
    elif inst == 'sllw': return encode_r(0x3b, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 1, 0)
    elif inst == 'srlw': return encode_r(0x3b, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 5, 0)
    elif inst == 'sraw': return encode_r(0x3b, parse_reg(parts[1]), parse_reg(parts[2]), parse_reg(parts[3]), 5, 0x20)
    
    # Load/Store
    elif inst in ['lb', 'lh', 'lw', 'lbu', 'lhu']:
        m = re.match(r'(\w+)\s*,\s*(-?\d+)\((\w+)\)', ' '.join(parts[1:]))
        if m:
            f3 = {'lb': 0, 'lh': 1, 'lw': 2, 'lbu': 4, 'lhu': 5}[inst]
            return encode_i(0x03, parse_reg(m.group(1)), parse_reg(m.group(3)), parse_imm(m.group(2)), f3)
    elif inst in ['sb', 'sh', 'sw']:
        m = re.match(r'(\w+)\s*,\s*(-?\d+)\((\w+)\)', ' '.join(parts[1:]))
        if m:
            f3 = {'sb': 0, 'sh': 1, 'sw': 2}[inst]
            return encode_s(0x23, parse_reg(m.group(3)), parse_reg(m.group(1)), parse_imm(m.group(2)), f3)
    
    # Branch
    elif inst in ['beq', 'bne', 'blt', 'bge', 'bltu', 'bgeu']:
        f3 = {'beq': 0, 'bne': 1, 'blt': 4, 'bge': 5, 'bltu': 6, 'bgeu': 7}[inst]
        return encode_b(0x63, parse_reg(parts[1]), parse_reg(parts[2]), parse_imm(parts[3]), f3)
    
    # Upper immediate
    elif inst == 'lui': return encode_u(0x37, parse_reg(parts[1]), parse_imm(parts[2]))
    elif inst == 'auipc': return encode_u(0x17, parse_reg(parts[1]), parse_imm(parts[2]))
    
    # Jump
    elif inst == 'jal':
        if len(parts) == 2: return encode_j(0x6f, 1, parse_imm(parts[1]))
        return encode_j(0x6f, parse_reg(parts[1]), parse_imm(parts[2]))
    
    # System
    elif inst == 'ecall': return 0x73
    elif inst == 'ebreak': return 0x100073
    elif inst == 'mret': return 0x30200073
    elif inst == 'sret': return 0x10200073
    elif inst == 'wfi': return 0x10500073
    elif inst == 'nop': return 0x13
    
    return None

def main():
    if len(sys.argv) < 2:
        print("Usage: assembler.py <input.s> [output.hex]")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else "instruction.hex"
    
    instructions = []
    with open(input_file) as f:
        for line in f:
            result = assemble_line(line)
            if result is not None:
                instructions.append(result)
    
    # Write hex file
    with open(output_file, 'w') as f:
        for inst in instructions:
            f.write(f'{inst:08x}\n')
    
    print(f"Assembled {len(instructions)} instructions to {output_file}")

if __name__ == '__main__':
    main()