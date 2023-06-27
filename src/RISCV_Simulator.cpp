#include "RISCV_Simulator.h"

void RISCV_Simulator::Run() {
  _LoadStoreBuffer._Memory.InitMemory();
  while (true) {
    //std::cout << "Flag1" << std::endl;
    Fetch();
    //std::cout << "Flag2" << std::endl;
    Update();
    //std::cout << "Flag3" << std::endl;
    Issue();
    //std::cout << "Flag4" << std::endl;
    Work();
    //std::cout << "Flag5" << std::endl;
    _clock++;
    Commit();
    Flush();
    std::cout << "Buffer First Index:" << _ReorderBuffer.Buffer.FirstIndex() << std::endl;
    std::cout << "Buffer Last Index:" << _ReorderBuffer.Buffer.LastIndex() << std::endl;
    for (int i = _ReorderBuffer.Buffer.FirstIndex(); i <= _ReorderBuffer.Buffer.LastIndex(); i++) {
      printf("Buffer%d: %x ,%d\n", i, _ReorderBuffer.Buffer[i].curPC, _ReorderBuffer.Buffer[i].ready);
    }
    std::cout << std::endl;
    for (int i = 0; i < 32; i++) {
      printf("Reg%d: %x Dependency: %d\n", i, _RegisterFile.ReadRegister(i), _RegisterFile.ReadDependency(i));
    }
    std::cout << std::endl;
    //sleep(1);
  }
}

void RISCV_Simulator::Flush() {
  _LoadStoreBuffer.Flush();
  _ReservationStation.Flush();
  _ReorderBuffer.FlushBuffer();
  _RegisterFile.FlushRegister();
}

