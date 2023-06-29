#ifndef RISCV_SIMULATOR_PREDICTOR
#define RISCV_SIMULATOR_PREDICTOR

#include <iostream>
#include <cstdio>
#include <cstring>
#include <bitset>
#include "Utils.h"

const int GlobalHistorySize = 4;
const int LocalHistorySize = 4;

class GlobalBranchPredictor {
private:
  int prev;
  int History[1 << GlobalHistorySize];
public:
  GlobalBranchPredictor() {
    prev = 0;
    memset(History, 0, sizeof(History));
  }
  GlobalBranchPredictor(const GlobalBranchPredictor& other) = default;
  ~GlobalBranchPredictor() = default;

  void UpdateBranchPredictor(bool newBranch);
  bool GetPrediction();
};

class LocalBranchPredictor {
private:
  int History[1 << 16];
  std::bitset<1 << LocalHistorySize>prediction[1 << 16];
public:
  LocalBranchPredictor() {
    memset(History, 0, sizeof(History));
  }
  LocalBranchPredictor(const LocalBranchPredictor& other) = default;
  ~LocalBranchPredictor() = default;

  void UpdateBranchPredictor(Line PC, bool newBranch);
  bool GetPrediction(Line PC);
};

class BranchPredictor {
private:
  GlobalBranchPredictor GBP;
  LocalBranchPredictor LBP;
  int prev = 0;
public:
  BranchPredictor() = default;
  BranchPredictor(const BranchPredictor& other) = default;
  ~BranchPredictor() = default;

  void UpdateBranchPredictor(Line PC, bool newBranch, bool choice);
  std::pair<bool, bool> GetPrediction(Line PC);
};

#endif