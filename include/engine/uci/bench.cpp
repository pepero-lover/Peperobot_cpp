//
// Created by PEPERO-LOVER on 26. 7. 24..
//

#include <iostream>
#include <algorithm>

#include "header/board/Board.h"
#include "header/engine/search.h"
#include "header/engine/tt.h"
#include "header/engine/uci/bench_fens.h"
#include "header/engine/uci/uci_parse.h"
#include "header/engine/uci/time_control.h"
#include "header/time/time_utils.h"

void run_bench(int depth) {
    Board bench_board;
    long long total_nodes = 0;

    long long start_time = get_time_ms();

    for (int i = 0; i < BENCH_FEN_COUNT; i++) {
        // TimeControl 상태 초기화 (이전 탐색의 잔여 상태가 남지 않도록)
        TimeControl::timeset = false;
        TimeControl::stopped = false;
        TimeControl::time_ms = -1;
        TimeControl::movetime = -1;

        // bench는 매 포지션마다 독립적이어야 하므로 TT를 클리어
        TT::clear_tt();

        std::string pos_command = std::string("position fen ") + BENCH_FENS[i];
        parse_position(bench_board, pos_command);

        total_nodes += search_position(bench_board, depth, /*silent=*/true);
    }

    long long elapsed_ms = std::max(1LL, get_time_ms() - start_time);
    long long nps = (total_nodes * 1000) / elapsed_ms;

    std::cout << total_nodes << " nodes " << nps << " nps" << std::endl;
}