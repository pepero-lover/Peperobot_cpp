//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/bitboard/random.h"

static unsigned int random_state = 1804289383;

static unsigned int get_random_u32() {
    unsigned int number = random_state;
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    random_state = number;
    return number;
}

U64 get_random_u64() {
    U64 n1 = (U64)(get_random_u32()) & 0xFFFF;
    U64 n2 = (U64)(get_random_u32()) & 0xFFFF;
    U64 n3 = (U64)(get_random_u32()) & 0xFFFF;
    U64 n4 = (U64)(get_random_u32()) & 0xFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

void set_random_state_for_magic_number() {
    random_state = 1804289383;
}

void set_random_state_for_hashing() {
    random_state = 1804289383;
}

U64 generate_magic_number() {
    return get_random_u64() & get_random_u64() & get_random_u64();
}