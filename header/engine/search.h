//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_SEARCH_H
#define PEPEROBOT_CPP_SEARCH_H

#include <cstdint>

#include "search_constants.h"
#include "header/board/Board.h"

#pragma once

class Search {
public:
    int thread_id = 0;
    int ply = 0;
    long long nodes = 0;

    int killer_moves[2][SearchConst::MAX_PLY] = {};
    int history_moves[12][64] = {};

    bool follow_pv = false;
    bool score_pv = false;

    int pv_length[SearchConst::MAX_PLY] = {};
    int pv_table[SearchConst::MAX_PLY][SearchConst::MAX_PLY] = {};

    int move_scores[256] = {};

    int best_move = 0;
    int root_ply = 0;

    // ---- quiet move tracking (history gravity/malus 용) ----
    static constexpr int MAX_QUIET_TRACKED = 64;
    int quiet_moves_at_ply[SearchConst::MAX_PLY][MAX_QUIET_TRACKED] = {};
    int quiet_count_at_ply[SearchConst::MAX_PLY] = {};

    static constexpr int MAX_HISTORY = 16384;

    // ---- continuation history ----
    static constexpr int NULL_MOVE_INDEX = 768;
    int cont_hist_1[NULL_MOVE_INDEX + 1][768] = {};
    int cont_hist_2[NULL_MOVE_INDEX + 1][768] = {};
    int played_index_at_ply[SearchConst::MAX_PLY + 1] = {};

    // ---- correction history (pawn structure) ----
    static constexpr int CORR_HIST_SIZE = 1 << 14;
    static constexpr int CORR_HIST_GRAIN = 256;

    static int CorrHistMaxCp;       // SPSA 튜닝 대상 (UCI setoption)
    static int CorrHistWeightScale; // SPSA 튜닝 대상
    static int CorrHistMinDepth;    // SPSA 튜닝 대상

    int pawn_corr_hist[2][CORR_HIST_SIZE] = {};

    // ---- improving flag용 static eval history ----
    int static_eval_stack[SearchConst::MAX_PLY + 2] = {};

    static int move_index(int piece, int to) { return piece * 64 + to; }

    int get_cont_hist_score(int piece, int to) const {
        int idx = move_index(piece, to);
        int score = 0;
        if (ply >= 1) score += cont_hist_1[played_index_at_ply[ply]][idx];
        if (ply >= 2) score += cont_hist_2[played_index_at_ply[ply - 1]][idx];
        return score;
    }

    void reset_for_new_search();
};

void search_position(Board& board, int depth);

#endif //PEPEROBOT_CPP_SEARCH_H