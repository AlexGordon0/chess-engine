#include "board.h"
#include <bit>
#include <map>
#include <sstream>
#include <vector>

std::array<std::array<int, 8>, 64> Board::numSquaresToEdge = {0};
std::array<uint64_t, 64> Board::knightMoveMasks = {0};
std::array<uint64_t, 64> Board::kingMoveMasks = {0};

Board::Board(std::string startingPos) {
    static bool masksInitialised = false;
    if (!masksInitialised) {
        calculateSquareData();
        calculateKnightMasks();
        calculateKingMasks();
        masksInitialised = true;
    }
    convertFromFen(startingPos);
    determineCheckStatus();
    generateLegalMoves();
}

Board::Board(std::array<int, 64> _state, std::array<uint64_t, 15> _bitboards, bool _whiteTurn, int _castlingRights,
             int _enPassantSquare) {
    state = _state;
    bitboards = _bitboards;
    whiteTurn = _whiteTurn;
    castlingRights = _castlingRights;
    enPassantSquare = _enPassantSquare;
    determineCheckStatus();
    generateLegalMoves();
}

std::array<int, 64> Board::getState() { return state; }

std::set<Move> Board::getMoves() { return moves; }

int Board::getEnPassantSquare() { return enPassantSquare; }

bool Board::getIsWhiteTurn() { return whiteTurn; }

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
        whiteTurn = true;
    } else if (segments[1] == "b") {
        whiteTurn = false;
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
        enPassantSquare = (segments[3][1] - 1) * 8 + (segments[3][0] - '0' - 1);
    }
}

void Board::calculateSquareData() {
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
}

void Board::calculateKnightMasks() {
    int knightMoveOffsets[8] = {10, -6, 17, -15, -17, 15, 6, -10};
    int start = 0;
    int end = 4;
    for (int file = 0; file < 8; file++) {
        if (file == 1)
            end = 6;
        if (file == 2)
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
            knightMoveMasks[squareIndex] = bitmap;
        }
    }
}

void Board::calculateKingMasks() {
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
            kingMoveMasks[squareIndex] = bitmap;
        }
    }
}

void Board::determineCheckStatus() {
    uint64_t pawnAttacks = generatePawnAttackMaps();
    uint64_t knightAttacks = generateKnightAttackMaps();
    uint64_t slidingAttacks = generateSlidingAttackMaps();
    uint64_t kingAttacks = generateKingAttackMaps();

    opponentAttackMap = pawnAttacks | knightAttacks | slidingAttacks | kingAttacks;

    int colourValue = whiteTurn ? 0 : 8;
    uint64_t kingLocations = bitboards[colourValue + 6];
    int numChecks = 0;
    if (pawnAttacks & kingLocations) {
        numChecks++;
    }
    if (knightAttacks & kingLocations) {
        numChecks++;
    }
    if (slidingAttacks & kingLocations) {
        numChecks++;
    }
    if (kingAttacks & kingLocations) {
        numChecks++;
    }

    if (numChecks > 1) {
        checkStatus = 2;
    } else if (numChecks == 1) {
        checkStatus = 1;
    } else {
        checkStatus = 0;
    }
}

uint64_t Board::generatePawnAttackMaps() {
    if (whiteTurn) {
        uint64_t westCaptures = bitboards[9] >> 9 & 0x7f7f7f7f7f7f7f7f;
        uint64_t eastCaptures = bitboards[9] >> 7 & 0xfefefefefefefefe;
        return westCaptures | eastCaptures;
    } else {
        uint64_t westCaptures = bitboards[1] << 7 & 0x7f7f7f7f7f7f7f7f;
        uint64_t eastCaptures = bitboards[1] << 9 & 0xfefefefefefefefe;
        return westCaptures | eastCaptures;
    }
}

uint64_t Board::generateKnightAttackMaps() {
    int colourValue = whiteTurn ? 8 : 0;
    uint64_t bitboardCopy = bitboards[colourValue + 2];
    uint64_t possibleMoves = 0;
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        possibleMoves |= knightMoveMasks[pieceSquare];
        bitboardCopy -= 1ULL << pieceSquare;
    }
    return possibleMoves;
}