void RISCV_Simulator::Commit() {
  std::cout << std::endl;
  if (!_ReorderBuffer.Buffer.empty()) {
    std::cout << "RoB Index:" << _ReorderBuffer.Buffer.FirstIndex() << std::endl;
    std::cout << "RoB front Instruction:" << static_cast<int>(_ReorderBuffer.Buffer.front().instruction) << std::endl;
    std::cout << "address:" << _ReorderBuffer.Buffer.front().address << std::endl;
    std::cout << "val:" << _ReorderBuffer.Buffer.front().val << std::endl;
    printf("PC:%x\n", _ReorderBuffer.Buffer.front().curPC);
  }
  //if (!_ReorderBuffer.nxtBuffer.empty()) {
  //  std::cout << "nxtRoB Index:" << _ReorderBuffer.nxtBuffer.FirstIndex() << std::endl;
  //  std::cout << "nxtRoB front Instruction:" << static_cast<int>(_ReorderBuffer.nxtBuffer.front().instruction) << std::endl;
  //  std::cout << "nxtaddress:" << _ReorderBuffer.nxtBuffer.front().address << std::endl;
  //  std::cout << "nxtval:" << _ReorderBuffer.nxtBuffer.front().val << std::endl;
  //  std::cout << "val:" << _ReorderBuffer.nxtBuffer.front().curPC << std::endl;
  //}
  std::cout << std::endl;
  if (_ReorderBuffer.Buffer.empty() || !_ReorderBuffer.Buffer.front().ready) return;
  //Dependency not resolved, not ready for commit
  auto _front = _ReorderBuffer.Buffer.front();
  std::cout << "Commit" << std::endl;
  switch (_front.type) {
    case ReorderBufferType::WR: {
      std::cout << _front.address << " " << _front.val << std::endl;
      if (_RegisterFile.ReadDependency(_front.address) == _ReorderBuffer.Buffer.FirstIndex()) {
        _RegisterFile.SetDependency(_front.address, -1);
      }//Register dependency always set on the last user, so only the last user can reset dependency
      _RegisterFile.WriteRegister(_front.address, _front.val);
      switch (_front.instruction) {
        case Instruction::LB:
        case Instruction::LH:
        case Instruction::LW:
        case Instruction::LBU:
        case Instruction::LHU: {
          _LoadStoreBuffer.LSB.pop();
          _LoadStoreBuffer.busy = 0;
          _LoadStoreBuffer.done = 0;
        }
      }
      break;
    }
    case ReorderBufferType::WM: {
      _LoadStoreBuffer._Memory.WriteMemory(_front.address, _front.val, _front.instruction);
      _LoadStoreBuffer.LSB.pop();
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
      std::cout << (((Line)_RegisterFile.ReadRegister(10)) & 255u) << std::endl;//a0 is at register 10
      exit(0);
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
    _InstructionUnit.PC = (_InstructionUnit.PC + immediate) & (~1);
    dependency = -1;
  }//JALR ready for process
}

void RISCV_Simulator::UpdateRS() {
  for (int i = 0; i < ReservationStation::RSSize; i++) {
    if (_ReservationStation.RS[i].busy && !_ReservationStation.RS[i].done) {
      if (_ReservationStation.RS[i].q1 != -1 && _ReorderBuffer.Buffer[_ReservationStation.RS[i].q1].ready) {
        _ReservationStation.RS[i].v1 = _ReorderBuffer.Buffer[_ReservationStation.RS[i].q1].val;
        _ReservationStation.RS[i].q1 = -1;
      }
      if (_ReservationStation.RS[i].q2 != -1 && _ReorderBuffer.Buffer[_ReservationStation.RS[i].q2].ready) {
        _ReservationStation.RS[i].v2 = _ReorderBuffer.Buffer[_ReservationStation.RS[i].q2].val;
        _ReservationStation.RS[i].q2 = -1;
      }
    }
    //Resolving dependency in RS, making operations available
  }
}

void RISCV_Simulator::FetchFromRS() {
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._add[i];
    if (curALU.done) {
      //std::cout << "RoBIndex:" << _ReservationStation.RS[curALU.RSindex].RoBIndex << std::endl;
      //printf("PC:%x\n", _ReorderBuffer.Buffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].curPC);
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.nxtBuffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].val = curALU.result;
      _ReorderBuffer.nxtBuffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._logic[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.nxtBuffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].val = curALU.result;
      _ReorderBuffer.nxtBuffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._shift[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.nxtBuffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].val = curALU.result;
      _ReorderBuffer.nxtBuffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
  for (int i = 0; i < ReservationStation::ALUSize; i++) {
    auto& curALU = _ReservationStation._set[i];
    if (curALU.done) {
      _ReservationStation.RS[curALU.RSindex].busy = 0;
      _ReservationStation.RS[curALU.RSindex].done = 1;
      _ReorderBuffer.nxtBuffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].val = curALU.result;
      _ReorderBuffer.nxtBuffer[_ReservationStation.RS[curALU.RSindex].RoBIndex].ready = true;
      curALU.busy = 0; curALU.done = 0;
    }
  }
}//Fetch previous cycle data from RS to RoB and broadcast it
//Actually this function fetches data from ALU directly

void RISCV_Simulator::UpdateLSB() {
  std::cout << "LSB busy done:" << _LoadStoreBuffer.busy << " " << _LoadStoreBuffer.done << std::endl;
  for (int i = _LoadStoreBuffer.LSB.FirstIndex(); i != _LoadStoreBuffer.LSB.LastIndex(); i = _LoadStoreBuffer.LSB.sub(i)) {
    //Not changed to iterator right now, future work
    //printf("LSB PC:%x\n", _ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].RoBIndex].curPC);
    //std::cout << "LSB RoBIndex: " << _LoadStoreBuffer.LSB[i].RoBIndex << std::endl;
    //std::cout << "Q1 " << _LoadStoreBuffer.LSB[i].q1 << std::endl;
    //std::cout << "Q2 " << _LoadStoreBuffer.LSB[i].q2 << std::endl;
    if (_LoadStoreBuffer.LSB[i].q1 != -1 && _ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q1].ready) {
      _LoadStoreBuffer.LSB[i].v1 = _ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q1].val;
      _LoadStoreBuffer.LSB[i].q1 = -1;
    }
    if (_LoadStoreBuffer.LSB[i].q2 != -1 && _ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q2].ready) {
      _LoadStoreBuffer.LSB[i].v2 = _ReorderBuffer.Buffer[_LoadStoreBuffer.LSB[i].q2].val;
      _LoadStoreBuffer.LSB[i].q2 = -1;
    }
  }
}

