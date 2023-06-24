#ifndef RISCV_SIMULATOR_PARSER
#define RISCV_SIMULATOR_PARSER

#include <iostream>
#include "Utils.h"

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
  Line SR1;
  Line SR2;
  Line DR;
  Line Immediate;
};

class InstructionParser {
private:

public:
  InstructionParser() = default;
  InstructionParser(const InstructionParser& other) = default;
  ~InstructionParser() = default;


};

Byte ParseOperation(Line instruction);
Line ParseJALImmediate(Line instruction);
Line ParseSR1(Line instruction);
Line ParseSR2(Line instruction);
Line ParseDR(Line instruction);
Line ParseArithmeticImmediate(Line instruction);
Line ParseStoreImmediate(Line instruction);
Line ParseLoadImmediate(Line instruction);
Line ParseBranchImmediate(Line instruction);
Line ParseShamt(Line instruction);
void Parse(Line instruction);

#endif