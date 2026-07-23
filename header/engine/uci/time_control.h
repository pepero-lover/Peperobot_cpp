//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_TIME_CONTROL_H
#define PEPEROBOT_CPP_TIME_CONTROL_H

namespace TimeControl {
    extern bool quit;
    extern bool stopped;

    extern int movestogo;
    extern int movetime;
    extern long long time_ms;
    extern long long inc;

    extern long long starttime;
    extern long long stoptime;
    extern long long optTime;
    extern long long maxTime;

    extern bool timeset;
    extern int moveOverhead;
}

#endif //PEPEROBOT_CPP_TIME_CONTROL_H
