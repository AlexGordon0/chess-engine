#include "board.h"
#include "magics.h"
#include "masks.h"
#include <bit>
#include <cstdint>
#include <map>
#include <sstream>

Board::Board(std::string startingPos) {
    convertFromFen(startingPos);
    moves.reserve(40);
    determineCheckStatus();
    calculatePinnedPieces();
    generateLegalMoves();
}

Board::Board(std::array<int, 64> _state, std::array<uint64_t, 15> _bitboards, bool _whiteTurn, int _castlingRights,
             int _enPassantSquare) {
    state = _state;
    bitboards = _bitboards;
    isWhiteTurn = _whiteTurn;
    castlingRights = _castlingRights;
    enPassantSquare = _enPassantSquare;
    moves.reserve(40);
    determineCheckStatus();
    calculatePinnedPieces();
    generateLegalMoves();
}

std::array<int, 64> Board::getState() { return state; }

std::vector<Move> Board::getMoves() { return moves; }

int Board::getEnPassantSquare() { return enPassantSquare; }

bool Board::getIsWhiteTurn() { return isWhiteTurn; }

uint64_t Board::getOpponentAttackMap() { return opponentAttackMap; }

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
            int colourValue = islower(c) ? 8 : 0;
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
}

void Board::determineCheckStatus() {
    checkEvasionMask = 0xffffffffffffffff;
    uint64_t pawnAttacks = generatePawnAttackMaps();
    uint64_t knightAttacks = generateKnightAttackMaps();
    uint64_t slidingAttacks = generateSlidingAttackMaps();
    uint64_t kingAttacks = generateKingAttackMaps();

    opponentAttackMap = pawnAttacks | knightAttacks | slidingAttacks | kingAttacks;

    int colourValue = isWhiteTurn ? 0 : 8;
    uint64_t kingLocation = bitboards[colourValue + 6];
    numChecks = 0;
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
        uint64_t oppositionKingBitboard = bitboards[6];

        uint64_t westCaptures = bitboards[9] >> 9 & 0x7f7f7f7f7f7f7f7f;
        if (westCaptures & oppositionKingBitboard) {
            attackingPawnMap = (westCaptures & oppositionKingBitboard) << 9;
        }

        uint64_t eastCaptures = bitboards[9] >> 7 & 0xfefefefefefefefe;
        if (eastCaptures & oppositionKingBitboard) {
            attackingPawnMap = (eastCaptures & oppositionKingBitboard) << 7;
        }

        if (attackingPawnMap << 8 & 1ULL << enPassantSquare) {
            attackingPawnMap |= 1ULL << enPassantSquare;
        }

        attackMap = westCaptures | eastCaptures;
    } else {
        uint64_t oppositionKingBitboard = bitboards[14];

        uint64_t westCaptures = bitboards[1] << 7 & 0x7f7f7f7f7f7f7f7f;
        if (westCaptures & oppositionKingBitboard) {
            attackingPawnMap = (westCaptures & oppositionKingBitboard) >> 7;
        }

        uint64_t eastCaptures = bitboards[1] << 9 & 0xfefefefefefefefe;
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
    int colourValue = isWhiteTurn ? 8 : 0;
    uint64_t bitboardCopy = bitboards[colourValue + 2];
    uint64_t oppositionKingBitboard = bitboards[14 - colourValue];

    uint64_t attackMap = 0;
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        if (masks::knightMoveMasks[pieceSquare] & oppositionKingBitboard) {
            checkEvasionMask = 1ULL << pieceSquare;
        }
        attackMap |= masks::knightMoveMasks[pieceSquare];
        bitboardCopy -= 1ULL << pieceSquare;
    }
    return attackMap;
}

