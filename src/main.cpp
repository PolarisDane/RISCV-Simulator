#include <iostream>
#include "RISCV_Simulator.h"

signed main() {
  freopen("bulgarian.data", "r", stdin);
  freopen("ans.out", "w", stdout);
  RISCV_Simulator RISCV;
  RISCV.Run();
  return 0;
}

