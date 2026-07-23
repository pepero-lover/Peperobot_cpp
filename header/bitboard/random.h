//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_RANDOM_H
#define PEPEROBOT_CPP_RANDOM_H

#pragma once
#include "constants.h"

void set_random_state_for_magic_number();
void set_random_state_for_hashing();
U64 get_random_u64();
U64 generate_magic_number();

#endif //PEPEROBOT_CPP_RANDOM_H
