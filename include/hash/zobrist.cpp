//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/hash/zobrist.h"
#include "header/bitboard/bb_utils.h"
#include "header/bitboard/random.h"
#include "header/board/pieces.h"

U64 piece_keys[12][64];
U64 enpassant_keys[64];
U64 castling_keys[16];
U64 side_key;

void init_hash_keys() {
    set_random_state_for_hashing();

    for (int piece = P; piece <= k; piece++)
        for (int square = 0; square < 64; square++)
            piece_keys[piece][square] = get_random_u64();

    for (int square = 0; square < 64; square++)
        enpassant_keys[square] = get_random_u64();

    for (int index = 0; index < 16; index++)
        castling_keys[index] = get_random_u64();

    side_key = get_random_u64();
}

U64 generate_hash_key(const Board& board) {
    U64 final_key = 0ULL;

    for (int piece = P; piece <= k; piece++) {
        U64 bitboard = board.bitboards[piece];

        while (bitboard) {
            int square = get_ls1b(bitboard);
            final_key ^= piece_keys[piece][square];
            pop_bit(bitboard, square);
        }
    }

    if (board.enpassant != no_sq)
        final_key ^= enpassant_keys[board.enpassant];

    final_key ^= castling_keys[board.castle];

    if (board.side == black)
        final_key ^= side_key;

    return final_key;
}