uint64_t Board::generateSlidingAttackMaps() {
    int directionOffsets[8] = {8, -8, -1, 1, 7, -7, -9, 9};
    int colourValue = whiteTurn ? 8 : 0;
    int start = 4;
    int end = 8;
    uint64_t possibleMoves = 0;
    for (int i = 3; i < 6; i++) {
        if (i == 4) {
            start = 0;
            end = 4;
        }
        if (i == 5) {
            end = 8;
        }
        uint64_t bitboardCopy = bitboards[colourValue + i];
        while (bitboardCopy) {
            int pieceSquare = std::countr_zero(bitboardCopy);
            for (int j = start; j < end; j++) {
                for (int k = 1; k <= numSquaresToEdge[pieceSquare][j]; k++) {
                    int destinationSquare = pieceSquare + directionOffsets[j] * k;
                    if (bitboards[8 - colourValue] >> destinationSquare & 1 ||
                        bitboards[colourValue] >> destinationSquare & 1) {
                        possibleMoves |= 1ULL << destinationSquare;
                        break;
                    }
                    possibleMoves |= 1ULL << destinationSquare;
                }
            }
            bitboardCopy -= 1ULL << pieceSquare;
        }
    }
    return possibleMoves;
}

uint64_t Board::generateKingAttackMaps() {
    int colourValue = whiteTurn ? 8 : 0;

    uint64_t bitboardCopy = bitboards[colourValue + 6];
    uint64_t possibleMoves = 0;
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        possibleMoves |= kingMoveMasks[pieceSquare];
        bitboardCopy -= 1ULL << pieceSquare;
    }
    return possibleMoves;
}

void Board::generateLegalMoves() {
    moves.clear();
    // If in double check only the king can move so no need to generate other moves
    if (checkStatus != 2) {
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
                moves.insert(Move(startSquare, destinationSquare, flags |= i));
            }
        }
        moves.insert(Move(startSquare, destinationSquare, flags));
        bitmap -= 1ULL << destinationSquare;
    }
}

void Board::generatePawnMoves() {
    if (whiteTurn) {
        uint64_t westCaptures = bitboards[1] << 7 & (bitboards[8] | (1ULL << enPassantSquare)) & 0x7f7f7f7f7f7f7f7f;
        addMovesFromBitmap(westCaptures, -7);

        uint64_t eastCaptures = bitboards[1] << 9 & (bitboards[8] | (1ULL << enPassantSquare)) & 0xfefefefefefefefe;
        addMovesFromBitmap(eastCaptures, -9);

        uint64_t forwardMoves = bitboards[1] << 8 & ~(bitboards[0] | bitboards[8]);
        addMovesFromBitmap(forwardMoves, -8);

        uint64_t unmovedPawns = bitboards[1] & 0xff00;
        uint64_t freeSquares = ~(bitboards[0] | bitboards[8]);
        uint64_t doublePawnMoves = (unmovedPawns << 8 & freeSquares) << 8 & (unmovedPawns << 16 & freeSquares);
        addMovesFromBitmap(doublePawnMoves, -16);
    } else {
        uint64_t westCaptures = bitboards[9] >> 9 & (bitboards[0] | (1ULL << enPassantSquare)) & 0x7f7f7f7f7f7f7f7f;
        addMovesFromBitmap(westCaptures, 9);

        uint64_t eastCaptures = bitboards[9] >> 7 & (bitboards[0] | (1ULL << enPassantSquare)) & 0xfefefefefefefefe;
        addMovesFromBitmap(eastCaptures, 7);

        uint64_t forwardMoves = bitboards[9] >> 8 & ~(bitboards[0] | bitboards[8]);
        addMovesFromBitmap(forwardMoves, 8);

        uint64_t unmovedPawns = bitboards[9] & 0xff000000000000;
        uint64_t freeSquares = ~(bitboards[0] | bitboards[8]);
        uint64_t doublePawnMoves = (unmovedPawns >> 8 & freeSquares) >> 8 & (unmovedPawns >> 16 & freeSquares);
        addMovesFromBitmap(doublePawnMoves, 16);
    }
}

