#include <iostream>
#include "RISCV_Simulator.h"

signed main() {
  freopen("array_test1.data", "r", stdin);
  freopen("my.out", "w", stdout);
  RISCV_Simulator RISCV;
  RISCV.Run();
  return 0;
}
