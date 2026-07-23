//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_ZOBRIST_H
#define PEPEROBOT_CPP_ZOBRIST_H

#pragma once
#include "header/bitboard/constants.h"
#include "header/board/Board.h"

extern U64 piece_keys[12][64];
extern U64 enpassant_keys[64];
extern U64 castling_keys[16];
extern U64 side_key;

void init_hash_keys();
U64 generate_hash_key(const Board& board);

#endif //PEPEROBOT_CPP_ZOBRIST_H
