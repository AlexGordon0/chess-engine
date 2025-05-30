#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "board.h"
#include <map>

#define WINDOW_SIZE 600 
#define SQUARE_SIZE (WINDOW_SIZE/8)

void drawPieces(SDL_Renderer *renderer, SDL_Texture *pieceTextures[14], Board &board) {
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      int squareValue = board.getState().at(rank * 8 + file);
      if (squareValue > 0) {
        SDL_Rect rect = {file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
        SDL_RenderCopy(renderer, pieceTextures[squareValue-1], NULL, &rect);
      }
    }
  }
}

void drawBoard(SDL_Renderer *renderer) {
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      if ((rank + file) % 2 == 0) {
        SDL_SetRenderDrawColor(renderer, 0xde, 0xe3, 0xe6, 0xff);
      } else {
        SDL_SetRenderDrawColor(renderer, 0x8c, 0xa2, 0xad, 0xff);
      }
      SDL_Rect square = {file * SQUARE_SIZE, rank * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
      SDL_RenderFillRect(renderer, &square);
    }
  }
}

void drawMoveOptions(SDL_Renderer *renderer, std::set<int> moveOptions) {
  SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0x55);
  for (int square: moveOptions) {
    int file = square % 8;
    int rank = 7 - square / 8;
    SDL_Rect rect = {file * SQUARE_SIZE, rank * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
    SDL_RenderFillRect(renderer, &rect);
  }
}

int calculateFlags(Board &board, int startSquare, int destinationSquare) {
  int flags = 0;
  if ((board.getState().at(startSquare) == 1 || board.getState().at(startSquare) == 9) && \
      (abs(destinationSquare - startSquare) == 16)) {
    flags |= 1;
  }
  if (board.getState().at(destinationSquare)) {
    flags |= 1 << 2;
  }
  if ((board.getState().at(startSquare) == 1 || board.getState().at(startSquare) == 9) && \
    destinationSquare == board.getEnPassantSquare()) {
    flags = 5;
  }
  return flags;
}

int perft(Board board, int depth) {
  if (depth == 0) {
    return 1;
  }
  int totalMoves = 0;
  for (Move move: board.getMoves()) {
    totalMoves += perft(board.makeMove(move), depth-1);
  }
  return totalMoves;
}

int main(int argc, char *argv[]) {

  std::string startingPos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
  int plyDepth = -1;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-b")) {
      i++;
      startingPos = argv[i];
    } else if (!strcmp(argv[i], "-p")) {
      i++;
      plyDepth = atoi(argv[i]);
    } else {
      std::cout << "Invalid usage" << '\n';
      return 1;
    }
  }

  if (plyDepth >= 0) {
    int moves = perft(Board(startingPos), plyDepth);
    std::cout << moves << '\n';
    return 0;
  }

  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG);
  SDL_Window *window = SDL_CreateWindow("Gorbot", SDL_WINDOWPOS_CENTERED, 
    SDL_WINDOWPOS_CENTERED, WINDOW_SIZE, WINDOW_SIZE, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  const std::map<int, std::string> piecePathMap = {
    {0, "wP"},
    {1, "wN"},
    {2, "wB"},
    {3, "wR"},
    {4, "wQ"},
    {5, "wK"},
    {8, "bP"},
    {9, "bN"},
    {10, "bB"},
    {11, "bR"},
    {12, "bQ"},
    {13, "bK"},
  };

  SDL_Texture *pieceTextures[14];
  for (int i = 0; i < 14; i++) {
    if (i == 6 || i == 7) continue;
    std::string filename = "assets/" + piecePathMap.at(i) + ".png";
    SDL_Texture *imageTexture = IMG_LoadTexture(renderer, &filename[0]);
    pieceTextures[i] = imageTexture;
  }

  Board board = Board(startingPos);

  bool running = true;
  SDL_Event event;

  int startSquare = -1;
  std::set<int> moveOptions;
  while (running) {
   
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
      if (event.type == SDL_MOUSEBUTTONDOWN) {
        int file = event.button.x / SQUARE_SIZE;
        int rank = 7 - event.button.y / SQUARE_SIZE;
        int squareClicked = rank * 8 + file;
        if (startSquare == -1) {
          moveOptions = board.getMoveOptions(squareClicked);
          if (!moveOptions.empty()) {
            startSquare = squareClicked;
          }
        }
        else if (startSquare >= 0) {
          int flags = calculateFlags(board, startSquare, squareClicked);
          if (moveOptions.contains(squareClicked)) {
            board = board.makeMove(Move(startSquare, squareClicked, flags));
            startSquare = -1;
            moveOptions.clear();
          } else {
            moveOptions = board.getMoveOptions(squareClicked);
            if (!moveOptions.empty() && startSquare != squareClicked) {
              startSquare = squareClicked;
            } else {
              startSquare = -1;
              moveOptions.clear();
            }
          }
        }
      }
    }

    SDL_RenderClear(renderer);
    drawBoard(renderer);
    if (!moveOptions.empty()) {
      drawMoveOptions(renderer, moveOptions);
    }
    drawPieces(renderer, pieceTextures, board);
    SDL_RenderPresent(renderer);

    SDL_Delay(20);
  }

  for (int i = 0; i < 14; i++) {
    if (i == 6 || i == 7) continue;
    SDL_DestroyTexture(pieceTextures[i]);
  }

  IMG_Quit();
  SDL_Quit();
  return 0;
} 
 