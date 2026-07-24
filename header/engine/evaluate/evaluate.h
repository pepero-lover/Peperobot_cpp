//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_EVALUATE_H
#define PEPEROBOT_CPP_EVALUATE_H

#pragma once
#include "header/board/Board.h"

namespace Evaluate {

    struct RawScores {
        int score_opening;
        int score_endgame;
        int game_phase_score;
        int game_phase; // 0=OPENING,1=ENDGAME,2=MIDDLEGAME
    };

    void set_material_score(int phase, int piece_type, int value);
    int  get_material_score(int phase, int piece_type);

    int get_king_safety_penalty(const Board& board, int side);

    RawScores compute_raw_scores(const Board& board);

    bool load_nnue(const std::string& path);
    bool load_nnue_embedded();

    void nnue_refresh_root(const Board& board);
    void nnue_do_move(const Board& board);

    int to_display_cp(int internal_value);

    int evaluate(const Board& board);
}

#endif //PEPEROBOT_CPP_EVALUATE_H