//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/movegen/move.h"
#include "header/bitboard/squares.h"
#include "header/board/pieces.h"

#include <cstdio>


static char promoted_to_char(int piece) {
    switch (piece) {
        case Q: case q: return 'q';
        case R: case r: return 'r';
        case B: case b: return 'b';
        case N: case n: return 'n';
        default: return '\0';
    }
}

std::string move_to_string(Move move) {
    int source = get_move_source(move);
    int target = get_move_target(move);
    int promoted = get_move_promoted(move);

    std::string result = std::string(square_to_coordinates[source]) +
                          square_to_coordinates[target];

    char promo_char = promoted_to_char(promoted);
    if (promo_char) result += promo_char;

    return result;
}

void print_move(Move move) {
    printf("%s\n", move_to_string(move).c_str());
}