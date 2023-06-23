#ifndef RISCV_SIMULATOR_ALU
#define RISCV_SIMULATOR_ALU

#include <iostream>
#include <cstdio>
#include "Utils.h"
#include "Parser.h"
#include "Bus.h"
#include "ReservationStation.h"

class ALU {
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
public:
  AddALU() = default;
  AddALU(const AddALU& other) = default;
  ~AddALU() = default;

  void work(Line in1, Line in2, int pos, Instruction instruction);
};

class ShiftALU :public ALU {
public:
  ShiftALU() = default;
  ShiftALU(const ShiftALU & other) = default;
  ~ShiftALU() = default;

  void work(Line in1, Line in2, int pos, Instruction instruction);
};

class LogicALU :public ALU {
public:
  LogicALU() = default;
  LogicALU(const LogicALU & other) = default;
  ~LogicALU() = default;

  void work(Line in1, Line in2, int pos, Instruction instruction);
};

class SetALU :public ALU {
public:
  SetALU() = default;
  SetALU(const SetALU & other) = default;
  ~SetALU() = default;

  void work(Line in1, Line in2, int pos, Instruction instruction);
};

#endif