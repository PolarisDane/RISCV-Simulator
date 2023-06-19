#ifndef RISCV_SIMULATOR_PARSER
#define RISCV_SIMULATOR_PARSER

#include <iostream>

enum instruction {

};

class Instruction {
  Instruction();
  Instruction(const Instruction& other) = default;
  Instruction(Instruction&&) = default;
  ~Instruction();
};

#endif