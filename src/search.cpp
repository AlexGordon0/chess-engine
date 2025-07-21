#include "search.h"
#include "evaluate.h"
#include <algorithm>
#include <limits>

Searcher::Searcher() : bestMove(-1, -1, -1), board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}

Move Searcher::getBestMove(Board _board) {
    bestMove = Move(-1, -1, -1);
    board = _board;
    negamax(6, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    return bestMove;
}

int Searcher::negamax(int depth, int alpha, int beta, bool updateBestMove) {
    std::vector<Move> moves = board.getMoves();
    std::sort(moves.begin(), moves.end(), std::greater<>());
    if (moves.size() == 0 || board.getGameStatus()) {
        return evaluate::evaluatePosition(board, depth);
    }
    if (depth <= 0) {
        if (!moves[0].isCapture() && !moves[0].isPromotion()) {
            return evaluate::evaluatePosition(board);
        } else {
            // return evaluate::evaluatePosition(board);
            return qSearch(depth, alpha, beta);
        }
    }
    int value = -1000000000;
    for (Move move : moves) {
        board.makeMove(move);
        int newValue = -negamax(depth - 1, -beta, -alpha, false);
        board.unmakeMove(move);
        if (updateBestMove) {
            /* std::cout << 7 - depth << ": " << move.getStart() << ", " << move.getDestination() << ": " << newValue
                      << '\n'; */
        }
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

int Searcher::qSearch(int depth, int alpha, int beta) {
    int bestValue = evaluate::evaluatePosition(board, depth);
    if (bestValue >= beta) {
        return bestValue;
    }
    alpha = std::max(alpha, bestValue);

    std::vector<Move> moves = board.getMoves();
    std::sort(moves.begin(), moves.end(), std::greater<>());
    for (Move move : moves) {
        if (!move.isPromotion() && !move.isCapture()) {
            break;
        }
        board.makeMove(move);
        int value = -qSearch(depth - 1, -beta, -alpha);
        board.unmakeMove(move);
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
