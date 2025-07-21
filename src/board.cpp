#include "board.h"
#include "magics.h"
#include "masks.h"
#include <algorithm>
#include <bit>
#include <cstdint>
#include <map>
#include <sstream>

#define WHITE 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6
#define BLACK 8

Board::Board(std::string startingPos) {
    convertFromFen(startingPos);
    setup();
}

void Board::setup() {
    moves.reserve(40);
    determineCheckStatus();
    calculatePinnedPieces();
    generateLegalMoves();
    gameStatus = 0;

    if (moves.size() == 0) {
        if (numChecks) {
            // Checkmate
            gameStatus = 1;
        } else {
            // Draw
            gameStatus = 2;
        }
    }
    // 50 move rule
    if (halfMoves >= 100) {
        gameStatus = 2;
    }

    // Three move repetition
    int repetitions = 0;
    for (int i = repetitionStart; i < zobristHashes.size(); i++) {
        if (zobristHashes[i] == currentPositionHash) {
            repetitions++;
        }
    }
    if (repetitions >= 2) {
        gameStatus = 2;
    }
}

std::array<short, 64> Board::getState() { return state; }

std::vector<Move> Board::getMoves() { return moves; }

uint64_t Board::getBitboard(int index) { return bitboards[index]; }

short Board::getEnPassantSquare() { return enPassantSquare; }

bool Board::getIsWhiteTurn() { return isWhiteTurn; }

uint64_t Board::getOpponentAttackMap() { return opponentAttackMap; }

short Board::getGameStatus() { return gameStatus; }

void Board::convertFromFen(std::string fenString) {
    std::map<char, int> pieceLetterToPieceNum = {{'p', 1}, {'n', 2}, {'b', 3}, {'r', 4}, {'q', 5}, {'k', 6}};

    int file = 0, rank = 7;

    std::vector<std::string> segments;
    std::string segment;
    std::stringstream ss(fenString);
    while (std::getline(ss, segment, ' ')) {
        segments.push_back(segment);
    }

    for (char c : segments[0]) {
        if (std::isdigit(c)) {
            file += c - '0';
        } else if (c == '/') {
            file = 0;
            rank--;
        } else {
            int colourValue = islower(c) ? BLACK : WHITE;
            int value = colourValue + pieceLetterToPieceNum.at(tolower(c));
            state[rank * 8 + file] = value;
            bitboards[value] |= 1ULL << (rank * 8 + file);
            bitboards[colourValue] |= 1ULL << (rank * 8 + file);
            file++;
        }
    }

    if (segments[1] == "w") {
        isWhiteTurn = true;
    } else if (segments[1] == "b") {
        isWhiteTurn = false;
    }

    int castlingValue = 0;
    for (char c : segments[2]) {
        if (c == 'K') {
            castlingValue += 8;
        }
        if (c == 'Q') {
            castlingValue += 4;
        }
        if (c == 'k') {
            castlingValue += 2;
        }
        if (c == 'q') {
            castlingValue += 1;
        }
    }
    castlingRights = castlingValue;

    if (segments[3] == "-") {
        enPassantSquare = -1;
    }
    if (segments[3].length() == 2) {
        enPassantSquare = (segments[3][1] - '0' - 1) * 8 + (segments[3][0] - 97);
    }

    ply = 0;
    repetitionStart = 0;
    halfMoves = segments[4][0] - '0';
    fullMoves = segments[5][0] - '0';
    currentPositionHash = zobrist();
}

uint64_t Board::zobrist() {
    uint64_t hash = 0;
    for (int i = 0; i < 64; i++) {
        int piece = state[i];
        if (piece) {
            hash ^= magics::zobristKeys[(piece - 1 - (piece / 8) * 2) * 64 + i];
        }
    }
    if (!isWhiteTurn) {
        hash ^= magics::zobristKeys[768];
    }
    hash ^= magics::zobristKeys[769 + castlingRights];
    if (enPassantSquare != -1) {
        int enPassantFile = enPassantSquare & 7;
        hash ^= magics::zobristKeys[785 + enPassantFile];
    }
    return hash;
}

void Board::determineCheckStatus() {
    checkEvasionMask = 0xffffffffffffffff;
    numChecks = 0;
    uint64_t pawnAttacks = generatePawnAttackMaps();
    uint64_t knightAttacks = generateKnightAttackMaps();
    uint64_t slidingAttacks = generateSlidingAttackMaps();
    uint64_t kingAttacks = generateKingAttackMaps();

    opponentAttackMap = pawnAttacks | knightAttacks | slidingAttacks | kingAttacks;

    int colourValue = isWhiteTurn ? WHITE : BLACK;
    uint64_t kingLocation = bitboards[colourValue + KING];
    if (pawnAttacks & kingLocation) {
        numChecks++;
    }
    if (knightAttacks & kingLocation) {
        numChecks++;
    }
    if (kingAttacks & kingLocation) {
        numChecks++;
    }
}

