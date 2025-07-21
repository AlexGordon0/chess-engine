#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"

class Searcher {
  public:
    Searcher();
    Move getBestMove(Board board);

  private:
    Board board;
    Move bestMove;
    int negamax(int depth, int alpha, int beta, bool updateBestMove = true);
    int qSearch(int depth, int alpha, int beta);
};

#endif
