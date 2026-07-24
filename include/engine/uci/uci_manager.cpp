//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include <iostream>
#include <string>

#include "header/board/fen.h"
#include "header/engine/search.h"
#include "header/engine/tt.h"
#include "header/engine/uci/time_control.h"
#include "header/engine/uci/uci_parse.h"
#include "header/time/time_utils.h"

#if defined(_WIN32)
#include <windows.h>
#include <conio.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif

Board board_uci;

bool stdin_has_data() {
    // 1. C++ std::cin 내부 버퍼에 아직 읽지 않은 데이터가 남아있는지 우선 확인
    if (std::cin.rdbuf()->in_avail() > 0) {
        return true;
    }

    // 2. OS 커널 파이프/스트림 검사
#if defined(_WIN32)
    static HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD bytes_avail = 0;
    DWORD type = GetFileType(h);

    // 터미널 직접 입력(Console)인 경우
    if (type == FILE_TYPE_CHAR) {
        return _kbhit() != 0;
    }
    // GUI 파이프 연결(Pipe)인 경우
    return PeekNamedPipe(h, NULL, 0, NULL, &bytes_avail, NULL) && bytes_avail > 0;
#else
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    struct timeval timeout = {0, 0};
    return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) > 0;
#endif
}

void communicate() {
    if (TimeControl::timeset && get_time_ms() > TimeControl::stoptime) {
        TimeControl::stopped = true;
    }

    if (stdin_has_data()) {
        std::string input;
        std::getline(std::cin, input);

        if (input.find("quit") != std::string::npos) {
            TimeControl::quit = true;
            TimeControl::stopped = true;
        } else if (input.find("stop") != std::string::npos) {
            TimeControl::stopped = true;
        }
    }
}

void uci_loop() {
    std::string input;

    parse_fen(board_uci, Board::start_position);

    while (std::getline(std::cin, input)) {
        if (input == "isready") {
            std::cout << "readyok\n";
        }
        else if (input.rfind("position", 0) == 0) {
            parse_position(board_uci, input);
        }
        else if (input == "ucinewgame") {
            board_uci.set_start_pos();
            TT::clear_tt();
        }
        else if (input.rfind("go", 0) == 0) {
            parse_go(board_uci, input);
        }
        else if (input.rfind("setoption name ", 0) == 0) {
            std::string option_str = input.substr(15);
            // trim
            option_str.erase(0, option_str.find_first_not_of(" \t"));

            std::string name, value;
            size_t value_pos = option_str.find(" value ");

            if (value_pos != std::string::npos) {
                name = option_str.substr(0, value_pos);
                value = option_str.substr(value_pos + 7);
            } else {
                name = option_str;
            }

            parse_option(name, value);
        }
        else if (input == "d") {
            print_board(board_uci);
        }
        else if (input == "quit") {
            TimeControl::quit = true;
            TimeControl::stopped = true;
            break;
        }
        else if (input == "uci") {
            std::cout << "id name Peperobot\n";
            std::cout << "id author pepero-lover\n\n";

            std::cout << "option name Threads type spin default 1 min 1 max 1024\n";
            std::cout << "option name Hash type spin default 16 min 1 max 33554432\n";
            std::cout << "option name Clear Hash type button\n";
            std::cout << "option name Move Overhead type spin default 50 min 0 max 5000\n";

            std::cout << "option name CorrHistMax type spin default 64 min 0 max 400\n";
            std::cout << "option name CorrHistWeightScale type spin default 256 min 1 max 4096\n";
            std::cout << "option name CorrHistMinDepth type spin default 4 min 0 max 16\n";

            std::cout << "option name SEMinDepth type spin default 7 min 4 max 10\n";
            std::cout << "option name SETTDepthMargin type spin default 3 min 0 max 8\n";
            std::cout << "option name SEMarginPerDepth type spin default 2 min 0 max 8\n";
            std::cout << "option name SEDoubleExtMargin type spin default 20 min 0 max 30\n";

            std::cout << "uciok\n";
        }
    }
}