#include "board.h"
#include "search.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <map>

#define WINDOW_SIZE 1200
#define SQUARE_SIZE (WINDOW_SIZE / 8)

const std::array<std::string, 64> squareNotation = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

struct BotSettings {
    bool enabled;
    bool AIControlsWhite;
};

class GameController {
  public:
    GameController(std::string _startingPos, BotSettings _botSettings)
        : board(_startingPos), botSettings(_botSettings), searcher() {
        startingPos = _startingPos;
    }

    std::array<short, 64> getPieceArray() {
        return board.getState();
    }

    bool isAITurn() {
        return botSettings.enabled && (botSettings.AIControlsWhite == board.getIsWhiteTurn()) && !board.getGameStatus();
    }

    bool isWhiteTurn() {
        return board.getIsWhiteTurn();
    }

    bool isPromotion(int startSquare, int destinationSquare) {
        return board.getState()[startSquare] == 1 && destinationSquare > 55 ||
               board.getState()[startSquare] == 9 && destinationSquare < 8;
    }

    bool isGameOver() { return board.getGameStatus() != 0; }

    int calculateFlag(int startSquare, int destinationSquare) {
        std::vector<Move> moves = board.getMoves();
        for (Move move : moves) {
            if (startSquare == move.getStart() && destinationSquare == move.getDestination()) {
                return move.getFlags();
            }
        }
        return 0;
    }

    std::set<int> getLegalMoveDestinations(int startSquare) {
        return board.getMoveOptions(startSquare);
    }

    void makeMove(Move move) {
        board.makeMove(move);
    }

    void makeAIMove() {
        Move bestMove = searcher.getBestMove(board);
        board.makeMove(bestMove);
    }

    void makePromotionMove(int column, int row, int startSquare, int destinationSquare) {
        int flags = 8;
        flags |= (row * 2 + column);
        if (board.getState()[destinationSquare]) {
            flags |= 4;
        }
        board.makeMove(Move(startSquare, destinationSquare, flags));
    }

    void perftTimer(int plyDepth) {
        for (int i = 0; i <= plyDepth; i++) {
            board = Board(startingPos);
            std::chrono::duration start = std::chrono::high_resolution_clock().now().time_since_epoch();
            int startMs = std::chrono::duration_cast<std::chrono::microseconds>(start).count();

            int moves = perft(i);

            std::chrono::duration end = std::chrono::high_resolution_clock().now().time_since_epoch();
            int endMs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
            double timeElapsed = endMs - startMs;
            double nps = 0;
            if (timeElapsed) {
                nps = (double)moves / timeElapsed * 1000000;
            }

            std::cout << i << ": " << moves << " " << timeElapsed / 1000 << "ms @ " << nps << "n/s" << '\n';
        }
    }

  private:
    Board board;
    std::string startingPos;
    BotSettings botSettings;
    Searcher searcher;

    int perft(int depth, bool topLevel = true) {
        if (depth == 0) {
            return 1;
        }
        int totalMoves = 0;
        std::vector<Move> moves = board.getMoves();
        for (Move move : moves) {
            board.makeMove(move);
            int numMoves = perft(depth - 1, false);
            board.unmakeMove(move);
            /*
            if (topLevel) {
                std::cout << squareNotation[move.getStart()] << "->" << squareNotation[move.getDestination()] << ", "
                          << move.getFlags() << ": " << numMoves << '\n';
            }
            */
            totalMoves += numMoves;
        }
        return totalMoves;
    }
};

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
            SDL_Rect rect = {x + (i / 2) * SQUARE_SIZE * 2, y + (i & 1) * SQUARE_SIZE * 2, width / 2, height / 2};
            SDL_RenderCopy(renderer, pieceTextures[pieceColour + i + 1], NULL, &rect);
        }
    }
};

void drawPieces(SDL_Renderer *renderer, SDL_Texture *pieceTextures[14], std::array<short, 64> pieceArray) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int squareValue = pieceArray[rank * 8 + file];
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
            if (((rank + file) & 1) == 0) {
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
        int file = square & 7;
        int rank = 7 - square / 8;
        SDL_Rect rect = {file * SQUARE_SIZE, rank * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void drawOpponentAttackMap(SDL_Renderer *renderer, uint64_t opponentAttackMap) {
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0x55);
    for (int square = 0; square < 64; square++) {
        if (1ULL << square & opponentAttackMap) {
            int file = square & 7;
            int rank = 7 - square / 8;
            SDL_Rect rect = {file * SQUARE_SIZE, rank * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

int main(int argc, char *argv[]) {

    std::string startingPos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    int plyDepth = -1;
    BotSettings botSettings = {false, false};

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-b")) {
            i++;
            startingPos = argv[i];
        } else if (!strcmp(argv[i], "-p")) {
            i++;
            plyDepth = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-c")) {
            i++;
            botSettings.enabled = true;
            if (!strcmp(argv[i], "w")) {
                botSettings.AIControlsWhite = true;
            } else if (strcmp(argv[i], "b")) {
                std::cout << "Invalid usage" << '\n';
                return 1;
            }
        } else {
            std::cout << "Invalid usage" << '\n';
            return 1;
        }
    }

    GameController gameController = GameController(startingPos, botSettings);

    if (plyDepth >= 0) {
        gameController.perftTimer(plyDepth);
        return 0;
    }

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    SDL_Window *window =
        SDL_CreateWindow("Chess Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_SIZE, WINDOW_SIZE, 0);
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
            if (event.type == SDL_MOUSEBUTTONDOWN && !gameController.isGameOver() && !gameController.isAITurn()) {
                if (promotionMenu.display) {
                    if (promotionMenu.isClicked(event.button.x, event.button.y)) {
                        int column = (event.button.x - promotionMenu.x) / (SQUARE_SIZE * 2);
                        int row = (event.button.y - promotionMenu.y) / (SQUARE_SIZE * 2);
                        gameController.makePromotionMove(column, row, startSquare, squareClicked);
                        promotionMenu.display = false;
                        startSquare = -1;
                        moveOptions.clear();
                    }
                } else {
                    int file = event.button.x / SQUARE_SIZE;
                    int rank = 7 - event.button.y / SQUARE_SIZE;
                    squareClicked = rank * 8 + file;
                    if (startSquare == -1) {
                        moveOptions = gameController.getLegalMoveDestinations(squareClicked);
                        if (!moveOptions.empty()) {
                            startSquare = squareClicked;
                        }
                    } else if (startSquare >= 0) {
                        if (moveOptions.contains(squareClicked)) {
                            if (gameController.isPromotion(startSquare, squareClicked)) {
                                promotionMenu.display = true;
                            } else {
                                int flags = gameController.calculateFlag(startSquare, squareClicked);
                                gameController.makeMove(Move(startSquare, squareClicked, flags));
                                startSquare = -1;
                                moveOptions.clear();
                            }
                        } else {
                            moveOptions = gameController.getLegalMoveDestinations(squareClicked);
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

        drawPieces(renderer, pieceTextures, gameController.getPieceArray());

        if (promotionMenu.display) {
            promotionMenu.draw(renderer, pieceTextures, gameController.isWhiteTurn());
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(20);

        if (gameController.isAITurn()) {
            gameController.makeAIMove();
        }
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
