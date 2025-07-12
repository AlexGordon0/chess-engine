#include "magics.h"
#include <random>

namespace magics {

const std::array<int, 64> bishopNumBits = {
    6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6,
};

const std::array<int, 64> rookNumBits = {
    12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10,
    10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10,
    10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12,
};

const std::array<uint64_t, 64> bishopOccupancyMasks = {
    18049651735527936, 70506452091904,    275415828992,      1075975168,        38021120,         8657588224,
    2216338399232,     567382630219776,   9024825867763712,  18049651735527424, 70506452221952,   275449643008,
    9733406720,        2216342585344,     567382630203392,   1134765260406784,  4512412933816832, 9024825867633664,
    18049651768822272, 70515108615168,    2491752130560,     567383701868544,   1134765256220672, 2269530512441344,
    2256206450263040,  4512412900526080,  9024834391117824,  18051867805491712, 637888545440768,  1135039602493440,
    2269529440784384,  4539058881568768,  1128098963916800,  2256197927833600,  4514594912477184, 9592139778506752,
    19184279556981248, 2339762086609920,  4538784537380864,  9077569074761728,  562958610993152,  1125917221986304,
    2814792987328512,  5629586008178688,  11259172008099840, 22518341868716544, 9007336962655232, 18014673925310464,
    2216338399232,     4432676798464,     11064376819712,    22137335185408,    44272556441600,   87995357200384,
    35253226045952,    70506452091904,    567382630219776,   1134765260406784,  2832480465846272, 5667157807464448,
    11333774449049600, 22526811443298304, 9024825867763712,  18049651735527936,
};

const std::array<uint64_t, 64> rookOccupancyMasks = {
    282578800148862,     565157600297596,     1130315200595066,    2260630401190006,    4521260802379886,
    9042521604759646,    18085043209519166,   36170086419038334,   282578800180736,     565157600328704,
    1130315200625152,    2260630401218048,    4521260802403840,    9042521604775424,    18085043209518592,
    36170086419037696,   282578808340736,     565157608292864,     1130315208328192,    2260630408398848,
    4521260808540160,    9042521608822784,    18085043209388032,   36170086418907136,   282580897300736,
    565159647117824,     1130317180306432,    2260632246683648,    4521262379438080,    9042522644946944,
    18085043175964672,   36170086385483776,   283115671060736,     565681586307584,     1130822006735872,
    2261102847592448,    4521664529305600,    9042787892731904,    18085034619584512,   36170077829103616,
    420017753620736,     699298018886144,     1260057572672512,    2381576680245248,    4624614895390720,
    9110691325681664,    18082844186263552,   36167887395782656,   35466950888980736,   34905104758997504,
    34344362452452352,   33222877839362048,   30979908613181440,   26493970160820224,   17522093256097792,
    35607136465616896,   9079539427579068672, 8935706818303361536, 8792156787827803136, 8505056726876686336,
    7930856604974452736, 6782456361169985536, 4485655873561051136, 9115426935197958144,
};

const std::array<uint64_t, 64> bishopMagics = {
    6897270834382336ULL,     324969629515286528ULL,  1127008646012929ULL,     3382099948078080ULL,
    578151869906419968ULL,   288529548609325121ULL,  145243291570938368ULL,   576500343346561540ULL,
    2306144344137073664ULL,  9322486418414567680ULL, 2307760862573693072ULL,  72457886365646892ULL,
    9223936156113047040ULL,  18051241298894848ULL,   144115609251356682ULL,   4611692619801037832ULL,
    9018470658294272ULL,     1161946315446552640ULL, 2256215042335234ULL,     4613942225649287185ULL,
    4521544035336224ULL,     36591756770428940ULL,   282866554671192ULL,      4611760830319693858ULL,
    2286992843083776ULL,     2308242229552232448ULL, 2612159321655741312ULL,  9042418557026312ULL,
    1729527392789078032ULL,  40541742646182176ULL,   11330468406396928ULL,    282849517503905ULL,
    3603451860280942592ULL,  74331944577665025ULL,   249073744478240ULL,      6791134106354177ULL,
    16141816138417045760ULL, 1450160767964971200ULL, 2306969546923321350ULL,  649083083538432512ULL,
    5264998245398233248ULL,  288936297515549184ULL,  2305862804869484544ULL,  17731304818688ULL,
    571819065082880ULL,      2314304876623970816ULL, 40545595147976768ULL,    19159610837639424ULL,
    2308332337975279888ULL,  81628310199074953ULL,   9223385413598577188ULL,  360296906204906112ULL,
    290517431238134288ULL,   9224568341242060800ULL, 4629702805224030230ULL,  4612266706621106690ULL,
    563502528663556ULL,      35498041549312ULL,      11538239880463271938ULL, 9230704165664098321ULL,
    18014424415666433ULL,    297238196072229120ULL,  185852094353924ULL,      73201378201209344ULL,
};
const std::array<uint64_t, 64> rookMagics = {
    36029348456694934ULL,   9529621621884584128ULL,  1297054561896253952ULL, 72075188908654856ULL,
    144119655536003584ULL,  5836666216720237568ULL,  9403535813175676288ULL, 1765412295174865024ULL,
    3476919663777054752ULL, 288300746238222339ULL,   9288811671472386ULL,    146648600474026240ULL,
    3799946587537536ULL,    704237264700928ULL,      10133167915730964ULL,   2305983769267405952ULL,
    9223634270415749248ULL, 10344480540467205ULL,    9376496898355021824ULL, 2323998695235782656ULL,
    9241527722809755650ULL, 189159985010188292ULL,   2310421375767019786ULL, 4647717014536733827ULL,
    5585659813035147264ULL, 1442911135872321664ULL,  140814801969667ULL,     1188959108457300100ULL,
    288815318485696640ULL,  758869733499076736ULL,   234750139167147013ULL,  2305924931420225604ULL,
    9403727128727390345ULL, 9223970239903959360ULL,  309094713112139074ULL,  38290492990967808ULL,
    3461016597114651648ULL, 181289678366835712ULL,   4927518981226496513ULL, 1155212901905072225ULL,
    36099167912755202ULL,   9024792514543648ULL,     4611826894462124048ULL, 291045264466247688ULL,
    83880127713378308ULL,   1688867174481936ULL,     563516973121544ULL,     9227888831703941123ULL,
    703691741225216ULL,     45203259517829248ULL,    693563138976596032ULL,  4038638777286134272ULL,
    865817582546978176ULL,  13835621555058516608ULL, 11541041685463296ULL,   288511853443695360ULL,
    283749161902275ULL,     176489098445378ULL,      2306124759338845321ULL, 720584805193941061ULL,
    4977040710267061250ULL, 10097633331715778562ULL, 325666550235288577ULL,  1100057149646ULL,
};

uint64_t calculateOccupancyPermutation(uint64_t mask, int permutationNumber) {
    uint64_t permutation = 0;
    int count = 0;
    while (mask) {
        int square = std::countr_zero(mask);
        if (permutationNumber >> count & 1) {
            permutation |= 1ULL << square;
        }
        count++;
        mask -= 1ULL << square;
    }
    return permutation;
}

uint64_t calculateBishopBlockMask(int square, uint64_t occupancyMask) {
    int file = square & 7;
    int rank = square / 8;
    uint64_t mask = 0;

    for (int i = file - 1, j = rank - 1; i >= 0 && j >= 0; i--, j--) {
        mask |= 1ULL << (j * 8 + i);
        if (occupancyMask >> (j * 8 + i) & 1) {
            break;
        }
    }
    for (int i = file - 1, j = rank + 1; i >= 0 && j < 8; i--, j++) {
        mask |= 1ULL << (j * 8 + i);
        if (occupancyMask >> (j * 8 + i) & 1) {
            break;
        }
    }
    for (int i = file + 1, j = rank - 1; i < 8 && j >= 0; i++, j--) {
        mask |= 1ULL << (j * 8 + i);
        if (occupancyMask >> (j * 8 + i) & 1) {
            break;
        }
    }
    for (int i = file + 1, j = rank + 1; i < 8 && j < 8; i++, j++) {
        mask |= 1ULL << (j * 8 + i);
        if (occupancyMask >> (j * 8 + i) & 1) {
            break;
        }
    }

    return mask;
}

uint64_t calculateRookBlockMask(int square, uint64_t occupancyMask) {
    int file = square & 7;
    int rank = square / 8;
    uint64_t mask = 0;

    for (int i = file - 1; i >= 0; i--) {
        mask |= 1ULL << (rank * 8 + i);
        if (occupancyMask >> (rank * 8 + i) & 1) {
            break;
        }
    }
    for (int i = file + 1; i < 8; i++) {
        mask |= 1ULL << (rank * 8 + i);
        if (occupancyMask >> (rank * 8 + i) & 1) {
            break;
        }
    }
    for (int j = rank - 1; j >= 0; j--) {
        mask |= 1ULL << (j * 8 + file);
        if (occupancyMask >> (j * 8 + file) & 1) {
            break;
        }
    }
    for (int j = rank + 1; j < 8; j++) {
        mask |= 1ULL << (j * 8 + file);
        if (occupancyMask >> (j * 8 + file) & 1) {
            break;
        }
    }

    return mask;
}

std::vector<std::vector<uint64_t>> calculateLookupTable(bool isBishop) {
    std::array<uint64_t, 64> magics = isBishop ? bishopMagics : rookMagics;
    std::array<uint64_t, 4096> occupancyPermutations, blockMasks;
    std::vector<std::vector<uint64_t>> result(64);

    for (int square = 0; square < 64; square++) {
        int numBits = isBishop ? bishopNumBits[square] : rookNumBits[square];
        int numPermutations = 1 << numBits;
        uint64_t magic = magics[square];
        std::vector<uint64_t> lookupTable(numPermutations);
        uint64_t mask = isBishop ? bishopOccupancyMasks[square] : rookOccupancyMasks[square];

        for (int i = 0; i < numPermutations; i++) {
            occupancyPermutations[i] = calculateOccupancyPermutation(mask, i);
            blockMasks[i] = isBishop ? calculateBishopBlockMask(square, occupancyPermutations[i])
                                     : calculateRookBlockMask(square, occupancyPermutations[i]);
        }

        for (int i = 0; i < numPermutations; i++) {
            int index = (occupancyPermutations[i] * magic) >> (64 - numBits);
            lookupTable[index] = blockMasks[i];
        }

        result[square] = lookupTable;
    }

    return result;
}

const std::vector<std::vector<uint64_t>> bishopLookupTable = calculateLookupTable(true);
const std::vector<std::vector<uint64_t>> rookLookupTable = calculateLookupTable(false);

std::array<uint64_t, 793> generateZobristKeys() {
    std::array<uint64_t, 793> keys = {};
    for (int i = 0; i < 793; i++) {
        uint64_t num1 = random();
        uint64_t num2 = random();
        keys[i] = num1 << 32 | num2;
    }
    return keys;
}

const std::array<uint64_t, 793> zobristKeys = generateZobristKeys();

} // namespace magics
