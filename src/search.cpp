#include "search.h"
#include "evaluate.h"
#include <limits>

namespace search {

Move bestMove = Move(-1, -1, -1);

Move getBestMove(Board board) {
    alphabeta(board, 6, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    return bestMove;
}

int alphabeta(Board board, int depth, int alpha, int beta, bool updateBestMove) {
    if (depth == 0 || board.getMoves().size() == 0) {
        return evaluate::evaluatePosition(board);
    }
    if (board.getIsWhiteTurn()) {
        for (Move move : board.getMoves()) {
            int value = alphabeta(board.makeMove(move), depth - 1, alpha, beta, false);
            if (value >= alpha) {
                alpha = value;
                if (updateBestMove) {
                    bestMove = move;
                }
            }
            if (alpha >= beta) {
                return alpha;
            }
        }
        return alpha;
    } else {
        for (Move move : board.getMoves()) {
            int value = alphabeta(board.makeMove(move), depth - 1, alpha, beta, false);
            if (value <= beta) {
                beta = value;
                if (updateBestMove) {
                    bestMove = move;
                }
            }
            if (beta <= alpha) {
                return beta;
            }
        }
        return beta;
    }
}

} // namespace search
