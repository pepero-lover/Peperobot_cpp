//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "search.h"
#include "header/engine/search.h"

#include "header/board/Board.h"
#include "header/movegen/move.h"
#include "header/movegen/movegen.h"
#include "header/bitboard/bb_utils.h"
#include "header/hash/zobrist.h"
#include "header/bitboard/constants.h"
#include "header/bitboard/squares.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <random>
#include <cstring>

#include "header/board/pieces.h"
#include "header/engine/search_constants.h"
#include "header/engine/tt.h"
#include "header/engine/evaluate/evaluate.h"
#include "header/engine/evaluate/score_move.h"
#include "header/engine/uci/time_control.h"
#include "header/engine/uci/uci_manager.h"
#include "header/engine/evaluate/see.h"
#include "header/time/time_utils.h"

using namespace SearchConst;

int Search::CorrHistMaxCp = 64;
int Search::CorrHistWeightScale = 256;
int Search::CorrHistMinDepth = 4;

inline bool is_repetition_draw(const Board& board, int root_ply) {
    int start = board.ply - board.half_ply;
    if (start < 0) start = 0;

    int count = 0;
    for (int i = board.ply - 2; i >= start; i -= 2) {
        if (board.hash_key_history[i] == board.hash_key) {
            count++;
            if (i >= root_ply || count >= 2) return true;
        }
    }
    return false;
}

static constexpr int VAL_WINDOW = 150;
static constexpr int FULL_DEPTH_MOVES = 3;
static constexpr int REDUCTION_LIMIT = 3;
static constexpr int FUTILITY_MARGIN[4] = { 0, 200, 300, 500 };
static constexpr int DELTA_MARGIN = 200;

// ---- singular extension ----
static constexpr int SE_MIN_DEPTH = 7;         // 이 depth 이상에서만 시도
static constexpr int SE_TT_DEPTH_MARGIN = 3;   // TT 저장 depth가 (depth - 이 값) 이상이어야 신뢰
static constexpr int SE_MARGIN_PER_DEPTH = 2;  // singular_beta = tt_score - depth * 이 값
static constexpr int SE_DOUBLE_EXT_MARGIN = 20; // 이만큼 더 낮으면 double extension

static int reduction_table[64][64];
static bool _reduction_table_init = [](){
    for (int depth = 1; depth < 64; depth++)
        for (int move_count = 1; move_count < 64; move_count++)
            reduction_table[depth][move_count] =
                (int)(0.5 + std::log(depth) * std::log(move_count) / 1.95);
    return true;
}();

// 폰 구조 전용 zobrist (correction history 용)
static U64 PAWN_HASH_KEYS[2][64];
static bool _pawn_keys_init = [](){
    std::mt19937_64 rng(0x9E3779B97F4A7C15ULL);
    for (int side = 0; side < 2; side++)
        for (int sq = 0; sq < 64; sq++)
            PAWN_HASH_KEYS[side][sq] = rng();
    return true;
}();

static U64 compute_pawn_key(const Board& board) {
    U64 key = 0ULL;

    U64 wp = board.bitboards[P];
    while (wp) {
        int sq = get_ls1b(wp);
        key ^= PAWN_HASH_KEYS[0][sq];
        pop_bit(wp, sq);
    }

    U64 bp = board.bitboards[p];
    while (bp) {
        int sq = get_ls1b(bp);
        key ^= PAWN_HASH_KEYS[1][sq];
        pop_bit(bp, sq);
    }

    return key;
}

static int apply_pawn_correction(Search& s, int side, int corr_index, int raw_static_eval) {
    return raw_static_eval + s.pawn_corr_hist[side][corr_index] / Search::CORR_HIST_GRAIN;
}

static void update_pawn_corr_hist(Search& s, int side, int corr_index,
                                   int best_score, int raw_static_eval, int depth) {
    if (std::abs(best_score) >= MATE_SCORE) return;
    if (depth < Search::CorrHistMinDepth) return;

    int corr_hist_max = Search::CorrHistMaxCp * Search::CORR_HIST_GRAIN;
    if (corr_hist_max <= 0) return;

    int diff = best_score - raw_static_eval;

    long long raw_bonus = (long long)diff * depth * Search::CORR_HIST_GRAIN
                            / std::max(1, Search::CorrHistWeightScale);
    int bonus = (int)std::max((long long)-corr_hist_max, std::min((long long)corr_hist_max, raw_bonus));

    int current = s.pawn_corr_hist[side][corr_index];
    s.pawn_corr_hist[side][corr_index] =
        current + bonus - current * std::abs(bonus) / corr_hist_max;
}

