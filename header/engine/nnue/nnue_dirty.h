//
// Created by PEPERO-LOVER on 26. 7. 23..
//

#ifndef PEPEROBOT_CPP_NNUE_DIRTY_H
#define PEPEROBOT_CPP_NNUE_DIRTY_H

struct NNUEDirtyPiece {
    static constexpr int MAX = 4;
    int removedPiece[MAX], removedSquare[MAX];
    int addedPiece[MAX],   addedSquare[MAX];
    int nRemoved = 0, nAdded = 0;
    bool kingMoved[2] = {false, false}; // [white, black]

    inline void remove(int piece, int sq) {
        removedPiece[nRemoved] = piece;
        removedSquare[nRemoved] = sq;
        nRemoved++;
    }
    inline void add(int piece, int sq) {
        addedPiece[nAdded] = piece;
        addedSquare[nAdded] = sq;
        nAdded++;
    }
};

#endif