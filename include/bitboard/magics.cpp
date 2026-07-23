//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/bitboard/magics.h"

#include <cstring>

#include "header/bitboard/attacks.h"
#include "header/bitboard/bb_utils.h"
#include "header/bitboard/constants.h"
#include "header/bitboard/magic_numbers.h"
#include "header/bitboard/random.h"

U64 find_magic_number(int square, int relevant_bits, bool bishop) {
    U64 occupancies[4096];
    U64 attacks[4096];
    U64 used_attacks[4096];

    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
    int occupancy_indices = 1 << relevant_bits;

    for (int index = 0; index < occupancy_indices; index++) {
        occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);
        attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index])
                                 : rook_attacks_on_the_fly(square, occupancies[index]);
    }

    for (int random_count = 0; random_count < 10000000; random_count++) {
        U64 magic_number = generate_magic_number();

        if (count_bit((attack_mask * magic_number) & 0xFF00000000000000ULL) < 6) continue;

        memset(used_attacks, 0, sizeof(used_attacks));

        int index;
        bool fail;

        for (index = 0, fail = false; !fail && index < occupancy_indices; index++) {
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));

            if (used_attacks[magic_index] == 0ULL) {
                used_attacks[magic_index] = attacks[index];
            } else if (used_attacks[magic_index] != attacks[index]) {
                fail = true;
            }
        }

        if (!fail) return magic_number;
    }

    printf("   Magic number fails!\n");
    return 0ULL;
}

void init_magic_numbers() {
    set_random_state_for_magic_number();

    for (int square = 0; square < 64; square++)
        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], ROOK);

    printf("\n\n");

    for (int square = 0; square < 64; square++)
        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], BISHOP);
}

void init_sliders_attacks(bool bishop) {
    for (int square = 0; square < 64; square++) {
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];
        int relevant_bits_count = count_bit(attack_mask);
        int occupancy_indices = 1 << relevant_bits_count;

        for (int index = 0; index < occupancy_indices; index++) {
            if (bishop) {
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                int magic_index = (int)((occupancy * bishop_magic_numbers[square])
                                         >> (64 - bishop_relevant_bits[square]));
                bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
            } else {
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                int magic_index = (int)((occupancy * rook_magic_numbers[square])
                                         >> (64 - rook_relevant_bits[square]));
                rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
            }
        }
    }
}