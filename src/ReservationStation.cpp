#include "ReservationStation.h"

void ReservationStation::Flush() {
  for (int i = 0; i < RSSize; i++)
    RS[i] = nxtRS[i];
}

void ReservationStation::Clear() {
  for (int i = 0; i < RSSize; i++)
    RS[i].busy = RS[i].done = 0;
}

void ReservationStation::Work() {
  for (int i = 0; i < RSSize; i++) {
    if (_ReservationStation.RS[i].q1 == -1 && _ReservationStation.RS[i].q2 == -1) {
      switch (_ReservationStation.RS[i].instruction) {
        case Instruction::ADD:
        case Instruction::SUB:
        case Instruction::ADDI: {
          for () {

          }
          break;
        }

        case Instruction::AND:
        case Instruction::ANDI:
        case Instruction::OR:
        case Instruction::ORI:
        case Instruction::XOR:
        case Instruction::XORI: {
          for () {

          }
          break;
        }

        case Instruction::SLTI:
        case Instruction::SLTIU:
        case Instruction::SLT:
        case Instruction::SLTU: {
          for () {

          }
          break;
        }

        case Instruction::SLLI:
        case Instruction::SRLI:
        case Instruction::SRAI:
        case Instruction::SLL:
        case Instruction::SRL:
        case Instruction::SRA: {
          for () {
            
          }
          break;
        }
      }
    }
  }
}

bool ReservationStation::AppendReservation(const ReservationStationEle& newEle) {
  for (int i = 0; i < RSSize; i++) {
    if (!nxtRS[i].busy) {
      nxtRS[i] = newEle;
      return true;
    }
  }
  return false;
}