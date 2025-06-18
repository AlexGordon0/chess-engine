#include <array>
#include <cstdint>
#include <iostream>
#include <set>

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

    std::set<int> getMoveOptions(int startSquare);
    std::array<int, 64> getState();
    std::set<Move> getMoves();
    int getEnPassantSquare();
    bool getIsWhiteTurn();

    void printBoard();
    void printBitboard(int index);
    void printMoves();

  private:
    static std::array<std::array<int, 8>, 64> numSquaresToEdge;
    static std::array<uint64_t, 64> knightMoveMasks;
    static std::array<uint64_t, 64> kingMoveMasks;

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

    bool whiteTurn;
    int castlingRights;
    int enPassantSquare;
    std::set<Move> moves;

    /* 0 = no check
     * 1 = check
     * 2 = double check */
    int checkStatus;
    uint64_t opponentAttackMap;
    uint64_t checkBlockMap;

    static void calculateSquareData();
    static void calculateKnightMasks();
    static void calculateKingMasks();

    void convertFromFen(std::string fenString);

    struct AttackMapPair {
        uint64_t attackMap;
        uint64_t blockPath;
    };

    void determineCheckStatus();
    uint64_t generatePawnAttackMaps();
    uint64_t generateKnightAttackMaps();
    AttackMapPair generateBishopAttackMaps();
    AttackMapPair generateRookAttackMaps();
    AttackMapPair generateQueenAttackMaps();
    uint64_t generateKingAttackMaps();

    void generateLegalMoves();
    void generatePawnMoves();
    void generateKnightMoves();
    void generateSlidingMoves();
    void generateKingMoves();
    void addMovesFromBitmap(uint64_t bitmap, int startSquareOffset);
};
