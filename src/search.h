#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"

#include <atomic>

class Searcher {
public:
    Searcher();
    Move getBestMove(Board board);

    void requestStop() { stopRequested.store(true, std::memory_order_relaxed); }

    void clearStop() { stopRequested.store(false, std::memory_order_relaxed); }

    bool wasStopped() const { return stopRequested.load(std::memory_order_relaxed); }

private:
    Board board;
    Move bestMove;
    std::atomic<bool> stopRequested{false};
    int negamax(int depth, int alpha, int beta, bool updateBestMove = true);
    int qSearch(int depth, int alpha, int beta);
};

#endif
