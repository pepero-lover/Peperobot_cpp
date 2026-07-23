//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/movegen/movegen.h"

#include "header/bitboard/attacks.h"
#include "header/bitboard/bb_utils.h"
#include "header/board/castling.h"
#include "header/board/pieces.h"
#include "header/hash/zobrist.h"

bool is_square_attacked(const Board& board, int square, int side) {
    if (side == white && (pawn_attacks[black][square] & board.bitboards[P])) return true;
    if (side == black && (pawn_attacks[white][square] & board.bitboards[p])) return true;
    if (knight_attacks[square] & (side == white ? board.bitboards[N] : board.bitboards[n])) return true;

    U64 occupancy = board.occupancies[both];

    U64 bishops_queens = (side == white) ? (board.bitboards[B] | board.bitboards[Q])
                                          : (board.bitboards[b] | board.bitboards[q]);
    if (get_bishop_attacks(square, occupancy) & bishops_queens) return true;

    U64 rooks_queens = (side == white) ? (board.bitboards[R] | board.bitboards[Q])
                                        : (board.bitboards[r] | board.bitboards[q]);
    if (get_rook_attacks(square, occupancy) & rooks_queens) return true;

    if (king_attacks[square] & (side == white ? board.bitboards[K] : board.bitboards[k])) return true;

    return false;
}

static void add_pawn_promotion_moves(MoveList& list, int source, int target, int piece, bool is_capture) {
    int turn = (piece == P) ? white : black;
    int prom_q = (turn == white) ? Q : q;
    int prom_r = (turn == white) ? R : r;
    int prom_b = (turn == white) ? B : b;
    int prom_n = (turn == white) ? N : n;

    list.add(encode_move(source, target, piece, prom_q, is_capture, false, false, false));
    list.add(encode_move(source, target, piece, prom_r, is_capture, false, false, false));
    list.add(encode_move(source, target, piece, prom_b, is_capture, false, false, false));
    list.add(encode_move(source, target, piece, prom_n, is_capture, false, false, false));
}

static void generate_white_castling(const Board& board, MoveList& list, int king_sq, U64 occupancy) {
    if ((board.castle & WK) && get_bit(board.bitboards[R], h1)) {
        if (!get_bit(occupancy, f1) && !get_bit(occupancy, g1)) {
            if (!is_square_attacked(board, king_sq, black) &&
                !is_square_attacked(board, f1, black) &&
                !is_square_attacked(board, g1, black)) {
                list.add(encode_move(king_sq, g1, K, 0, false, false, false, true));
            }
        }
    }
    if ((board.castle & WQ) && get_bit(board.bitboards[R], a1)) {
        if (!get_bit(occupancy, b1) && !get_bit(occupancy, c1) && !get_bit(occupancy, d1)) {
            if (!is_square_attacked(board, king_sq, black) &&
                !is_square_attacked(board, d1, black) &&
                !is_square_attacked(board, c1, black)) {
                list.add(encode_move(king_sq, c1, K, 0, false, false, false, true));
            }
        }
    }
}

static void generate_black_castling(const Board& board, MoveList& list, int king_sq, U64 occupancy) {
    if ((board.castle & BK) && get_bit(board.bitboards[r], h8)) {
        if (!get_bit(occupancy, f8) && !get_bit(occupancy, g8)) {
            if (!is_square_attacked(board, king_sq, white) &&
                !is_square_attacked(board, f8, white) &&
                !is_square_attacked(board, g8, white)) {
                list.add(encode_move(king_sq, g8, k, 0, false, false, false, true));
            }
        }
    }
    if ((board.castle & BQ) && get_bit(board.bitboards[r], a8)) {
        if (!get_bit(occupancy, b8) && !get_bit(occupancy, c8) && !get_bit(occupancy, d8)) {
            if (!is_square_attacked(board, king_sq, white) &&
                !is_square_attacked(board, d8, white) &&
                !is_square_attacked(board, c8, white)) {
                list.add(encode_move(king_sq, c8, k, 0, false, false, false, true));
            }
        }
    }
}

