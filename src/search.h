#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"

namespace search {

Move getBestMove(Board board);
int alphabeta(Board board, int depth, int alpha, int beta, bool updateBestMove = true);

}

#endif
