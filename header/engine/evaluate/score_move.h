//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_SCORE_MOVE_H
#define PEPEROBOT_CPP_SCORE_MOVE_H

#pragma once
#include "search.h"
#include "header/engine/search.h"
#include "header/movegen/move.h"

int score_move(Search& search, const board& board, int move, int hash_move);
void score_moves(Search& search, const board& board, MoveList& list, int hash_move);
void score_quiescence_moves(Search& search, const board& board, MoveList& list, int hash_move);
int pick_next_move(Search& search, int step_index, int move_count, MoveList& list);

#endif //PEPEROBOT_CPP_SCORE_MOVE_H