void generate_moves(const Board& board, MoveList& list) {
    list.count = 0;
    int side = board.side;
    U64 occupancy = board.occupancies[both];

    int start_piece = (side == white) ? P : p;
    int end_piece   = (side == white) ? K : k;

    for (int piece = start_piece; piece <= end_piece; piece++) {
        U64 bitboard = board.bitboards[piece];

        if (side == white) {
            if (piece == P) {
                while (bitboard) {
                    int source_square = get_ls1b(bitboard);
                    int target_square = source_square + 8;

                    if (target_square <= h8 && !get_bit(occupancy, target_square)) {
                        if (source_square >= a7 && source_square <= h7) {
                            add_pawn_promotion_moves(list, source_square, target_square, piece, false);
                        } else {
                            list.add(encode_move(source_square, target_square, piece, 0, false, false, false, false));

                            if ((source_square >= a2 && source_square <= h2) &&
                                !get_bit(occupancy, target_square + 8)) {
                                list.add(encode_move(source_square, target_square + 8, piece, 0, false, true, false, false));
                            }
                        }
                    }

                    U64 attacks = pawn_attacks[white][source_square] & board.occupancies[black];
                    while (attacks) {
                        int t = get_ls1b(attacks);
                        if (source_square >= a7 && source_square <= h7)
                            add_pawn_promotion_moves(list, source_square, t, piece, true);
                        else
                            list.add(encode_move(source_square, t, piece, 0, true, false, false, false));
                        pop_bit(attacks, t);
                    }

                    if (board.enpassant != no_sq) {
                        U64 ep = pawn_attacks[side][source_square] & (1ULL << board.enpassant);
                        if (ep) {
                            int t = get_ls1b(ep);
                            list.add(encode_move(source_square, t, piece, 0, true, false, true, false));
                        }
                    }

                    pop_bit(bitboard, source_square);
                }
            }

            if (piece == K) {
                int source_square = get_ls1b(board.bitboards[K]);
                generate_white_castling(board, list, source_square, occupancy);
            }
        }
        else {
            if (piece == p) {
                while (bitboard) {
                    int source_square = get_ls1b(bitboard);
                    int target_square = source_square - 8;

                    if (target_square >= a1 && !get_bit(occupancy, target_square)) {
                        if (source_square >= a2 && source_square <= h2) {
                            add_pawn_promotion_moves(list, source_square, target_square, piece, false);
                        } else {
                            list.add(encode_move(source_square, target_square, piece, 0, false, false, false, false));

                            if ((source_square >= a7 && source_square <= h7) &&
                                !get_bit(occupancy, target_square - 8)) {
                                list.add(encode_move(source_square, target_square - 8, piece, 0, false, true, false, false));
                            }
                        }
                    }

                    U64 attacks = pawn_attacks[black][source_square] & board.occupancies[white];
                    while (attacks) {
                        int t = get_ls1b(attacks);
                        if (source_square >= a2 && source_square <= h2)
                            add_pawn_promotion_moves(list, source_square, t, piece, true);
                        else
                            list.add(encode_move(source_square, t, piece, 0, true, false, false, false));
                        pop_bit(attacks, t);
                    }

                    if (board.enpassant != no_sq) {
                        U64 ep = pawn_attacks[side][source_square] & (1ULL << board.enpassant);
                        if (ep) {
                            int t = get_ls1b(ep);
                            list.add(encode_move(source_square, t, piece, 0, true, false, true, false));
                        }
                    }

                    pop_bit(bitboard, source_square);
                }
            }

            if (piece == k) {
                int source_square = get_ls1b(board.bitboards[k]);
                generate_black_castling(board, list, source_square, occupancy);
            }
        }

        if ((side == white) ? piece == N : piece == n) {
            while (bitboard) {
                int source_square = get_ls1b(bitboard);
                U64 attacks = knight_attacks[source_square] & ~board.occupancies[side];
                while (attacks) {
                    int t = get_ls1b(attacks);
                    bool is_capture = get_bit(board.occupancies[side ^ 1], t);
                    list.add(encode_move(source_square, t, piece, 0, is_capture, false, false, false));
                    pop_bit(attacks, t);
                }
                pop_bit(bitboard, source_square);
            }
        }
        else if ((side == white && (piece == B || piece == R || piece == Q)) ||
                 (side == black && (piece == b || piece == r || piece == q))) {
            while (bitboard) {
                int source_square = get_ls1b(bitboard);
                U64 attacks;
                if (piece == B || piece == b) attacks = get_bishop_attacks(source_square, occupancy);
                else if (piece == R || piece == r) attacks = get_rook_attacks(source_square, occupancy);
                else attacks = get_queen_attacks(source_square, occupancy);

                attacks &= ~board.occupancies[side];

                while (attacks) {
                    int t = get_ls1b(attacks);
                    bool is_capture = get_bit(board.occupancies[side ^ 1], t);
                    list.add(encode_move(source_square, t, piece, 0, is_capture, false, false, false));
                    pop_bit(attacks, t);
                }
                pop_bit(bitboard, source_square);
            }
        }
        else if ((side == white) ? piece == K : piece == k) {
            while (bitboard) {
                int source_square = get_ls1b(bitboard);
                U64 attacks = king_attacks[source_square] & ~board.occupancies[side];
                while (attacks) {
                    int t = get_ls1b(attacks);
                    bool is_capture = get_bit(board.occupancies[side ^ 1], t);
                    list.add(encode_move(source_square, t, piece, 0, is_capture, false, false, false));
                    pop_bit(attacks, t);
                }
                pop_bit(bitboard, source_square);
            }
        }
    }
}

