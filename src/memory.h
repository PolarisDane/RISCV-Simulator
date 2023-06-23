#ifndef RISCV_SIMULATOR_MEMORY
#define RISCV_SIMULATOR_MEMORY

#include <iostream>
#include <cstdio>
#include "Parser.h"
#include "Utils.h"
#include "LoadStoreBuffer.h"

class Memory {
  friend class LoadStoreBuffer;
  friend class RISCV_Simulator;
private:
  static const int MemorySize = 65536;
  Line RAM[MemorySize];
public:
  Memory() = default;
  Memory(const Mermory& other) = default;
  ~Memory() = default;

  Line ReadMemory(Line MemoryIndex, Instruction instruction);
  void WriteMemory(Line MemoryIndex, Line val, Instruction instruction);
  void ResetMemory();
};

#endif