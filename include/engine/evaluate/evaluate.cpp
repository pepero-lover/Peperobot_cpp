//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include <chrono>

#include "header/bitboard/bb_utils.h"
#include "header/board/Board.h"
#include "header/board/pieces.h"
#include <fstream>
#include "header/engine/nnue/nnue_adapter.h"

namespace Evaluate {
    /*
    // ---- Game Phases / Piece Types ----
    static constexpr int OPENING = 0;
    static constexpr int ENDGAME = 1;
    static constexpr int MIDDLEGAME = 2;

    static constexpr int PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5;

    static constexpr int OPENING_PHASE_SCORE = 6192;
    static constexpr int ENDGAME_PHASE_SCORE = 518;

    static int material_score[2][12] = {
        { 87, 337, 365, 477, 1025, 12000, -87, -337, -365, -477, -1025, -12000 },
        { 100, 281, 297, 512,  936, 12000, -100, -281, -297, -512,  -936, -12000 }
    };

    void set_material_score(int phase, int piece_type, int value) {
        material_score[phase][piece_type] = value;
        material_score[phase][piece_type + 6] = -value;
    }
    int get_material_score(int phase, int piece_type) {
        return material_score[phase][piece_type];
    }

    static int bishop_unit = 4;
    static int queen_unit = 9;

    static int bishop_pair_bonus_opening = 30;
    static int bishop_pair_bonus_endgame = 50;

    static int bishop_mobility_opening = 2;
    static int bishop_mobility_endgame = 3;
    static int queen_mobility_opening = 1;
    static int queen_mobility_endgame = 2;

    static int knight_unit = 4;
    static int knight_mobility_opening = 4;
    static int knight_mobility_endgame = 2;

    static int rook_unit = 7;
    static int rook_mobility_opening = 2;
    static int rook_mobility_endgame = 4;

    static int space_unit = 6;

    static int UNDEVELOPED_MINOR_PENALTY = 6;
    static int CENTER_PAWN_DUO_BONUS = 24;
    static int CENTRAL_FILE_HOLE_PENALTY = 18;

    // [game phase][piece][square] (PeSTO) — Java에서 그대로 복사
    static int positional_score[2][6][64] = {
        {
            { 0,0,0,0,0,0,0,0, 98,134,61,95,68,126,34,-11, -6,7,26,31,65,56,25,-20, -14,13,6,21,16,12,17,-23, -27,-2,-5,12,17,6,10,-25, -26,-4,-4,-10,3,3,33,-12, -35,-1,-20,-23,-15,24,38,-22, 0,0,0,0,0,0,0,0 },
            { -167,-89,-34,-49,61,-97,-15,-107, -73,-41,72,36,23,62,7,-17, -47,60,37,65,84,129,73,44, -9,17,19,53,37,69,18,22, -13,4,16,13,28,19,21,-8, -23,-9,12,10,19,17,25,-16, -29,-53,-12,-3,-1,18,-14,-19, -105,-21,-58,-33,-17,-28,-19,-23 },
            { -29,4,-82,-37,-25,-42,7,-8, -26,16,-18,-13,30,59,18,-47, -16,37,43,40,35,50,37,-2, -4,5,19,50,37,37,7,-2, -6,13,13,26,34,12,10,4, 0,15,15,15,14,27,18,10, 4,15,16,0,7,21,33,1, -33,-3,-14,-21,-13,-12,-39,-21 },
            { 32,42,32,51,63,9,31,43, 27,32,58,62,80,67,26,44, -5,19,26,36,17,45,61,16, -24,-11,7,26,24,35,-8,-20, -36,-26,-12,-1,9,-7,6,-23, -45,-25,-16,-17,3,0,-5,-33, -44,-16,-20,-9,-1,11,-6,-71, -19,-13,1,17,16,7,-37,-26 },
            { -28,0,29,12,59,44,43,45, -24,-39,-5,1,-16,57,28,54, -13,-17,7,8,29,56,47,57, -27,-27,-16,-16,-1,17,-2,1, -9,-26,-9,-10,-2,-4,3,-3, -14,2,-11,-2,-5,2,14,5, -35,-8,11,2,8,15,-3,1, -1,-18,-9,10,-15,-25,-31,-50 },
            { -65,23,16,-15,-56,-34,2,13, 29,-1,-20,-7,-8,-4,-38,-29, -9,24,2,-16,-20,6,22,-22, -17,-20,-12,-27,-30,-25,-14,-36, -49,-1,-27,-39,-46,-44,-33,-51, -14,-14,-22,-46,-44,-30,-15,-27, 1,7,-8,-64,-43,-16,9,8, -15,36,12,-54,8,-28,24,14 }
        },
        {
            { 0,0,0,0,0,0,0,0, 178,173,158,134,147,132,165,187, 94,100,85,67,56,53,82,84, 32,24,13,5,-2,4,17,17, 13,9,-3,-7,-7,-8,3,-1, 4,7,-6,1,0,-5,-1,-8, 13,8,8,10,13,0,2,-7, 0,0,0,0,0,0,0,0 },
            { -58,-38,-13,-28,-31,-27,-63,-99, -25,-8,-25,-2,-9,-25,-24,-52, -24,-20,10,9,-1,-9,-19,-41, -17,3,22,22,22,11,8,-18, -18,-6,16,25,16,17,4,-18, -23,-3,-1,15,10,-3,-20,-22, -42,-20,-10,-5,-2,-20,-23,-44, -29,-51,-23,-15,-22,-18,-50,-64 },
            { -14,-21,-11,-8,-7,-9,-17,-24, -8,-4,7,-12,-3,-13,-4,-14, 2,-8,0,-1,-2,6,0,4, -3,9,12,9,14,10,3,2, -6,3,13,19,7,10,-3,-9, -12,-3,8,10,13,3,-7,-15, -14,-18,-7,-1,4,-9,-15,-27, -23,-9,-23,-5,-9,-16,-5,-17 },
            { 13,10,18,15,12,12,8,5, 11,13,13,11,-3,3,8,3, 7,7,7,5,4,-3,-5,-3, 4,3,13,1,2,1,-1,2, 3,5,8,4,-5,-6,-8,-11, -4,0,-5,-1,-7,-12,-8,-16, -6,-6,0,2,-9,-9,-11,-3, -9,2,3,-1,-5,-13,4,-20 },
            { -9,22,22,27,27,19,10,20, -17,20,32,41,58,25,30,0, -20,6,9,49,47,35,19,9, 3,22,24,45,57,40,57,36, -18,28,19,47,31,34,39,23, -16,-27,15,6,9,17,10,5, -22,-23,-30,-16,-16,-23,-36,-32, -33,-28,-22,-43,-5,-32,-20,-41 },
            { -74,-35,-18,-18,-11,15,4,-17, -12,17,14,17,17,38,23,11, 10,17,23,15,20,45,44,13, -8,22,24,27,26,33,26,3, -18,-4,21,24,27,23,9,-11, -19,-3,11,21,23,16,7,-9, -27,-11,4,13,14,4,-5,-17, -53,-34,-21,-11,-28,-14,-24,-43 }
        }
    };

    static U64 file_masks[64];
    static U64 rank_masks[64];
    static U64 isolated_masks[64];
    static U64 white_passed_masks[64];
    static U64 black_passed_masks[64];

    static constexpr int double_pawn_penalty_opening = -5;
    static constexpr int double_pawn_penalty_endgame = -10;
    static constexpr int isolated_pawn_penalty_opening = -5;
    static constexpr int isolated_pawn_penalty_endgame = -10;

    static constexpr int passed_pawn_bonus_opening[8] = { 0, 5, 8, 12, 20, 30, 50, 70 };
    static constexpr int passed_pawn_bonus_endgame[8] = { 0, 10, 30, 50, 75, 100, 150, 200 };

    static constexpr int semi_open_file_score = 10;
    static constexpr int open_file_score = 15;

    static constexpr int SAFETY_TABLE[100] = {
        0,0,1,2,3,5,7,9,12,15,
        18,22,26,30,35,39,44,50,56,62,
        68,75,82,89,97,105,113,122,131,140,
        150,160,170,181,192,203,215,227,239,252,
        265,278,292,306,320,335,350,366,382,398,
        415,432,450,468,487,506,526,546,567,588,
        610,632,655,678,702,726,751,776,802,828,
        855,882,910,938,967,996,1026,1056,1087,1118,
        1150,1182,1215,1248,1282,1316,1351,1386,1422,1458,
        1495,1532,1570,1608,1647,1686,1726,1766,1807,1848
    };

    static U64 set_file_rank_mask(int file_number, int rank_number) {
        U64 mask = 0ULL;
        for (int rank = 0; rank < 8; rank++) {
            for (int file = 0; file < 8; file++) {
                int square = rank * 8 + file;
                if (file_number != -1 && file == file_number) {
                    mask |= (1ULL << square);
                } else if (rank_number != -1 && rank == rank_number) {
                    mask |= (1ULL << square);
                }
            }
        }
        return mask;
    }

    static void init_evaluation_masks() {
        for (int rank = 0; rank < 8; rank++) {
            for (int file = 0; file < 8; file++) {
                int square = rank * 8 + file;
                file_masks[square] |= set_file_rank_mask(file, -1);
                rank_masks[square] |= set_file_rank_mask(-1, rank);
                isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
                isolated_masks[square] |= set_file_rank_mask(file + 1, -1);
            }
        }

        for (int rank = 0; rank < 8; rank++) {
            for (int file = 0; file < 8; file++) {
                int square = rank * 8 + file;

                white_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
                white_passed_masks[square] |= set_file_rank_mask(file, -1);
                white_passed_masks[square] |= set_file_rank_mask(file + 1, -1);
                for (int i = 0; i < rank + 1; i++) {
                    white_passed_masks[square] &= ~rank_masks[i * 8 + file];
                }

                black_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
                black_passed_masks[square] |= set_file_rank_mask(file, -1);
                black_passed_masks[square] |= set_file_rank_mask(file + 1, -1);
                for (int i = 0; i < (8 - rank); i++) {
                    black_passed_masks[square] &= ~rank_masks[(7 - i) * 8 + file];
                }
            }
        }
    }

    // 프로그램 시작 시(main 이전) 1회 초기화
    static bool _masks_init_done = [](){ init_evaluation_masks(); return true; }();

    static int get_game_phase_score(const Board& board) {
        int white_piece_scores = 0, black_piece_scores = 0;

        for (int piece = N; piece <= Q; piece++)
            white_piece_scores += count_bit(board.bitboards[piece]) * material_score[OPENING][piece];
        for (int piece = n; piece <= q; piece++)
            black_piece_scores += count_bit(board.bitboards[piece]) * -material_score[OPENING][piece];

        return white_piece_scores + black_piece_scores;
    }

    static int get_total_material(const Board& board, bool is_white) {
        if (is_white) {
            return count_bit(board.bitboards[P]) * 100
                 + count_bit(board.bitboards[N]) * 320
                 + count_bit(board.bitboards[B]) * 330
                 + count_bit(board.bitboards[R]) * 500
                 + count_bit(board.bitboards[Q]) * 900;
        } else {
            return count_bit(board.bitboards[p]) * 100
                 + count_bit(board.bitboards[n]) * 320
                 + count_bit(board.bitboards[b]) * 330
                 + count_bit(board.bitboards[r]) * 500
                 + count_bit(board.bitboards[q]) * 900;
        }
    }

    static int get_non_pawn_material(const Board& board, bool is_white) {
        if (is_white) {
            return count_bit(board.bitboards[N]) * 320
                 + count_bit(board.bitboards[B]) * 330
                 + count_bit(board.bitboards[R]) * 500
                 + count_bit(board.bitboards[Q]) * 900;
        } else {
            return count_bit(board.bitboards[n]) * 320
                 + count_bit(board.bitboards[b]) * 330
                 + count_bit(board.bitboards[r]) * 500
                 + count_bit(board.bitboards[q]) * 900;
        }
    }

    static int evaluate_king_safety(const Board& board, int king_square, int side) {
        int penalty = 0;
        U64 king_file = file_masks[king_square];
        int king_file_idx = king_square % 8;

        U64 own_pawns    = board.bitboards[(side == white) ? P : p];
        U64 enemy_pawns  = board.bitboards[(side == white) ? p : P];
        U64 enemy_queen  = board.bitboards[(side == white) ? q : Q];
        U64 enemy_rooks  = board.bitboards[(side == white) ? r : R];
        U64 enemy_knights= board.bitboards[(side == white) ? n : N];
        U64 enemy_bishops= board.bitboards[(side == white) ? b : B];

        bool is_center = (king_file_idx == 3 || king_file_idx == 4);

        int king_rank_idx = king_square / 8;
        int rank_step = (side == white) ? 1 : -1;

        U64 immediate_shield_zone = 0ULL;
        U64 extended_shield_zone  = 0ULL;

        for (int df = -1; df <= 1; df++) {
            int f = king_file_idx + df;
            if (f < 0 || f > 7) continue;

            int r1 = king_rank_idx + rank_step;
            if (r1 >= 0 && r1 <= 7) immediate_shield_zone |= (1ULL << (r1 * 8 + f));

            int r2 = king_rank_idx + rank_step * 2;
            if (r2 >= 0 && r2 <= 7) extended_shield_zone |= (1ULL << (r2 * 8 + f));
        }

        int immediate_pawns = count_bit(own_pawns & immediate_shield_zone);
        int extended_pawns  = count_bit(own_pawns & extended_shield_zone);

        int shield_score = immediate_pawns * 2 + extended_pawns;

        if (!is_center) {
            if (shield_score < 3) penalty += 140;
            else if (shield_score < 5) penalty += 40;
        }

        U64 enemy_heavy_pieces = enemy_queen | enemy_rooks;
        bool is_file_attacked = (enemy_heavy_pieces & king_file) != 0;

        if ((own_pawns & king_file) == 0) {
            int file_penalty = is_center ? 30 : 60;
            if ((enemy_pawns & king_file) == 0) {
                file_penalty += is_center ? 40 : 100;
            }
            if (enemy_heavy_pieces == 0) {
                file_penalty = 0;
            } else if (!is_file_attacked) {
                file_penalty /= 2;
            }
            penalty += file_penalty;
        }

        int king_rank = king_square / 8;
        U64 both_occ = board.occupancies[both];

        U64 king_zone = king_attacks[king_square] | (1ULL << king_square);

        int attack_weight = 0;
        int attackers_count = 0;
        int q_count = 0;

        U64 knights_bb = enemy_knights;
        while (knights_bb) {
            int sq = get_ls1b(knights_bb);
            if ((knight_attacks[sq] & king_zone) != 0) {
                attackers_count++;
                attack_weight += 15;
            }
            pop_bit(knights_bb, sq);
        }

        U64 bishops_bb = enemy_bishops;
        while (bishops_bb) {
            int sq = get_ls1b(bishops_bb);
            if ((get_bishop_attacks(sq, both_occ) & king_zone) != 0) {
                attackers_count++;
                attack_weight += 15;
            }
            pop_bit(bishops_bb, sq);
        }

        U64 rooks_bb = enemy_rooks;
        while (rooks_bb) {
            int sq = get_ls1b(rooks_bb);
            if ((get_rook_attacks(sq, both_occ) & king_zone) != 0) {
                attackers_count++;
                attack_weight += 25;
            }
            pop_bit(rooks_bb, sq);
        }

        U64 queens_bb = enemy_queen;
        while (queens_bb) {
            int sq = get_ls1b(queens_bb);
            if ((get_queen_attacks(sq, both_occ) & king_zone) != 0) {
                attackers_count++;
                q_count++;
                attack_weight += 40;
            }
            pop_bit(queens_bb, sq);
        }

        if (attackers_count >= 2 || q_count > 0) {
            if (shield_score == 0) attack_weight = (attack_weight * 3) / 2;
            else if (shield_score <= 2) attack_weight = (attack_weight * 5) / 4;

            int rank_distance = (side == white) ? king_rank : (7 - king_rank);
            if (rank_distance >= 1) attack_weight += (rank_distance * 25);

            attack_weight = std::min(attack_weight, 99);
            int current_penalty = SAFETY_TABLE[attack_weight];

            if (q_count == 0) current_penalty = std::min(current_penalty / 2, 150);

            penalty += current_penalty;
        }

        return penalty >> 2;
    }

    int get_king_safety_penalty(const Board& board, int side) {
        int king_square = get_ls1b(board.bitboards[side == white ? K : k]);
        return evaluate_king_safety(board, king_square, side); // forward decl below
    }

    static int evaluate_space(const Board& board) {
        U64 center_files = file_masks[c1] | file_masks[d1] | file_masks[e1] | file_masks[f1];

        int white_space = 0;
        U64 white_center_pawns = board.bitboards[P] & center_files;
        while (white_center_pawns) {
            int sq = get_ls1b(white_center_pawns);
            int ranks_advanced = (sq / 8) - 1;
            if (ranks_advanced > 0) white_space += ranks_advanced;
            pop_bit(white_center_pawns, sq);
        }

        int black_space = 0;
        U64 black_center_pawns = board.bitboards[p] & center_files;
        while (black_center_pawns) {
            int sq = get_ls1b(black_center_pawns);
            int ranks_advanced = 6 - (sq / 8);
            if (ranks_advanced > 0) black_space += ranks_advanced;
            pop_bit(black_center_pawns, sq);
        }

        return (white_space - black_space) * space_unit;
    }

    static int evaluate_development_and_center(const Board& board) {
        int score = 0;

        U64 white_home_minors = (1ULL << 1) | (1ULL << 6) | (1ULL << 2) | (1ULL << 5); // b1,g1,c1,f1
        U64 white_still_home = (board.bitboards[N] | board.bitboards[B]) & white_home_minors;
        score -= count_bit(white_still_home) * UNDEVELOPED_MINOR_PENALTY;

        U64 black_home_minors = (1ULL << 57) | (1ULL << 62) | (1ULL << 58) | (1ULL << 61); // b8,g8,c8,f8
        U64 black_still_home = (board.bitboards[n] | board.bitboards[b]) & black_home_minors;
        score += count_bit(black_still_home) * UNDEVELOPED_MINOR_PENALTY;

        // Java의 numberOfLeadingZeros(63-...) 대신, MSB index 함수로 대체
        auto msb_index = [](U64 bb) {
            return 63 - __builtin_clzll(bb);
        };

        // a1=0 좌표계에서는 "가장 전진한 폰"이 백은 MSB(랭크가 클수록 전진), 흑은 LS1B(랭크가 작을수록 전진)이다.
        U64 white_d_file = board.bitboards[P] & file_masks[d1];
        U64 white_e_file = board.bitboards[P] & file_masks[e1];
        bool white_duo = white_d_file != 0 && white_e_file != 0
                          && (msb_index(white_d_file) / 8) >= 3
                          && (msb_index(white_e_file) / 8) >= 3;
        if (white_duo) score += CENTER_PAWN_DUO_BONUS;

        U64 black_d_file = board.bitboards[p] & file_masks[d1];
        U64 black_e_file = board.bitboards[p] & file_masks[e1];
        bool black_duo = black_d_file != 0 && black_e_file != 0
                          && (get_ls1b(black_d_file) / 8) <= 4
                          && (get_ls1b(black_e_file) / 8) <= 4;
        if (black_duo) score -= CENTER_PAWN_DUO_BONUS;

        int white_non_pawn_material = get_non_pawn_material(board, true);
        int black_non_pawn_material = get_non_pawn_material(board, false);

        bool white_no_d_pawn = (board.bitboards[P] & file_masks[d1]) == 0;
        bool white_no_e_pawn = (board.bitboards[P] & file_masks[e1]) == 0;
        int white_hole_penalty = (white_non_pawn_material >= black_non_pawn_material)
                                  ? CENTRAL_FILE_HOLE_PENALTY / 3 : CENTRAL_FILE_HOLE_PENALTY;
        if (white_no_d_pawn) score -= white_hole_penalty;
        if (white_no_e_pawn) score -= white_hole_penalty;

        bool black_no_d_pawn = (board.bitboards[p] & file_masks[d1]) == 0;
        bool black_no_e_pawn = (board.bitboards[p] & file_masks[e1]) == 0;
        int black_hole_penalty = (black_non_pawn_material >= white_non_pawn_material)
                                  ? CENTRAL_FILE_HOLE_PENALTY / 3 : CENTRAL_FILE_HOLE_PENALTY;
        if (black_no_d_pawn) score += black_hole_penalty;
        if (black_no_e_pawn) score += black_hole_penalty;

        return score;
    }

    RawScores compute_raw_scores(const Board& board) {
        int game_phase_score = get_game_phase_score(board);
        int game_phase;

        if (game_phase_score > OPENING_PHASE_SCORE) game_phase = OPENING;
        else if (game_phase_score < ENDGAME_PHASE_SCORE) game_phase = ENDGAME;
        else game_phase = MIDDLEGAME;

        int score_opening = 0, score_endgame = 0;
        int double_pawns;

        U64 white_occupancies = 0ULL, black_occupancies = 0ULL;
        for (int i = P; i <= K; i++) white_occupancies |= board.bitboards[i];
        for (int i = p; i <= k; i++) black_occupancies |= board.bitboards[i];
        U64 both_occupancies = white_occupancies | black_occupancies;

        for (int bb_piece = P; bb_piece <= k; bb_piece++) {
            U64 bitboard = board.bitboards[bb_piece];

            while (bitboard) {
                int square = get_ls1b(bitboard);

                score_opening += material_score[OPENING][bb_piece];
                score_endgame += material_score[ENDGAME][bb_piece];

                switch (bb_piece) {
                    case P: {
                        score_opening += positional_score[OPENING][PAWN][square ^ 56];
                        score_endgame += positional_score[ENDGAME][PAWN][square ^ 56];

                        double_pawns = count_bit(board.bitboards[P] & file_masks[square]);
                        if (double_pawns > 1) {
                            score_opening += (double_pawns - 1) * double_pawn_penalty_opening;
                            score_endgame += (double_pawns - 1) * double_pawn_penalty_endgame;
                        }
                        if ((board.bitboards[P] & isolated_masks[square]) == 0) {
                            score_opening += isolated_pawn_penalty_opening;
                            score_endgame += isolated_pawn_penalty_endgame;
                        }
                        if ((white_passed_masks[square] & board.bitboards[p]) == 0) {
                            int rank = square / 8;
                            score_opening += passed_pawn_bonus_opening[rank];
                            score_endgame += passed_pawn_bonus_endgame[rank];
                        }
                        break;
                    }
                    case N: {
                        score_opening += positional_score[OPENING][KNIGHT][square ^ 56];
                        score_endgame += positional_score[ENDGAME][KNIGHT][square ^ 56];

                        U64 att = knight_attacks[square] & ~board.occupancies[white];
                        score_opening += (count_bit(att) - knight_unit) * knight_mobility_opening;
                        score_endgame += (count_bit(att) - knight_unit) * knight_mobility_endgame;
                        break;
                    }
                    case B: {
                        score_opening += positional_score[OPENING][BISHOP][square ^ 56];
                        score_endgame += positional_score[ENDGAME][BISHOP][square ^ 56];

                        U64 att = get_bishop_attacks(square, both_occupancies);
                        score_opening += (count_bit(att) - bishop_unit) * bishop_mobility_opening;
                        score_endgame += (count_bit(att) - bishop_unit) * bishop_mobility_endgame;
                        if (square == d3 && (board.bitboards[P] & (1ULL << d2)) != 0) {
                            score_opening -= 45; score_endgame -= 30;
                        }
                        if (square == e3 && (board.bitboards[P] & (1ULL << e2)) != 0) {
                            score_opening -= 45; score_endgame -= 30;
                        }
                        break;
                    }
                    case R: {
                        score_opening += positional_score[OPENING][ROOK][square ^ 56];
                        score_endgame += positional_score[ENDGAME][ROOK][square ^ 56];
                        if ((board.bitboards[P] & file_masks[square]) == 0) {
                            score_opening += semi_open_file_score;
                            score_endgame += semi_open_file_score;
                        }
                        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
                            score_opening += open_file_score;
                            score_endgame += open_file_score;
                        }
                        U64 att = get_rook_attacks(square, both_occupancies) & ~board.occupancies[white];
                        score_opening += (count_bit(att) - rook_unit) * rook_mobility_opening;
                        score_endgame += (count_bit(att) - rook_unit) * rook_mobility_endgame;
                        break;
                    }
                    case Q: {
                        score_opening += positional_score[OPENING][QUEEN][square ^ 56];
                        score_endgame += positional_score[ENDGAME][QUEEN][square ^ 56];

                        U64 att = get_queen_attacks(square, both_occupancies);
                        score_opening += (count_bit(att) - queen_unit) * queen_mobility_opening;
                        score_endgame += (count_bit(att) - queen_unit) * queen_mobility_endgame;
                        break;
                    }
                    case K: {
                        score_opening += positional_score[OPENING][KING][square ^ 56];
                        score_endgame += positional_score[ENDGAME][KING][square ^ 56];
                        score_opening -= evaluate_king_safety(board, square, white);
                        break;
                    }
                    case p: {
                        score_opening -= positional_score[OPENING][PAWN][square];
                        score_endgame -= positional_score[ENDGAME][PAWN][square];

                        double_pawns = count_bit(board.bitboards[p] & file_masks[square]);
                        if (double_pawns > 1) {
                            score_opening -= (double_pawns - 1) * double_pawn_penalty_opening;
                            score_endgame -= (double_pawns - 1) * double_pawn_penalty_endgame;
                        }
                        if ((board.bitboards[p] & isolated_masks[square]) == 0) {
                            score_opening -= isolated_pawn_penalty_opening;
                            score_endgame -= isolated_pawn_penalty_endgame;
                        }
                        if ((black_passed_masks[square] & board.bitboards[P]) == 0) {
                            int rank = 7 - (square / 8);
                            score_opening -= passed_pawn_bonus_opening[rank];
                            score_endgame -= passed_pawn_bonus_endgame[rank];
                        }
                        break;
                    }
                    case n: {
                        score_opening -= positional_score[OPENING][KNIGHT][square];
                        score_endgame -= positional_score[ENDGAME][KNIGHT][square];

                        U64 att = knight_attacks[square] & ~board.occupancies[black];
                        score_opening -= (count_bit(att) - knight_unit) * knight_mobility_opening;
                        score_endgame -= (count_bit(att) - knight_unit) * knight_mobility_endgame;
                        break;
                    }
                    case b: {
                        score_opening -= positional_score[OPENING][BISHOP][square];
                        score_endgame -= positional_score[ENDGAME][BISHOP][square];

                        U64 att = get_bishop_attacks(square, both_occupancies);
                        score_opening -= (count_bit(att) - bishop_unit) * bishop_mobility_opening;
                        score_endgame -= (count_bit(att) - bishop_unit) * bishop_mobility_endgame;
                        if (square == d6 && (board.bitboards[p] & (1ULL << d7)) != 0) {
                            score_opening += 45; score_endgame += 30;
                        }
                        if (square == e6 && (board.bitboards[p] & (1ULL << e7)) != 0) {
                            score_opening += 45; score_endgame += 30;
                        }
                        break;
                    }
                    case r: {
                        score_opening -= positional_score[OPENING][ROOK][square];
                        score_endgame -= positional_score[ENDGAME][ROOK][square];

                        if ((board.bitboards[p] & file_masks[square]) == 0) {
                            score_opening -= semi_open_file_score;
                            score_endgame -= semi_open_file_score;
                        }
                        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
                            score_opening -= open_file_score;
                            score_endgame -= open_file_score;
                        }
                        U64 att = get_rook_attacks(square, both_occupancies) & ~board.occupancies[black];
                        score_opening -= (count_bit(att) - rook_unit) * rook_mobility_opening;
                        score_endgame -= (count_bit(att) - rook_unit) * rook_mobility_endgame;
                        break;
                    }
                    case q: {
                        score_opening -= positional_score[OPENING][QUEEN][square];
                        score_endgame -= positional_score[ENDGAME][QUEEN][square];

                        U64 att = get_queen_attacks(square, both_occupancies);
                        score_opening -= (count_bit(att) - queen_unit) * queen_mobility_opening;
                        score_endgame -= (count_bit(att) - queen_unit) * queen_mobility_endgame;
                        break;
                    }
                    case k: {
                        score_opening -= positional_score[OPENING][KING][square];
                        score_endgame -= positional_score[ENDGAME][KING][square];

                        if ((board.bitboards[p] & file_masks[square]) == 0) {
                            score_opening += semi_open_file_score;
                            score_endgame += semi_open_file_score;
                        }
                        if (((board.bitboards[P] | board.bitboards[p]) & file_masks[square]) == 0) {
                            score_opening += open_file_score;
                            score_endgame += open_file_score;
                        }
                        score_opening += evaluate_king_safety(board, square, black);
                        break;
                    }
                }

                pop_bit(bitboard, square);
            }
        }

        if (count_bit(board.bitboards[B]) >= 2) {
            score_opening += bishop_pair_bonus_opening;
            score_endgame += bishop_pair_bonus_endgame;
        }
        if (count_bit(board.bitboards[b]) >= 2) {
            score_opening -= bishop_pair_bonus_opening;
            score_endgame -= bishop_pair_bonus_endgame;
        }

        int structural_bonus = evaluate_space(board) + evaluate_development_and_center(board);
        int material_diff = get_total_material(board, true) - get_total_material(board, false);

        if (material_diff > 0 && structural_bonus < 0) structural_bonus /= 2;
        else if (material_diff < 0 && structural_bonus > 0) structural_bonus /= 2;

        score_opening += structural_bonus;

        RawScores r;
        r.score_opening = score_opening;
        r.score_endgame = score_endgame;
        r.game_phase_score = game_phase_score;
        r.game_phase = game_phase;
        return r;
    }

    int evaluate(const Board& board) {
        RawScores r = compute_raw_scores(board);
        int score;

        if (r.game_phase == MIDDLEGAME) {
            score = (r.score_opening * r.game_phase_score
                    + r.score_endgame * (OPENING_PHASE_SCORE - r.game_phase_score)) / OPENING_PHASE_SCORE;
        } else if (r.game_phase == OPENING) {
            score = r.score_opening;
        } else {
            score = r.score_endgame;
        }

        return (board.side == white) ? score : -score;
    }*/

