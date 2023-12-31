#include "RISCV_Simulator.h"


void RISCV_Simulator::Run() {
  _LoadStoreBuffer._Memory.InitMemory();
  int a[5] = { 0,1,2,3,4 };
  srand(time(NULL) * 20040301);
  std::random_shuffle(a, a + 4);
  while (true) {
    //std::cout << "clock: " << _clock << std::endl;
    std::random_shuffle(a, a + 4);
    for (int i = 0; i <= 4; i++) {
      if (a[i] == 0) Fetch();
      if (a[i] == 1) Update();
      if (a[i] == 2) Issue();
      if (a[i] == 3) Work();
      if (a[i] == 4) Commit();
    }
    Flush();
    _clock++;
    //Fetch();
    //Update();
    //Issue();
    //Work();
    //_clock++;
    //Commit();
    //Flush();
  }
}

void RISCV_Simulator::Flush() {
  _LoadStoreBuffer.Flush();
  _ReservationStation.Flush();
  _ReorderBuffer.FlushBuffer();
  _RegisterFile.FlushRegister();
}

void RISCV_Simulator::Commit() {
#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "Buffer Last Index:" << _ReorderBuffer.Buffer.LastIndex() << std::endl;
  std::cout << "nxtBuffer Last Index:" << _ReorderBuffer.nxtBuffer.LastIndex() << std::endl;
  if (!_ReorderBuffer.Buffer.empty()) {
    std::cout << "RoB Index:" << _ReorderBuffer.Buffer.FirstIndex() << std::endl;
    std::cout << "RoB front Instruction:" << static_cast<int>(_ReorderBuffer.Buffer.front().instruction) << std::endl;
    std::cout << "address:" << _ReorderBuffer.Buffer.front().address << std::endl;
    std::cout << "val:" << _ReorderBuffer.Buffer.front().val << std::endl;
    printf("PC:%x\n", _ReorderBuffer.Buffer.front().curPC);
  }
#endif
  if (_ReorderBuffer.Buffer.empty() || !_ReorderBuffer.Buffer.front().ready) return;
  //Dependency not resolved, not ready for commit
  auto _front = _ReorderBuffer.Buffer.front();
#ifdef DEBUG
  std::cout << "Commit" << std::endl;
  std::cout << "RoB First Index: " << _ReorderBuffer.Buffer.FirstIndex() << std::endl;
  printf("RoB Front PC: %x\n", _front.curPC);
#endif
  switch (_front.type) {
    case ReorderBufferType::WR: {
#ifdef DEBUG
      std::cout << _front.address << " " << _front.val << std::endl;
#endif
      _RegisterFile.RestoreRegister(_front.address, _ReorderBuffer.Buffer.FirstIndex());
      //Register dependency always set on the last user, so only the last user can reset dependency
      _RegisterFile.WriteRegister(_front.address, _front.val);
      switch (_front.instruction) {
        case Instruction::LB:
        case Instruction::LH:
        case Instruction::LW:
        case Instruction::LBU:
        case Instruction::LHU: {
#ifdef DEBUG
          //printf("Read Memory: %d from %d\n", _front.val, _front.address);
#endif
          _LoadStoreBuffer.LSB.pop();
          _LoadStoreBuffer.busy = 0;
          _LoadStoreBuffer.done = 0;
        }
      }
      break;
    }
    case ReorderBufferType::WM: {
#ifdef DEBUG
      //printf("Write Memory: %d to %d\n", _front.val, _front.address);
#endif
      _LoadStoreBuffer._Memory.WriteMemory(_front.address, _front.val, _front.instruction);
      _LoadStoreBuffer.LSB.pop();
      _LoadStoreBuffer.busy = 0;
      _LoadStoreBuffer.done = 0;
      break;
    }
    case ReorderBufferType::BR: {
      BranchCount++;
      bool RealBranch = _front.val;
      _BranchPredictor.UpdateBranchPredictor(_front.curPC, RealBranch, _front.prediction.second);
      if (RealBranch != _front.prediction.first) {
        _ReorderBuffer.ClearBuffer();
        _ReservationStation.Clear();
        _RegisterFile.ResetRegister();
        _LoadStoreBuffer.Clear();
        dependency = -1;
        if (RealBranch) _InstructionUnit.PC = _front.address;
        else _InstructionUnit.PC = _front.curPC + 4;
#ifdef DEBUG
        //std::cout << "Branch Prediction failed" << std::endl;
        //printf("PC changed to %x\n", _InstructionUnit.PC);
#endif
        return;//ReorderBuffer already cleared
      }
      else CorrectCount++;
      break;
    }
    case ReorderBufferType::END: {
      std::cout << "Correct rate on branch prediction:" << CorrectCount << "/" << BranchCount << std::endl;
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
    _InstructionUnit.PC = (_ReorderBuffer.Buffer[dependency].val + immediate) & (~1);
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
#ifdef DEBUG
  std::cout << "LSB busy done:" << _LoadStoreBuffer.busy << " " << _LoadStoreBuffer.done << std::endl;
#endif
  if (_LoadStoreBuffer.LSB.empty()) return;
  for (int i = _LoadStoreBuffer.LSB.FirstIndex(); i != _LoadStoreBuffer.LSB.EndIndex(); i = _LoadStoreBuffer.LSB.sub(i)) {
    //Not changed to iterator right now, future work
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
#ifdef DEBUG
  std::cout << "LSB BUSY DONE:" << _LoadStoreBuffer.busy << " " << _LoadStoreBuffer.done << std::endl;
#endif
  if (_LoadStoreBuffer.LSB.empty()) return;
  auto curLS = _LoadStoreBuffer.LSB.front();
  if (_LoadStoreBuffer.done) {
    switch (curLS.instruction) {
      case Instruction::LB:
      case Instruction::LH:
      case Instruction::LW:
      case Instruction::LBU:
      case Instruction::LHU: {
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].val = curLS.result;
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].ready = true;
        break;
      }
      case Instruction::SB:
      case Instruction::SH:
      case Instruction::SW: {
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].address = curLS.result;
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].val = curLS.v2;
        _ReorderBuffer.nxtBuffer[curLS.RoBIndex].ready = true;
        break;
      }//Store instruction's destination can only be calculated in LSB, not ready before
    }
    //cleanse of LSB should not be here
  }
}

