//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/bitboard/init.h"

#include "header/bitboard/attacks.h"
#include "header/bitboard/magics.h"
#include "header/board/castling.h"
#include "header/hash/zobrist.h"
#include "header/engine/evaluate/evaluate.h"

#include <filesystem>

namespace fs = std::filesystem;

void init_all(const fs::path& exe_dir) {
    init_leapers_attacks();
    init_sliders_attacks(BISHOP);
    init_sliders_attacks(ROOK);
    init_hash_keys();
    init_castling_masks();

    if (!Evaluate::load_nnue_embedded()) {
        fprintf(stderr, "info string ERROR: failed to load embedded NNUE\n");
        exit(1);
    }
}