#include "ReservationStation.h"

void ReservationStation::Clear() {
  for (int i = 0; i < RSSize; i++)
    RS[i].busy = RS[i].done = 0;
  for (int i = 0; i < ALUSize; i++) {
    _add[i].Clear();
    _logic[i].Clear();
    _shift[i].Clear();
    _set[i].Clear();
  }
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
#ifdef DEBUG
    if (RS[i].busy) {
      std::cout << "RS" << i << std::endl;
      std::cout << "RS RoBIndex: " << RS[i].RoBIndex << std::endl;
      std::cout << "Q1: " << RS[i].q1 << std::endl;
      std::cout << "Q2: " << RS[i].q2 << std::endl;
      std::cout << "busy: " << RS[i].busy << std::endl;
      std::cout << "done: " << RS[i].done << std::endl;
    }
#endif
    if (RS[i].q1 == -1 && RS[i].q2 == -1 && RS[i].busy && !RS[i].done) {
      switch (RS[i].instruction) {
        case Instruction::ADD:
        case Instruction::SUB:
        case Instruction::ADDI: {
          for (int j = 0; j < ALUSize; j++) {
            if (!_add[j].busy) {
              _add[j].work(RS[i].v1, RS[i].v2, i, RS[i].instruction);
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
          for (int j = 0; j < ALUSize; j++) {
            if (!_logic[j].busy) {
              _logic[j].work(RS[i].v1, RS[i].v2, i, RS[i].instruction);
              RS[i].done = 1;
              return;
            }
          }
          break;
        }

        case Instruction::SLTI:
        case Instruction::SLTIU:
        case Instruction::SLT:
        case Instruction::SLTU:
        case Instruction::BEQ:
        case Instruction::BGE:
        case Instruction::BGEU:
        case Instruction::BLT:
        case Instruction::BLTU:
        case Instruction::BNE: {
          for (int j = 0; j < ALUSize; j++) {
            if (!_set[j].busy) {
              _set[j].work(RS[i].v1, RS[i].v2, i, RS[i].instruction);
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
          for (int j = 0; j < ALUSize; j++) {
            if (!_shift[j].busy) {
              _shift[j].work(RS[i].v1, RS[i].v2, i, RS[i].instruction);
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