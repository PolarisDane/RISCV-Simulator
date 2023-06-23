#include "LoadStoreBuffer.h"

void LoadStoreBuffer::Clear() {
  for (int i = 0; i < LSBSize; i++)
    LSB[i].busy = LSB[i].done = 0;
}

void LoadStoreBuffer::flush() {
  if (count) {
    count--;
    if (!count) {
      if 
    }
  }
}

void LoadStoreBuffer::Work() {
  for (int i = 0; i < LSBSize; i++) {
    if (LSB[i].q == -1 && LSB[i].busy && !LSB[i].done) {
      busy = 1; count = 3;
      switch (LSB[i].instruction) {
        case Instruction::LB:
        case Instruction::LH:
        case Instruction::LW:
        case Instruction::LBU:
        case Instruction::LHU: {
          LSB[i].result = _Memory.ReadMemory(LSB[i].v + LSB[i].imm, LSB[i].instruction);
          //Wait for 3 cycles for the result to return
        }

        case Instruction::SB:
        case Instruction::SH:
        case Instruction::SW: {
          //Do nothing perhaps?
        }
      }
    }
  }
}

bool LoadStoreBuffer::AppendBuffer(const LoadStoreBufferEle& newEle) {
  for (int i = 0; i < LSBSize; i++) {
    if (!LSB[i].busy) {
      LSB[i] = newEle;
      return true;
    }
  }
  return false;
}
