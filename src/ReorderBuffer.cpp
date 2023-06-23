#include "ReorderBuffer.h"

void ReorderBuffer::FlushBuffer() {
  Buffer = nxtBuffer;
}

void ReorderBuffer::ClearBuffer() {
  nxtBuffer.clear();
}

void ReorderBuffer::AppendBuffer(ReorderBufferInfo newInfo, RISCV_Simulator& _RISCV_Simulator) {
  nxtBuffer.push(newInfo);
  if (newInfo.type == ReorderBufferType::WR) {
    _RISCV_Simulator.SetRegisterDependency(newInfo.address, nxtBuffer.LastIndex());
    //Dependency set to the last index of RoB
  }
}

void ReorderBuffer::WriteBuffer() {

}