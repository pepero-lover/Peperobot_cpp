//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/board/board.h"

#include <cstring>

#include "header/board/fen.h"
#include "header/hash/zobrist.h"

const std::string board::start_position =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

board::board() {
    reset();
}

void board::reset() {
    memset(bitboards, 0, sizeof(bitboards));
    memset(occupancies, 0, sizeof(occupancies));

    memset(enpassant_history, 0, sizeof(enpassant_history));
    memset(castle_history, 0, sizeof(castle_history));
    memset(half_ply_history, 0, sizeof(half_ply_history));
    memset(hash_key_history, 0, sizeof(hash_key_history));
    memset(captured_piece_history, 0, sizeof(captured_piece_history));

    side = white;
    enpassant = no_sq;
    castle = 0;

    hash_key = generate_hash_key(*this);

    ply = 0;
    half_ply = 0;
}

void board::set_start_pos() {
    reset();
    parse_fen(*this, start_position);
}