#include "evaluate.h"

namespace evaluate {

const int pieceSquareTables[6][64] = {
    {// pawn
     0,  0,   0,  0, 0,  0,  0,  0,   50,  50, 50, 50, 50, 50, 50, 50, 10, 10, 20, 30, 30,  20,
     10, 10,  5,  5, 10, 25, 25, 10,  5,   5,  0,  0,  0,  20, 20, 0,  0,  0,  5,  -5, -10, 0,
     0,  -10, -5, 5, 5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,  0,  0,  0,  0,  0},
    {
        // knight
        -50, -40, -30, -30, -30, -30, -40, -50, 
        -40, -20,   0,   0,   0,   0, -20, -40, 
        -30,   0,  10,  15,  15,  10,   0, -30, 
        -30,   5,  15,  20,  20,  15,   5, -30, 
        -30,   0,  15,  20,  20,  15,   0, -30, 
        -30,   5,  10,  15,  15,  10,   5, -30, 
        -40, -20,   0,   5,   5,   0, -20, -40, 
        -50, -40, -30, -30, -30, -30, -40, -50,
    },
    {
        // bishop
        -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,   0,   0,   -10, -10, 0,   5,   10,  10, 5,
        0,   -10, -10, 5,   5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,   -10, -10, 10,  10, 10,
        10,  10,  10,  -10, -10, 5,   0,   0,   0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20,
    },
    {// rook
     0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,  -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5,
     -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0, 0, -5, 0,  0, 0, 5, 5, 0, 0, 0},
    {// queen
     -20, -10, -10, -5,  -5,  -10, -10, -20, -10, 0,  0, 0,   0,   0,   0,   -10, -10, 0,   5,   5,  5, 5,
     0,   -10, -5,  0,   5,   5,   5,   5,   0,   -5, 0, 0,   5,   5,   5,   5,   0,   -5,  -10, 5,  5, 5,
     5,   5,   0,   -10, -10, 0,   5,   0,   0,   0,  0, -10, -20, -10, -10, -5,  -5,  -10, -10, -20},
    {// king
     -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40,
     -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30, -20, -10, -20, -20, -20,
     -20, -20, -20, -10, 20,  20,  0,   0,   0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20}};

int sumPieceValues(Board board, bool isWhitePieces) {
    const int pieceValues[5] = {100, 300, 320, 500, 900};
    int colourValue = isWhitePieces ? 0 : 8;
    int total = 0;
    for (int i = 1; i < 6; i++) {
        uint64_t bitboardCopy = board.getBitboard(colourValue + i);
        while (bitboardCopy) {
            int pieceSquare = std::countr_zero(bitboardCopy);
            int file = pieceSquare & 7;
            int rank = pieceSquare / 8;
            if (isWhitePieces) {
                rank = 7 - rank;
            }
            total += pieceValues[i - 1] + pieceSquareTables[i - 1][rank * 8 + file];
            bitboardCopy &= bitboardCopy - 1;
        }
    }
    return total;
}

int evaluatePosition(Board board) {
    int colourMultiplier = board.getIsWhiteTurn() ? 1 : -1;
    if (board.getGameStatus() == 1) {
        return -1000000;
    }
    if (board.getGameStatus() == 2) {
        return 0;
    }

    int score = sumPieceValues(board, true) - sumPieceValues(board, false);
    return colourMultiplier * score;
}

} // namespace evaluate
