#include "InstructionUnit.h"

Byte ParseOperation(Line instruction) {
  return instruction & (0x7F);
}

Line ParseLUIImmediate(Line instruction) {
  return instruction & (0xFFFFF000);
}

Line ParseAUIPCImmediate(Line instruction) {
  return instruction & (0xFFFFF000);
}

Line ParseJALImmediate(Line instruction) {
  Line immediate = 0;
  immediate |= instruction & (0xFF000);//19:12
  immediate |= (instruction >> 9) & (0x800);//11
  immediate |= (instruction >> 20) & (0x7FE);//10:1
  immediate |= (instruction >> 11) & (0x100000);//20
  return immediate & (0x100000) ? immediate | (0xFFF00000) : immediate;//SEXT
}

Line ParseJALRImmediate(Line instruction) {
  Line immediate = (instruction >> 20) & (0xFFF);
  return immediate & (0x800) ? immediate | (0xFFFFF000) : immediate;
}

Line ParseSR1(Line instruction) {
  return (instruction >> 15) & (0x1F);
}

Line ParseSR2(Line instruction) {
  return (instruction >> 20) & (0x1F);
}

Line ParseDR(Line instruction) {
  return (instruction >> 7) & (0x1F);
}

Instruction ParseBranchInstruction(Line instruction) {
  Line opt = (instruction >> 12) & (0b111);
  switch (opt) {
    case 0b000:
      return Instruction::BEQ;
    case 0b001:
      return Instruction::BNE;
    case 0b100:
      return Instruction::BLT;
    case 0b101:
      return Instruction::BGE;
    case 0b110: 
      return Instruction::BLTU;
    case 0b111:
      return Instruction::BGEU;
    default:
      assert(0);
  }
}

Instruction ParseLoadInstruction(Line instruction) {
  Line opt = (instruction >> 12) & (0b111);
  switch (opt) {
    case 0b000:
      return Instruction::LB;
    case 0b001:
      return Instruction::LH;
    case 0b010:
      return Instruction::LW;
    case 0b100:
      return Instruction::LBU;
    case 0b101:
      return Instruction::LHU;
    default:
      assert(0);
  }
}

Instruction ParseStoreInstruction(Line instruction) {
  Line opt = (instruction >> 12) & (0b111);
  switch (opt) {
    case 0b000:
      return Instruction::SB;
    case 0b001:
      return Instruction::SH;
    case 0b010:
      return Instruction::SW;
    default:
      assert(0);
  }
}

Instruction ParseArithmeticImmediateInstruction(Line instruction) {
  Line opt = (instruction >> 12) & (0b111);
  switch (opt) {
    case 0b000:
      return Instruction::ADDI;
    case 0b010:
      return Instruction::SLTI;
    case 0b011:
      return Instruction::SLTIU;
    case 0b100:
      return Instruction::XORI;
    case 0b110:
      return Instruction::ORI;
    case 0b111:
      return Instruction::ANDI;
    case 0b001:
    case 0b101:
      return ParseShiftImmediateInstruction(instruction);
    default:
      assert(0);
  }
}

Instruction ParseShiftImmediateInstruction(Line instruction) {
  Line opt1 = (instruction >> 12) & (0b111);
  switch (opt1) {
    case 0b001:
      return Instruction::SLLI;
    case 0b101: {
      Line opt2 = (instruction >> 25) & (0b1111111);
      if (opt2 == 0b0000000) return Instruction::SRLI;
      else return Instruction::SRAI;
    }
    default:
      assert(0);
  }
}

Instruction ParseArithmeticInstruction(Line instruction) {
  Line opt = (instruction >> 12) & (0b111);
  switch (opt) {
    case 0b000: {
      Line opt2 = (instruction >> 25) & (0b1111111);
      if (opt2 == 0b0000000) return Instruction::ADD;
      else return Instruction::SUB;
    }
    case 0b001:
      return Instruction::SLL;
    case 0b010:
      return Instruction::SLT;
    case 0b011:
      return Instruction::SLTU;
    case 0b100:
      return Instruction::XOR;
    case 0b101: {
      Line opt2 = (instruction >> 25) & (0b1111111);
      if (opt2 == 0b0000000) return Instruction::SRL;
      else return Instruction::SRA;
    }
    case 0b110:
      return Instruction::OR;
    case 0b111:
      return Instruction::AND;
    default:
      assert(0);
  }
}

