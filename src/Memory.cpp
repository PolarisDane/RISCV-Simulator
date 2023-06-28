#include "Memory.h"

Line Memory::ReadMemory(Line MemoryIndex, Instruction instruction) {
  switch (instruction) {
    case Instruction::LB: {
      return static_cast<SignedLine>(*(reinterpret_cast<SignedByte*>(RAM + MemoryIndex)));
      break;
    }
    case Instruction::LH: {
      return static_cast<SignedLine>(*(reinterpret_cast<SignedHalfLine*>(RAM + MemoryIndex)));
      break;
    }
    case Instruction::LW: {
      return *(reinterpret_cast<Line*>(RAM + MemoryIndex));
      break;
    }
    case Instruction::LBU: {
      return *(reinterpret_cast<Byte*>(RAM + MemoryIndex));
      break;
    }
    case Instruction::LHU: {
      return *(reinterpret_cast<HalfLine*>(RAM + MemoryIndex));
      break;
    }
    default:
      assert(0);
  }
}//Different type of read with different data type returned

void Memory::WriteMemory(Line MemoryIndex, Line val, Instruction instruction) {
  switch (instruction) {
    case Instruction::SB: {
      RAM[MemoryIndex] = val;
      break;
    }
    case Instruction::SH: {
      *(reinterpret_cast<HalfLine*>(RAM + MemoryIndex)) = val;
      break;
    }
    case Instruction::SW: {
      *(reinterpret_cast<Line*>(RAM + MemoryIndex)) = val;
      break;
    }
  }
}

void Memory::ResetMemory() {
  memset(RAM, 0, sizeof(RAM));
}


void Memory::InitMemory() {
  std::string data;
  int address;
  while (std::cin >> data) {
    if (data[0] == '@') {
      address = std::stoi(data.substr(1), nullptr, 16);
    }
    else {
      RAM[address] = std::stoi(data, nullptr, 16);
      address++;
    }
  }
}