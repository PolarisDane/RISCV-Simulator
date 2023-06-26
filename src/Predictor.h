#ifndef RISCV_SIMULATOR_PREDICTOR
#define RISCV_SIMULATOR_PREDICTOR

#include <iostream>
#include <cstdio>
#include <cstring>
#include "Utils.h"

class BranchPredictor {
private:
  int prev;
  int History[16];
public:
  BranchPredictor() {
    prev = 0;
    memset(History, 0, sizeof(History));
  }
  BranchPredictor(const BranchPredictor& other) = default;
  ~BranchPredictor() = default;

  void UpdateBranchPredictor(bool newBranch);
  bool GetPrediction();
};

#endif