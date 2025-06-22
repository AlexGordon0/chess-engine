#include "board.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstdint>
#include <map>
#include <chrono>

#define WINDOW_SIZE 1200
#define SQUARE_SIZE (WINDOW_SIZE / 8)

class PromotionMenu {
  public:
    int x, y;
    int width, height;
    bool display;

    PromotionMenu() {
        x = SQUARE_SIZE * 2;
        y = SQUARE_SIZE * 2;
        width = SQUARE_SIZE * 4;
        height = SQUARE_SIZE * 4;
        display = false;
    }

    bool isClicked(int xVal, int yVal) {
        bool xInRange = xVal >= x && xVal < x + width;
        bool yInRange = yVal >= y && yVal < y + height;
        return xInRange && yInRange;
    }

    void draw(SDL_Renderer *renderer, SDL_Texture *pieceTextures[14], bool isWhiteTurn) {
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_Rect rect = {x, y, width, height};
        SDL_RenderFillRect(renderer, &rect);
        int pieceColour = isWhiteTurn ? 0 : 8;
        for (int i = 0; i < 4; i++) {
            SDL_Rect rect = {x + (i / 2) * SQUARE_SIZE * 2, y + (i % 2) * SQUARE_SIZE * 2, width / 2, height / 2};
            SDL_RenderCopy(renderer, pieceTextures[pieceColour + i + 1], NULL, &rect);
        }
    }
};

void drawPieces(SDL_Renderer *renderer, SDL_Texture *pieceTextures[14], Board &board) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int squareValue = board.getState().at(rank * 8 + file);
            if (squareValue > 0) {
                SDL_Rect rect = {file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                SDL_RenderCopy(renderer, pieceTextures[squareValue - 1], NULL, &rect);
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
    for (int square : moveOptions) {
        int file = square % 8;
        int rank = 7 - square / 8;
        SDL_Rect rect = {file * SQUARE_SIZE, rank * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void drawOpponentAttackMap(SDL_Renderer *renderer, uint64_t opponentAttackMap) {
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0x55);
    for (int square = 0; square < 64; square++) {
        if (1ULL << square & opponentAttackMap) {
            int file = square % 8;
            int rank = 7 - square / 8;
            SDL_Rect rect = {file * SQUARE_SIZE, rank * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

int perft(Board board, int depth) {
    if (depth == 0) {
        return 1;
    }
    int totalMoves = 0;
    for (Move move : board.getMoves()) {
        int numMoves = perft(board.makeMove(move), depth - 1);
        totalMoves += numMoves;
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
        for (int i = 0; i <= plyDepth; i++) {
            std::chrono::duration start = std::chrono::high_resolution_clock().now().time_since_epoch();
            int startMs = std::chrono::duration_cast<std::chrono::microseconds>(start).count();
            
            int moves = perft(Board(startingPos), i);

            std::chrono::duration end = std::chrono::high_resolution_clock().now().time_since_epoch();
            int endMs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
            double timeElapsed = endMs - startMs;
            double nps = 0;
            if (timeElapsed) {
                nps = (double) moves / timeElapsed * 1000000;
            }

            std::cout << i << ": " << moves << " " << timeElapsed / 1000 << "ms @ " << nps << "n/s" << '\n';
        }
        return 0;
    }

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    SDL_Window *window =
        SDL_CreateWindow("Gorbot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_SIZE, WINDOW_SIZE, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const std::map<int, std::string> piecePathMap = {
        {0, "wP"}, {1, "wN"}, {2, "wB"},  {3, "wR"},  {4, "wQ"},  {5, "wK"},
        {8, "bP"}, {9, "bN"}, {10, "bB"}, {11, "bR"}, {12, "bQ"}, {13, "bK"},
    };

    SDL_Texture *pieceTextures[14];
    for (int i = 0; i < 14; i++) {
        if (i == 6 || i == 7)
            continue;
        std::string filename = "assets/" + piecePathMap.at(i) + ".png";
        SDL_Texture *imageTexture = IMG_LoadTexture(renderer, &filename[0]);
        pieceTextures[i] = imageTexture;
    }

    PromotionMenu promotionMenu = PromotionMenu();
    Board board = Board(startingPos);

    bool running = true;
    SDL_Event event;

    int startSquare = -1;
    int squareClicked = -1;
    std::set<int> moveOptions;

    while (running) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (promotionMenu.display) {
                    if (promotionMenu.isClicked(event.button.x, event.button.y)) {
                        int column = (event.button.x - promotionMenu.x) / (SQUARE_SIZE * 2);
                        int row = (event.button.y - promotionMenu.y) / (SQUARE_SIZE * 2);
                        int flags = 8;
                        flags |= (row * 2 + column);
                        if (board.getState()[squareClicked]) {
                            flags |= 4;
                        }
                        board = board.makeMove(Move(startSquare, squareClicked, flags));
                        promotionMenu.display = false;
                        startSquare = -1;
                        moveOptions.clear();
                    }
                } else {
                    int file = event.button.x / SQUARE_SIZE;
                    int rank = 7 - event.button.y / SQUARE_SIZE;
                    squareClicked = rank * 8 + file;
                    if (startSquare == -1) {
                        moveOptions = board.getMoveOptions(squareClicked);
                        if (!moveOptions.empty()) {
                            startSquare = squareClicked;
                        }
                    } else if (startSquare >= 0) {
                        if (moveOptions.contains(squareClicked)) {
                            int flag = 0;
                            if (board.getState()[startSquare] == 1 && squareClicked > 55 ||
                                board.getState()[startSquare] == 9 && squareClicked < 8) {
                                promotionMenu.display = true;
                            } else {
                                board = board.makeMove(Move(startSquare, squareClicked, flag));
                                startSquare = -1;
                                moveOptions.clear();
                            }
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
        }

        SDL_RenderClear(renderer);
        drawBoard(renderer);

        if (!moveOptions.empty()) {
            drawMoveOptions(renderer, moveOptions);
        }
        // drawOpponentAttackMap(renderer, board.getOpponentAttackMap());

        drawPieces(renderer, pieceTextures, board);

        if (promotionMenu.display) {
            promotionMenu.draw(renderer, pieceTextures, board.getIsWhiteTurn());
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }

    for (int i = 0; i < 14; i++) {
        if (i == 6 || i == 7)
            continue;
        SDL_DestroyTexture(pieceTextures[i]);
    }

    IMG_Quit();
    SDL_Quit();
    return 0;
}
