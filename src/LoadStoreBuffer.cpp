#include "LoadStoreBuffer.h"

void LoadStoreBuffer::Clear() {
  for (int i = 0; i < LSBSize; i++)
    LSB[i].busy = LSB[i].done = 0;
}

void LoadStoreBuffer::Work() {
  for (int i = 0; i < LSBSize; i++) {
    if ()
  }
}

bool LoadStoreBuffer::AppendBuffer(const LoadStoreBufferEle& newEle) {
  for (int i = 0; i < LSBSize; i++) {
    if (!LSB[i].busy) {
      LSB[i] = newEle;
      return true;
    }
  }
  return false;
}
