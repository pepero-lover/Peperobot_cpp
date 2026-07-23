//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_UCI_PARSE_H
#define PEPEROBOT_CPP_UCI_PARSE_H

#pragma once
#include <string>
#include "header/board/Board.h"
#include "header/movegen/move.h"

Move parse_move(Board& board, const std::string& command);
void parse_position(Board& board, const std::string& command);
void parse_go(Board& board, const std::string& command);
void parse_option(const std::string& name, const std::string& value);

#endif //PEPEROBOT_CPP_UCI_PARSE_H
