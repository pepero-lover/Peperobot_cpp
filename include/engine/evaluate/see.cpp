//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include <algorithm>

#include "header/bitboard/attacks.h"
#include "header/bitboard/bb_utils.h"
#include "header/board/board.h"
#include "header/board/pieces.h"
#include "header/movegen/move.h"

namespace {

constexpr int PIECE_VALUE[12] = {
    100, 320, 330, 500, 900, 20000,
    100, 320, 330, 500, 900, 20000
};

int get_piece_on_square(const board& board, int square, int side) {
    U64 mask = 1ULL << square;
    int start = (side == white) ? P : p;
    for (int i = 0; i <= 5; i++) {
        if (board.bitboards[start + i] & mask) return start + i;
    }
    return -1;
}

U64 get_attackers_to(const board& board, int square, U64 occupancy) {
    U64 attackers = 0ULL;

    attackers |= pawn_attacks[black][square] & board.bitboards[P];
    attackers |= pawn_attacks[white][square] & board.bitboards[p];

    attackers |= knight_attacks[square] & (board.bitboards[N] | board.bitboards[n]);

    U64 bishop_atk = get_bishop_attacks(square, occupancy);
    attackers |= bishop_atk & (board.bitboards[B] | board.bitboards[b] |
                               board.bitboards[Q] | board.bitboards[q]);

    U64 rook_atk = get_rook_attacks(square, occupancy);
    attackers |= rook_atk & (board.bitboards[R] | board.bitboards[r] |
                             board.bitboards[Q] | board.bitboards[q]);

    attackers |= king_attacks[square] & (board.bitboards[K] | board.bitboards[k]);

    return attackers & occupancy;
}

int least_valuable_attacker_square(const board& board, U64 attackers, int side) {
    int start = (side == white) ? P : p;
    for (int i = 0; i <= 5; i++) {
        U64 bb = board.bitboards[start + i] & attackers;
        if (bb) return get_ls1b(bb);
    }
    return -1;
}

} // 익명 네임스페이스

int see(const board& board, int move) {
    int from = get_move_source(move);
    int to   = get_move_target(move);
    int side = board.side;

    int attacker_piece = get_move_piece(move);
    bool is_enpassant = get_move_enpassant(move);

    int victim_piece = get_piece_on_square(board, to, side ^ 1);
    if (victim_piece == -1 && !is_enpassant) return 0;

    int gain[32];
    int d = 0;

    gain[0] = is_enpassant ? PIECE_VALUE[(side == white) ? p : P] : PIECE_VALUE[victim_piece];

    U64 occupied = board.occupancies[both];
    occupied &= ~(1ULL << from);

    if (is_enpassant) {
        // a1=0: 캡처하는 쪽이 white면 target+8이 아니라 target-8 (앙파상 방향 스왑 규칙)
        int captured_pawn_sq = (side == white) ? to - 8 : to + 8;
        occupied &= ~(1ULL << captured_pawn_sq);
    }

    U64 attackers = get_attackers_to(board, to, occupied);

    int side_to_move = side ^ 1;
    int mover_value = PIECE_VALUE[attacker_piece];

    while (true) {
        d++;
        gain[d] = mover_value - gain[d - 1];

        if (std::max(-gain[d - 1], gain[d]) < 0) break;

        U64 side_occupancy = (side_to_move == white) ? board.occupancies[white] : board.occupancies[black];
        U64 side_attackers = attackers & occupied & side_occupancy;
        if (side_attackers == 0) break;

        int lva_sq = least_valuable_attacker_square(board, side_attackers, side_to_move);
        int lva_piece = get_piece_on_square(board, lva_sq, side_to_move);
        mover_value = PIECE_VALUE[lva_piece];

        occupied &= ~(1ULL << lva_sq);
        attackers = get_attackers_to(board, to, occupied);  // x-ray 재계산

        side_to_move ^= 1;
    }

    while (--d > 0) {
        gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
    }

    return gain[0];
}

int captured_value(const board& board, int move) {
    int side = board.side;
    if (get_move_enpassant(move)) {
        return PIECE_VALUE[(side == white) ? p : P];
    }
    int victim = get_piece_on_square(board, get_move_target(move), side ^ 1);
    return (victim == -1) ? 0 : PIECE_VALUE[victim];
}