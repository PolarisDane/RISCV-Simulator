#ifndef RISCV_SIMULATOR_LOADSTOREBUFFER
#define RISCV_SIMULATOR_LOADSTOREBUFFER

#include <iostream>
#include <cstdio>
#include "Utils.h"
#include "CircularQueue.h"
#include "InstructionUnit.h"
#include "Memory.h"

class LoadStoreBufferEle {
public:
  Line result;
  int q1, q2;
  Line v1, v2;
  Line offset;
  size_t RoBIndex;
  Instruction instruction;
};

class LoadStoreBuffer {
  friend class RISCV_Simulator;
private:
  static const int LSBSize = 32;
  CircularQueue<LoadStoreBufferEle, LSBSize> LSB;
  int count = 0;
  bool busy = 0;
  bool done = 0;
  Memory _Memory;
public:
  LoadStoreBuffer() = default;
  LoadStoreBuffer(const LoadStoreBuffer& other) = default;
  ~LoadStoreBuffer() = default;

  void Flush();
  void Work();
  bool AppendBuffer(const LoadStoreBufferEle& newEle);
  //Possible append failure must be dealt with
  void Clear();

};

#endif