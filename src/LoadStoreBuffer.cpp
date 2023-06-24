#include "LoadStoreBuffer.h"

void LoadStoreBuffer::Clear() {
  for (int i = 0; i < LSBSize; i++)
    LSB[i].busy = LSB[i].done = 0;
}

void LoadStoreBuffer::Flush() {
  if (count) {
    count--;
    if (!count) {
      LSB[curLSBIndex].busy = 0;
      LSB[curLSBIndex].done = 1;
    }
  }
}

void LoadStoreBuffer::Work() {
  if (count) return;
  for (int i = 0; i < LSBSize; i++) {
    if (LSB[i].q1 == -1 && LSB[i].q2==-1 && LSB[i].busy && !LSB[i].done) {
      count = 3; curLSBIndex = i;
      switch (LSB[i].instruction) {
        case Instruction::LB:
        case Instruction::LH:
        case Instruction::LW:
        case Instruction::LBU:
        case Instruction::LHU: {
          LSB[i].result = _Memory.ReadMemory(LSB[i].v1 + LSB[i].offset, LSB[i].instruction);
          //Wait for 3 cycles for the result to return
          break;
        }

        case Instruction::SB:
        case Instruction::SH:
        case Instruction::SW: {
          //LSB[i].result = _Memory.WriteMemory(LSB[i].v1 + LSB[i].offset, LSB[i].v2, instruction);
          //Do nothing perhaps?
          break;
          //If not for simulation this work can be done here
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
