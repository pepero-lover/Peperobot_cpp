//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_MOVE_H
#define PEPEROBOT_CPP_MOVE_H

#pragma once
#include <string>

typedef int Move;

// source(6) | target(6) | piece(4) | promoted(4) | capture(1) | double(1) | enpassant(1) | castling(1)
#define encode_move(source, target, piece, promoted, capture, double_push, enpassant, castling) \
( (source)            | \
((target)   << 6)    | \
((piece)    << 12)   | \
((promoted) << 16)   | \
((capture)     ? (1 << 20) : 0) | \
((double_push) ? (1 << 21) : 0) | \
((enpassant)   ? (1 << 22) : 0) | \
((castling)    ? (1 << 23) : 0) )

#define get_move_source(move)   ((move) & 0x3f)
#define get_move_target(move)   (((move) & 0xfc0)   >> 6)
#define get_move_piece(move)    (((move) & 0xf000)  >> 12)
#define get_move_promoted(move) (((move) & 0xf0000) >> 16)
#define get_move_capture(move)    ((move) & 0x100000)
#define get_move_double(move)     ((move) & 0x200000)
#define get_move_enpassant(move)  ((move) & 0x400000)
#define get_move_castling(move)   ((move) & 0x800000)

std::string move_to_string(Move move);
void print_move(Move move);

struct MoveList {
    Move moves[256];
    int count = 0;

    void add(Move move) {
        moves[count++] = move;
    }
};


#endif //PEPEROBOT_CPP_MOVE_H
