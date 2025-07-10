#include "search.h"
#include "evaluate.h"
#include <algorithm>
#include <limits>

namespace search {

Move bestMove = Move(-1, -1, -1);

Move getBestMove(Board board) {
    negamax(board, 6, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    return bestMove;
}

int negamax(Board board, int depth, int alpha, int beta, bool updateBestMove) {
    if (board.getMoves().size() == 0) {
        return evaluate::evaluatePosition(board);
    }
    if (depth <= 0) {
        if (!board.getMoves()[0].isCapture() && !board.getMoves()[0].isPromotion()) {
            return evaluate::evaluatePosition(board);
        } else {
            // return evaluate::evaluatePosition(board);
            return qSearch(board, alpha, beta);
        }
    }
    int value = -1000000000;
    for (Move move : board.getMoves()) {
        int newValue = -negamax(board.makeMove(move), depth - 1, -beta, -alpha, false);
        /*
        if (updateBestMove) {
            std::cout << 5 - depth << ": " << move.getStart() << ", " << move.getDestination() << ": " << newValue << '\n';
        }
        */
        if (newValue > value) {
            value = newValue;
            if (updateBestMove) {
                bestMove = move;
            }
        }
        alpha = std::max(alpha, value);
        if (alpha >= beta) {
            break;
        }
    }
    return value;
}

int qSearch(Board board, int alpha, int beta) {
    int bestValue = evaluate::evaluatePosition(board);
    if (bestValue >= beta) {
        return bestValue;
    }
    alpha = std::max(alpha, bestValue);

    for (Move move : board.getMoves()) {
        if (!move.isPromotion() && !move.isCapture()) {
            break;
        }
        int value = -qSearch(board.makeMove(move), -beta, -alpha);
        if (value >= beta) {
            return value;
        }
        if (value > bestValue) {
            bestValue = value;
        }
        alpha = std::max(alpha, value);
    }

    return bestValue;
}

} // namespace search
