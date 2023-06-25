#include "RISCV_Simulator.h"

void RISCV_Simulator::Run() {
  while (true) {
    
    _clock++;
    Flush();
  }
}

void RISCV_Simulator::Flush() {
  _LoadStoreBuffer.Flush();
  _ReorderBuffer.FlushBuffer();
  _RegisterFile.FlushRegister();
}

void RISCV_Simulator::Commit() {
  if (_ReorderBuffer.Buffer.empty() || !_ReorderBuffer.Buffer.front().ready) return;
  //Dependency not resolved, not ready for commit
  auto _front = _ReorderBuffer.Buffer.front();
  switch (_front.ReorderBufferType) {
    case ReorderBufferType::WR: {
      if (_RegisterFile.ReadDependency(_front.address) == _ReorderBuffer.Buffer.FirstIndex()) {
        _RegisterFile.SetDependency(_front.address, -1);
      }//Register dependency always set on the last user, so only the last user can reset dependency
      _RegisterFile.ReadRegister(_front.address) = _front.val;
      break;
    }
    case ReorderBufferType::WM: {
      _LoadStoreBuffer._Memory.WriteMemory(_front.address, _front.val);
      break;
    }
    case ReorderBufferType::BR: {
      bool RealBranch = _front.val;
      bool Prediction = _BranchPredictor.GetPrediction();
      _BranchPredictor.UpdateBranchPredictor(RealBranch);
      if (RealBranch != Prediction) {
        _ReorderBuffer.ClearBuffer();
        _ReservationStation.Clear();
        _RegisterFile.ResetRegister();
        _LoadStoreBuffer.Clear();
        _InstructionUnit.PC = RealBranch;
        return;//ReorderBuffer already cleared
      }
      break;
    }
  }
  _ReorderBuffer.nxtBuffer.pop();
}

void RISCV_Simulator::AppendReorderBuffer(ReorderBufferInfo newInfo) {
  _ReorderBuffer.nxtBuffer.push(newInfo);
  if (newInfo.type == ReorderBufferType::WR) {
    _RegisterFile.SetDependency(newInfo.address, _ReorderBuffer.nxtBuffer.LastIndex());
    //Dependency set to the last index of RoB
  }
}

void RISCV_Simulator::Update() {
  UpdateRS();
  UpdateLSB();
  if (dependency != -1 && _ReorderBuffer.Buffer[dependency].ready) {
    _InstructionUnit.PC = _InstructionUnit.PC + immediate;
    dependency = -1;
  }//JALR ready for process
}

void RISCV_Simulator::UpdateRS() {
  for (int i = 0; i < ReservationStation::RSSize; i++) {
    if (_ReservationStation.RS[i].busy && !_ReservationStation.RS[i].done) {
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

void RISCV_Simulator::UpdateLSB() {
  for (int i = 0; i < LoadStoreBuffer::LSBSize; i++) {
    if (_LoadStoreBuffer.LSB[i].busy && !_LoadStoreBuffer.LSB[i].done) {
      if (_ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q].ready) {
        _LoadStoreBuffer.LSB[i].q = -1;
        _LoadStoreBuffer.LSB[i].v = _ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q].val;
      }
    }
  }
}

void RISCV_Simulator::FetchFromLSB() {
  auto curLSB = _LoadStoreBuffer.LSB[_LoadStoreBuffer.curLSBIndex];
  if (curLSB.done) {
    _ReorderBuffer.Buffer[curLSB.RoBIndex].val = curLSB.result;
    _ReorderBuffer.Buffer[curLSB.RoBIndex].ready = true;
    _LoadStoreBuffer.LSB[_LoadStoreBuffer.curLSBIndex].busy = 0;
    _LoadStoreBuffer.LSB[_LoadStoreBuffer.curLSBIndex].done = 0;
  }
}

void RISCV_Simulator::Issue() {
  if (dependency != -1) return;//Blocked by JALR
  Line newLine = _Memory.ReadMemory(_InstructionUnit.PC);
  InstructionInfo instruction = ParseInstruction(newLine);
  switch (instruction) {
    case Instruction::LUI: {
      ReorderBufferInfo newInfo;
      newInfo.type = ReorderBufferType::WR;
      newInfo.address = instruction.DR;
      newInfo.val = instruction.Immediate;
      newInfo.ready = 0;
      AppendReorderBuffer(newInfo);
      _InstructionUnit.PC += 4;
      break;
    }
    case Instruction::AUIPC: {
      ReorderBufferInfo newInfo;
      newInfo.type = ReorderBufferType::WR;
      newInfo.address = instruction.DR;
      newInfo.val = instruction.Immediate + _InstructionUnit.PC;
      newInfo.ready = 0;
      AppendReorderBuffer(newInfo);
      _Instruction.PC += 4;
      break;
    }
    case Instruction::JAL: {
      ReorderBufferInfo newInfo;
      newInfo.type = ReorderBufferType::WR;
      newInfo.address = instruction.DR;
      newInfo.val = _InstructionUnit.PC + 4;
      newInfo.ready = 1;
      AppendReorderBuffer(newInfo);
      //Writing PC+4 into DR
      _InstructionUnit.PC = instruction.Immediate + _InstructionUnit.PC;
      break;
    }
    case Instruction::JALR: {
      ReorderBufferInfo newInfo;
      newInfo.type = ReorderBufferType::WR;
      newInfo.address = instruction.DR;
      newInfo.val = _InstructionUnit.PC + 4;
      newInfo.ready = 0;
      AppendReorderBuffer(newInfo);
      //Writing PC+4 into DR
      if (_RegisterFile.ReadDependency(instruction.DR) == -1) {
        _InstructionUnit.PC = _InstructionUnit.PC + instruction.Immediate;
      }//If there exists no dependency at the current moment, then it can be carried out right now
      else {
        immediate = instruction.Immediate;
        dependency = ReadDependency(instruction.DR);
      }
      break;
    }//JALR need a special dependency record for its usage of register
    //If there is a JALR, then the issue process should be stopped for we could never know where PC should go
    //So instead we will stop the process and wait until the dependency is resolved
    case Instruction::BEQ: {
      ReorderBufferInfo newInfo;
      ReservationStationEle newEle;
      newEle.instruction = Instruction::SUB;
      _ReservationStation.AppendReservation();
      newInfo.type = ReorderBufferType::BR;
      newInfo.address = instruction;
    }
  }
}