void Board::generateKnightMoves() {
    int colourValue = whiteTurn ? 0 : 8;
    uint64_t bitboardCopy = bitboards[colourValue + 2];
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        uint64_t possibleMoves = knightMoveMasks[pieceSquare] & ~bitboards[colourValue];
        while (possibleMoves) {
            int destinationSquare = std::countr_zero(possibleMoves);
            int flags = 0;
            if (state[destinationSquare]) {
                flags += 1 << 2;
            }
            moves.insert(Move(pieceSquare, destinationSquare, flags));
            possibleMoves -= 1ULL << destinationSquare;
        }
        bitboardCopy -= 1ULL << pieceSquare;
    }
}

void Board::generateSlidingMoves() {
    int directionOffsets[8] = {8, -8, -1, 1, 7, -7, -9, 9};
    int colourValue = whiteTurn ? 0 : 8;
    int start = 4;
    int end = 8;
    for (int i = 3; i < 6; i++) {
        if (i == 4) {
            start = 0;
            end = 4;
        }
        if (i == 5) {
            end = 8;
        }
        uint64_t bitboardCopy = bitboards[colourValue + i];
        while (bitboardCopy) {
            int pieceSquare = std::countr_zero(bitboardCopy);
            for (int j = start; j < end; j++) {
                for (int k = 1; k <= numSquaresToEdge[pieceSquare][j]; k++) {
                    int destinationSquare = pieceSquare + directionOffsets[j] * k;
                    if (bitboards[colourValue] >> destinationSquare & 1) {
                        break;
                    }
                    if (bitboards[8 - colourValue] >> destinationSquare & 1) {
                        moves.insert(Move(pieceSquare, destinationSquare, 4));
                        break;
                    }
                    moves.insert(Move(pieceSquare, destinationSquare, 0));
                }
            }
            bitboardCopy -= 1ULL << pieceSquare;
        }
    }
}

void Board::generateKingMoves() {
    int colourValue = whiteTurn ? 0 : 8;
    int colourCastlingRights = whiteTurn ? castlingRights >> 2 : castlingRights & 3;
    int kingStartingSquare = whiteTurn ? 4 : 60;

    uint64_t bitboardCopy = bitboards[colourValue + 6];
    while (bitboardCopy) {
        int pieceSquare = std::countr_zero(bitboardCopy);
        if (pieceSquare == kingStartingSquare && !checkStatus) {
            if (colourCastlingRights & 1) {
                // Check queenside castling
                uint64_t queensideCastlingMask = 1ULL << (pieceSquare - 1) | 1ULL << (pieceSquare - 2);
                if (!(state[pieceSquare - 1] | state[pieceSquare - 2] | state[pieceSquare - 3]) &&
                    !(queensideCastlingMask & opponentAttackMap)) {
                    moves.insert(Move(pieceSquare, pieceSquare - 2, 3));
                }
            }
            if (colourCastlingRights & 2) {
                // Check kingside castling
                uint64_t kingsideCastlingMask = 1ULL << (pieceSquare + 1) | 1ULL << (pieceSquare + 2);
                if (!(state[pieceSquare + 1] | state[pieceSquare + 2]) && !(kingsideCastlingMask & opponentAttackMap)) {
                    moves.insert(Move(pieceSquare, pieceSquare + 2, 2));
                }
            }
        }
        uint64_t possibleMoves = kingMoveMasks[pieceSquare] & ~bitboards[colourValue] & ~opponentAttackMap;
        while (possibleMoves) {
            int destinationSquare = std::countr_zero(possibleMoves);
            int flags = 0;
            if (state[destinationSquare]) {
                flags += 1 << 2;
            }
            moves.insert(Move(pieceSquare, destinationSquare, flags));
            possibleMoves -= 1ULL << destinationSquare;
        }
        bitboardCopy -= 1ULL << pieceSquare;
    }
}

