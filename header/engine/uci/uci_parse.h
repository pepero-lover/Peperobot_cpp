//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_UCI_PARSE_H
#define PEPEROBOT_CPP_UCI_PARSE_H

#pragma once
#include <string>
#include "header/board/board.h"
#include "header/movegen/move.h"

Move parse_move(board& board, const std::string& command);
void parse_position(board& board, const std::string& command);
void parse_go(board& board, const std::string& command);
void parse_option(const std::string& name, const std::string& value);

#endif //PEPEROBOT_CPP_UCI_PARSE_H