uint64_t Board::generatePawnAttackMaps() {
    uint64_t attackMap = 0;
    uint64_t attackingPawnMap = 0;
    if (isWhiteTurn) {
        uint64_t oppositionKingBitboard = bitboards[WHITE + KING];

        uint64_t westCaptures = bitboards[BLACK + PAWN] >> 9 & 0x7f7f7f7f7f7f7f7f;
        if (westCaptures & oppositionKingBitboard) {
            attackingPawnMap = (westCaptures & oppositionKingBitboard) << 9;
        }

        uint64_t eastCaptures = bitboards[BLACK + PAWN] >> 7 & 0xfefefefefefefefe;
        if (eastCaptures & oppositionKingBitboard) {
            attackingPawnMap = (eastCaptures & oppositionKingBitboard) << 7;
        }

        if (attackingPawnMap << 8 & 1ULL << enPassantSquare) {
            attackingPawnMap |= 1ULL << enPassantSquare;
        }

        attackMap = westCaptures | eastCaptures;
    } else {
        uint64_t oppositionKingBitboard = bitboards[BLACK + KING];

        uint64_t westCaptures = bitboards[WHITE + PAWN] << 7 & 0x7f7f7f7f7f7f7f7f;
        if (westCaptures & oppositionKingBitboard) {
            attackingPawnMap = (westCaptures & oppositionKingBitboard) >> 7;
        }

        uint64_t eastCaptures = bitboards[WHITE + PAWN] << 9 & 0xfefefefefefefefe;
        if (eastCaptures & oppositionKingBitboard) {
            attackingPawnMap = (eastCaptures & oppositionKingBitboard) >> 9;
        }

        if (attackingPawnMap >> 8 & 1ULL << enPassantSquare) {
            attackingPawnMap |= 1ULL << enPassantSquare;
        }

        attackMap = westCaptures | eastCaptures;
    }
    if (attackingPawnMap) {
        checkEvasionMask = attackingPawnMap;
    }
    return attackMap;
}

uint64_t Board::generateKnightAttackMaps() {
    int colourValue = isWhiteTurn ? BLACK : WHITE;
    uint64_t bitboardCopy = bitboards[colourValue + KNIGHT];
    uint64_t oppositionKingBitboard = bitboards[8 - colourValue + KING];

    uint64_t attackMap = 0;
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        if (masks::knightMoveMasks[pieceSquare] & oppositionKingBitboard) {
            checkEvasionMask = 1ULL << pieceSquare;
        }
        attackMap |= masks::knightMoveMasks[pieceSquare];
        bitboardCopy &= bitboardCopy - 1;
    }
    return attackMap;
}

uint64_t Board::generateSlidingAttackMaps() {
    int colourValue = isWhiteTurn ? BLACK : WHITE;
    uint64_t attackMap = 0;
    uint64_t oppositionKing = bitboards[8 - colourValue + KING];

    for (int i = 3; i < 6; i++) {
        uint64_t bitboardCopy = bitboards[colourValue + i];
        while (bitboardCopy) {
            int pieceSquare = std::countr_zero(bitboardCopy);

            if (i == BISHOP | i == QUEEN) {
                uint64_t occupancyMask = magics::bishopOccupancyMasks[pieceSquare];
                uint64_t magic = magics::bishopMagics[pieceSquare];
                int numBits = magics::bishopNumBits[pieceSquare];
                int index = ((((bitboards[WHITE] | bitboards[BLACK]) & ~oppositionKing) & occupancyMask) * magic) >>
                            (64 - numBits);
                uint64_t possibleMoves = magics::bishopLookupTable[pieceSquare][index];
                attackMap |= possibleMoves;
                if (possibleMoves & oppositionKing) {
                    numChecks++;
                    calculateBlockMask(pieceSquare, std::countr_zero(oppositionKing), true);
                }
            }
            if (i == ROOK | i == QUEEN) {
                uint64_t occupancyMask = magics::rookOccupancyMasks[pieceSquare];
                uint64_t magic = magics::rookMagics[pieceSquare];
                int numBits = magics::rookNumBits[pieceSquare];
                int index = ((((bitboards[WHITE] | bitboards[BLACK]) & ~oppositionKing) & occupancyMask) * magic) >>
                            (64 - numBits);
                uint64_t possibleMoves = magics::rookLookupTable[pieceSquare][index];
                attackMap |= possibleMoves;
                if (possibleMoves & oppositionKing) {
                    numChecks++;
                    calculateBlockMask(pieceSquare, std::countr_zero(oppositionKing), false);
                }
            }

            bitboardCopy &= bitboardCopy - 1;
        }
    }

    return attackMap;
}