void RISCV_Simulator::FetchFromLSB() {
  //std::cout << "LSB BUSY DONE:" << _LoadStoreBuffer.busy << " " << _LoadStoreBuffer.done << std::endl;
  if (_LoadStoreBuffer.LSB.empty()) return;
  auto curLS = _LoadStoreBuffer.LSB.front();
  if (_LoadStoreBuffer.done) {
    switch (curLS.instruction) {
      case Instruction::LB:
      case Instruction::LH:
      case Instruction::LW:
      case Instruction::LBU:
      case Instruction::LHU: {
        //std::cout << "LOAD:" << std::endl;
        //printf("PC:%x\n", _ReorderBuffer.Buffer[curLS.RoBIndex].curPC);
        //std::cout << "RoBIndex:" << curLS.RoBIndex << std::endl;
        //std::cout << "Result:" << curLS.result << std::endl;
        //std::cout << "Address:" << _ReorderBuffer.Buffer[curLS.RoBIndex].address << std::endl;
        //std::cout << "NxtAddress:" << _ReorderBuffer.nxtBuffer[curLS.RoBIndex].address << std::endl;
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].val = curLS.result;
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].ready = true;
        break;
      }
      case Instruction::SB:
      case Instruction::SH:
      case Instruction::SW: {
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].address = curLS.result;
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].val = curLS.v2;
        //std::cout << std::endl << "Store Address: " << curLS.result << std::endl;
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].ready = true;
        break;
      }//Store instruction's destination can only be calculated in LSB, not ready before
    }
    //cleanse of LSB should not be here
  }
}

