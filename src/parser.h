#ifndef RISCV_SIMULATOR_PARSER
#define RISCV_SIMULATOR_PARSER

#include <iostream>

enum Instruction {
  LUI,
  AUIPC,
  JAL,
  JALR,
  BEQ,
  BNE,
  BLT,
  BGE,
  BLTU,
  BGEU,
  LB,
  LH,
  LW,
  LBU,
  LHU,
  SB,
  SH,
  SW,
  ADDI,
  SLTI,
  SLTIU,
  XORI,
  ORI,
  ANDI,
  SLLI,
  SRLI,
  SRAI,
  ADD,
  SUB,
  SLL,
  SLT,
  SLTU,
  XOR,
  SRL,
  SRA,
  OR,
  AND,

};

class InstructionInfo {
public:
  Instruction InstructionType;

};

class InstructionParser {
private:

public:
  Instruction();
  Instruction(const Instruction& other) = default;
  Instruction(Instruction&&) = default;
  ~Instruction();
};

#endif