void Board::calculateBlockMask(int pieceSquare, int kingSquare, bool isBishop) {
    uint64_t magic = isBishop ? magics::bishopMagics[pieceSquare] : magics::rookMagics[pieceSquare];
    uint64_t occupancyMask =
        isBishop ? magics::bishopOccupancyMasks[pieceSquare] : magics::rookOccupancyMasks[pieceSquare];
    int numBits = isBishop ? magics::bishopNumBits[pieceSquare] : magics::rookNumBits[pieceSquare];

    int pieceIndex = (((bitboards[WHITE] | bitboards[BLACK]) & occupancyMask) * magic) >> (64 - numBits);
    uint64_t pieceAttacks = isBishop ? magics::bishopLookupTable[pieceSquare][pieceIndex]
                                     : magics::rookLookupTable[pieceSquare][pieceIndex];

    magic = isBishop ? magics::bishopMagics[kingSquare] : magics::rookMagics[kingSquare];
    occupancyMask = isBishop ? magics::bishopOccupancyMasks[kingSquare] : magics::rookOccupancyMasks[kingSquare];
    numBits = isBishop ? magics::bishopNumBits[kingSquare] : magics::rookNumBits[kingSquare];

    int kingIndex = (((bitboards[WHITE] | bitboards[BLACK]) & occupancyMask) * magic) >> (64 - numBits);
    uint64_t kingAttacks =
        isBishop ? magics::bishopLookupTable[kingSquare][kingIndex] : magics::rookLookupTable[kingSquare][kingIndex];

    checkEvasionMask = pieceAttacks & kingAttacks | 1ULL << pieceSquare;
}

uint64_t Board::generateKingAttackMaps() {
    int colourValue = isWhiteTurn ? BLACK : WHITE;

    uint64_t bitboardCopy = bitboards[colourValue + KING];
    uint64_t possibleMoves = 0;
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        possibleMoves |= masks::kingMoveMasks[pieceSquare];
        bitboardCopy &= bitboardCopy - 1;
    }
    return possibleMoves;
}

void Board::calculatePinnedPieces() {
    int colourValue = isWhiteTurn ? WHITE : BLACK;
    uint64_t pieceMap = bitboards[WHITE] | bitboards[BLACK];
    pinnedPieces = 0;

    int kingLocation = std::countr_zero(bitboards[colourValue + KING]);
    int kingFile = kingLocation & 7;
    int kingRank = kingLocation / 8;

    uint64_t kingBishopMask = masks::bishopMoveMasks[kingLocation];
    uint64_t kingRookMask = masks::rookMoveMasks[kingLocation];
    uint64_t pinningPieces =
        (bitboards[8 - colourValue + BISHOP] | bitboards[8 - colourValue + QUEEN]) & kingBishopMask |
        (bitboards[8 - colourValue + ROOK] | bitboards[8 - colourValue + QUEEN]) & kingRookMask;
    while (pinningPieces) {
        int pieceSquare = std::countr_zero(pinningPieces);
        int pieceFile = pieceSquare & 7;
        int pieceRank = pieceSquare / 8;
        int chebyshevDistance = std::max(abs(kingFile - pieceFile), abs(kingRank - pieceRank));
        int kingOffset = (pieceSquare - kingLocation) / chebyshevDistance;

        uint64_t kingSightMap = 0;
        for (int i = 1;; i++) {
            int square = kingLocation + kingOffset * i;
            kingSightMap |= 1ULL << square;
            if (pieceMap >> square & 1) {
                break;
            }
        }
        uint64_t pieceSightMap = 1ULL << pieceSquare;
        for (int i = 1;; i++) {
            int square = pieceSquare - kingOffset * i;
            pieceSightMap |= 1ULL << square;
            if (pieceMap >> square & 1) {
                break;
            }
        }
        uint64_t pinnedPieceBitboard = kingSightMap & pieceSightMap & bitboards[colourValue];
        if (pinnedPieceBitboard) {
            int pinnedPieceSquare = std::countr_zero(pinnedPieceBitboard);
            pinnedPieces |= pinnedPieceBitboard;
            uint64_t possibleMovesRay = (kingSightMap | pieceSightMap) & ~pinnedPieceBitboard & checkEvasionMask;
            if ((state[pinnedPieceSquare] & 0b111) == PAWN) {
                addPinnedPawnMoves(pinnedPieceSquare, possibleMovesRay);
            } else {
                bool isDiagonalPin = !(abs(kingOffset) == 1 || abs(kingOffset) == 8);
                addPinnedPieceMoves(pinnedPieceSquare, possibleMovesRay, isDiagonalPin);
            }
        }
        pinningPieces &= pinningPieces - 1;
    }
}

