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
  static const int MemorySize = 65536;
  Line RAM[MemorySize];
public:
  Memory() = default;
  Memory(const Memory& other) = default;
  ~Memory() = default;

  Line ReadMemory(Line MemoryIndex, Instruction instruction);
  void WriteMemory(Line MemoryIndex, Line val, Instruction instruction);
  void ResetMemory();
  void InitMemory();
};

#endif