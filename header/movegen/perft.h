//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_PERFT_H
#define PEPEROBOT_CPP_PERFT_H
#include <cstdio>
#include <chrono>
#include "movegen.h"
#include "move.h"

static long long nodes = 0;

void perft_driver(board& board, int depth) {
    if (depth == 0) {
        nodes++;
        return;
    }

    MoveList list;
    generate_moves(board, list);

    for (int i = 0; i < list.count; i++) {
        Move move = list.moves[i];
        if (make_move(board, move)) {
            perft_driver(board, depth - 1);
            unmake_move(board, move);
        }
    }
}

void perft_test(board& board, int depth) {
    printf("\n    Performance test    \n\n");
    nodes = 0;

    MoveList list;
    generate_moves(board, list);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < list.count; i++) {
        Move move = list.moves[i];

        if (!make_move(board, move)) continue;

        long long cumulative = nodes;
        perft_driver(board, depth - 1);
        long long old_nodes = nodes - cumulative;
        unmake_move(board, move);

        printf("    move: %-6s nodes: %lld\n", move_to_string(move).c_str(), old_nodes);
    }

    auto end = std::chrono::high_resolution_clock::now();
    long long duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    long long duration_ms = duration_ns / 1'000'000;

    double seconds = duration_ns / 1e9;
    long long nps = (duration_ns > 0) ? (long long)(nodes / seconds) : 0;

    printf("\n\n    Depth: %d\n", depth);
    printf("    Nodes: %lld\n", nodes);
    printf("     Time: %lld ms ( + %lld ns)\n", duration_ms, duration_ns % 1'000'000);
    printf("      NPS: %lld (%.2f MNPS)\n", nps, nps / 1'000'000.0);
}

#endif //PEPEROBOT_CPP_PERFT_H
