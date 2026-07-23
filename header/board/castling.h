//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_CASTLING_H
#define PEPEROBOT_CPP_CASTLING_H

#pragma once

enum { WK = 1, WQ = 2, BK = 4, BQ = 8 };

extern int castling_update_mask[64];
void init_castling_masks();

#endif //PEPEROBOT_CPP_CASTLING_H
