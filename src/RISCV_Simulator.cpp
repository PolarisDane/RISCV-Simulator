#include "RISCV_Simulator.h"

void RISCV_Simulator::Run() {
  _LoadStoreBuffer._Memory.InitMemory();
  while (true) {
    Fetch();
    Update();
    Issue();
    Work();
    _clock++;
    Commit();
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
  switch (_front.type) {
    case ReorderBufferType::WR: {
      if (_RegisterFile.ReadDependency(_front.address) == _ReorderBuffer.Buffer.FirstIndex()) {
        _RegisterFile.SetDependency(_front.address, -1);
      }//Register dependency always set on the last user, so only the last user can reset dependency
      _RegisterFile.WriteRegister(_front.address, _front.val);
      break;
    }
    case ReorderBufferType::WM: {
      _LoadStoreBuffer._Memory.WriteMemory(_front.address, _front.val, _front.instruction);
      _LoadStoreBuffer.busy = 0;
      _LoadStoreBuffer.done = 0;
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
        if (RealBranch) _InstructionUnit.PC = _front.address;
        else _InstructionUnit.PC = _front.curPC;
        return;//ReorderBuffer already cleared
      }
      break;
    }
    case ReorderBufferType::END: {
      exit(((Line)_RegisterFile.ReadRegister(10)) & 255u);//a0 is at register 10
      break;
    }
  }
  _ReorderBuffer.nxtBuffer.pop();
}

bool RISCV_Simulator::AppendReorderBuffer(ReorderBufferInfo newInfo) {
  if (_ReorderBuffer.nxtBuffer.full()) return false;
  _ReorderBuffer.nxtBuffer.push(newInfo);
  if (newInfo.type == ReorderBufferType::WR) {
    _RegisterFile.SetDependency(newInfo.address, _ReorderBuffer.nxtBuffer.LastIndex());
    //Dependency set to the last index of RoB
    //This doesn't need to be restored if RS is fulled because dependency of a register is used only if a new instruction is issued
  }
  return true;
}

void RISCV_Simulator::Work() {
  _ReservationStation.Work();
  _LoadStoreBuffer.Work();
}

void RISCV_Simulator::Fetch() {
  FetchFromLSB();
  FetchFromRS();
}

void RISCV_Simulator::Update() {
  UpdateLSB();
  UpdateRS();
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
        _ReservationStation.RS[i].v1 = _ReorderBuffer.Buffer[_ReservationStation.RS[i].q1].val;
      }
      if (_ReorderBuffer.Buffer[_ReservationStation.RS[i].q2].ready) {
        _ReservationStation.RS[i].q2 = -1;
        _ReservationStation.RS[i].v2 = _ReorderBuffer.Buffer[_ReservationStation.RS[i].q2].val;
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
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].val = curALU.result;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._logic[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].val = curALU.result;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._shift[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].val = curALU.result;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._set[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].val = curALU.result;
      _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
}//Fetch previous cycle data from RS to RoB and broadcast it
//Actually this function fetches data from ALU directly

void RISCV_Simulator::UpdateLSB() {
  for (int i = 0; i < LoadStoreBuffer::LSBSize; i++) {
    if (_ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q1].ready) {
      _LoadStoreBuffer.LSB[i].q1 = -1;
      _LoadStoreBuffer.LSB[i].v1 = _ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q1].val;
    }
    if (_ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q2].ready) {
      _LoadStoreBuffer.LSB[i].q2 = -1;
      _LoadStoreBuffer.LSB[i].v2 = _ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q2].val;
    }
  }
}

void RISCV_Simulator::FetchFromLSB() {
  auto curLS = _LoadStoreBuffer.LSB.front();
  if (_LoadStoreBuffer.done) {
    _ReorderBuffer.Buffer[curLS.RoBIndex].val = curLS.result;
    _ReorderBuffer.Buffer[curLS.RoBIndex].ready = true;
  }
}