uint64_t Board::generateSlidingAttackMaps() {
    int colourValue = isWhiteTurn ? 8 : 0;
    uint64_t attackMap = 0;
    uint64_t oppositionKing = bitboards[14 - colourValue];

    for (int i = 3; i < 6; i++) {
        uint64_t bitboardCopy = bitboards[colourValue + i];
        while (bitboardCopy) {
            int pieceSquare = std::countr_zero(bitboardCopy);

            if (i == 3 | i == 5) {
                uint64_t occupancyMask = magics::bishopOccupancyMasks[pieceSquare];
                uint64_t magic = magics::bishopMagics[pieceSquare];
                int numBits = magics::bishopNumBits[pieceSquare];
                int index =
                    ((((bitboards[0] | bitboards[8]) & ~oppositionKing) & occupancyMask) * magic) >> (64 - numBits);
                uint64_t possibleMoves = magics::bishopLookupTable[pieceSquare][index];
                attackMap |= possibleMoves;
                if (possibleMoves & oppositionKing) {
                    numChecks++;
                    calculateBlockMask(pieceSquare, std::countr_zero(oppositionKing), true);
                }
            }
            if (i == 4 | i == 5) {
                uint64_t occupancyMask = magics::rookOccupancyMasks[pieceSquare];
                uint64_t magic = magics::rookMagics[pieceSquare];
                int numBits = magics::rookNumBits[pieceSquare];
                int index =
                    ((((bitboards[0] | bitboards[8]) & ~oppositionKing) & occupancyMask) * magic) >> (64 - numBits);
                uint64_t possibleMoves = magics::rookLookupTable[pieceSquare][index];
                attackMap |= possibleMoves;
                if (possibleMoves & oppositionKing) {
                    numChecks++;
                    calculateBlockMask(pieceSquare, std::countr_zero(oppositionKing), false);
                }
            }

            bitboardCopy -= 1ULL << pieceSquare;
        }
    }

    return attackMap;
}

void Board::calculateBlockMask(int pieceSquare, int kingSquare, bool isBishop) {
    uint64_t magic = isBishop ? magics::bishopMagics[pieceSquare] : magics::rookMagics[pieceSquare];
    uint64_t occupancyMask =
        isBishop ? magics::bishopOccupancyMasks[pieceSquare] : magics::rookOccupancyMasks[pieceSquare];
    int numBits = isBishop ? magics::bishopNumBits[pieceSquare] : magics::rookNumBits[pieceSquare];

    int pieceIndex = (((bitboards[0] | bitboards[8]) & occupancyMask) * magic) >> (64 - numBits);
    uint64_t pieceAttacks = isBishop ? magics::bishopLookupTable[pieceSquare][pieceIndex]
                                     : magics::rookLookupTable[pieceSquare][pieceIndex];

    magic = isBishop ? magics::bishopMagics[kingSquare] : magics::rookMagics[kingSquare];
    occupancyMask = isBishop ? magics::bishopOccupancyMasks[kingSquare] : magics::rookOccupancyMasks[kingSquare];
    numBits = isBishop ? magics::bishopNumBits[kingSquare] : magics::rookNumBits[kingSquare];

    int kingIndex = (((bitboards[0] | bitboards[8]) & occupancyMask) * magic) >> (64 - numBits);
    uint64_t kingAttacks =
        isBishop ? magics::bishopLookupTable[kingSquare][kingIndex] : magics::rookLookupTable[kingSquare][kingIndex];

    checkEvasionMask = pieceAttacks & kingAttacks | 1ULL << pieceSquare;
}

uint64_t Board::generateKingAttackMaps() {
    int colourValue = isWhiteTurn ? 8 : 0;

    uint64_t bitboardCopy = bitboards[colourValue + 6];
    uint64_t possibleMoves = 0;
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        possibleMoves |= masks::kingMoveMasks[pieceSquare];
        bitboardCopy -= 1ULL << pieceSquare;
    }
    return possibleMoves;
}

void Board::calculatePinnedPieces() {
    int colourValue = isWhiteTurn ? 0 : 8;
    uint64_t pieceMap = bitboards[0] | bitboards[8];
    pinnedPieces = 0;

    int kingLocation = std::countr_zero(bitboards[colourValue + 6]);
    int kingFile = kingLocation & 7;
    int kingRank = kingLocation / 8;

    uint64_t kingBishopMask = masks::bishopMoveMasks[kingLocation];
    uint64_t kingRookMask = masks::rookMoveMasks[kingLocation];
    uint64_t pinningPieces = (bitboards[11 - colourValue] | bitboards[13 - colourValue]) & kingBishopMask |
                             (bitboards[12 - colourValue] | bitboards[13 - colourValue]) & kingRookMask;
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
            if ((state[pinnedPieceSquare] & 0b111) == 1) {
                addPinnedPawnMoves(pinnedPieceSquare, possibleMovesRay);
            } else {
                bool isDiagonalPin = !(abs(kingOffset) == 1 || abs(kingOffset) == 8);
                addPinnedPieceMoves(pinnedPieceSquare, possibleMovesRay, isDiagonalPin);
            }
        }
        pinningPieces -= 1ULL << pieceSquare;
    }
}

