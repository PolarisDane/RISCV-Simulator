#ifndef RISCV_SIMULATOR_BUS
#define RISCV_SIMULATOR_BUS

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
  Memory _Memory;
  InstructionParser _InstructionParser;
  size_t _clock = 0;
public:
  RISCV_Simulator() = default;
  RISCV_Simulator(const RISCV_Simulator& other) = default;
  ~RISCV_Simulator() = default;

  void Run();
  void Commit();
  void Flush();
  void AppendReorderBuffer(ReorderBufferInfo newInfo);
  void UpdateReservation();

};

#endif