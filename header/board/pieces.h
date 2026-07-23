//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#ifndef PEPEROBOT_CPP_PIECES_H
#define PEPEROBOT_CPP_PIECES_H

#pragma once

enum { P, N, B, R, Q, K, p, n, b, r, q, k };

const char ascii_pieces[12] = { 'P','N','B','R','Q','K','p','n','b','r','q','k' };

inline int char_to_piece(char c) {
    switch (c) {
        case 'P': return P; case 'N': return N; case 'B': return B;
        case 'R': return R; case 'Q': return Q; case 'K': return K;
        case 'p': return p; case 'n': return n; case 'b': return b;
        case 'r': return r; case 'q': return q; case 'k': return k;
        default:  return -1;
    }
}

#endif //PEPEROBOT_CPP_PIECES_H
