#include <iostream>
#include <cstdint>
#include <array>
#include <set>

class Move {
public:
  Move(unsigned int start, unsigned int destination, unsigned int flags);
  unsigned int getStart();
  unsigned int getDestination();
  unsigned int isCapture();
  unsigned int getFlags();

private:
  unsigned int move;
};

class Board {
public:
  Board(std::string startingPos);
  Board(std::array<int, 64> state, std::array<uint64_t, 15> bitboards, bool whiteTurn, int castlingRights, int enPassantSquare);
  std::set<int> getMoveOptions(int startSquare);
  Board makeMove(Move move);
  std::array<int, 64> getState();
  std::set<Move> getMoves();
  int getEnPassantSquare();
  void printBoard();
  void printBitboard(int index);
  void printMoves();

private:
  static std::array<std::array<int, 8>, 64> numSquaresToEdge;
  static std::array<uint64_t, 64> knightMoveMasks;
  static std::array<uint64_t, 64> kingMoveMasks;
  std::array<int, 64> state = {0};
  std::array<uint64_t, 15> bitboards = {0};
  std::set<Move> moves;
  bool whiteTurn;
  int castlingRights;
  int enPassantSquare;

  static void calculateSquareData();
  static void calculateKnightMasks();
  static void calculateKingMasks();
  void convertFromFen(std::string fenString);
  void generateLegalMoves();
  void generatePawnMoves();
  void generateKnightMoves();
  void generateSlidingMoves();
  void generateKingMoves();
  void addMovesFromBitmap(uint64_t bitmap, int startSquareOffset);
};