//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_ATTACKS_H
#define PEPEROBOT_CPP_ATTACKS_H
#pragma once

#include "constants.h"

constexpr bool BISHOP = true;
constexpr bool ROOK = false;

constexpr U64 not_a_file  = 18374403900871474942ULL;
constexpr U64 not_h_file  = 9187201950435737471ULL;
constexpr U64 not_hg_file = 4557430888798830399ULL;
constexpr U64 not_ab_file = 18229723555195321596ULL;

extern const int bishop_relevant_bits[64];
extern const int rook_relevant_bits[64];

extern U64 pawn_attacks[2][64];
extern U64 knight_attacks[64];
extern U64 king_attacks[64];
extern U64 bishop_masks[64];
extern U64 rook_masks[64];
extern U64 bishop_attacks[64][512];
extern U64 rook_attacks[64][4096];

U64 mask_pawn_attacks(int side, int square);
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);
U64 mask_bishop_attacks(int square);
U64 mask_rook_attacks(int square);

U64 bishop_attacks_on_the_fly(int square, U64 block);
U64 rook_attacks_on_the_fly(int square, U64 block);

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);

void init_leapers_attacks();

U64 get_bishop_attacks(int square, U64 occupancy);
U64 get_rook_attacks(int square, U64 occupancy);
U64 get_queen_attacks(int square, U64 occupancy);


#endif //PEPEROBOT_CPP_ATTACKS_H
