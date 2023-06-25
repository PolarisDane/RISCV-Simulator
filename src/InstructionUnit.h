#ifndef RISCV_SIMULATOR_PARSER
#define RISCV_SIMULATOR_PARSER

#include <iostream>
#include "Utils.h"
#include "Memory.h"
#include "RISCV_Simulator.h"

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

class InstructionUnit {
  friend class RISCV_Simulator;
private:
  Line PC;
public:
  InstructionUnit() = default;
  InstructionUnit(const InstructionUnit& other) = default;
  ~InstructionUnit() = default;

};

Byte ParseOperation(Line instruction);
Line ParseLUIImmediate(Line instruction);
Line ParseAUIPCImmediate(Line instruction);
Line ParseJALImmediate(Line instruction);
Line ParseJALRImmediate(Line instruction);
Line ParseSR1(Line instruction);
Line ParseSR2(Line instruction);
Line ParseDR(Line instruction);
Instruction ParseBranchInstruction(Line instruction);
Instruction ParseLoadInstruction(Line instruction);
Instruction ParseStoreInstruction(Line instruction);
Instruction ParseArithmeticImmediateInstruction(Line instruction);
Instruction ParseShiftImmediateInstruction(Line instruction);
Instruction ParseArithmeticInstruction(Line instruction);
Line ParseArithmeticImmediate(Line instruction);
Line ParseStoreImmediate(Line instruction);
Line ParseLoadImmediate(Line instruction);
Line ParseBranchImmediate(Line instruction);
Line ParseShamt(Line instruction);
InstructionInfo ParseInstruction(Line instruction);

#endif