Board Board::makeMove(Move passedInMove) {
    Move requestedMove = passedInMove;
    if (passedInMove.getFlags() == 0) {
        for (Move move : moves) {
            if (passedInMove.getStart() == move.getStart() && passedInMove.getDestination() == move.getDestination()) {
                requestedMove = move;
                break;
            }
        }
    }
    std::array<int, 64> newState = state;
    std::array<uint64_t, 15> newBitboards = bitboards;
    int newCastlingRights = castlingRights;
    int newEnPassantSquare = -1;

    int colourValue = whiteTurn ? 0 : 8;

    if (requestedMove.isCapture()) {
        // Check if en passant or normal capture
        if (requestedMove.getFlags() == 5) {
            int offset = whiteTurn ? -8 : 8;
            int pieceTaken = state[requestedMove.getDestination() + offset];
            newBitboards[pieceTaken] -= 1ULL << (requestedMove.getDestination() + offset);
            newBitboards[8 - colourValue] -= 1ULL << (requestedMove.getDestination() + offset);
            newState[requestedMove.getDestination() + offset] = 0;
        } else {
            int pieceTaken = state[requestedMove.getDestination()];
            newBitboards[pieceTaken] -= 1ULL << requestedMove.getDestination();
            newBitboards[8 - colourValue] -= 1ULL << requestedMove.getDestination();
        }
    }

    // Double pawn push
    if (requestedMove.getFlags() == 1) {
        int offset = whiteTurn ? -8 : 8;
        newEnPassantSquare = requestedMove.getDestination() + offset;
    }

    // Kingside castle
    if (requestedMove.getFlags() == 2) {
        newCastlingRights = whiteTurn ? newCastlingRights &= 3 : newCastlingRights &= 12;
        newBitboards[colourValue + 4] -= 1ULL << (requestedMove.getStart() + 3);
        newBitboards[colourValue + 4] += 1ULL << (requestedMove.getStart() + 1);
        newBitboards[colourValue] -= 1ULL << (requestedMove.getStart() + 3);
        newBitboards[colourValue] += 1ULL << (requestedMove.getStart() + 1);
        newState[requestedMove.getStart() + 3] = 0;
        newState[requestedMove.getStart() + 1] = colourValue + 4;
    }

    // Queenside castle
    if (requestedMove.getFlags() == 3) {
        newCastlingRights = whiteTurn ? newCastlingRights &= 3 : newCastlingRights &= 12;
        newBitboards[colourValue + 4] -= 1ULL << (requestedMove.getStart() - 4);
        newBitboards[colourValue + 4] += 1ULL << (requestedMove.getStart() - 1);
        newBitboards[colourValue] -= 1ULL << (requestedMove.getStart() - 4);
        newBitboards[colourValue] += 1ULL << (requestedMove.getStart() - 1);
        newState[requestedMove.getStart() - 4] = 0;
        newState[requestedMove.getStart() - 1] = colourValue + 4;
    }

    int pieceMoved = state[requestedMove.getStart()];
    newBitboards[pieceMoved] -= 1ULL << requestedMove.getStart();
    newBitboards[colourValue] -= 1ULL << requestedMove.getStart();
    newBitboards[colourValue] += 1ULL << requestedMove.getDestination();
    newState[requestedMove.getStart()] = 0;

    if (requestedMove.isPromotion()) {
        int newPiece = (requestedMove.getFlags() & 3) + colourValue + 2;
        newBitboards[newPiece] += 1ULL << requestedMove.getDestination();
        newState[requestedMove.getDestination()] = newPiece;
    } else {
        newBitboards[pieceMoved] += 1ULL << requestedMove.getDestination();
        newState[requestedMove.getDestination()] = pieceMoved;
    }

    // Update castling rights
    if (whiteTurn) {
        if (requestedMove.getStart() == 0) {
            newCastlingRights &= 11;
        } else if (requestedMove.getStart() == 7) {
            newCastlingRights &= 7;
        } else if (requestedMove.getDestination() == 56) {
            newCastlingRights &= 14;
        } else if (requestedMove.getDestination() == 63) {
            newCastlingRights &= 13;
        }
    } else {
        if (requestedMove.getStart() == 56) {
            newCastlingRights &= 14;
        } else if (requestedMove.getStart() == 63) {
            newCastlingRights &= 13;
        } else if (requestedMove.getDestination() == 0) {
            newCastlingRights &= 11;
        } else if (requestedMove.getDestination() == 7) {
            newCastlingRights &= 7;
        }
    }

    return Board(newState, newBitboards, !whiteTurn, newCastlingRights, newEnPassantSquare);
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

void Board::printBitboard(int index) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int pixel = bitboards[index] >> (rank * 8 + file) & 1;
            std::cout << pixel << ' ';
        }
        std::cout << '\n';
    }
}

void Board::printMoves() {
    for (Move move : moves) {
        std::cout << move.getStart() << ", " << move.getDestination() << '\n';
    }
}