void Board::addPinnedPieceMoves(int pieceSquare, uint64_t pinnedMovesMask, bool isDiagonalPin) {
    bool diagonalAndBishop = isDiagonalPin && (state[pieceSquare] & 0b111) == BISHOP;
    bool notDiagonalAndRook = !isDiagonalPin && (state[pieceSquare] & 0b111) == ROOK;
    bool isQueen = (state[pieceSquare] & 0b111) == QUEEN;
    if (diagonalAndBishop || notDiagonalAndRook || isQueen) {
        while (pinnedMovesMask) {
            int destination = std::countr_zero(pinnedMovesMask);
            if (state[destination]) {
                moves.push_back(Move(pieceSquare, destination, 4));
            } else {
                moves.push_back(Move(pieceSquare, destination, 0));
            }
            pinnedMovesMask &= pinnedMovesMask - 1;
        }
    }
}

void Board::addPinnedPawnMoves(int pieceSquare, uint64_t pinnedMovesMask) {
    uint64_t pinnedPawnBitboard = 1ULL << pieceSquare;
    if (isWhiteTurn) {
        uint64_t westCaptures = pinnedPawnBitboard << 7 & (bitboards[BLACK] | (1ULL << enPassantSquare)) &
                                0x7f7f7f7f7f7f7f7f & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(westCaptures, -7);

        uint64_t eastCaptures = pinnedPawnBitboard << 9 & (bitboards[BLACK] | (1ULL << enPassantSquare)) &
                                0xfefefefefefefefe & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(eastCaptures, -9);

        uint64_t forwardMoves =
            pinnedPawnBitboard << 8 & ~(bitboards[WHITE] | bitboards[BLACK]) & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(forwardMoves, -8);

        uint64_t unmovedPawns = pinnedPawnBitboard & 0xff00;
        uint64_t freeSquares = ~(bitboards[WHITE] | bitboards[BLACK]);
        uint64_t doublePawnMoves = (unmovedPawns << 8 & freeSquares) << 8 & (unmovedPawns << 16 & freeSquares) &
                                   checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(doublePawnMoves, -16);
    } else {
        uint64_t westCaptures = pinnedPawnBitboard >> 9 & (bitboards[WHITE] | (1ULL << enPassantSquare)) &
                                0x7f7f7f7f7f7f7f7f & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(westCaptures, 9);

        uint64_t eastCaptures = pinnedPawnBitboard >> 7 & (bitboards[WHITE] | (1ULL << enPassantSquare)) &
                                0xfefefefefefefefe & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(eastCaptures, 7);

        uint64_t forwardMoves =
            pinnedPawnBitboard >> 8 & ~(bitboards[WHITE] | bitboards[BLACK]) & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(forwardMoves, 8);

        uint64_t unmovedPawns = pinnedPawnBitboard & 0xff000000000000;
        uint64_t freeSquares = ~(bitboards[WHITE] | bitboards[BLACK]);
        uint64_t doublePawnMoves = (unmovedPawns >> 8 & freeSquares) >> 8 & (unmovedPawns >> 16 & freeSquares) &
                                   checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(doublePawnMoves, 16);
    }
}

void Board::generateLegalMoves() {
    // If in double check only the king can move so no need to generate other moves
    if (numChecks < 2) {
        generatePawnMoves();
        generateKnightMoves();
        generateSlidingMoves();
    }
    generateKingMoves();
}

void Board::addMovesFromBitmap(uint64_t bitmap, int startSquareOffset) {
    while (bitmap) {
        int destinationSquare = std::countr_zero(bitmap);
        int startSquare = destinationSquare + startSquareOffset;
        unsigned int flags = 0;
        if (std::abs(startSquareOffset) == 16) {
            flags |= 1;
        }
        if (state[destinationSquare]) {
            flags |= 1 << 2;
        }
        if (destinationSquare == enPassantSquare) {
            flags |= 5;
        }
        if (destinationSquare < 8 || destinationSquare > 55) {
            // Pawn has been promoted
            flags |= 1 << 3;
            for (int i = 1; i < 4; i++) {
                moves.push_back(Move(startSquare, destinationSquare, flags | i));
            }
        }

        if (flags == 5) {
            if (!checkEnPassantPin(startSquare)) {
                moves.push_back(Move(startSquare, destinationSquare, flags));
            }
        } else {
            moves.push_back(Move(startSquare, destinationSquare, flags));
        }
        bitmap &= bitmap - 1;
    }
}