bool make_move(Board& board, Move move) {
    board.enpassant_history[board.ply] = board.enpassant;
    board.castle_history[board.ply] = board.castle;
    board.half_ply_history[board.ply] = board.half_ply;
    board.hash_key_history[board.ply] = board.hash_key;
    board.captured_piece_history[board.ply] = -1;

    NNUEDirtyPiece& dirty = board.nnue_dirty_history[board.ply];
    dirty = NNUEDirtyPiece{}; // 초기화

    int source_square = get_move_source(move);
    int target_square = get_move_target(move);
    int piece = get_move_piece(move);
    int promoted_piece = get_move_promoted(move);
    bool capture = get_move_capture(move);
    bool double_push = get_move_double(move);
    bool enpass = get_move_enpassant(move);
    bool castling = get_move_castling(move);

    if (piece == K) dirty.kingMoved[white] = true;
    if (piece == k) dirty.kingMoved[black] = true;

    if (!castling) {
        pop_bit(board.bitboards[piece], source_square);
        set_bit(board.bitboards[piece], target_square);
        dirty.remove(piece, source_square);
        dirty.add(piece, target_square);

        board.hash_key ^= piece_keys[piece][source_square];
        board.hash_key ^= piece_keys[piece][target_square];
    } else {
        int king_target, rook_target, rook_piece, rook_source;

        if (board.side == white) {
            rook_piece = R;
            if (target_square > source_square) { king_target = g1; rook_target = f1; rook_source = h1; }
            else                                { king_target = c1; rook_target = d1; rook_source = a1; }
        } else {
            rook_piece = r;
            if (target_square > source_square) { king_target = g8; rook_target = f8; rook_source = h8; }
            else                                { king_target = c8; rook_target = d8; rook_source = a8; }
        }

        pop_bit(board.bitboards[piece], source_square);
        set_bit(board.bitboards[piece], king_target);
        dirty.remove(piece, source_square);
        dirty.add(piece, king_target);
        board.hash_key ^= piece_keys[piece][source_square];
        board.hash_key ^= piece_keys[piece][king_target];

        pop_bit(board.bitboards[rook_piece], rook_source);
        set_bit(board.bitboards[rook_piece], rook_target);
        dirty.remove(rook_piece, rook_source);
        dirty.add(rook_piece, rook_target);
        board.hash_key ^= piece_keys[rook_piece][rook_source];
        board.hash_key ^= piece_keys[rook_piece][rook_target];
    }

    if (capture) {
        int sp, ep;
        if (board.side == white) { sp = p; ep = k; }
        else                     { sp = P; ep = K; }

        for (int bb_piece = sp; bb_piece <= ep; bb_piece++) {
            if (get_bit(board.bitboards[bb_piece], target_square)) {
                pop_bit(board.bitboards[bb_piece], target_square);
                dirty.remove(bb_piece, target_square);
                board.hash_key ^= piece_keys[bb_piece][target_square];
                board.captured_piece_history[board.ply] = bb_piece;
                break;
            }
        }
    }

    if (promoted_piece) {
        int pawn = (board.side == white) ? P : p;
        pop_bit(board.bitboards[pawn], target_square);
        dirty.remove(pawn, target_square);
        board.hash_key ^= piece_keys[pawn][target_square];

        set_bit(board.bitboards[promoted_piece], target_square);
        dirty.add(promoted_piece, target_square);
        board.hash_key ^= piece_keys[promoted_piece][target_square];
    }

    if (enpass) {
        int captured_sq = (board.side == white) ? target_square - 8 : target_square + 8;
        int captured_pawn = (board.side == white) ? p : P;
        pop_bit(board.bitboards[captured_pawn], captured_sq);
        dirty.remove(captured_pawn, captured_sq);
        board.hash_key ^= piece_keys[captured_pawn][captured_sq];
    }

    if (board.enpassant != no_sq)
        board.hash_key ^= enpassant_keys[board.enpassant];

    board.enpassant = no_sq;

    if (double_push) {
        int ep_sq = (board.side == white) ? target_square - 8 : target_square + 8;
        board.enpassant = ep_sq;
        board.hash_key ^= enpassant_keys[ep_sq];
    }

    board.hash_key ^= castling_keys[board.castle];
    board.castle &= castling_update_mask[source_square] & castling_update_mask[target_square];
    board.hash_key ^= castling_keys[board.castle];

    board.occupancies[white] = board.bitboards[P] | board.bitboards[N] | board.bitboards[B] |
                                board.bitboards[R] | board.bitboards[Q] | board.bitboards[K];
    board.occupancies[black] = board.bitboards[p] | board.bitboards[n] | board.bitboards[b] |
                                board.bitboards[r] | board.bitboards[q] | board.bitboards[k];
    board.occupancies[both]  = board.occupancies[white] | board.occupancies[black];

    board.side ^= 1;
    board.hash_key ^= side_key;

    if (capture || piece == p || piece == P) board.half_ply = 0;
    else board.half_ply++;

    board.ply++;

    int moved_side = board.side ^ 1;
    int king_sq = get_ls1b(moved_side == white ? board.bitboards[K] : board.bitboards[k]);

    if (is_square_attacked(board, king_sq, board.side)) {
        unmake_move(board, move);
        return false;
    }
    return true;
}

