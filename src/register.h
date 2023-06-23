#ifndef RISCV_SIMULATOR_REGISTER
#define RISCV_SIMULATOR_REGISTER

#include <iostream>
#include <cstdio>
#include "Utils.h"
#include "RISCV_Simulator.h"

class Register{
  friend class RegisterFile;
private:
  Line data = 0;
  int dependency = 0;
public:
  Register() = default;
  Register(const Register& other) = default;
  ~Register() = default;
  void ResetDependency();
  int Dependency() const;
  void SetDependency(int _dependency);
  Line& ReadData();
}

class RegisterFile {
  friend class RISCV_Simulator;
private:
  static int RegisterFileSize = 32;
  Register curRegister[RegisterFileSize];
  Register nxtRegister[RegisterFileSize];
public:
  RegisterFile() = default;
  RegisterFile(const RegisterFile& other) = default;
  ~RegisterFile() = default;
  void FlushRegister();
  Line& ReadRegister(Line RegisterIndex);
  void WriteRegister(Line RegisterIndex, Line _data, int _dependency);
  void ResetRegister();
  int ReadDependency(Line RegisterIndex);
  void SetDependency(Line RegisterIndex, int _dependency);
};

#endif