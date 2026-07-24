//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_UCI_MANAGER_H
#define PEPEROBOT_CPP_UCI_MANAGER_H

#pragma once
#include "header/board/board.h"

extern board board_uci;

void communicate();
void uci_loop();
void warmup();

#endif //PEPEROBOT_CPP_UCI_MANAGER_H
