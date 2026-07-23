//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_TT_H
#define PEPEROBOT_CPP_TT_H

#pragma once
#include <cstdint>

#include "header/board/Board.h"

namespace TT {
    extern int TT_SIZE;

    constexpr int NO_HASH_ENTRY = 100000;

    constexpr int HASH_FLAG_EXACT = 0;
    constexpr int HASH_FLAG_ALPHA = 1;
    constexpr int HASH_FLAG_BETA  = 2;

    void resize_tt(int hash_size_mb);
    void clear_tt();

    int read_hash_move(const Board& board);
    int read_hash_entry(const Board& board, int alpha, int beta, int depth, int ply);
    void write_hash_entry(const Board& board, int score, int depth, int hash_flag, int best_move, int ply);
}

#endif //PEPEROBOT_CPP_TT_H
