#include "Register.h"

void Register::ResetDependency() {
  dependency = 0;
}

int Register::Dependency() const {
  return dependency;
}

void Register::SetDependency(int _dependency) {
  dependency = _dependency;
}

Line& Register::ReadData() {
  return data;
}

void RegisterFile::FlushRegister() {
  for (int i = 0; i < RegisterFileSize; i++)
    curRegister[i] = nxtRegister[i];
}

Line RegisterFile::ReadRegister(Line RegisterIndex) {
  return nxtRegister[RegisterIndex].ReadData();
}

void RegisterFile::ResetRegister() {
  for (int i = 0; i < RegisterFileSize; i++)
    curRegister[i].ResetDependency();
}

int RegisterFile::ReadDependency(Line RegisterIndex) {
  return curRegister[RegisterIndex].Dependency();
}

void RegisterFile::SetDependency(Line RegisterIndex, int _dependency) {
  nxtRegister[RegisterIndex].dependency = _dependency;
}

void RegisterFile::WriteRegister(Line RegisterIndex, Line _data) {
  if (!RegisterIndex) return;//x0 should not be modified
  nxtRegister[RegisterIndex].data = _data;
}