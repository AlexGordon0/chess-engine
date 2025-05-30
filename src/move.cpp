#include "board.h"

Move::Move(unsigned int start, unsigned int destination, unsigned int flags) {
  move = ((flags & 0xf) << 12) | ((start & 0x3f) << 6) | (destination & 0x3f);
}

unsigned int Move::getStart() {
  return (move >> 6) & 0x3f;
}

unsigned int Move::getDestination() {
  return move & 0x3f;
}

unsigned int Move::isCapture() {
  return (move >> 14) & 1;
}

unsigned int Move::getFlags() {
  return (move >> 12) & 0xf;
}