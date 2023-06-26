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
  ReorderBufferInfo& operator [](size_t index) {
    return nxtBuffer[index];
  }//Left value is from the new buffer
  const ReorderBufferInfo& operator [](size_t index)const {
    return Buffer[index];
  }//Right value is from the old buffer
  void FlushBuffer();
  void ClearBuffer();
};

#endif