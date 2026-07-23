//
// Created by PEPERO-LOVER on 26. 7. 21..
//


#include "header/board/castling.h"
#include "header/bitboard/squares.h"

int castling_update_mask[64];

void init_castling_masks() {
    for (int i = 0; i < 64; i++) castling_update_mask[i] = 15;

    castling_update_mask[e1] = 15 & ~(WK | WQ);  // 12
    castling_update_mask[a1] = 15 & ~WQ;         // 13
    castling_update_mask[h1] = 15 & ~WK;         // 14
    castling_update_mask[e8] = 15 & ~(BK | BQ);  // 3
    castling_update_mask[a8] = 15 & ~BQ;         // 7
    castling_update_mask[h8] = 15 & ~BK;         // 11
}