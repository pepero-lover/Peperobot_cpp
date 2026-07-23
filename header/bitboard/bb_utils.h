//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_BB_UTILS_H
#define PEPEROBOT_CPP_BB_UTILS_H
#pragma once

#include <cstdint>
#include <cstdio>

#include "constants.h"

// get, set, pop bit
#define get_bit(bitboard, square)  ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard, square)  ((bitboard) |= (1ULL << (square)))
#define pop_bit(bitboard, square)  ((bitboard) &= ~(1ULL << (square)))

// count bit
static inline int count_bit(U64 bitboard) {
    return __builtin_popcountll(bitboard);
}

// get ls1b index
static inline int get_ls1b(U64 bitboard) {
    if (bitboard == 0) return -1;
    return __builtin_ctzll(bitboard);
}

static inline void print_bitboard(U64 bitboard) {
    printf("\n");

    for (int rank = 7; rank >= 0; rank--) {
        printf("  %d  ", rank + 1);

        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            int bit = get_bit(bitboard, square) ? 1 : 0;
            printf(" %d", bit);
        }
        printf("\n");
    }

    printf("\n      a b c d e f g h \n\n");
    printf("  Bitboard : %llu\n", (unsigned long long)bitboard);
}

#endif //PEPEROBOT_CPP_BB_UTILS_H
