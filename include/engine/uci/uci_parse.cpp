//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include <sstream>
#include <vector>
#include <algorithm>

#include "header/board/board.h"
#include "header/board/fen.h"
#include "header/board/pieces.h"
#include "header/engine/search.h"
#include "header/engine/tt.h"
#include "header/engine/uci/time_control.h"
#include "header/movegen/move.h"
#include "header/movegen/movegen.h"
#include "header/time/time_utils.h"

Move parse_move(board& board, const std::string& command) {
    MoveList list;
    generate_moves(board, list);

    // a1=0 기준: '1'->rank0, '8'->rank7
    int source_file = command[0] - 'a';
    int source_rank = (command[1] - '0') - 1;
    int target_file = command[2] - 'a';
    int target_rank = (command[3] - '0') - 1;

    int source_square = source_rank * 8 + source_file;
    int target_square = target_rank * 8 + target_file;

    for (int i = 0; i < list.count; i++) {
        Move move = list.moves[i];

        if (source_square == get_move_source(move) && target_square == get_move_target(move)) {
            int promoted = get_move_promoted(move);

            if (promoted != 0) {
                if ((promoted == Q || promoted == q) && command.size() > 4 && command[4] == 'q') return move;
                if ((promoted == R || promoted == r) && command.size() > 4 && command[4] == 'r') return move;
                if ((promoted == B || promoted == b) && command.size() > 4 && command[4] == 'b') return move;
                if ((promoted == N || promoted == n) && command.size() > 4 && command[4] == 'n') return move;
                continue;
            }

            return move;
        }
    }

    return 0;  // illegal
}

void parse_position(board& board, const std::string& command) {
    std::istringstream iss(command);
    std::string token;

    iss >> token;  // "position" 토큰 버림
    if (!(iss >> token)) return;

    if (token == "fen") {
        std::ostringstream fen;
        bool ok = true;
        for (int i = 0; i < 6; i++) {
            if (iss >> token) {
                fen << token << " ";
            } else {
                ok = false;
                break;
            }
        }
        parse_fen(board, ok ? fen.str() : board::start_position);
        TT::clear_tt();
    } else {
        board.set_start_pos();
    }

    if (iss >> token && token == "moves") {
        while (iss >> token) {
            Move move = parse_move(board, token);
            if (move != 0) make_move(board, move);
        }
    }
}

void parse_go(board& board, const std::string& command) {
    int depth = -1;

    TimeControl::time_ms = -1;
    TimeControl::inc = 0;
    TimeControl::movetime = -1;
    TimeControl::movestogo = 30;
    TimeControl::timeset = false;
    TimeControl::stopped = false;

    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);

    for (size_t i = 0; i < tokens.size(); i++) {
        try {
            if (tokens[i] == "depth" && i + 1 < tokens.size()) {
                depth = std::stoi(tokens[i + 1]);
            } else if (tokens[i] == "nodes" && i + 1 < tokens.size()) {
            } else if (tokens[i] == "wtime" && i + 1 < tokens.size() && board.side == white) {
                TimeControl::time_ms = std::stoll(tokens[i + 1]);
            } else if (tokens[i] == "btime" && i + 1 < tokens.size() && board.side == black) {
                TimeControl::time_ms = std::stoll(tokens[i + 1]);
            } else if (tokens[i] == "winc" && i + 1 < tokens.size() && board.side == white) {
                TimeControl::inc = std::stoll(tokens[i + 1]);
            } else if (tokens[i] == "binc" && i + 1 < tokens.size() && board.side == black) {
                TimeControl::inc = std::stoll(tokens[i + 1]);
            } else if (tokens[i] == "movestogo" && i + 1 < tokens.size()) {
                TimeControl::movestogo = std::stoi(tokens[i + 1]);
            } else if (tokens[i] == "movetime" && i + 1 < tokens.size()) {
                TimeControl::movetime = std::stoi(tokens[i + 1]);
            }
        } catch (...) {
            // 파싱 실패 무시 (자바 catch(Exception) 대응)
        }
    }

    TimeControl::starttime = get_time_ms();
    int overhead = TimeControl::moveOverhead;

    if (TimeControl::movetime != -1) {
        TimeControl::timeset = true;

        long long m_time = TimeControl::movetime - overhead;
        m_time = std::max(m_time, 50LL);

        TimeControl::optTime = m_time;
        TimeControl::maxTime = m_time;
        TimeControl::stoptime = TimeControl::starttime + m_time;
    }
    else if (TimeControl::time_ms != -1) {
        TimeControl::timeset = true;

        long long time_val = TimeControl::time_ms;
        long long inc_val = TimeControl::inc;
        int movestogo = TimeControl::movestogo;

        if (movestogo == 0) movestogo = 30;
        int effective_movestogo = std::max(movestogo, 1);

        long long opt_time = (time_val / effective_movestogo) + (inc_val * 3 / 4);
        long long max_time = std::min(time_val / 5, opt_time * 5);

        opt_time -= overhead;
        max_time -= overhead;

        opt_time = std::max(opt_time, 50LL);
        max_time = std::max(max_time, 50LL);

        TimeControl::optTime = opt_time;
        TimeControl::maxTime = max_time;
        TimeControl::stoptime = TimeControl::starttime + max_time;
    }

    if (depth == -1) depth = SearchConst::MAX_PLY;

    search_position(board, depth);
}

void parse_option(const std::string& name, const std::string& value) {
    std::string option_name = name;
    std::transform(option_name.begin(), option_name.end(), option_name.begin(), ::tolower);

    try {
        if (option_name == "threads") {
        } else if (option_name == "hash") {
            TT::resize_tt(std::stoi(value));
        } else if (option_name == "clear hash") {
            TT::clear_tt();
        } else if (option_name == "move overhead") {
            TimeControl::moveOverhead = std::stoi(value);
        } else if (option_name == "corrhistmax") {
            Search::CorrHistMaxCp = std::stoi(value);
        } else if (option_name == "corrhistweightscale") {
            Search::CorrHistWeightScale = std::stoi(value);
        } else if (option_name == "corrhistmindepth") {
            Search::CorrHistMinDepth = std::stoi(value);
        } else if (option_name == "semindepth") {
            Search::SE_MIN_DEPTH = std::stoi(value);
        } else if (option_name == "settdepthmargin") {
            Search::SE_TT_DEPTH_MARGIN = std::stoi(value);
        } else if (option_name == "semarginperdepth") {
            Search::SE_MARGIN_PER_DEPTH = std::stoi(value);
        } else if (option_name == "sedoubleextmargin") {
            Search::SE_DOUBLE_EXT_MARGIN = std::stoi(value);
        }
    } catch (...) {
        // 잘못된 값 무시
    }
}