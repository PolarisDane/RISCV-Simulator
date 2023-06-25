#ifndef RISCV_SIMULATOR
#define RISCV_SIMULATOR

#include <iostream>
#include "ReorderBuffer.h"
#include "ReservationStation.h"
#include "InstructionUnit.h"
#include "ALU.h"
#include "Memory.h"
#include "Predictor.h"
#include "Utils.h"
#include "CircularQueue.h"

class RISCV_Simulator {
private:
  ReorderBuffer _ReorderBuffer;
  ReservationStation _ReservationStation;
  ALU _ALU;
  RegisterFile _RegisterFile;
  LoadStoreBuffer _LoadStoreBuffer;
  InstructionUnit _InstructionUnit;
  BranchPredictor _BranchPredictor;
  Line PC;
  size_t _clock = 0;
  int dependency = -1;
  Line immediate;
public:
  RISCV_Simulator() = default;
  RISCV_Simulator(const RISCV_Simulator& other) = default;
  ~RISCV_Simulator() = default;

  void Run();
  void Flush();
  void Update();
  
  void Commit();
  void AppendReorderBuffer(ReorderBufferInfo newInfo);

  void UpdateRS();
  void FetchFromRS();

  void UpdateLSB();
  void FetchFromLSB();

  void Issue(InstructionInfo newInfo);
  
};

#endif