#include "LoadStoreBuffer.h"

void LoadStoreBuffer::Clear() {
  LSB.clear();
  done = busy = 0;
}

void LoadStoreBuffer::Flush() {
  if (count) {
    count--;
    if (!count) {
      done = 1;
    }
  }
}

void LoadStoreBuffer::Work() {
  if (busy || LSB.empty()) return;
  busy = 1;
  auto& curLS = LSB.front();
  if (curLS.q1 == -1 && curLS.q2 == -1) {
    count = 3;
    switch (curLS.instruction) {
      case Instruction::LB:
      case Instruction::LH:
      case Instruction::LW:
      case Instruction::LBU:
      case Instruction::LHU: {
        curLS.result = _Memory.ReadMemory(curLS[i].v1 + curLS[i].offset, curLS[i].instruction);
        //Wait for 3 cycles for the result to return
        break;
      }
  
      case Instruction::SB:
      case Instruction::SH:
      case Instruction::SW: {
        //curLS[i] .result = _Memory.WriteMemory(curLS[i].v1 + curLS[i].offset, curLS[i].v2, instruction);
        //Do nothing perhaps?
        break;
      }
    }
  }
}

bool LoadStoreBuffer::AppendBuffer(const LoadStoreBufferEle& newEle) {
  if (LSB.full()) return false;
  LSB.push(newEle);
  return true;
}