// Checks the edge case where en passant would reveal an attack on the king.
bool Board::checkEnPassantPin(int startSquare) {
    int rank = startSquare / 8;
    uint64_t rankMask = 0xffULL << (rank * 8);
    int colourValue = isWhiteTurn ? WHITE : BLACK;

    if (!(rankMask & bitboards[colourValue + KING])) {
        return false;
    }
    if (!(rankMask & (bitboards[8 - colourValue + ROOK] | bitboards[8 - colourValue + QUEEN]))) {
        return false;
    }

    int offset = isWhiteTurn ? -8 : 8;
    int capturedPawn = enPassantSquare + offset;

    uint64_t pawnsRemoved = (bitboards[WHITE] | bitboards[BLACK]) & ~(1ULL << startSquare) & ~(1ULL << capturedPawn);
    int kingSquare = std::countr_zero(bitboards[colourValue + KING]);
    int offsets[2] = {-1, 1};
    for (int i = 0; i < 2; i++) {
        for (int j = 1; i <= masks::numSquaresToEdge[kingSquare][i + 2]; j++) {
            int square = kingSquare + offsets[i] * j;
            if (pawnsRemoved & 1ULL << square) {
                if (state[square] == (8 - colourValue + ROOK) || state[square] == (8 - colourValue + QUEEN)) {
                    return true;
                }
                break;
            }
        }
    }
    return false;
}

void Board::generatePawnMoves() {
    if (isWhiteTurn) {
        uint64_t pawnsWithoutPins = bitboards[WHITE + PAWN] & ~pinnedPieces;
        uint64_t westCaptures = pawnsWithoutPins << 7 & (bitboards[BLACK] | (1ULL << enPassantSquare)) &
                                0x7f7f7f7f7f7f7f7f & checkEvasionMask;
        addMovesFromBitmap(westCaptures, -7);

        uint64_t eastCaptures = pawnsWithoutPins << 9 & (bitboards[BLACK] | (1ULL << enPassantSquare)) &
                                0xfefefefefefefefe & checkEvasionMask;
        addMovesFromBitmap(eastCaptures, -9);

        uint64_t forwardMoves = pawnsWithoutPins << 8 & ~(bitboards[WHITE] | bitboards[BLACK]) & checkEvasionMask;
        addMovesFromBitmap(forwardMoves, -8);

        uint64_t unmovedPawns = pawnsWithoutPins & 0xff00;
        uint64_t freeSquares = ~(bitboards[WHITE] | bitboards[BLACK]);
        uint64_t doublePawnMoves =
            (unmovedPawns << 8 & freeSquares) << 8 & (unmovedPawns << 16 & freeSquares) & checkEvasionMask;
        addMovesFromBitmap(doublePawnMoves, -16);
    } else {
        uint64_t pawnsWithoutPins = bitboards[BLACK + PAWN] & ~pinnedPieces;
        uint64_t westCaptures = pawnsWithoutPins >> 9 & (bitboards[WHITE] | (1ULL << enPassantSquare)) &
                                0x7f7f7f7f7f7f7f7f & checkEvasionMask;
        addMovesFromBitmap(westCaptures, 9);

        uint64_t eastCaptures = pawnsWithoutPins >> 7 & (bitboards[WHITE] | (1ULL << enPassantSquare)) &
                                0xfefefefefefefefe & checkEvasionMask;
        addMovesFromBitmap(eastCaptures, 7);

        uint64_t forwardMoves = pawnsWithoutPins >> 8 & ~(bitboards[WHITE] | bitboards[BLACK]) & checkEvasionMask;
        addMovesFromBitmap(forwardMoves, 8);

        uint64_t unmovedPawns = pawnsWithoutPins & 0xff000000000000;
        uint64_t freeSquares = ~(bitboards[WHITE] | bitboards[BLACK]);
        uint64_t doublePawnMoves =
            (unmovedPawns >> 8 & freeSquares) >> 8 & (unmovedPawns >> 16 & freeSquares) & checkEvasionMask;
        addMovesFromBitmap(doublePawnMoves, 16);
    }
}

void Board::generateKnightMoves() {
    int colourValue = isWhiteTurn ? WHITE : BLACK;
    uint64_t bitboardCopy = bitboards[colourValue + KNIGHT] & ~pinnedPieces;
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        uint64_t possibleMoves = masks::knightMoveMasks[pieceSquare] & ~bitboards[colourValue] & checkEvasionMask;
        while (possibleMoves) {
            int destinationSquare = std::countr_zero(possibleMoves);
            int flags = 0;
            if (state[destinationSquare]) {
                flags += 1 << 2;
            }
            moves.push_back(Move(pieceSquare, destinationSquare, flags));
            possibleMoves &= possibleMoves - 1;
        }
        bitboardCopy &= bitboardCopy - 1;
    }
}

