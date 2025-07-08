#include "evaluate.h"
#include <limits>

namespace evaluate {

int sumPieceValues(Board board, bool isWhitePieces) {
    const int pieceValues[5] = {100, 300, 320, 500, 900};
    int colourValue = isWhitePieces ? 0 : 8;
    int total = 0;
    for (int i = 1; i < 6; i++) { uint64_t bitboardCopy = board.getBitboard(colourValue + i);
        while (bitboardCopy) {
            int pieceSquare = std::countr_zero(bitboardCopy);
            total += pieceValues[i - 1];
            bitboardCopy &= bitboardCopy - 1;
        }
    }
    return total;
}

int evaluatePosition(Board board) {
    int colourMultiplier = board.getIsWhiteTurn() ? 1 : -1;
    if (board.getGameStatus() == 1) {
        return colourMultiplier * -1000000;
    }
    if (board.getGameStatus() == 2) {
        return 0;
    }

    int whitePiecesValue = sumPieceValues(board, true);
    int blackPiecesValue = sumPieceValues(board, false);

    return whitePiecesValue - blackPiecesValue;
}

} // namespace evaluate