static int history_bonus(int depth) {
    return std::min(1200, 16 * depth * depth);
}

static void update_quiet_history(Search& s, int move, int bonus) {
    int piece = get_move_piece(move);
    int to = get_move_target(move);

    int clamped = std::max(-Search::MAX_HISTORY, std::min(Search::MAX_HISTORY, bonus));
    int current = s.history_moves[piece][to];

    s.history_moves[piece][to] = current + clamped - current * std::abs(clamped) / Search::MAX_HISTORY;
}

static void update_cont_hist_entry(int table[][768], int context_idx, int move_idx, int bonus) {
    if (context_idx < 0) return;

    int clamped = std::max(-Search::MAX_HISTORY, std::min(Search::MAX_HISTORY, bonus));
    int current = table[context_idx][move_idx];

    table[context_idx][move_idx] = current + clamped - current * std::abs(clamped) / Search::MAX_HISTORY;
}

static void apply_cont_hist_update(Search& s, int ply_index, int move, int bonus) {
    int move_idx = Search::move_index(get_move_piece(move), get_move_target(move));

    if (ply_index >= 1)
        update_cont_hist_entry(s.cont_hist_1, s.played_index_at_ply[ply_index], move_idx, bonus);
    if (ply_index >= 2)
        update_cont_hist_entry(s.cont_hist_2, s.played_index_at_ply[ply_index - 1], move_idx, bonus);
}

static void apply_quiet_history_update(Search& s, int ply_index, int best_quiet_move, int bonus) {
    update_quiet_history(s, best_quiet_move, bonus);
    apply_cont_hist_update(s, ply_index, best_quiet_move, bonus);

    int qcount = s.quiet_count_at_ply[ply_index];
    for (int i = 0; i < qcount; i++) {
        int qm = s.quiet_moves_at_ply[ply_index][i];
        if (qm != best_quiet_move) {
            update_quiet_history(s, qm, -bonus);
            apply_cont_hist_update(s, ply_index, qm, -bonus);
        }
    }
}

// ---------------------------------------------------------------------

static int quiescence(Search& s, Board& board, int alpha, int beta) {
    if (TimeControl::stopped) return 0;

    if (s.thread_id == 0 && (s.nodes & 2047) == 0) {
        communicate();
        if (TimeControl::stopped) return 0;
    }

    s.nodes++;

    if (s.ply >= SearchConst::MAX_PLY) {
        return Evaluate::evaluate(board);
    }

    int alpha_orig = alpha;

    int tt_score = TT::read_hash_entry(board, alpha, beta, 0, s.ply);
    if (tt_score != TT::NO_HASH_ENTRY) {
        return tt_score;
    }

    int hash_move = TT::read_hash_move(board);

    bool in_check = is_square_attacked(board,
        get_ls1b(board.bitboards[board.side == white ? K : k]),
        board.side ^ 1);

    int evaluation = Evaluate::evaluate(board);

    if (!in_check) {
        U64 qs_pawn_key = compute_pawn_key(board);
        int qs_corr_index = (int)(qs_pawn_key & (Search::CORR_HIST_SIZE - 1));
        evaluation = apply_pawn_correction(s, board.side, qs_corr_index, evaluation);
    }

    if (!in_check) {
        if (evaluation >= beta) {
            TT::write_hash_entry(board, beta, 0, TT::HASH_FLAG_BETA, 0, s.ply);
            return beta;
        }
        if (evaluation > alpha) alpha = evaluation;
    }

    MoveList list;
    generate_moves(board, list);

    if (in_check) {
        score_moves(s, board, list, hash_move);
    } else {
        score_quiescence_moves(s, board, list, hash_move);
    }

    int legal_moves = 0;
    int tt_best_move = 0;

    for (int count = 0; count < list.count; count++) {
        int move = pick_next_move(s, count, list.count, list);

        if (!in_check && !get_move_capture(move)) {
            continue;
        }

        if (!in_check) {
            bool near_mate = std::abs(alpha) >= MATE_SCORE - SearchConst::MAX_PLY;
            bool is_promotion = get_move_promoted(move) != 0;

            if (!is_promotion && !near_mate) {
                int captured_val = captured_value(board, move);
                if (evaluation + captured_val + DELTA_MARGIN < alpha) {
                    continue;
                }
                if (see(board, move) < 0) {
                    continue;
                }
            }
        }

        if (!make_move(board, move)) {
            continue;
        }
        Evaluate::nnue_do_move(board);

        s.ply++;
        legal_moves++;

        int score = -quiescence(s, board, -beta, -alpha);

        s.ply--;
        unmake_move(board, move);

        if (TimeControl::stopped) return 0;

        if (score > alpha) {
            alpha = score;
            tt_best_move = move;

            if (score >= beta) {
                TT::write_hash_entry(board, beta, 0, TT::HASH_FLAG_BETA, move, s.ply);
                return beta;
            }
        }
    }

    if (in_check && legal_moves == 0) {
        return -MATE_VALUE + s.ply;
    }

    int qs_hash_flag = (alpha > alpha_orig) ? TT::HASH_FLAG_EXACT : TT::HASH_FLAG_ALPHA;
    TT::write_hash_entry(board, alpha, 0, qs_hash_flag, tt_best_move, s.ply);

    return alpha;
}

