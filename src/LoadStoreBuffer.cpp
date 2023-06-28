#include "LoadStoreBuffer.h"

void LoadStoreBuffer::Clear() {
  LSB.clear();
  done = busy = 0;
  count = 0;
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
  auto& curLS = LSB.front();
  //std::cout << "q1: " << curLS.q1 << " q2: " << curLS.q2 << std::endl;
  if (curLS.q1 == -1 && curLS.q2 == -1) {
#ifdef DEBUG
    std::cout << "LSB RoBIndex:" << curLS.RoBIndex << std::endl;
#endif
    busy = 1; count = 3;
    switch (curLS.instruction) {
      case Instruction::LB:
      case Instruction::LH:
      case Instruction::LW:
      case Instruction::LBU:
      case Instruction::LHU: {
        curLS.result = _Memory.ReadMemory(curLS.v1 + curLS.offset, curLS.instruction);
#ifdef DEBUG
        printf("Read Memory: %d from %d\n", curLS.result, curLS.v1 + curLS.offset);
#endif
        //Wait for 3 cycles for the result to return
        break;
      }
  
      case Instruction::SB:
      case Instruction::SH:
      case Instruction::SW: {
        curLS.result = curLS.v1 + curLS.offset;
#ifdef DEBUG
        printf("Write Memory: %d to %d\n", curLS.v2, curLS.v1 + curLS.offset);
#endif
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