void Board::addSlidingMoves(uint64_t possibleMoves, int startSquare) {
    while (possibleMoves) {
        int destinationSquare = std::countr_zero(possibleMoves);
        if (state[destinationSquare]) {
            moves.push_back(Move(startSquare, destinationSquare, 4));
        } else {
            moves.push_back(Move(startSquare, destinationSquare, 0));
        }
        possibleMoves &= possibleMoves - 1;
    }
}

void Board::generateSlidingMoves() {
    int colourValue = isWhiteTurn ? WHITE : BLACK;
    for (int i = 3; i < 6; i++) {
        uint64_t bitboardCopy = bitboards[colourValue + i] & ~pinnedPieces;
        while (bitboardCopy) {
            int pieceSquare = std::countr_zero(bitboardCopy);
            if (i == BISHOP | i == QUEEN) {
                uint64_t occupancyMask = magics::bishopOccupancyMasks[pieceSquare];
                uint64_t magic = magics::bishopMagics[pieceSquare];
                int numBits = magics::bishopNumBits[pieceSquare];
                int index = (((bitboards[WHITE] | bitboards[BLACK]) & occupancyMask) * magic) >> (64 - numBits);
                uint64_t possibleMoves =
                    magics::bishopLookupTable[pieceSquare][index] & checkEvasionMask & ~bitboards[colourValue];
                addSlidingMoves(possibleMoves, pieceSquare);
            }
            if (i == ROOK | i == QUEEN) {
                uint64_t occupancyMask = magics::rookOccupancyMasks[pieceSquare];
                uint64_t magic = magics::rookMagics[pieceSquare];
                int numBits = magics::rookNumBits[pieceSquare];
                int index = (((bitboards[WHITE] | bitboards[BLACK]) & occupancyMask) * magic) >> (64 - numBits);
                uint64_t possibleMoves =
                    magics::rookLookupTable[pieceSquare][index] & checkEvasionMask & ~bitboards[colourValue];
                addSlidingMoves(possibleMoves, pieceSquare);
            }
            bitboardCopy &= bitboardCopy - 1;
        }
    }
}

void Board::generateKingMoves() {
    int colourValue = isWhiteTurn ? WHITE : BLACK;
    int colourCastlingRights = isWhiteTurn ? castlingRights >> 2 : castlingRights & 3;
    int kingStartingSquare = isWhiteTurn ? 4 : 60;

    uint64_t bitboardCopy = bitboards[colourValue + KING];
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        if (pieceSquare == kingStartingSquare && !numChecks) {
            if (colourCastlingRights & 1) {
                // Check queenside castling
                uint64_t queensideCastlingMask = 1ULL << (pieceSquare - 1) | 1ULL << (pieceSquare - 2);
                if (!(state[pieceSquare - 1] | state[pieceSquare - 2] | state[pieceSquare - 3]) &&
                    !(queensideCastlingMask & opponentAttackMap)) {
                    moves.push_back(Move(pieceSquare, pieceSquare - 2, 3));
                }
            }
            if (colourCastlingRights & 2) {
                // Check kingside castling
                uint64_t kingsideCastlingMask = 1ULL << (pieceSquare + 1) | 1ULL << (pieceSquare + 2);
                if (!(state[pieceSquare + 1] | state[pieceSquare + 2]) && !(kingsideCastlingMask & opponentAttackMap)) {
                    moves.push_back(Move(pieceSquare, pieceSquare + 2, 2));
                }
            }
        }
        uint64_t possibleMoves = masks::kingMoveMasks[pieceSquare] & ~bitboards[colourValue] & ~opponentAttackMap;
        while (possibleMoves) {
            int destinationSquare = std::countr_zero(possibleMoves);
            int flags = 0;
            if (state[destinationSquare]) {
                flags += 1 << 2;
            }
            moves.push_back(Move(pieceSquare, destinationSquare, flags));
            possibleMoves &= possibleMoves - 1;
        }
        bitboardCopy &= bitboardCopy - 1;
    }
}

