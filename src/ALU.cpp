#include "ALU.h"

void ALU::Clear() {
  busy = done = 0;
  RSindex = -1; result = 0;
}

void AddALU::work(Line in1, Line in2, int pos, Instruction instruction) {
  busy = true;
  switch (instruction) {
    case Instruction::SUB: {
      this->result = in1 - in2;
      break;
    }
    case Instruction::ADD:
    case Instruction::ADDI: {
      this->result = in1 + in2;
      break;
    }
  }
  this->RSindex = pos;
}

void ShiftALU::work(Line in1, Line in2, int pos, Instruction instruction) {
  busy = true;
  switch (instruction) {
    case Instruction::SLL:
    case Instruction::SLLI:{
      this->result = in1 << in2;
      break;
    }
    case Instruction::SRL:
    case Instruction::SRLI: {
      this->result = in1 >> in2;
      break;
    }
    case Instruction::SRA:
    case Instruction::SRAI: {
      this->result = (reinterpret_cast<SignedLine>(in1)) >> in2;
      //Right shift by arithmetic, need to be done in signed form
      break;
    }
  }
  this->RSindex = pos;
}

void LogicALU::work(Line in1, Line in2, int pos, Instruction instruction) {
  busy = true;
  switch (instruction) {
    case Instruction::AND:
    case Instruction::ANDI:{
      this->result = in1 & in2;
      break;
    }
    case Instruction::OR:
    case Instruction::ORI: {
      this->result = in1 | in2;
      break;
    }
    case Instruction::XOR:
    case Instruction::XORI: {
      this->result = in1 ^ in2;
      break;
    }
  }
  this->RSindex = pos;
}

void SetALU::work(Line in1, Line in2, int pos, Instruction instruction) {
  busy = true;
  switch (instruction) {
    case Instruction::SLT:
    case Instruction::SLTI:{
      this->result = (reinterpret_cast<SignedLine>(in1)) < (reinterpret_cast<SignedLine>(in2));
      break;
    }
    case Instruction::SLTU:
    case Instruction::SLTIU: {
      this->result = in1 < in2;
      break;
    }
  }
  this->RSindex = pos;
}