void unmake_move(Board& board, Move move) {
    board.ply--;
    board.side ^= 1;

    board.enpassant = board.enpassant_history[board.ply];
    board.castle = board.castle_history[board.ply];
    board.half_ply = board.half_ply_history[board.ply];
    board.hash_key = board.hash_key_history[board.ply];

    int captured_piece = board.captured_piece_history[board.ply];

    int source_square = get_move_source(move);
    int target_square = get_move_target(move);
    int piece = get_move_piece(move);
    int promoted_piece = get_move_promoted(move);
    bool capture = get_move_capture(move);
    bool enpass = get_move_enpassant(move);
    bool castling = get_move_castling(move);

    if (castling) {
        int k_target, r_target, rook_piece, rook_source;

        if (board.side == white) {
            rook_piece = R;
            if (target_square > source_square) { k_target = g1; r_target = f1; rook_source = h1; }
            else                                { k_target = c1; r_target = d1; rook_source = a1; }
        } else {
            rook_piece = r;
            if (target_square > source_square) { k_target = g8; r_target = f8; rook_source = h8; }
            else                                { k_target = c8; r_target = d8; rook_source = a8; }
        }

        pop_bit(board.bitboards[piece], k_target);
        set_bit(board.bitboards[piece], source_square);

        pop_bit(board.bitboards[rook_piece], r_target);
        set_bit(board.bitboards[rook_piece], rook_source);
    } else {
        if (promoted_piece) pop_bit(board.bitboards[promoted_piece], target_square);
        else                pop_bit(board.bitboards[piece], target_square);

        set_bit(board.bitboards[piece], source_square);
    }

    if (capture && !enpass) {
        set_bit(board.bitboards[captured_piece], target_square);
    }

    if (enpass) {
        if (board.side == white) set_bit(board.bitboards[p], target_square - 8);
        else                     set_bit(board.bitboards[P], target_square + 8);
    }

    board.occupancies[white] = board.bitboards[P] | board.bitboards[N] | board.bitboards[B] |
                                board.bitboards[R] | board.bitboards[Q] | board.bitboards[K];
    board.occupancies[black] = board.bitboards[p] | board.bitboards[n] | board.bitboards[b] |
                                board.bitboards[r] | board.bitboards[q] | board.bitboards[k];
    board.occupancies[both]  = board.occupancies[white] | board.occupancies[black];
}