void Board::addPinnedPieceMoves(int pieceSquare, uint64_t pinnedMovesMask, bool isDiagonalPin) {
    bool diagonalAndBishop = isDiagonalPin && (state[pieceSquare] & 0b111) == 3;
    bool notDiagonalAndRook = !isDiagonalPin && (state[pieceSquare] & 0b111) == 4;
    bool isQueen = (state[pieceSquare] & 0b111) == 5;
    if (diagonalAndBishop || notDiagonalAndRook || isQueen) {
        while (pinnedMovesMask) {
            int destination = std::countr_zero(pinnedMovesMask);
            if (state[destination]) {
                moves.push_back(Move(pieceSquare, destination, 4));
            } else {
                moves.push_back(Move(pieceSquare, destination, 0));
            }
            pinnedMovesMask -= 1ULL << destination;
        }
    }
}

void Board::addPinnedPawnMoves(int pieceSquare, uint64_t pinnedMovesMask) {
    uint64_t pinnedPawnBitboard = 1ULL << pieceSquare;
    if (isWhiteTurn) {
        uint64_t westCaptures = pinnedPawnBitboard << 7 & (bitboards[8] | (1ULL << enPassantSquare)) &
                                0x7f7f7f7f7f7f7f7f & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(westCaptures, -7);

        uint64_t eastCaptures = pinnedPawnBitboard << 9 & (bitboards[8] | (1ULL << enPassantSquare)) &
                                0xfefefefefefefefe & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(eastCaptures, -9);

        uint64_t forwardMoves =
            pinnedPawnBitboard << 8 & ~(bitboards[0] | bitboards[8]) & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(forwardMoves, -8);

        uint64_t unmovedPawns = pinnedPawnBitboard & 0xff00;
        uint64_t freeSquares = ~(bitboards[0] | bitboards[8]);
        uint64_t doublePawnMoves = (unmovedPawns << 8 & freeSquares) << 8 & (unmovedPawns << 16 & freeSquares) &
                                   checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(doublePawnMoves, -16);
    } else {
        uint64_t westCaptures = pinnedPawnBitboard >> 9 & (bitboards[0] | (1ULL << enPassantSquare)) &
                                0x7f7f7f7f7f7f7f7f & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(westCaptures, 9);

        uint64_t eastCaptures = pinnedPawnBitboard >> 7 & (bitboards[0] | (1ULL << enPassantSquare)) &
                                0xfefefefefefefefe & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(eastCaptures, 7);

        uint64_t forwardMoves =
            pinnedPawnBitboard >> 8 & ~(bitboards[0] | bitboards[8]) & checkEvasionMask & pinnedMovesMask;
        addMovesFromBitmap(forwardMoves, 8);

        uint64_t unmovedPawns = pinnedPawnBitboard & 0xff000000000000;
        uint64_t freeSquares = ~(bitboards[0] | bitboards[8]);
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
        bitmap -= 1ULL << destinationSquare;
    }
}