Line ParseArithmeticImmediate(Line instruction) {
  Line immediate = (instruction >> 20) & (0xFFF);
  return immediate & (0x800) ? immediate | (0xFFFFF000) : immediate;
}

Line ParseStoreImmediate(Line instruction) {
  Line immediate = 0;
  immediate |= (instruction >> 7) & (0x1F);
  immediate |= (instruction >> 20) & (0xFE0);
  return immediate & (0x800) ? immediate | (0xFFFFF000) : immediate;
}

Line ParseLoadImmediate(Line instruction) {
  Line immediate = (instruction >> 20) & (0xFFF);
  return immediate & (0x800) ? immediate | (0xFFFFF000) : immediate;
}

Line ParseBranchImmediate(Line instruction) {
  Line immediate = 0;
  immediate |= (instruction << 4) & (0x800);
  immediate |= (instruction >> 7) & (0x1E);
  immediate |= (instruction >> 20) & (0x7E0);
  immediate |= (instruction >> 19) & (0x1000);
  return immediate & (0x1000) ? immediate | (0xFFFFF000) : immediate;
}

Line ParseShamt(Line instruction) {
  return instruction & (0x1F00000);
}

InstructionInfo ParseInstruction(Line instruction) {
  InstructionInfo newInfo;
  if (instruction == 0x0ff00513) {
    newInfo.InstructionType = Instruction::END;
    return newInfo;
  }
  Byte operation = ParseOperation(instruction);
  switch (operation) {
    case 0b0110111: {
      newInfo.InstructionType = Instruction::LUI;
      newInfo.Immediate = ParseLUIImmediate(instruction);
      newInfo.DR = ParseDR(instruction);
      break;
    }
    case 0b0010111: {
      newInfo.InstructionType = Instruction::AUIPC;
      newInfo.Immediate = ParseAUIPCImmediate(instruction);
      newInfo.DR = ParseDR(instruction);
      break;
    }
    case 0b1101111: {
      newInfo.InstructionType = Instruction::JAL;
      newInfo.Immediate = ParseJALImmediate(instruction);
      newInfo.DR = ParseDR(instruction);
      break;
    }
    case 0b1100111: {
      newInfo.InstructionType = Instruction::JALR;
      newInfo.Immediate = ParseJALRImmediate(instruction);
      newInfo.SR1 = ParseSR1(instruction);
      newInfo.DR = ParseDR(instruction);
      break;
    }
    case 0b1100011: {
      newInfo.InstructionType = ParseBranchInstruction(instruction);
      newInfo.Immediate = ParseBranchImmediate(instruction);
      newInfo.SR1 = ParseSR1(instruction);
      newInfo.SR2 = ParseSR2(instruction);
      newInfo.DR = ParseDR(instruction);
      break;
    }
    case 0b0000011: {
      newInfo.InstructionType = ParseLoadInstruction(instruction);
      newInfo.Immediate = ParseLoadImmediate(instruction);
      newInfo.SR1 = ParseSR1(instruction);
      newInfo.DR = ParseDR(instruction);
      break;
    }
    case 0b0100011: {
      newInfo.InstructionType = ParseStoreInstruction(instruction);
      newInfo.Immediate = ParseStoreImmediate(instruction);
      newInfo.SR1 = ParseSR1(instruction);
      newInfo.SR2 = ParseSR2(instruction);
      break;
    }
    case 0b0010011: {
      newInfo.InstructionType = ParseArithmeticImmediateInstruction(instruction);
      newInfo.Immediate = ParseArithmeticImmediate(instruction);
      newInfo.SR1 = ParseSR1(instruction);
      newInfo.DR = ParseDR(instruction);
      break;
    }
    case 0b0110011: {
      newInfo.InstructionType = ParseArithmeticInstruction(instruction);
      newInfo.SR1 = ParseSR1(instruction);
      newInfo.SR2 = ParseSR2(instruction);
      newInfo.DR = ParseDR(instruction);
      break;
    }
    default:
      assert(0);
  }
  return newInfo;
}




