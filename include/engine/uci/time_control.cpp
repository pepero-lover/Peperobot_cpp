//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/engine/uci/time_control.h"

namespace TimeControl {
    bool quit = false;
    bool stopped = false;

    int movestogo = 30;
    int movetime = -1;
    long long time_ms = -1;
    long long inc = 0;

    long long starttime = 0;
    long long stoptime = 0;
    long long optTime = 0;
    long long maxTime = 0;

    bool timeset = false;
    int moveOverhead = 50;
}