// Checks the edge case where en passant would reveal an attack on the king.
bool Board::checkEnPassantPin(int startSquare) {
    int rank = startSquare / 8;
    uint64_t rankMask = 0xffULL << (rank * 8);
    int colourValue = isWhiteTurn ? 0 : 8;

    if (!(rankMask & bitboards[colourValue + 6])) {
        return false;
    }
    if (!(rankMask & (bitboards[12 - colourValue] | bitboards[13 - colourValue]))) {
        return false;
    }

    int offset = isWhiteTurn ? -8 : 8;
    int capturedPawn = enPassantSquare + offset;

    uint64_t pawnsRemoved = (bitboards[0] | bitboards[8]) & ~(1ULL << startSquare) & ~(1ULL << capturedPawn);
    int kingSquare = std::countr_zero(bitboards[colourValue + 6]);
    int offsets[2] = {-1, 1};
    for (int i = 0; i < 2; i++) {
        for (int j = 1; i <= masks::numSquaresToEdge[kingSquare][i + 2]; j++) {
            int square = kingSquare + offsets[i] * j;
            if (pawnsRemoved & 1ULL << square) {
                if (state[square] == (12 - colourValue) || state[square] == (13 - colourValue)) {
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
        uint64_t pawnsWithoutPins = bitboards[1] & ~pinnedPieces;
        uint64_t westCaptures =
            pawnsWithoutPins << 7 & (bitboards[8] | (1ULL << enPassantSquare)) & 0x7f7f7f7f7f7f7f7f & checkEvasionMask;
        addMovesFromBitmap(westCaptures, -7);

        uint64_t eastCaptures =
            pawnsWithoutPins << 9 & (bitboards[8] | (1ULL << enPassantSquare)) & 0xfefefefefefefefe & checkEvasionMask;
        addMovesFromBitmap(eastCaptures, -9);

        uint64_t forwardMoves = pawnsWithoutPins << 8 & ~(bitboards[0] | bitboards[8]) & checkEvasionMask;
        addMovesFromBitmap(forwardMoves, -8);

        uint64_t unmovedPawns = pawnsWithoutPins & 0xff00;
        uint64_t freeSquares = ~(bitboards[0] | bitboards[8]);
        uint64_t doublePawnMoves =
            (unmovedPawns << 8 & freeSquares) << 8 & (unmovedPawns << 16 & freeSquares) & checkEvasionMask;
        addMovesFromBitmap(doublePawnMoves, -16);
    } else {
        uint64_t pawnsWithoutPins = bitboards[9] & ~pinnedPieces;
        uint64_t westCaptures =
            pawnsWithoutPins >> 9 & (bitboards[0] | (1ULL << enPassantSquare)) & 0x7f7f7f7f7f7f7f7f & checkEvasionMask;
        addMovesFromBitmap(westCaptures, 9);

        uint64_t eastCaptures =
            pawnsWithoutPins >> 7 & (bitboards[0] | (1ULL << enPassantSquare)) & 0xfefefefefefefefe & checkEvasionMask;
        addMovesFromBitmap(eastCaptures, 7);

        uint64_t forwardMoves = pawnsWithoutPins >> 8 & ~(bitboards[0] | bitboards[8]) & checkEvasionMask;
        addMovesFromBitmap(forwardMoves, 8);

        uint64_t unmovedPawns = pawnsWithoutPins & 0xff000000000000;
        uint64_t freeSquares = ~(bitboards[0] | bitboards[8]);
        uint64_t doublePawnMoves =
            (unmovedPawns >> 8 & freeSquares) >> 8 & (unmovedPawns >> 16 & freeSquares) & checkEvasionMask;
        addMovesFromBitmap(doublePawnMoves, 16);
    }
}

void Board::generateKnightMoves() {
    int colourValue = isWhiteTurn ? 0 : 8;
    uint64_t bitboardCopy = bitboards[colourValue + 2] & ~pinnedPieces;
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
            possibleMoves -= 1ULL << destinationSquare;
        }
        bitboardCopy -= 1ULL << pieceSquare;
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
        possibleMoves -= 1ULL << destinationSquare;
    }
}

void Board::generateSlidingMoves() {
    int colourValue = isWhiteTurn ? 0 : 8;
    for (int i = 3; i < 6; i++) {
        uint64_t bitboardCopy = bitboards[colourValue + i] & ~pinnedPieces;
        while (bitboardCopy) {
            int pieceSquare = std::countr_zero(bitboardCopy);
            if (i == 3 | i == 5) {
                uint64_t occupancyMask = magics::bishopOccupancyMasks[pieceSquare];
                uint64_t magic = magics::bishopMagics[pieceSquare];
                int numBits = magics::bishopNumBits[pieceSquare];
                int index = (((bitboards[0] | bitboards[8]) & occupancyMask) * magic) >> (64 - numBits);
                uint64_t possibleMoves =
                    magics::bishopLookupTable[pieceSquare][index] & checkEvasionMask & ~bitboards[colourValue];
                addSlidingMoves(possibleMoves, pieceSquare);
            }
            if (i == 4 | i == 5) {
                uint64_t occupancyMask = magics::rookOccupancyMasks[pieceSquare];
                uint64_t magic = magics::rookMagics[pieceSquare];
                int numBits = magics::rookNumBits[pieceSquare];
                int index = (((bitboards[0] | bitboards[8]) & occupancyMask) * magic) >> (64 - numBits);
                uint64_t possibleMoves =
                    magics::rookLookupTable[pieceSquare][index] & checkEvasionMask & ~bitboards[colourValue];
                addSlidingMoves(possibleMoves, pieceSquare);
            }
            bitboardCopy -= 1ULL << pieceSquare;
        }
    }
}

void Board::generateKingMoves() {
    int colourValue = isWhiteTurn ? 0 : 8;
    int colourCastlingRights = isWhiteTurn ? castlingRights >> 2 : castlingRights & 3;
    int kingStartingSquare = isWhiteTurn ? 4 : 60;

    uint64_t bitboardCopy = bitboards[colourValue + 6];
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
            possibleMoves -= 1ULL << destinationSquare;
        }
        bitboardCopy -= 1ULL << pieceSquare;
    }
}

