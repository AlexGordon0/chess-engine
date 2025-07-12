#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"

namespace search {

Move getBestMove(Board board);
int negamax(Board board, int depth, int alpha, int beta, bool updateBestMove = true);
int qSearch(Board board, int depth, int alpha, int beta);

}

#endif
