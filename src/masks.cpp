#include "masks.h"

namespace masks {

std::array<std::array<int, 8>, 64> calculateNumSquaresToEdge() {
    std::array<std::array<int, 8>, 64> numSquaresToEdge;
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int numNorth = 7 - rank;
            int numSouth = rank;
            int numWest = file;
            int numEast = 7 - file;

            int index = rank * 8 + file;

            numSquaresToEdge[index][0] = numNorth;
            numSquaresToEdge[index][1] = numSouth;
            numSquaresToEdge[index][2] = numWest;
            numSquaresToEdge[index][3] = numEast;
            numSquaresToEdge[index][4] = std::min(numNorth, numWest);
            numSquaresToEdge[index][5] = std::min(numSouth, numEast);
            numSquaresToEdge[index][6] = std::min(numSouth, numWest);
            numSquaresToEdge[index][7] = std::min(numNorth, numEast);
        }
    }
    return numSquaresToEdge;
}

std::array<uint64_t, 64> calculateKnightMasks() {
    std::array<uint64_t, 64> knightMask;
    int knightMoveOffsets[8] = {10, -6, 17, -15, -17, 15, 6, -10};
    int start = 0;
    int end = 4;
    for (int file = 0; file < 8; file++) {
        if (file == 1)
            end = 6; if (file == 2)
            end = 8;
        if (file == 6)
            start = 2;
        if (file == 7)
            start = 4;

        for (int rank = 0; rank < 8; rank++) {
            uint64_t bitmap = 0;
            int squareIndex = rank * 8 + file;
            for (int directionIndex = start; directionIndex < end; directionIndex++) {
                int possibleSquare = squareIndex + knightMoveOffsets[directionIndex];
                if (possibleSquare >= 0 && possibleSquare < 64) {
                    bitmap += 1ULL << possibleSquare;
                }
            }
            knightMask[squareIndex] = bitmap;
        }
    }
    return knightMask;
}

std::array<uint64_t, 64> calculateBishopMasks() {
    std::array<uint64_t, 64> bishopMask;
    int bishopMoveOffsets[4] = {7, -7, -9, 9};
    for (int square = 0; square < 64; square++) {
        uint64_t bitmap = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= numSquaresToEdge[square][i + 4]; j++) {
                bitmap |= 1ULL << (square + bishopMoveOffsets[i] * j);
            }
        }
        bishopMask[square] = bitmap;
    }
    return bishopMask;
}

std::array<uint64_t, 64> calculateRookMasks() {
    std::array<uint64_t, 64> rookMask;
    int rookMoveOffsets[4] = {8, -8, -1, 1};
    for (int square = 0; square < 64; square++) {
        uint64_t bitmap = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j <= numSquaresToEdge[square][i]; j++) {
                bitmap |= 1ULL << (square + rookMoveOffsets[i] * j);
            }
        }
        rookMask[square] = bitmap;
    }
    return rookMask;
}

std::array<uint64_t, 64> calculateKingMasks() {
    std::array<uint64_t, 64> kingMask;
    int kingMoveOffsets[9] = {8, 9, 1, -7, -8, -9, -1, 7, 8};
    int start = 0;
    int end = 5;
    for (int file = 0; file < 8; file++) {
        if (file == 1)
            end = 8;
        if (file == 7) {
            start = 4;
            end = 9;
        }
        for (int rank = 0; rank < 8; rank++) {
            uint64_t bitmap = 0;
            int squareIndex = rank * 8 + file;
            for (int directionIndex = start; directionIndex < end; directionIndex++) {
                int possibleSquare = squareIndex + kingMoveOffsets[directionIndex];
                if (possibleSquare >= 0 && possibleSquare < 64) {
                    bitmap += 1ULL << possibleSquare;
                }
            }
            kingMask[squareIndex] = bitmap;
        }
    }
    return kingMask;
}

std::array<std::array<int, 8>, 64> numSquaresToEdge = calculateNumSquaresToEdge();
std::array<uint64_t, 64> knightMoveMasks = calculateKnightMasks();
std::array<uint64_t, 64> bishopMoveMasks = calculateBishopMasks();
std::array<uint64_t, 64> rookMoveMasks = calculateRookMasks();
std::array<uint64_t, 64> kingMoveMasks = calculateKingMasks();

} // namespace masks
