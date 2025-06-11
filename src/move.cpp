#include "board.h"

/*
 * Move flags:
 * Source: https://www.chessprogramming.org/Encoding_Moves
 *  0 - quiet moves
 *  1 - double pawn push
 *  2 - kingside castle
 *  3 - queenside castle
 *  4 - captures
 *  5 - en-passant capture
 *  8 - knight-promotion
 *  9 - bishop-promotion
 * 10 - rook-promotion
 * 11 - queen-promotion
 * 12 - knight-promo capture
 * 13 - bishop-promo capture
 * 14 - rook-promo capture
 * 15 - queen-promo capture
 */

Move::Move(unsigned int start, unsigned int destination, unsigned int flags) {
    move = ((flags & 0xf) << 12) | ((start & 0x3f) << 6) | (destination & 0x3f);
}

unsigned int Move::getStart() { return (move >> 6) & 0x3f; }

unsigned int Move::getDestination() { return move & 0x3f; }

unsigned int Move::isCapture() { return (move >> 14) & 1; }

unsigned int Move::getFlags() { return (move >> 12) & 0xf; }
