#include "magics.h"
#include <iostream>

uint64_t random64Bits() {
    uint64_t num1 = random() & 0xffff;
    uint64_t num2 = random() & 0xffff;
    uint64_t num3 = random() & 0xffff;
    uint64_t num4 = random() & 0xffff;

    return (num1 << 48) | (num2 << 32) | (num3 << 16) | num4;
}

uint64_t getRandom() { return random64Bits() & random64Bits() & random64Bits(); }

uint64_t findMagics(int square, bool isBishop) {
    int numBits = isBishop ? magics::bishopNumBits[square] : magics::rookNumBits[square];
    int numPermutations = 1 << numBits;
    std::array<uint64_t, 4096> occupancyPermutations, blockMasks, used;
    uint64_t magic;

    uint64_t mask = isBishop ? magics::bishopOccupancyMasks[square] : magics::rookOccupancyMasks[square];

    for (int i = 0; i < numPermutations; i++) {
        occupancyPermutations[i] = magics::calculateOccupancyPermutation(mask, i);
        blockMasks[i] = isBishop ? magics::calculateBishopBlockMask(square, occupancyPermutations[i])
                                 : magics::calculateRookBlockMask(square, occupancyPermutations[i]);
    }

    for (int n = 0; n < 1000000; n++) {
        bool collision = false;
        magic = getRandom();
        for (int i = 0; i < numPermutations; i++) {
            used[i] = 0;
        }
        for (int i = 0; i < numPermutations; i++) {
            int index = (occupancyPermutations[i] * magic) >> (64 - numBits);
            if (used[index] && used[index] != blockMasks[i]) {
                collision = true;
                break;
            }
            used[index] = blockMasks[i];
        }
        if (!collision) {
            return magic;
        }
    }

    std::cout << "Failed to find magic number for square " << square << "." << '\n';
    return 0;
}

int main() {
    std::cout << "const std::array<uint64_t, 64> bishopMagics = {" << '\n';
    for (int square = 0; square < 64; square++) {
        std::cout << findMagics(square, true) << "ULL, " << '\n';
    }
    std::cout << "};" << '\n';

    std::cout << "const std::array<uint64_t, 64> rookMagics = {" << '\n';
    for (int square = 0; square < 64; square++) {
        std::cout << findMagics(square, false) << "ULL, " << '\n';
    }
    std::cout << "};" << '\n';

    return 0;
}
