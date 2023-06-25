#ifndef RISCV_SIMULATOR_RESERVATIONSTATION
#define RISCV_SIMULATOR_RESERVATIONSTATION

#include <iostream>
#include <cstdio>
#include "Utils.h"
#include "RISCV_Simulator.h"
#include "InstructionUnit.h"
#include "ALU.h"

class ReservationStationEle {
public:
  Instruction instruction;
  bool busy = 0;
  bool done = 0;
  Line v1, v2;
  size_t q1, q2;
  size_t RoBIndex;
};

class ReservationStation {
  friend class RISCV_Simulator;
private:
  static const int RSSize = 32;
  static const int ALUSize = 3;
  ReservationStationEle RS[RSSize];

  AddALU _add[ALUSize];
  ShiftALU _shift[ALUSize];
  LogicALU _logic[ALUSize];
  SetALU _set[ALUSize];

public:
  ReservationStation() = default;
  ReservationStation(const ReservationStation& other) = default;
  ~ReservationStation() = default;
  void Work();
  bool AppendReservation(const ReservationStationEle& newEle);
  //Possible append failure must be dealt with
  void Clear();
};

#endif