void Board::makeMove(Move move) {
    short start = move.getStart();
    short destination = move.getDestination();
    short flags = move.getFlags();
    short pieceTaken = -1;
    short oldHalfMoves = halfMoves;

    zobristHashes.push_back(currentPositionHash);
    currentPositionHash ^= magics::zobristKeys[768];

    if (enPassantSquare != -1) {
        int enPassantFile = enPassantSquare & 7;
        currentPositionHash ^= magics::zobristKeys[785 + enPassantFile];
    }

    int newCastlingRights = castlingRights;
    ply++;
    halfMoves++;
    if (state[start] == WHITE + PAWN || state[start] == BLACK + PAWN) {
        halfMoves = 0;
    }

    if (!isWhiteTurn) {
        fullMoves++;
    }

    int colourValue = isWhiteTurn ? WHITE : BLACK;

    if (move.isCapture()) {
        halfMoves = 0;
        // Check if en passant or normal capture
        if (flags == 5) {
            int offset = isWhiteTurn ? -8 : 8;
            pieceTaken = state[destination + offset];
            currentPositionHash ^=
                magics::zobristKeys[(pieceTaken - 1 - (pieceTaken / 8) * 2) * 64 + destination + offset];
            bitboards[pieceTaken] -= 1ULL << (destination + offset);
            bitboards[8 - colourValue] -= 1ULL << (destination + offset);
            state[destination + offset] = 0;
        } else {
            pieceTaken = state[destination];
            currentPositionHash ^= magics::zobristKeys[(pieceTaken - 1 - (pieceTaken / 8) * 2) * 64 + destination];
            bitboards[pieceTaken] -= 1ULL << destination;
            bitboards[8 - colourValue] -= 1ULL << destination;
        }
    }

    gameHistory.push_back(BoardData(castlingRights, enPassantSquare, oldHalfMoves, pieceTaken));

    // Double pawn push
    if (flags == 1) {
        int offset = isWhiteTurn ? -8 : 8;
        enPassantSquare = destination + offset;
        int enPassantFile = enPassantSquare & 7;
        currentPositionHash ^= magics::zobristKeys[785 + enPassantFile];
    } else {
        enPassantSquare = -1;
    }

    // Kingside castle
    if (flags == 2) {
        newCastlingRights = isWhiteTurn ? newCastlingRights &= 3 : newCastlingRights &= 12;
        bitboards[colourValue + ROOK] -= 1ULL << (start + 3);
        bitboards[colourValue + ROOK] += 1ULL << (start + 1);
        bitboards[colourValue] -= 1ULL << (start + 3);
        bitboards[colourValue] += 1ULL << (start + 1);
        state[start + 3] = 0;
        state[start + 1] = colourValue + ROOK;
        currentPositionHash ^= (magics::zobristKeys[(colourValue + ROOK - 1 - (colourValue / 8) * 2) * 64 + start + 3] ^
                                magics::zobristKeys[(colourValue + ROOK - 1 - (colourValue / 8) * 2) * 64 + start + 1]);
    }

    // Queenside castle
    if (flags == 3) {
        newCastlingRights = isWhiteTurn ? newCastlingRights &= 3 : newCastlingRights &= 12;
        bitboards[colourValue + ROOK] -= 1ULL << (start - 4);
        bitboards[colourValue + ROOK] += 1ULL << (start - 1);
        bitboards[colourValue] -= 1ULL << (start - 4);
        bitboards[colourValue] += 1ULL << (start - 1);
        state[start - 4] = 0;
        state[start - 1] = colourValue + ROOK;
        currentPositionHash ^= (magics::zobristKeys[(colourValue + ROOK - 1 - (colourValue / 8) * 2) * 64 + start - 4] ^
                                magics::zobristKeys[(colourValue + ROOK - 1 - (colourValue / 8) * 2) * 64 + start - 1]);
    }

    int pieceMoved = state[start];
    bitboards[pieceMoved] -= 1ULL << start;
    bitboards[colourValue] -= 1ULL << start;
    bitboards[colourValue] += 1ULL << destination;
    state[start] = 0;
    currentPositionHash ^= magics::zobristKeys[(pieceMoved - 1 - (pieceMoved / 8) * 2) * 64 + start];

    if (move.isPromotion()) {
        int newPiece = (flags & 3) + colourValue + 2;
        bitboards[newPiece] += 1ULL << destination;
        state[destination] = newPiece;
        currentPositionHash ^= magics::zobristKeys[(newPiece - 1 - (newPiece / 8) * 2) * 64 + destination];
    } else {
        bitboards[pieceMoved] += 1ULL << destination;
        state[destination] = pieceMoved;
        currentPositionHash ^= magics::zobristKeys[(pieceMoved - 1 - (pieceMoved / 8) * 2) * 64 + destination];
    }

    // Update castling rights
    if (isWhiteTurn) {
        if (start == 0) {
            newCastlingRights &= 11;
        } else if (start == 4) {
            newCastlingRights &= 3;
        } else if (start == 7) {
            newCastlingRights &= 7;
        } else if (destination == 56) {
            newCastlingRights &= 14;
        } else if (destination == 63) {
            newCastlingRights &= 13;
        }
    } else {
        if (start == 56) {
            newCastlingRights &= 14;
        } else if (start == 60) {
            newCastlingRights &= 12;
        } else if (start == 63) {
            newCastlingRights &= 13;
        } else if (destination == 0) {
            newCastlingRights &= 11;
        } else if (destination == 7) {
            newCastlingRights &= 7;
        }
    }

    if (newCastlingRights != castlingRights) {
        currentPositionHash ^= magics::zobristKeys[769 + castlingRights];
        currentPositionHash ^= magics::zobristKeys[769 + newCastlingRights];
        castlingRights = newCastlingRights;
    }

    if (!halfMoves) {
        repetitionStart = ply;
    }

    isWhiteTurn = !isWhiteTurn;

    moves.clear();
    setup();
}