static int negamax(Search& s, Board& board, int alpha, int beta, int depth) {
    if (TimeControl::stopped) return 0;

    if (s.thread_id == 0 && (s.nodes & 2047) == 0) {
        communicate();
        if (TimeControl::stopped) return 0;
    }

    int score;
    int hash_flag = TT::HASH_FLAG_ALPHA;

    s.pv_length[s.ply] = s.ply;

    if (s.ply != 0 && (board.half_ply >= 100 || is_repetition_draw(board, s.root_ply))) {
        return 0;
    }

    bool pv_node = beta - alpha > 1;

    if (s.ply > 0) {
        int mating_value = MATE_VALUE - s.ply;
        if (mating_value < beta) {
            beta = mating_value;
            if (alpha >= mating_value) return mating_value;
        }
        int mated_value = -MATE_VALUE + s.ply;
        if (mated_value > alpha) {
            alpha = mated_value;
            if (beta <= mated_value) return mated_value;
        }
    }

    int tt_score = TT::read_hash_entry(board, alpha, beta, depth, s.ply);
    if (s.ply != 0 && s.excluded_move[s.ply] == 0 && tt_score != TT::NO_HASH_ENTRY && !pv_node) {
        return tt_score;
    }

    int hash_move = TT::read_hash_move(board);

    if (s.ply >= SearchConst::MAX_PLY) {
        return Evaluate::evaluate(board);
    }

    if (depth <= 0) {
        return quiescence(s, board, alpha, beta);
    }

    s.nodes++;

    bool in_check = is_square_attacked(board,
        get_ls1b(board.bitboards[board.side == white ? K : k]),
        board.side ^ 1);

    if (in_check) depth++;

    if (depth >= 4 && hash_move == 0 && !in_check && TT::TT_SIZE != 0) {
        depth--;
    }

    // ---- singular extension 후보 판정 ----
    // hash_move가 다른 수들보다 압도적으로 좋다고 TT가 보증하면, 검증 탐색 후 depth를 늘려준다.
    bool singular_candidate = false;
    int singular_beta = 0;

    if (s.ply != 0 && s.excluded_move[s.ply] == 0 && depth >= SE_MIN_DEPTH && hash_move != 0) {
        int tt_raw_score, tt_raw_depth, tt_raw_flag;
        if (TT::probe_raw(board, s.ply, tt_raw_score, tt_raw_depth, tt_raw_flag)
            && tt_raw_depth >= depth - SE_TT_DEPTH_MARGIN
            && tt_raw_flag != TT::HASH_FLAG_ALPHA
            && std::abs(tt_raw_score) < MATE_SCORE - SearchConst::MAX_PLY) {
            singular_candidate = true;
            singular_beta = tt_raw_score - depth * SE_MARGIN_PER_DEPTH;
        }
    }

    int legal_moves = 0;

    bool has_non_pawn_material;
    if (board.side == white) {
        has_non_pawn_material = (board.bitboards[N] | board.bitboards[B] |
                                  board.bitboards[R] | board.bitboards[Q]) != 0;
    } else {
        has_non_pawn_material = (board.bitboards[n] | board.bitboards[b] |
                                  board.bitboards[r] | board.bitboards[q]) != 0;
    }

    int raw_static_eval = Evaluate::evaluate(board);

    int side_to_move = board.side;
    U64 pawn_key = in_check ? 0ULL : compute_pawn_key(board);
    int pawn_corr_index = (int)(pawn_key & (Search::CORR_HIST_SIZE - 1));
    int static_eval = in_check ? raw_static_eval
                      : apply_pawn_correction(s, side_to_move, pawn_corr_index, raw_static_eval);

    s.static_eval_stack[s.ply] = static_eval;
    bool improving = !in_check && s.ply >= 2 && static_eval > s.static_eval_stack[s.ply - 2];

    if (depth <= 5 && !in_check && !pv_node && std::abs(beta) < MATE_SCORE) {
        int rfp_margin = (improving ? 100 : 120) * depth;
        if (static_eval - rfp_margin >= beta) {
            return static_eval;
        }
    }

    if (depth >= 3 && !in_check && s.ply != 0 && has_non_pawn_material) {
        int saved_enpassant = board.enpassant;
        U64 saved_hash_key = board.hash_key;
        int saved_half_ply = board.half_ply;

        s.ply++;
        s.played_index_at_ply[s.ply] = Search::NULL_MOVE_INDEX;

        if (board.enpassant != no_sq) {
            board.hash_key ^= enpassant_keys[board.enpassant];
        }
        board.side ^= 1;
        board.hash_key ^= side_key;
        board.enpassant = no_sq;

        int R = 2 + (depth / 6);
        int nmp_depth = std::max(0, depth - 1 - R);
        score = -negamax(s, board, -beta, -beta + 1, nmp_depth);

        s.ply--;
        board.side ^= 1;
        board.enpassant = saved_enpassant;
        board.hash_key = saved_hash_key;
        board.half_ply = saved_half_ply;

        if (TimeControl::stopped) return 0;

        if (score >= beta) {
            return beta;
        }
    }

    if (depth <= 3 && !in_check && !pv_node && std::abs(alpha) < MATE_SCORE) {
        if (static_eval + FUTILITY_MARGIN[depth] <= alpha) {
            return quiescence(s, board, alpha, beta);
        }
    }

    MoveList list;
    generate_moves(board, list);

    if (s.follow_pv) {
        s.follow_pv = false;
        for (int count = 0; count < list.count; count++) {
            if (s.pv_table[0][s.ply] == list.moves[count]) {
                s.score_pv = true;
                s.follow_pv = true;
                break;
            }
        }
    }

    score_moves(s, board, list, hash_move);

    int moves_searched = 0;
    int tt_best_move = 0;

    s.quiet_count_at_ply[s.ply] = 0;

    for (int count = 0; count < list.count; count++) {
        int move = pick_next_move(s, count, list.count, list);

        if (move == s.excluded_move[s.ply]) {
            continue;
        }

        bool is_capture = get_move_capture(move);
        bool is_promotion = get_move_promoted(move) != 0;
        bool is_killer_lmp = (move == s.killer_moves[0][s.ply] || move == s.killer_moves[1][s.ply]);

        if (!pv_node && !in_check && depth <= 8
            && !is_capture && !is_promotion && !is_killer_lmp
            && std::abs(alpha) < MATE_SCORE) {
            int lmp_threshold = 3 + depth * depth;
            if (moves_searched >= lmp_threshold) {
                continue;
            }
        }

        if (!pv_node && !in_check && is_capture && depth <= 7 && moves_searched > 0
            && std::abs(alpha) < MATE_SCORE) {
            int see_margin = -90 * depth;
            if (see(board, move) < see_margin) {
                continue;
            }
        }

        if (!is_capture && s.quiet_count_at_ply[s.ply] < Search::MAX_QUIET_TRACKED) {
            s.quiet_moves_at_ply[s.ply][s.quiet_count_at_ply[s.ply]++] = move;
        }

        // ---- singular extension 검증 탐색 (수를 두기 전, 같은 노드/같은 ply에서 수행) ----
        int extension = 0;

        if (singular_candidate && move == hash_move) {
            s.excluded_move[s.ply] = hash_move;
            int se_depth = (depth - 1) / 2;
            int se_score = negamax(s, board, singular_beta - 1, singular_beta, se_depth);
            s.excluded_move[s.ply] = 0;

            if (TimeControl::stopped) return 0;

            if (se_score < singular_beta) {
                extension = 1;
                if (!pv_node && se_score < singular_beta - SE_DOUBLE_EXT_MARGIN) {
                    extension = 2;
                }
            } else if (singular_beta >= beta) {
                // multi-cut: hash_move 없이도 다른 수가 beta를 넘길 만큼 강함
                return singular_beta;
            }
        }

        if (!make_move(board, move)) {
            continue;
        }
        Evaluate::nnue_do_move(board);

        s.ply++;
        s.played_index_at_ply[s.ply] = Search::move_index(get_move_piece(move), get_move_target(move));
        legal_moves++;

        int new_depth = depth - 1 + extension;

        bool gives_check = is_square_attacked(board,
            get_ls1b(board.bitboards[board.side == white ? K : k]),
            board.side ^ 1);

        if (moves_searched == 0) {
            score = -negamax(s, board, -beta, -alpha, new_depth);
        } else {
            bool is_killer = (move == s.killer_moves[0][s.ply] || move == s.killer_moves[1][s.ply]);

            bool do_lmr = (moves_searched >= FULL_DEPTH_MOVES &&
                           depth >= REDUCTION_LIMIT &&
                           !in_check &&
                           !gives_check &&
                           !get_move_capture(move) &&
                           get_move_promoted(move) == 0 &&
                           !is_killer);

            if (do_lmr) {
                int d = std::min(depth, 63);
                int m = std::min(moves_searched, 63);
                int reduction = reduction_table[d][m];

                if (!pv_node) reduction++;
                if (reduction < 1) reduction = 1;

                int lmr_depth = std::max(0, new_depth - reduction);

                score = -negamax(s, board, -(alpha + 1), -alpha, lmr_depth);

                if (score > alpha) {
                    score = -negamax(s, board, -(alpha + 1), -alpha, new_depth);
                }
            } else {
                score = -negamax(s, board, -(alpha + 1), -alpha, new_depth);
            }

            if (score > alpha && score < beta) {
                score = -negamax(s, board, -beta, -alpha, new_depth);
            }
        }

        s.ply--;
        unmake_move(board, move);

        if (TimeControl::stopped) return 0;
        moves_searched++;

        if (score > alpha) {
            tt_best_move = move;
            hash_flag = TT::HASH_FLAG_EXACT;

            alpha = score;
            s.pv_table[s.ply][s.ply] = move;

            if (s.pv_length[s.ply + 1] - (s.ply + 1) >= 0) {
                std::memcpy(&s.pv_table[s.ply][s.ply + 1],
                            &s.pv_table[s.ply + 1][s.ply + 1],
                            (s.pv_length[s.ply + 1] - (s.ply + 1)) * sizeof(int));
            }
            s.pv_length[s.ply] = s.pv_length[s.ply + 1];

            if (score >= beta) {
                if (s.excluded_move[s.ply] == 0) {
                    TT::write_hash_entry(board, beta, depth, TT::HASH_FLAG_BETA, move, s.ply);
                }

                if (!get_move_capture(move)) {
                    apply_quiet_history_update(s, s.ply, move, history_bonus(depth));
                    s.killer_moves[1][s.ply] = s.killer_moves[0][s.ply];
                    s.killer_moves[0][s.ply] = move;
                }

                if (!in_check) {
                    update_pawn_corr_hist(s, side_to_move, pawn_corr_index, score, raw_static_eval, depth);
                }

                return beta;
            }
        }
    }

    if (legal_moves == 0) {
        if (in_check) return -MATE_VALUE + s.ply;
        else return 0;
    }

    if (hash_flag == TT::HASH_FLAG_EXACT && tt_best_move != 0 && !get_move_capture(tt_best_move)) {
        apply_quiet_history_update(s, s.ply, tt_best_move, history_bonus(depth) / 2);
    }

    if (!in_check) {
        update_pawn_corr_hist(s, side_to_move, pawn_corr_index, alpha, raw_static_eval, depth);
    }

    if (s.excluded_move[s.ply] == 0) {
        TT::write_hash_entry(board, alpha, depth, hash_flag, tt_best_move, s.ply);
    }
    return alpha;
}

