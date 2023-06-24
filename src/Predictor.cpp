#include "Predictor.h"

void BranchPredictor::UpdateBranchPredictor(bool newBranch) {
  if (newBranch)
    History[prev] = std::min(History[prev] + 1, 3);
  else
    History[prev] = std::max(History[prev] - 1, 0);
  prev = ((prev << 1) | newBranch) & 0b1111;
}

void BranchPredictor::GetPrediction() {
  return History[prev] >= 2;
}