void RISCV_Simulator::Issue() {
  if (dependency != -1) {
    std::cout << "dependency:" << dependency << std::endl;
    printf("PC: %x\n", _InstructionUnit.PC);
    std::cout << "Blocked by JALR" << std::endl;
    return;//Blocked by JALR
  }
  printf("PC: %x\n", _InstructionUnit.PC);
  Line newLine = _LoadStoreBuffer._Memory.ReadMemory(_InstructionUnit.PC, Instruction::LW);
  printf("Line:%x\n", newLine);
  InstructionInfo instruction = ParseInstruction(newLine);
  std::cout << "Instruction:" << static_cast<int>(instruction.InstructionType) << std::endl;
  std::cout << "Immediate:" << instruction.Immediate << std::endl;
  std::cout << "SR1: " << instruction.SR1 << std::endl;
  std::cout << "SR2: " << instruction.SR2 << std::endl;
  std::cout << "DR: " << instruction.DR << std::endl;
  std::cout << "Register ra: " << _RegisterFile.ReadRegister(1) << std::endl;
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
      printf("val:%x\n", newInfo.val);
      printf("PC:%x\n", _InstructionUnit.PC);
      //Writing PC+4 into DR
      newInfo.ready = 1;
      if (!AppendReorderBuffer(newInfo)) return;
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
      newInfo.ready = 1;
      if (!AppendReorderBuffer(newInfo)) return;
      //Writing PC+4 into DR
      std::cout << "JALR SR1:" << instruction.SR1 << std::endl;
      if (_RegisterFile.ReadDependency(instruction.SR1) == -1) {
        _InstructionUnit.PC = _RegisterFile.ReadRegister(instruction.SR1) + instruction.Immediate;
        printf("PC changed to: %x\n", _InstructionUnit.PC);
      }//If there exists no dependency at the current moment, then it can be carried out right now
      else {
        immediate = instruction.Immediate;
        dependency = _RegisterFile.ReadDependency(instruction.SR1);
        std::cout << "JALR dependency:" << dependency << std::endl;
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
      newEle.RoBIndex = _ReorderBuffer.nxtBuffer.LastIndex();
      newEle.busy = 1; newEle.done = 0;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (newEle.q1 != -1) {
        if (_ReorderBuffer.Buffer[newEle.q1].ready) {
          newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
          newEle.q1 = -1;
        }
      }
      else newEle.v1 = _RegisterFile.ReadRegister(instruction.SR1);
      newEle.q2 = _RegisterFile.ReadDependency(instruction.SR2);
      if (newEle.q2 != -1) {
        if (_ReorderBuffer.Buffer[newEle.q2].ready) {
          newEle.v2 = _ReorderBuffer.Buffer[newEle.q2].val;
          newEle.q2 = -1;
        }
      }
      else newEle.v2 = _RegisterFile.ReadRegister(instruction.SR2);
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
      newEle.RoBIndex = _ReorderBuffer.nxtBuffer.LastIndex();
      newEle.done = 0;
      newEle.offset = instruction.Immediate;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (newEle.q1 != -1) {
        if (_ReorderBuffer.Buffer[newEle.q1].ready) {
          newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
          newEle.q1 = -1;
        }
      }
      else newEle.v1 = _RegisterFile.ReadRegister(instruction.SR1);
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
      //address not determined yet
      if (!AppendReorderBuffer(newInfo)) return;
      LoadStoreBufferEle newEle;
      newEle.RoBIndex = _ReorderBuffer.nxtBuffer.LastIndex();
      newEle.done = 0;
      newEle.offset = instruction.Immediate;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (newEle.q1 != -1) {
        if (_ReorderBuffer.Buffer[newEle.q1].ready) {
          newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
          newEle.q1 = -1;
        }
      }
      else newEle.v1 = _RegisterFile.ReadRegister(instruction.SR1);
      newEle.q2 = _RegisterFile.ReadDependency(instruction.SR2);
      if (newEle.q2 != -1) {
        if (_ReorderBuffer.Buffer[newEle.q2].ready) {
          newEle.v2 = _ReorderBuffer.Buffer[newEle.q2].val;
          newEle.q2 = -1;
        }
      }
      else newEle.v2 = _RegisterFile.ReadRegister(instruction.SR2);
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
      newEle.RoBIndex = _ReorderBuffer.nxtBuffer.LastIndex();
      newEle.busy = 1; newEle.done = 0;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (newEle.q1 != -1) {
        if (_ReorderBuffer.Buffer[newEle.q1].ready) {
          newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
          newEle.q1 = -1;
        }
      }
      else newEle.v1 = _RegisterFile.ReadRegister(instruction.SR1);
      //std::cout << "v1:" << newEle.v1 << std::endl;
      newEle.v2 = instruction.Immediate;
      newEle.q2 = -1;
      //std::cout << "v2:" << newEle.v2 << std::endl;
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
      newEle.RoBIndex = _ReorderBuffer.nxtBuffer.LastIndex();
      newEle.busy = 1; newEle.done = 0;
      newEle.instruction = instruction.InstructionType;
      newEle.q1 = _RegisterFile.ReadDependency(instruction.SR1);
      if (newEle.q1 != -1) {
        if (_ReorderBuffer.Buffer[newEle.q1].ready) {
          newEle.v1 = _ReorderBuffer.Buffer[newEle.q1].val;
          newEle.q1 = -1;
        }
      }
      else newEle.v1 = _RegisterFile.ReadRegister(instruction.SR1);
      newEle.q2 = _RegisterFile.ReadDependency(instruction.SR2);
      if (newEle.q2 != -1) {
        if (_ReorderBuffer.Buffer[newEle.q2].ready) {
          newEle.v2 = _ReorderBuffer.Buffer[newEle.q2].val;
          newEle.q2 = -1;
        }
      }
      else newEle.v2 = _RegisterFile.ReadRegister(instruction.SR2);
      if (!_ReservationStation.AppendReservation(newEle)) {
        _ReorderBuffer.nxtBuffer.popback();
        return;
      }
      _InstructionUnit.PC += 4;
      break;
    }
  }
}

