//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_BOARD_H
#define PEPEROBOT_CPP_BOARD_H
#pragma once

#include <string>

#include "header/bitboard/constants.h"
#include "header/bitboard/squares.h"
#include "header/engine/nnue/nnue_dirty.h"

class Board {
public:
    static constexpr int MAX_DEPTH = 1024;
    static const std::string start_position;

    NNUEDirtyPiece nnue_dirty_history[MAX_DEPTH] = {};

    U64 bitboards[12] = {};

    // white, black, both
    U64 occupancies[3] = {};

    int side = white;
    int enpassant = no_sq;
    int castle = 0;

    U64 hash_key = 0;

    int ply = 0;
    int half_ply = 0;

    // make / unmake history
    int enpassant_history[MAX_DEPTH] = {};
    int castle_history[MAX_DEPTH] = {};
    int half_ply_history[MAX_DEPTH] = {};
    U64 hash_key_history[MAX_DEPTH] = {};
    int captured_piece_history[MAX_DEPTH] = {};

    Board();

    void reset();
    void set_start_pos();

    U64 get_bitboard_piece(int piece) const { return bitboards[piece]; }
};


#endif //PEPEROBOT_CPP_BOARD_H
