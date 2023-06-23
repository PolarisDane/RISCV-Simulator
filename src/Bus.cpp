#include "Bus.h"

void Run() {
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

void Flush() {

}

void RISCV_Simulator::AppendBuffer(ReorderBufferInfo newInfo) {
  _ReorderBuffer.nxtBuffer.push(newInfo);
  if (newInfo.type == ReorderBufferType::WR) {
    _RegisterFile.SetDependency(newInfo.address, _ReorderBuffer.nxtBuffer.LastIndex());
    //Dependency set to the last index of RoB
  }
}

void RISCV_Simulator::UpdateReservation() {
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
  }
}



