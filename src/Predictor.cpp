#include "Predictor.h"

void GlobalBranchPredictor::UpdateBranchPredictor(bool newBranch) {
  if (newBranch)
    History[prev] = std::min(History[prev] + 1, 3);
  else
    History[prev] = std::max(History[prev] - 1, 0);
  prev = ((prev << 1) | newBranch) & 0b1111;
}

bool GlobalBranchPredictor::GetPrediction() {
  return History[prev] >= 2;
}

void LocalBranchPredictor::UpdateBranchPredictor(Line PC, bool newBranch) {
  prediction[PC][History[PC]] = newBranch;
  History[PC] = ((History[PC] << 1) | newBranch) & 0b1111;
}

bool LocalBranchPredictor::GetPrediction(Line PC) {
  return prediction[PC][History[PC]];
}

void BranchPredictor::UpdateBranchPredictor(Line PC, bool newBranch,bool choice) {
  if (choice) {
    if (LBP.GetPrediction(PC) == newBranch) prev = std::min(prev + 1, 3);
    else prev = std::max(prev - 1, 0);
  }
  else {
    if (GBP.GetPrediction() == newBranch) prev = std::max(prev - 1, 0);
    else prev = std::min(prev + 1, 3);
  }
  LBP.UpdateBranchPredictor(PC, newBranch);
  GBP.UpdateBranchPredictor(newBranch);
}

std::pair<bool, bool> BranchPredictor::GetPrediction(Line PC) {
  if (prev >= 2) return std::make_pair(LBP.GetPrediction(PC), 1);
  else return std::make_pair(GBP.GetPrediction(), 0);
}