#ifndef RISCV_SIMULATOR_LOADSTOREBUFFER
#define RISCV_SIMULATOR_LOADSTOREBUFFER

#include <iostream>
#include <cstdio>
#include "Utils.h"
#include "RISCV_Simulator.h"

class LoadStoreBufferEle {
public:
  bool busy = 0;
  bool done = 0;
  Line result;
  size_t q;
  Line v;
  size_t RoBindex;
};

class LoadStoreBuffer {
  friend class RISCV_Simulator;
private:
  static const int LSBSize = 32;
  LoadStoreBufferEle LSB[LSBSize];

public:
  LoadStoreBuffer() = default;
  LoadStoreBuffer(const LoadStoreBuffer& other) = default;
  ~LoadStoreBuffer() = default;

  void Work();
  bool AppendBuffer(const LoadStoreBufferEle& newEle);
  //Possible append failure must be dealt with
  void Clear();

};

#endif