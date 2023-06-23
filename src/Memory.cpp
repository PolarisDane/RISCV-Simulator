#include "Memory.h"

Line Memory::ReadMemory(Line MemoryIndex, Instruction instruction) {
  switch (instruction) {
    case Instruction::LB: {
      return reinterpret_cast<SignedByte>(RAM[MemoryIndex]);
      break;
    }
    case Instruction::LH: {
      return reinterpret_cast<SignedHalfLine>(RAM[MemoryIndex]);
      break;
    }
    case Instruction::LW: {
      return RAM[MemoryIndex];
      break;
    }
    case Instruction::LBU: {
      return reinterpret_cast<Byte>(RAM[MemoryIndex]);
      break;
    }
    case Instruction::LHU: {
      return reinterpret_cast<HalfLine>(RAM[MemoryIndex]);
      break;
    }
  }
}//Different type of read with different data type returned

void Memory::WriteMemory(Line MemoryIndex, Line val, Instruction instruction) {
  switch (instruction) {
    case Instruction::SB: {
      RAM[MemoryIndex] = reinterpret_cast<Byte>(val);
      break;
    }
    case Instruction::SH: {
      RAM[MemoryIndex] = reinterpret_cast<HalfLine>(val);
      break;
    }
    case Instruction::SW: {
      RAM[MemoryIndex] = val;
      break;
    }
  }
}

void ResetMemory() {
  memset(RAM, 0, sizeof(RAM));
}