void RISCV_Simulator::Issue() {
  if (dependency != -1) return;//Blocked by JALR
  Line newLine = _LoadStoreBuffer._Memory.ReadMemory(_InstructionUnit.PC, Instruction::LW);
  InstructionInfo instruction = ParseInstruction(newLine);
  switch (instruction.InstructionType) {
    case Instruction::END: {
      ReorderBufferInfo newInfo;
      newInfo.type = ReorderBufferType::END;
      newInfo.instruction = instruction.InstructionType;
      newInfo.ready = 1;//Ready to halt the simulator
      if (!AppendReorderBuffer(newInfo)) return;
      //The end of program, no PC increment anymore
      break;
    }
    case Instruction::LUI: {
      ReorderBufferInfo newInfo;
      newInfo.curPC = _InstructionUnit.PC;
      newInfo.type = ReorderBufferType::WR;
      newInfo.instruction = instruction.InstructionType;
      newInfo.address = instruction.DR;
      newInfo.val = instruction.Immediate;
      newInfo.ready = 1;//Always ready for commit
      if (!AppendReorderBuffer(newInfo)) return;
      _InstructionUnit.PC += 4;
      break;
    }
    case Instruction::AUIPC: {
      ReorderBufferInfo newInfo;
      newInfo.curPC = _InstructionUnit.PC;
      newInfo.type = ReorderBufferType::WR;
      newInfo.instruction = instruction.InstructionType;
      newInfo.address = instruction.DR;
      newInfo.val = instruction.Immediate + _InstructionUnit.PC;
      newInfo.ready = 1;//Always ready for commit
      AppendReorderBuffer(newInfo);
      _InstructionUnit.PC += 4;
      break;
    }
    case Instruction::JAL: {
      ReorderBufferInfo newInfo;
      newInfo.curPC = _InstructionUnit.PC;
      newInfo.type = ReorderBufferType::WR;
      newInfo.instruction = instruction.InstructionType;
      newInfo.address = instruction.DR;
      newInfo.val = _InstructionUnit.PC + 4;
      newInfo.ready = 1;
      if (!AppendReorderBuffer(newInfo)) return;
      //Writing PC+4 into DR
      _InstructionUnit.PC = instruction.Immediate + _InstructionUnit.PC;
      break;
    }
    case Instruction::JALR: {
      ReorderBufferInfo newInfo;
      newInfo.curPC = _InstructionUnit.PC;
      newInfo.type = ReorderBufferType::WR;
      newInfo.instruction = instruction.InstructionType;
      newInfo.address = instruction.DR;
      newInfo.val = _InstructionUnit.PC + 4;
      newInfo.ready = 0;
      if (!AppendReorderBuffer(newInfo)) return;
      //Writing PC+4 into DR
      if (_RegisterFile.ReadDependency(instruction.DR) == -1) {
        _InstructionUnit.PC = _InstructionUnit.PC + instruction.Immediate;
      }//If there exists no dependency at the current moment, then it can be carried out right now
      else {
        immediate = instruction.Immediate;
        dependency = _RegisterFile.ReadDependency(instruction.DR);
      }
      break;
    }//JALR need a special dependency record for its usage of register
    //If there is a JALR, then the issue process should be stopped for we could never know where PC should go
    //So instead we will stop the process and wait until the dependency is resolved
    case Instruction::BEQ:
    case Instruction::BNE:
    case Instruction::BLT:
    case Instruction::BGE:
    case Instruction::BLTU:
    case Instruction::BGEU: {
      ReorderBufferInfo newInfo;
      newInfo.curPC = _InstructionUnit.PC;
      newInfo.type = ReorderBufferType::BR;
      newInfo.instruction = instruction.InstructionType;
      newInfo.address = _InstructionUnit.PC + instruction.Immediate;
      newInfo.val = 0;
      if (!AppendReorderBuffer(newInfo)) return;
      ReservationStationEle newEle;
      newEle.RoBIndex = _ReorderBuffer.Buffer.LastIndex();
      newEle.busy = 1; newEle.done = 0;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (_ReorderBuffer.Buffer[newEle.q1].ready) {
        newEle.q1 = -1;
        newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
      }
      newEle.q2 = _RegisterFile.ReadDependency(instruction.SR2);
      if (_ReorderBuffer.Buffer[newEle.q2].ready) {
        newEle.q2 = -1;
        newEle.v2 = _ReorderBuffer.Buffer[newEle.q2].val;
      }
      if (!_ReservationStation.AppendReservation(newEle)) {
        _ReorderBuffer.nxtBuffer.popback();
        return;
      }
      bool Prediction = _BranchPredictor.GetPrediction();
      if (Prediction) _InstructionUnit.PC += instruction.Immediate;
      else _InstructionUnit.PC += 4;
      break;
    }
    case Instruction::LB:
    case Instruction::LH:
    case Instruction::LW:
    case Instruction::LBU:
    case Instruction::LHU: {
      ReorderBufferInfo newInfo;
      newInfo.curPC = _InstructionUnit.PC;
      newInfo.type = ReorderBufferType::WR;
      newInfo.instruction = instruction.InstructionType;
      newInfo.address = instruction.DR;
      if (!AppendReorderBuffer(newInfo)) return;
      LoadStoreBufferEle newEle;
      newEle.RoBIndex = _ReorderBuffer.Buffer.LastIndex();
      newEle.done = 0;
      newEle.offset = instruction.Immediate;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (_ReorderBuffer.Buffer[newEle.q1].ready) {
        newEle.q1 = -1;
        newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
      }
      newEle.q2 = -1;
      if (!_LoadStoreBuffer.AppendBuffer(newEle)) {
        _ReorderBuffer.nxtBuffer.popback();
        return;
      }
      _InstructionUnit.PC += 4;
      break;
    }
    case Instruction::SB:
    case Instruction::SH:
    case Instruction::SW: {
      ReorderBufferInfo newInfo;
      newInfo.curPC = _InstructionUnit.PC;
      newInfo.type = ReorderBufferType::WM;
      newInfo.instruction = instruction.InstructionType;
      newInfo.address = instruction.Immediate;//Problems here!
      if (!AppendReorderBuffer(newInfo)) return;
      LoadStoreBufferEle newEle;
      newEle.RoBIndex = _ReorderBuffer.Buffer.LastIndex();
      newEle.done = 0;
      newEle.offset = instruction.Immediate;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (_ReorderBuffer.Buffer[newEle.q1].ready) {
        newEle.q1 = -1;
        newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
      }
      newEle.q2 = _RegisterFile.ReadDependency(instruction.SR2);
      if (_ReorderBuffer.Buffer[newEle.q2].ready) {
        newEle.q2 = -1;
        newEle.v2 = _ReorderBuffer.Buffer[newEle.q2].val;
      }
      if (!_LoadStoreBuffer.AppendBuffer(newEle)) {
        _ReorderBuffer.nxtBuffer.popback();
        return;
      }
      _InstructionUnit.PC += 4;
      break;
    }
    case Instruction::ADDI:
    case Instruction::SLTI:
    case Instruction::SLTIU:
    case Instruction::XORI:
    case Instruction::ORI:
    case Instruction::ANDI:
    case Instruction::SLLI:
    case Instruction::SRLI: {
      ReorderBufferInfo newInfo;
      newInfo.curPC = _InstructionUnit.PC;
      newInfo.type = ReorderBufferType::WR;
      newInfo.instruction = instruction.InstructionType;
      newInfo.address = instruction.DR;
      if (!AppendReorderBuffer(newInfo)) return;
      ReservationStationEle newEle;
      newEle.RoBIndex = _ReorderBuffer.Buffer.LastIndex();
      newEle.busy = 0; newEle.done = 0;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (_ReorderBuffer.Buffer[newEle.q1].ready) {
        newEle.q1 = -1;
        newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
      }
      newEle.v2 = instruction.Immediate;
      newEle.q2 = -1;
      if (!_ReservationStation.AppendReservation(newEle)) {
        _ReorderBuffer.nxtBuffer.popback();
        return;
      }
      _InstructionUnit.PC += 4;
      break;
    }
    case Instruction::ADD:
    case Instruction::SUB:
    case Instruction::SLL:
    case Instruction::SLT:
    case Instruction::SLTU:
    case Instruction::XOR:
    case Instruction::SRL:
    case Instruction::SRA:
    case Instruction::OR:
    case Instruction::AND: {
      ReorderBufferInfo newInfo;
      newInfo.curPC = _InstructionUnit.PC;
      newInfo.type = ReorderBufferType::WR;
      newInfo.instruction = instruction.InstructionType;
      newInfo.address = instruction.DR;
      if (!AppendReorderBuffer(newInfo)) return;
      ReservationStationEle newEle;
      newEle.RoBIndex = _ReorderBuffer.Buffer.LastIndex();
      newEle.busy = 0; newEle.done = 0;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (_ReorderBuffer.Buffer[newEle.q1].ready) {
        newEle.q1 = -1;
        newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
      }
      newEle.q2 = _RegisterFile.ReadDependency(instruction.SR2);
      if (_ReorderBuffer.Buffer[newEle.q2].ready) {
        newEle.q2 = -1;
        newEle.v2 = _ReorderBuffer.Buffer[newEle.q2].val;
      }
      if (!_ReservationStation.AppendReservation(newEle)) {
        _ReorderBuffer.nxtBuffer.popback();
        return;
      }
      _InstructionUnit.PC += 4;
      break;
    }
  }
}

