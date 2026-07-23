//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_SEARCH_CONSTANTS_H
#define PEPEROBOT_CPP_SEARCH_CONSTANTS_H

#pragma once

namespace SearchConst {
    constexpr int MAX_PLY = 256;
    constexpr int INFINITY_VAL = 50000;   // INFINITY는 <cmath>와 이름 충돌 방지
    constexpr int MATE_VALUE = 49000;
    constexpr int MATE_SCORE = 48000;
}

#endif //PEPEROBOT_CPP_SEARCH_CONSTANTS_H
