//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_MAGICS_H
#define PEPEROBOT_CPP_MAGICS_H

#pragma once
#include "constants.h"

U64 find_magic_number(int square, int relevant_bits, bool bishop);
void init_magic_numbers();
void init_sliders_attacks(bool bishop);

#endif //PEPEROBOT_CPP_MAGICS_H
