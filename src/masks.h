#include <array>
#include <cstdint>

namespace masks {

std::array<std::array<int, 8>, 64> calculateNumSquaresToEdge();
std::array<uint64_t, 64> calculateKnightMasks();
std::array<uint64_t, 64> calculateBishopMasks();
std::array<uint64_t, 64> calculateRookMasks();
std::array<uint64_t, 64> calculateKingMasks();

extern const std::array<std::array<int, 8>, 64> numSquaresToEdge;
extern const std::array<uint64_t, 64> knightMoveMasks;
extern const std::array<uint64_t, 64> bishopMoveMasks;
extern const std::array<uint64_t, 64> rookMoveMasks;
extern const std::array<uint64_t, 64> kingMoveMasks;

} // namespace masks