Board Board::makeMove(Move requestedMove) {
    int start = requestedMove.getStart();
    int destination = requestedMove.getDestination();
    int flags = requestedMove.getFlags();

    std::array<int, 64> newState = state;
    std::array<uint64_t, 15> newBitboards = bitboards;
    int newCastlingRights = castlingRights;
    int newEnPassantSquare = -1;

    int colourValue = isWhiteTurn ? 0 : 8;

    if (requestedMove.isCapture()) {
        // Check if en passant or normal capture
        if (flags == 5) {
            int offset = isWhiteTurn ? -8 : 8;
            int pieceTaken = state[destination + offset];
            newBitboards[pieceTaken] -= 1ULL << (destination + offset);
            newBitboards[8 - colourValue] -= 1ULL << (destination + offset);
            newState[destination + offset] = 0;
        } else {
            int pieceTaken = state[destination];
            newBitboards[pieceTaken] -= 1ULL << destination;
            newBitboards[8 - colourValue] -= 1ULL << destination;
        }
    }

    // Double pawn push
    if (flags == 1) {
        int offset = isWhiteTurn ? -8 : 8;
        newEnPassantSquare = destination + offset;
    }

    // Kingside castle
    if (flags == 2) {
        newCastlingRights = isWhiteTurn ? newCastlingRights &= 3 : newCastlingRights &= 12;
        newBitboards[colourValue + 4] -= 1ULL << (start + 3);
        newBitboards[colourValue + 4] += 1ULL << (start + 1);
        newBitboards[colourValue] -= 1ULL << (start + 3);
        newBitboards[colourValue] += 1ULL << (start + 1);
        newState[start + 3] = 0;
        newState[start + 1] = colourValue + 4;
    }

    // Queenside castle
    if (flags == 3) {
        newCastlingRights = isWhiteTurn ? newCastlingRights &= 3 : newCastlingRights &= 12;
        newBitboards[colourValue + 4] -= 1ULL << (start - 4);
        newBitboards[colourValue + 4] += 1ULL << (start - 1);
        newBitboards[colourValue] -= 1ULL << (start - 4);
        newBitboards[colourValue] += 1ULL << (start - 1);
        newState[start - 4] = 0;
        newState[start - 1] = colourValue + 4;
    }

    int pieceMoved = state[start];
    newBitboards[pieceMoved] -= 1ULL << start;
    newBitboards[colourValue] -= 1ULL << start;
    newBitboards[colourValue] += 1ULL << destination;
    newState[start] = 0;

    if (requestedMove.isPromotion()) {
        int newPiece = (flags & 3) + colourValue + 2;
        newBitboards[newPiece] += 1ULL << destination;
        newState[destination] = newPiece;
    } else {
        newBitboards[pieceMoved] += 1ULL << destination;
        newState[destination] = pieceMoved;
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

    return Board(newState, newBitboards, !isWhiteTurn, newCastlingRights, newEnPassantSquare);
}

int Board::calculateFlag(int startSquare, int destinationSquare) {
    for (Move move : moves) {
        if (startSquare == move.getStart() && destinationSquare == move.getDestination()) {
            return move.getFlags();
        }
    }
    return 0;
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
        std::cout << move.getStart() << ", " << move.getDestination() << '\n';
    }
}
