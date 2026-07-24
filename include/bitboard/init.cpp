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

    fs::path nnue_path = exe_dir / "nnue" / "nn-5af11540bbfe.nnue";
    if (!Evaluate::load_nnue(nnue_path.string())) {
        fprintf(stderr, "info string ERROR: failed to load NNUE file at %s\n", nnue_path.string().c_str());
        exit(1);
    }
}
