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

    // alpha/beta 윈도우로 걸러지지 않은 raw entry 조회 (singular extension 용).
    // 엔트리가 존재하면 true를 반환하고 out_* 에 값을 채움. mate score는 ply 보정됨.
    bool probe_raw(const Board& board, int ply, int& out_score, int& out_depth, int& out_flag);
}

#endif //PEPEROBOT_CPP_TT_H