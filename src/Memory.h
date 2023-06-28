#ifndef RISCV_SIMULATOR_MEMORY
#define RISCV_SIMULATOR_MEMORY

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include "InstructionUnit.h"
#include "Utils.h"

class Memory {
private:
  static const int MemorySize = 524288;
  Byte* RAM;
public:
  Memory() {
    RAM = new Byte[MemorySize];
  }
  Memory(const Memory& other) = default;
  ~Memory() {
    delete[] RAM;
  }

  Line ReadMemory(Line MemoryIndex, Instruction instruction);
  void WriteMemory(Line MemoryIndex, Line val, Instruction instruction);
  void ResetMemory();
  void InitMemory();
};

#endif