//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/time/time_utils.h"
#include <chrono>

long long get_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch()
    ).count();
}