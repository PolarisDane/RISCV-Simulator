#ifndef RISCV_SIMULATOR_REGISTER
#define RISCV_SIMULATOR_REGISTER

#include <iostream>

class Register{
private:
  
public:

}

class RegisterGroup {
private:
  static int RegisterGroupSize = 32;
  Register curRegister[RegisterGroupSize];
  Register nxtRegister[RegisterGroupSize];
public:
  void RenewRegister();
  void 
};

#endif