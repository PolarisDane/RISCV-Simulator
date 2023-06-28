#ifndef RISCV_SIMULATOR_REORDERBUFFER
#define RISCV_SIMULATOR_REORDERBUFFER

#include <iostream>
#include <cstdio>
#include "InstructionUnit.h"
#include "CircularQueue.h"
#include "Register.h"

enum ReorderBufferType {
  WR,//Write Register
  WM,//Write Memory
  BR,//Branch
  END//End of program
};

class ReorderBufferInfo {
public:
  bool ready = false;
  ReorderBufferType type;
  Instruction instruction;
  Line curPC;//Where this instruction is at, used for possible BR reflow
  Line val;
  Line address;//Register/Memory address
  bool prediction = false;
};

class ReorderBuffer {
  friend class RISCV_Simulator;
private:
  CircularQueue<ReorderBufferInfo, 32> Buffer;
  CircularQueue<ReorderBufferInfo, 32> nxtBuffer;
public:
  ReorderBuffer() = default;
  ReorderBuffer(const ReorderBuffer& other) = default;
  ~ReorderBuffer() = default;
  void FlushBuffer();
  void ClearBuffer();
};

#endif