//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_FEN_H
#define PEPEROBOT_CPP_FEN_H

#pragma once
#include <string>
#include "board.h"

void parse_fen(board& board, const std::string& fen);
void print_board(const board& board);

#endif //PEPEROBOT_CPP_FEN_H
