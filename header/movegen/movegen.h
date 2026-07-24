//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_MOVEGEN_H
#define PEPEROBOT_CPP_MOVEGEN_H

#pragma once
#include "move.h"
#include "header/board/board.h"

bool is_square_attacked(const board& board, int square, int side);
void generate_moves(const board& board, MoveList& list);

bool make_move(board& board, Move move);
void unmake_move(board& board, Move move);

#endif //PEPEROBOT_CPP_MOVEGEN_H
