#include "Parser.h"

Byte ParseOperation(Line instruction) {
  return instruction & (0x7F);
}

Line ParseJALImmediate(Line instruction) {
  Line immediate = 0;
  immediate |= instruction & (0xFF000);//19:12
  immediate |= (instruction >> 9) & (0x800);//11
  immediate |= (instruction >> 20) & (0x7FE);
  immediate |= (instruction >> 11) & (0x100000);
  return immediate & (0x100000) ? immediate | (0xFFF00000) : immediate;//SEXT
}

Line ParseSR1(Line instruction) {
  return instruction & (0xF8000);
}

Line ParseSR2(Line instruction) {
  return instruction & (0x1F00000);
}

Line ParseDR(Line instruction) {
  return instruction & (0xF80);
}

Line ParseArithmeticImmediate(Line instruction) {
  return (instruction >> 20) & (0xFFF);
}

Line ParseStoreImmediate(Line instruction) {
  Line immediate = 0;
  immediate |= (instruction >> 7) & (0x1F);
  immediate |= (instruction >> 20) & (0xFE0);
  return immediate;
}

Line ParseLoadImmediate(Line instruction) {
  return (instruction >> 20) & (0xFFF);
}

Line ParseBranchImmediate(Line instruction) {
  Line immediate = 0;
  immediate |= (immediate << 4) & (0x800);
  immediate |= (immediate >> 7) & (0x1E);
  immediate |= (immediate >> 20) & (0x7E0);
  immediate |= (immediate >> 19) & (0x800);
  return immediate & (0x800) ? immediate & (0xFFFFF000) : immediate;
}

Line ParseShamt(Line instruction) {
  return instruction & (0x1F00000);
}



