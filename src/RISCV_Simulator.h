#ifndef RISCV_SIMULATOR
#define RISCV_SIMULATOR

#include <iostream>
#include "ReorderBuffer.h"
#include "ReservationStation.h"
#include "Parser.h"
#include "ALU.h"
#include "Memory.h"
#include "Utils.h"

class RISCV_Simulator {
private:
  ReorderBuffer _ReorderBuffer;
  ReservationStation _ReservationStation;
  ALU _ALU;
  RegisterFile _RegisterFile;
  LoadStoreBuffer _LoadStoreBuffer;
  InstructionParser _InstructionParser;
  size_t _clock = 0;
public:
  RISCV_Simulator() = default;
  RISCV_Simulator(const RISCV_Simulator& other) = default;
  ~RISCV_Simulator() = default;

  void Run();
  void Flush();
  
  void Commit();
  void AppendReorderBuffer(ReorderBufferInfo newInfo);

  void UpdateRS();
  void FetchFromRS();

  void UpdateLSB();
  void FetchFromLSB()
  
};

#endif