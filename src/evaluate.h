#ifndef EVALUATE_H
#define EVALUATE_H

#include "board.h"

namespace evaluate {

int sumPieceValues(Board board, bool isWhitePieces);
int evaluatePosition(Board board, int depth = 0);

}

#endif
