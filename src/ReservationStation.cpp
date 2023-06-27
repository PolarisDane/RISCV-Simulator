#include "ReservationStation.h"

void ReservationStation::Clear() {
  for (int i = 0; i < RSSize; i++)
    RS[i].busy = RS[i].done = 0;
}

void ReservationStation::Flush() {
  for (int i = 0; i < ALUSize; i++) {
    _add[i].Flush();
    _logic[i].Flush();
    _shift[i].Flush();
    _set[i].Flush();
  }
}

void ReservationStation::Work() {
  for (int i = 0; i < RSSize; i++) {
    if (RS[i].q1 == -1 && RS[i].q2 == -1 && RS[i].busy && !RS[i].done) {
      switch (RS[i].instruction) {
        case Instruction::ADD:
        case Instruction::SUB:
        case Instruction::ADDI: {
          for (int i = 0; i < ALUSize; i++) {
            if (!_add[i].busy) {
              _add[i].work(RS[i].v1, RS[i].v2, i, RS[i].instruction);
              RS[i].done = 1;
              return;
            }
          }
          break;
        }

        case Instruction::AND:
        case Instruction::ANDI:
        case Instruction::OR:
        case Instruction::ORI:
        case Instruction::XOR:
        case Instruction::XORI: {
          for (int i = 0; i < ALUSize; i++) {
            if (!_logic[i].busy) {
              _logic[i].work(RS[i].v1, RS[i].v2, i, RS[i].instruction);
              RS[i].done = 1;
              return;
            }
          }
          break;
        }

        case Instruction::SLTI:
        case Instruction::SLTIU:
        case Instruction::SLT:
        case Instruction::SLTU: {
          for (int i = 0; i < ALUSize; i++) {
            if (!_set[i].busy) {
              _set[i].work(RS[i].v1, RS[i].v2, i, RS[i].instruction);
              RS[i].done = 1;
              return;
            }
          }
          break;
        }

        case Instruction::SLLI:
        case Instruction::SRLI:
        case Instruction::SRAI:
        case Instruction::SLL:
        case Instruction::SRL:
        case Instruction::SRA: {
          for (int i = 0; i < ALUSize; i++) {
            if (!_shift[i].busy) {
              _shift[i].work(RS[i].v1, RS[i].v2, i, RS[i].instruction);
              RS[i].done = 1;
              return;
            }
          }
          break;
        }
      }
    }
  }
}

bool ReservationStation::AppendReservation(const ReservationStationEle& newEle) {
  for (int i = 0; i < RSSize; i++) {
    if (!RS[i].busy) {
      RS[i] = newEle;
      return true;
    }
  }
  return false;
}