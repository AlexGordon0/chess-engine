# Chess Engine

A chess engine written in C++ with an SDL2 graphical interface.

## Features

- Chess board GUI built using SDL2.
- Move generation using bitboards, with magic bitboards to optimise sliding-piece move generation.
- Implements negamax search function with alpha-beta pruning and expanded quiescence search.
- Includes perft tests to validate move generation by counting the legal move sequences available at a given depth.
- Displays nodes per second after perft test for benchmarking purposes.

## Building and Running

### Prerequisites

The chess engine relies on the following to build:
- CMake
- SDL2

Clone the repository using:
```bash
git clone https://github.com/AlexGordon0/chess-engine.git
```

### Building

- Navigate to the directory using `cd chess-engine`
- Run `cmake -S . -B build` to generate build files.
- Build the application using `cmake --build build/`

### Running

Run the program with `./chess-engine`. 
If no parameters are included a chess game from the starting position between two humans players is loaded.

Optional parameters include:
- `-b [FEN string]` - Allows a custom starting position to be specified in the form of a FEN string.
- `-p [n]` - Runs a performance test up to a depth of `n` moves on the specified starting position.
- `-c [w / b]` - Enables the AI, with the argument `w` meaning the human player controls the white pieces and `b` meaning the human player controls the black pieces.

## Benchmarks

The engine achieved the following perft results from the starting position:
