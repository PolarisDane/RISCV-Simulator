#ifndef RISCV_SIMULATOR_RESERVATIONSTATION
#define RISCV_SIMULATOR_RESERVATIONSTATION

#include <iostream>
#include <cstdio>
#include "Utils.h"
#include "Bus.h"
#include "Parser.h"
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
  ReservationStationEle RS[RSSize];
  ReservationStationEle nxtRS[RSSize];

  AddALU _add[3];
  ShiftALU _shift[3];
  LogicALU _logic[3];
  SetALU _set[3];

public:
  ReservationStation() = default;
  ReservationStation(const ReservationStation& other) = default;
  ~ReservationStation() = default;
  void Flush();
  void Work();
  void AppendReservation(const ReservationStationEle& newEle);
  void Clear();
};


#endif