void Search::reset_for_new_search() {
    nodes = 0;
    follow_pv = false;
    score_pv = false;
    best_move = 0;

    std::memset(killer_moves, 0, sizeof(killer_moves));
    std::memset(history_moves, 0, sizeof(history_moves));
    std::memset(pv_table, 0, sizeof(pv_table));
    std::memset(cont_hist_1, 0, sizeof(cont_hist_1));
    std::memset(cont_hist_2, 0, sizeof(cont_hist_2));
    std::memset(pawn_corr_hist, 0, sizeof(pawn_corr_hist));
    std::fill(std::begin(played_index_at_ply), std::end(played_index_at_ply), NULL_MOVE_INDEX);
    std::memset(pv_length, 0, sizeof(pv_length));
    std::memset(static_eval_stack, 0, sizeof(static_eval_stack));
    std::memset(excluded_move, 0, sizeof(excluded_move));
}

void search_position(Board& board, int depth) {
    static Search s;
    s.reset_for_new_search();
    Evaluate::nnue_refresh_root(board);

    // [안전장치] 탐색 시작 즉시 stop을 받더라도 최소한의 합법수를 출력하도록 설정
    MoveList root_list;
    generate_moves(board, root_list);
    for (int i = 0; i < root_list.count; i++) {
        if (make_move(board, root_list.moves[i])) {
            unmake_move(board, root_list.moves[i]);
            s.best_move = root_list.moves[i];
            break;
        }
    }

    long long start_time_ms = get_time_ms();
    int score;

    int alpha = -INFINITY_VAL;
    int beta = INFINITY_VAL;

    int last_best_move = 0;
    int fail_count = 0;

    for (int current_depth = 1; current_depth <= depth; current_depth++) {
        if (TimeControl::stopped) break;

        s.follow_pv = true;
        score = negamax(s, board, alpha, beta, current_depth);

        if (TimeControl::stopped) break;

        if (score <= alpha || score >= beta) {
            fail_count++;
            int expand = VAL_WINDOW * (1 << std::min(fail_count, 5));

            if (score <= alpha) alpha = std::max(-INFINITY_VAL, alpha - expand);
            else beta = std::min(INFINITY_VAL, beta + expand);

            current_depth--;
            continue;
        }

        fail_count = 0;
        alpha = score - VAL_WINDOW;
        beta = score + VAL_WINDOW;

        long long stop_time = get_time_ms();
        std::string out = "info";
        if (score > -MATE_VALUE && score < -MATE_SCORE) {
            out += " score mate " + std::to_string(-(score + MATE_VALUE) / 2 - 1);
        } else if (score > MATE_SCORE && score < MATE_VALUE) {
            out += " score mate " + std::to_string((MATE_VALUE - score) / 2 + 1);
        } else {
            out += " score cp " + std::to_string(Evaluate::to_display_cp(score));
        }
        out += " depth " + std::to_string(current_depth);
        out += " nodes " + std::to_string(s.nodes);
        out += " time " + std::to_string(stop_time - start_time_ms);
        out += " pv ";
        for (int count = 0; count < s.pv_length[0]; count++) {
            out += move_to_string(s.pv_table[0][count]) + " ";
        }
        std::cout << out << std::endl;

        if (!TimeControl::stopped && s.pv_length[0] > 0) {
            s.best_move = s.pv_table[0][0];
        }

        if (TimeControl::timeset) {
            int current_best_move = s.pv_table[0][0];
            long long elapsed = get_time_ms() - TimeControl::starttime;

            long long soft_limit = (current_best_move != last_best_move)
                ? (long long)(TimeControl::optTime * 0.7)
                : (long long)(TimeControl::optTime * 0.5);

            if (elapsed > soft_limit) break;

            last_best_move = current_best_move;
        }
    }

    std::cout << "bestmove " << move_to_string(s.best_move) << std::endl;
}