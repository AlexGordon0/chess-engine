#include <array>
#include <cstdint>
#include <vector>

namespace magics {

uint64_t calculateOccupancyPermutation(uint64_t mask, int permutationNumber);
uint64_t calculateBishopBlockMask(int square, uint64_t occupancyMask);
uint64_t calculateRookBlockMask(int square, uint64_t occupancyMask);

extern const std::array<int, 64> bishopNumBits;
extern const std::array<int, 64> rookNumBits;

extern const std::array<uint64_t, 64> bishopOccupancyMasks;
extern const std::array<uint64_t, 64> rookOccupancyMasks;

extern const std::array<uint64_t, 64> bishopMagics;
extern const std::array<uint64_t, 64> rookMagics;

extern const std::vector<std::vector<uint64_t>> bishopLookupTable;
extern const std::vector<std::vector<uint64_t>> rookLookupTable;

}
