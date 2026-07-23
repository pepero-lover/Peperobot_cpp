//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_FEN_H
#define PEPEROBOT_CPP_FEN_H

#pragma once
#include <string>
#include "Board.h"

void parse_fen(Board& board, const std::string& fen);
void print_board(const Board& board);

#endif //PEPEROBOT_CPP_FEN_H
