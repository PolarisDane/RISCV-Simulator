#ifndef RISCV_SIMULATOR_REORDERBUFFER
#define RISCV_SIMULATOR_REORDERBUFFER

#include <iostream>
#include <cstdio>
#include "CicularQueue.h"
#include "Register.h"
#include "RISCV_Simulator.h"

enum ReorderBufferType {
  WR,//Write Register
  WM,//Write Memory
  BR//Branch
};

class ReorderBufferInfo {
public:
  bool ready = false;
  ReorderBufferType type;
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
  ReorderBuffer& operator [](size_t index) {
    return nxtBuffer[index];
  }//Left value is from the new buffer
  const ReorderBuffer& operator [](size_t index) {
    return Buffer[index];
  }//Right value is from the old buffer
  void FlushBuffer();
  void ClearBuffer();
  void AppendBuffer(ReorderBufferInfo newInfo, RISCV_Simulator& _RISCV_Simulator);
  void WriteBuffer(size_t index, Line _val);
};

#endif