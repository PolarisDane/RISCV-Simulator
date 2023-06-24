#ifndef RISCV_SIMULATOR_LOADSTOREBUFFER
#define RISCV_SIMULATOR_LOADSTOREBUFFER

#include <iostream>
#include <cstdio>
#include "Utils.h"
#include "Parse.h"
#include "Memory.h"
#include "RISCV_Simulator.h"

class LoadStoreBufferEle {
public:
  bool busy = 0;
  bool done = 0;
  Line result;
  size_t q1, q2;
  Line v1, v2;
  Line offset;
  size_t RoBindex;
  Instruction instruction;
};

class LoadStoreBuffer {
  friend class RISCV_Simulator;
private:
  static const int LSBSize = 32;
  LoadStoreBufferEle LSB[LSBSize];
  int count = 0;
  size_t curLSBIndex;
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