void RISCV_Simulator::Issue() {
  if (dependency != -1) {
#ifdef DEBUG
    std::cout << "dependency:" << dependency << std::endl;
    printf("PC: %x\n", _InstructionUnit.PC);
    std::cout << "Blocked by JALR" << std::endl;
#endif
    return;//Blocked by JALR
  }
#ifdef DEBUG
  std::cout << "LSB front index: " << _LoadStoreBuffer.LSB.FirstIndex() << std::endl;
  std::cout << "LSB size: " << _LoadStoreBuffer.LSB.size() << std::endl;
  std::cout << "LSB first index: " << _LoadStoreBuffer.LSB.FirstIndex() << std::endl;
#endif
  Line newLine = _LoadStoreBuffer._Memory.ReadMemory(_InstructionUnit.PC, Instruction::LW);
  InstructionInfo instruction = ParseInstruction(newLine);
#ifdef DEBUG
  printf("PC: %x\n", _InstructionUnit.PC);
  std::cout << "Immediate:" << instruction.Immediate << std::endl;
  std::cout << "SR1: " << instruction.SR1 << std::endl;
  std::cout << "SR2: " << instruction.SR2 << std::endl;
  std::cout << "DR: " << instruction.DR << std::endl;
#endif
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
      if (_RegisterFile.ReadDependency(instruction.SR1) == -1) {
        _InstructionUnit.PC = _RegisterFile.ReadRegister(instruction.SR1) + instruction.Immediate;
      }//If there exists no dependency at the current moment, then it can be carried out right now
      else {
        immediate = instruction.Immediate;
        dependency = _RegisterFile.ReadDependency(instruction.SR1);
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
      newInfo.val = 0; newInfo.prediction = _BranchPredictor.GetPrediction(_InstructionUnit.PC);
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
      if (newInfo.prediction.first) _InstructionUnit.PC += instruction.Immediate;
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
#ifdef DEBUG
      std::cout << "STPC " << newInfo.curPC << std::endl;
#endif
      LoadStoreBufferEle newEle;
      newEle.RoBIndex = _ReorderBuffer.nxtBuffer.LastIndex();
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
    case Instruction::SRLI:
    case Instruction::SRAI:{
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

