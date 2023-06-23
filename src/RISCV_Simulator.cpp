#include "RISCV_Simulator.h"

void RISCV_Simulator::Run() {
  while (true) {

    _clock++;
    Flush();
  }
}

void RISCV_Simulator::Commit() {
  if (_ReorderBuffer.Buffer.empty() || !_ReorderBuffer.Buffer.front().ready) return;
  //Dependency not resolved, not ready for commit
  auto _front = _ReorderBuffer.Buffer.front();
  switch (_front.ReorderBufferType) {
    case ReorderBufferType::WR: {
      _RegisterFile.WriteRegister(_front.address, _front.val, -1);
      break;
    }
    case ReorderBufferType::WM: {
        
      break;
    }
    case ReorderBufferType::BR: {
        
      break;
    }
  }
  _ReorderBuffer.nxtBuffer.pop();
}

void RISCV_Simulator::Flush() {
  _ReorderBuffer.FlushBuffer();
  _RegisterFile.FlushRegister();
}

void RISCV_Simulator::AppendReorderBuffer(ReorderBufferInfo newInfo) {
  _ReorderBuffer.nxtBuffer.push(newInfo);
  if (newInfo.type == ReorderBufferType::WR) {
    _RegisterFile.SetDependency(newInfo.address, _ReorderBuffer.nxtBuffer.LastIndex());
    //Dependency set to the last index of RoB
  }
}

void RISCV_Simulator::UpdateRS() {
  for (int i = 0; i < ReservationStation::RSSize; i++) {
    if (_ReservationStation.RS[i].busy) {
      if (_ReorderBuffer.Buffer[_ReservationStation.RS[i].q1].ready) {
        _ReservationStation.RS[i].q1 = -1;
        _ReservationStation.RS[i].v1 = __ReorderBuffer.Buffer[_ReservationStation.RS[i].q1].val;
      }
      if (_ReorderBuffer.Buffer[_ReservationStation.RS[i].q2].ready) {
        _ReservationStation.RS[i].q2 = -1;
        _ReservationStation.RS[i].v2 = __ReorderBuffer.Buffer[_ReservationStation.RS[i].q2].val;
      }
    }
    //Resolving dependency in RS, making operations available
  }
}

void RISCV_Simulator::UpdateLoadStoreBuffer() {
  for (int i = 0; i < LoadStoreBuffer::LSBSize; i++) {

  }
}

void RISCV_Simulator::FetchFromRS() {
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._add[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RobIndex].val = curALU.result;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RobIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._logic[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RobIndex].val = curALU.result;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RobIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._shift[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RobIndex].val = curALU.result;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RobIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._set[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RobIndex].val = curALU.result;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RobIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
}//Fetch previous cycle data from RS to RoB and broadcast it
//Actually this function fetches data from ALU directly


