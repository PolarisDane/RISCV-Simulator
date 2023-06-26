#ifndef RISCV_SIMULATOR_ALU
#define RISCV_SIMULATOR_ALU

#include <iostream>
#include <cstdio>
#include "Utils.h"
#include "InstructionUnit.h"

class ALU {
  friend class RISCV_Simulator;
  friend class ReservationStation;
protected:
  bool busy;
  bool done;
  int RSindex;
  Line result;
public:
  ALU() = default;
  ALU(const ALU& other) = default;
  ~ALU() = default;

  void Clear();
  virtual void work(Line in1, Line in2, int pos, Instruction instruction) = 0;
};

class AddALU :public ALU {
  friend class RISCV_Simulator;
  friend class ReservationStation;
public:
  AddALU() = default;
  AddALU(const AddALU& other) = default;
  ~AddALU() = default;

  void work(Line in1, Line in2, int pos, Instruction instruction);
};

class ShiftALU :public ALU {
  friend class RISCV_Simulator;
  friend class ReservationStation;
public:
  ShiftALU() = default;
  ShiftALU(const ShiftALU & other) = default;
  ~ShiftALU() = default;

  void work(Line in1, Line in2, int pos, Instruction instruction);
};

class LogicALU :public ALU {
  friend class RISCV_Simulator;
  friend class ReservationStation;
public:
  LogicALU() = default;
  LogicALU(const LogicALU & other) = default;
  ~LogicALU() = default;

  void work(Line in1, Line in2, int pos, Instruction instruction);
};

class SetALU :public ALU {
  friend class RISCV_Simulator;
  friend class ReservationStation;
public:
  SetALU() = default;
  SetALU(const SetALU & other) = default;
  ~SetALU() = default;

  void work(Line in1, Line in2, int pos, Instruction instruction);
};

#endif