    NNUEAdapter::MyFeatureTransformer g_featureTransformer;
    NNUEAdapter::Network g_networks[NNUEAdapter::LayerStacks];

    namespace {
        bool read_header(std::istream& stream, std::uint32_t* version,
                          std::uint32_t* hashValue, std::string* architecture) {
            *version   = Stockfish::Eval::NNUE::read_little_endian<std::uint32_t>(stream);
            *hashValue = Stockfish::Eval::NNUE::read_little_endian<std::uint32_t>(stream);
            std::uint32_t size = Stockfish::Eval::NNUE::read_little_endian<std::uint32_t>(stream);
            if (!stream || size == 0) return false;
            architecture->resize(size);
            stream.read(&(*architecture)[0], size);
            return bool(stream);
        }
    }

    bool load_nnue(const std::string& path) {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) return false;

        std::uint32_t version, hashValue;
        std::string architecture;
        if (!read_header(stream, &version, &hashValue, &architecture)) return false;

        // FeatureTransformer 해시(4바이트) 스킵
        Stockfish::Eval::NNUE::read_little_endian<std::uint32_t>(stream);
        if (!g_featureTransformer.read_parameters(stream)) return false;

        // 각 Network 앞에 있는 해시(4바이트) 스킵
        for (auto& net : g_networks) {
            Stockfish::Eval::NNUE::read_little_endian<std::uint32_t>(stream);
            if (!net.read_parameters(stream)) return false;
        }

        return true;
    }

    void nnue_refresh_root(const Board& board) {
        NNUEAdapter::nnue_refresh_root(board, g_featureTransformer);
    }

    void nnue_do_move(const Board& board) {
        NNUEAdapter::nnue_do_move(board, g_featureTransformer);
    }

    constexpr int NormalizeToPawnValue = 361;

    int to_display_cp(int internal_value) {
        return internal_value * 100 / NormalizeToPawnValue;
    }

    int evaluate(const Board& board) {
        return NNUEAdapter::evaluate(board, g_networks);
    }
}
