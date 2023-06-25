#include "ReorderBuffer.h"

void ReorderBuffer::FlushBuffer() {
  Buffer = nxtBuffer;
}

void ReorderBuffer::ClearBuffer() {
  nxtBuffer.clear();
}
