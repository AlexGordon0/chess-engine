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

class Board {
  public:
    Board(std::string startingPos);
    Board(std::array<int, 64> state, std::array<uint64_t, 15> bitboards, bool whiteTurn, int castlingRights,
          int enPassantSquare);

    Board makeMove(Move move);
    int calculateFlag(int startSquare, int destinationSquare);

    std::set<int> getMoveOptions(int startSquare);
    std::array<int, 64> getState();
    std::vector<Move> getMoves();
    int getEnPassantSquare();
    bool getIsWhiteTurn();
    uint64_t getOpponentAttackMap();

    void printBoard();
    void printBitboard(uint64_t bitboard);
    void printMoves();

  private:
    std::array<int, 64> state = {0};

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
    int castlingRights;
    int enPassantSquare;
    std::vector<Move> moves;

    /* 0 = no check
     * 1 = check
     * 2 = double check */
    int numChecks;
    uint64_t opponentAttackMap;
    uint64_t checkEvasionMask;
    uint64_t pinnedPieces;

    void convertFromFen(std::string fenString);

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
