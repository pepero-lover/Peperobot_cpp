//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/board/pieces.h"
#include "header/engine/search.h"
#include "header/engine/evaluate/mvv_lva.h"
#include "header/movegen/move.h"

int score_move(Search& search, const Board& board, int move, int hash_move) {
    if (move == hash_move && hash_move != 0) {
        return 2000000;
    }

    if (search.score_pv) {
        if (search.pv_table[0][search.ply] == move) {
            search.score_pv = false;
            return 20000;
        }
    }

    int promoted_piece = get_move_promoted(move);
    int promotion_bonus = 0;

    if (promoted_piece == Q || promoted_piece == q) {
        promotion_bonus = 15000;
    } else if (promoted_piece != 0) {
        promotion_bonus = 10000;
    }

    if (get_move_capture(move)) {
        int start_piece = (board.side == white) ? p : P;

        U64 target_mask = 1ULL << get_move_target(move);
        int target_piece = start_piece;

        if (board.bitboards[start_piece + 1] & target_mask) target_piece = start_piece + 1;
        else if (board.bitboards[start_piece + 2] & target_mask) target_piece = start_piece + 2;
        else if (board.bitboards[start_piece + 3] & target_mask) target_piece = start_piece + 3;
        else if (board.bitboards[start_piece + 4] & target_mask) target_piece = start_piece + 4;

        return MVV_LVA[get_move_piece(move)][target_piece] + 10000 + promotion_bonus;

    } else {
        if (promotion_bonus > 0) return promotion_bonus;

        if (search.killer_moves[0][search.ply] == move) return 9000;
        if (search.killer_moves[1][search.ply] == move) return 8000;

        int piece = get_move_piece(move);
        int to = get_move_target(move);
        return search.history_moves[piece][to] + search.get_cont_hist_score(piece, to);
    }
}

void score_moves(Search& search, const Board& board, MoveList& list, int hash_move) {
    for (int i = 0; i < list.count; i++) {
        search.move_scores[i] = score_move(search, board, list.moves[i], hash_move);
    }
}

void score_quiescence_moves(Search& search, const Board& board, MoveList& list, int hash_move) {
    for (int i = 0; i < list.count; i++) {
        if (get_move_capture(list.moves[i])) {
            search.move_scores[i] = score_move(search, board, list.moves[i], hash_move);
        } else {
            search.move_scores[i] = -1000000;  // QS에서 조용한 수 제외
        }
    }
}

int pick_next_move(Search& search, int step_index, int move_count, MoveList& list) {
    int best_index = step_index;
    int best_score = search.move_scores[step_index];

    for (int i = step_index + 1; i < move_count; i++) {
        if (search.move_scores[i] > best_score) {
            best_score = search.move_scores[i];
            best_index = i;
        }
    }

    if (best_index != step_index) {
        std::swap(list.moves[step_index], list.moves[best_index]);
        std::swap(search.move_scores[step_index], search.move_scores[best_index]);
    }

    return list.moves[step_index];
}