void Board::unmakeMove(Move move) {
    short start = move.getStart();
    short destination = move.getDestination();
    short flags = move.getFlags();

    short colourValue = isWhiteTurn ? BLACK : WHITE;

    currentPositionHash = zobristHashes.back();
    zobristHashes.pop_back();
    ply--;
    if (isWhiteTurn) {
        fullMoves--;
    }

    castlingRights = gameHistory.back().castlingRights;
    enPassantSquare = gameHistory.back().enPassantSquare;
    halfMoves = gameHistory.back().halfMoves;

    if (move.isPromotion()) {
        short promotionPiece = state[destination];
        bitboards[promotionPiece] -= 1ULL << destination;
        bitboards[colourValue + PAWN] += 1ULL << start;
        state[start] = colourValue + PAWN;
    } else {
        short pieceMoved = state[destination];
        bitboards[pieceMoved] -= 1ULL << destination;
        bitboards[pieceMoved] += 1ULL << start;
        state[start] = pieceMoved;
    }

    bitboards[colourValue] -= 1ULL << destination;
    bitboards[colourValue] += 1ULL << start;
    state[destination] = 0;

    if (move.isCapture()) {
        // Check if en passant or normal capture
        if (flags == 5) {
            int offset = isWhiteTurn ? 8 : -8;
            int pieceTaken = gameHistory.back().capturedPiece;
            bitboards[pieceTaken] += 1ULL << (destination + offset);
            bitboards[8 - colourValue] += 1ULL << (destination + offset);
            state[destination + offset] = pieceTaken;
        } else {
            int pieceTaken = gameHistory.back().capturedPiece;
            bitboards[pieceTaken] += 1ULL << destination;
            bitboards[8 - colourValue] += 1ULL << destination;
            state[destination] = pieceTaken;
        }
    }

    // Kingside castle
    if (flags == 2) {
        bitboards[colourValue + ROOK] += 1ULL << (start + 3);
        bitboards[colourValue + ROOK] -= 1ULL << (start + 1);
        bitboards[colourValue] += 1ULL << (start + 3);
        bitboards[colourValue] -= 1ULL << (start + 1);
        state[start + 1] = 0;
        state[start + 3] = colourValue + ROOK;
    }

    // Queenside castle
    if (flags == 3) {
        bitboards[colourValue + ROOK] += 1ULL << (start - 4);
        bitboards[colourValue + ROOK] -= 1ULL << (start - 1);
        bitboards[colourValue] += 1ULL << (start - 4);
        bitboards[colourValue] -= 1ULL << (start - 1);
        state[start - 1] = 0;
        state[start - 4] = colourValue + ROOK;
    }

    repetitionStart = std::max(ply - halfMoves, 0);
    isWhiteTurn = !isWhiteTurn;

    gameHistory.pop_back();
}

std::set<int> Board::getMoveOptions(int startSquare) {
    std::set<int> moveOptions;
    for (Move move : moves) {
        if (move.getStart() == startSquare) {
            moveOptions.insert(move.getDestination());
        }
    }
    return moveOptions;
}

void Board::printBoard() {
    std::map<int, char> pieceNumToPieceLetter = {
        {0, 'x'}, {1, 'p'}, {2, 'n'}, {3, 'b'}, {4, 'r'}, {5, 'q'}, {6, 'k'},
    };

    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int num = state[rank * 8 + file];
            char c = pieceNumToPieceLetter.at(num % 8);

            if (num < 8) {
                c = std::toupper(c);
            }
            std::cout << c << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

void Board::printBitboard(uint64_t bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int pixel = bitboard >> (rank * 8 + file) & 1;
            std::cout << pixel << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

void Board::printMoves() {
    for (Move move : moves) {
        std::cout << move.getFlags() << ", " << move.getStart() << ", " << move.getDestination() << '\n';
    }
}
