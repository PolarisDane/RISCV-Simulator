#ifndef RISCV_SIMULATOR
#define RISCV_SIMULATOR

#include <iostream>
#include <unistd.h>
#include <algorithm>
#include "ReorderBuffer.h"
#include "ReservationStation.h"
#include "Register.h"
#include "InstructionUnit.h"
#include "ALU.h"
#include "Memory.h"
#include "Predictor.h"
#include "Utils.h"
#include "LoadStoreBuffer.h"
#include "CircularQueue.h"

class RISCV_Simulator {
private:
  ReorderBuffer _ReorderBuffer;
  ReservationStation _ReservationStation;
  RegisterFile _RegisterFile;
  LoadStoreBuffer _LoadStoreBuffer;
  InstructionUnit _InstructionUnit;
  BranchPredictor _BranchPredictor;
  size_t _clock = 0;
  //Line RoBIndex;
  int dependency = -1;
  Line immediate;
  //Used for JALR only
public:
  RISCV_Simulator() = default;
  RISCV_Simulator(const RISCV_Simulator& other) = default;
  ~RISCV_Simulator() = default;

  void Run();
  void Flush();
  void Update();
  void Fetch();
  void Work();
  
  void Commit();
  bool AppendReorderBuffer(ReorderBufferInfo newInfo);

  void UpdateRS();
  void FetchFromRS();

  void UpdateLSB();
  void FetchFromLSB();

  void Issue();
  
};

#endif