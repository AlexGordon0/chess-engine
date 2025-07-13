#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <cstdint>
#include <iostream>
#include <set>
#include <vector>

class Move {
  public:
    Move(unsigned int start, unsigned int destination, unsigned int flags);

    unsigned int getStart();
    unsigned int getDestination();
    unsigned int isCapture();
    unsigned int isPromotion();
    unsigned int getFlags();

    auto operator<=>(const Move &other) const = default;

  private:
    unsigned int move;
};

struct BoardData {
    short castlingRights;
    short enPassantSquare;
    short halfMoves;
    short capturedPiece;
};

class Board {
  public:
    Board(std::string startingPos);

    int calculateFlag(int startSquare, int destinationSquare);
    void makeMove(Move move);
    void unmakeMove(Move move);

    std::set<int> getMoveOptions(int startSquare);
    std::array<short, 64> getState();
    std::vector<Move> getMoves();
    uint64_t getBitboard(int index);
    short getEnPassantSquare();
    bool getIsWhiteTurn();
    uint64_t getOpponentAttackMap();
    short getGameStatus();

    void printBoard();
    void printBitboard(uint64_t bitboard);
    void printMoves();

  private:
    std::array<short, 64> state = {0};

    /* Bitboards indexes are:
     *  0 - White pieces
     *  1 - White pawns
     *  2 - White knights
     *  3 - White bishops
     *  4 - White rooks
     *  5 - White queens
     *  6 - White king
     *  7 - empty
     *  8 - Black pieces
     *  9 - Black pawns
     * 10 - Black knights
     * 11 - Black bishops
     * 12 - Black rooks
     * 13 - Black queens
     * 14 - Black king
     * 15 - empty */
    std::array<uint64_t, 15> bitboards = {0};

    bool isWhiteTurn;
    short castlingRights;
    short enPassantSquare;
    short halfMoves;
    short fullMoves;
    short ply;
    short repetitionStart;
    std::vector<uint64_t> zobristHashes;
    std::vector<Move> moves;
    /* 0 = game not ended
     * 1 = checkmate
     * 2 = draw */
    short gameStatus;

    short numChecks;
    uint64_t currentPositionHash;
    uint64_t opponentAttackMap;
    uint64_t checkEvasionMask;
    uint64_t pinnedPieces;

    void convertFromFen(std::string fenString);
    void setup();
    uint64_t zobrist();
    std::vector<BoardData> gameHistory;

    void determineCheckStatus();
    uint64_t generatePawnAttackMaps();
    uint64_t generateKnightAttackMaps();
    uint64_t generateSlidingAttackMaps();
    void calculateBlockMask(int pieceSquare, int kingSquare, bool isBishop);
    uint64_t generateKingAttackMaps();

    void calculatePinnedPieces();
    void addPinnedPieceMoves(int pieceSquare, uint64_t pinnedMovesMask, bool isDiagonalPin);
    void addPinnedPawnMoves(int pieceSquare, uint64_t pinnedMovesMask);

    void generateLegalMoves();
    void generatePawnMoves();
    void generateKnightMoves();
    void addSlidingMoves(uint64_t possibleMoves, int startSquare);
    void generateSlidingMoves();
    void generateKingMoves();
    void addMovesFromBitmap(uint64_t bitmap, int startSquareOffset);
    bool checkEnPassantPin(int startSquare);
};

#endif
