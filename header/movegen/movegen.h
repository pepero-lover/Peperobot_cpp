//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_MOVEGEN_H
#define PEPEROBOT_CPP_MOVEGEN_H

#pragma once
#include "move.h"
#include "header/board/Board.h"

bool is_square_attacked(const Board& board, int square, int side);
void generate_moves(const Board& board, MoveList& list);

bool make_move(Board& board, Move move);
void unmake_move(Board& board, Move move);

#endif //PEPEROBOT_CPP_MOVEGEN_H
