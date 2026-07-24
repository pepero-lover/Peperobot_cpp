//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include "header/engine/tt.h"

#include <vector>
#include <algorithm>
#include <cstdint>

#include "header/engine/search_constants.h"

class board;

namespace TT {

int TT_SIZE = 0x2000000;

// 엔트리 하나당 16바이트 (check 8B + data 8B), 자바 원본과 동일한 비트 패킹
constexpr int ENTRY_SIZE = 16;

constexpr int MOVE_SHIFT  = 0;
constexpr int SCORE_SHIFT = 32;
constexpr int DEPTH_SHIFT = 54;
constexpr int FLAG_SHIFT  = 62;

constexpr uint64_t MOVE_MASK  = 0xFFFFFFFFULL;
constexpr uint64_t SCORE_MASK = 0x3FFFFFULL;
constexpr uint64_t DEPTH_MASK = 0xFFULL;
constexpr uint64_t FLAG_MASK  = 0x3ULL;

constexpr int64_t SCORE_OFFSET = 1LL << 20;

// 단일스레드라 그냥 std::vector<uint64_t>. 나중에 멀티스레딩 붙이면
// 이 두 배열만 std::vector<std::atomic<uint64_t>>로 바꾸고
// getAcquire/setRelease에 해당하는 memory_order_acquire/release로 교체하면 됨.
static std::vector<uint64_t> tt_checks(TT_SIZE, 0);
static std::vector<uint64_t> tt_data(TT_SIZE, 0);

static uint64_t pack_data(int depth, int flag, int score, int best_move) {
    uint64_t d = (static_cast<uint64_t>(best_move) & MOVE_MASK) << MOVE_SHIFT;
    d |= ((static_cast<uint64_t>(score) + SCORE_OFFSET) & SCORE_MASK) << SCORE_SHIFT;
    d |= (static_cast<uint64_t>(depth) & DEPTH_MASK) << DEPTH_SHIFT;
    d |= (static_cast<uint64_t>(flag) & FLAG_MASK) << FLAG_SHIFT;
    return d;
}

static int unpack_move(uint64_t data)  { return static_cast<int>((data >> MOVE_SHIFT) & MOVE_MASK); }
static int unpack_score(uint64_t data) { return static_cast<int>(((data >> SCORE_SHIFT) & SCORE_MASK)) - SCORE_OFFSET; }
static int unpack_depth(uint64_t data) { return static_cast<int>((data >> DEPTH_SHIFT) & DEPTH_MASK); }
static int unpack_flag(uint64_t data)  { return static_cast<int>((data >> FLAG_SHIFT) & FLAG_MASK); }

void resize_tt(int hash_size_mb) {
    if (hash_size_mb <= 0) {
        TT_SIZE = 0;
        tt_checks.clear();
        tt_data.clear();
        return;
    }

    long long bytes = static_cast<long long>(hash_size_mb) * 1024 * 1024;
    long long max_entries = bytes / ENTRY_SIZE;

    int target_entries = static_cast<int>(std::min<long long>(max_entries, INT32_MAX));
    target_entries = std::max(target_entries, 1);

    // 2의 거듭제곱으로 내림 (자바 Integer.highestOneBit 대응)
    int power = 1;
    while (power * 2 <= target_entries) power *= 2;
    target_entries = power;

    TT_SIZE = target_entries;
    tt_checks.assign(target_entries, 0);
    tt_data.assign(target_entries, 0);
}

void clear_tt() {
    std::fill(tt_checks.begin(), tt_checks.end(), 0);
    std::fill(tt_data.begin(), tt_data.end(), 0);
}

int read_hash_move(const board& board) {
    if (TT_SIZE == 0) return 0;

    int index = static_cast<int>(board.hash_key & (TT_SIZE - 1));

    uint64_t check = tt_checks[index];
    uint64_t data = tt_data[index];

    if ((check ^ data) == board.hash_key) {
        return unpack_move(data);
    }
    return 0;
}

int read_hash_entry(const board& board, int alpha, int beta, int depth, int ply) {
    if (TT_SIZE == 0) return NO_HASH_ENTRY;

    int index = static_cast<int>(board.hash_key & (TT_SIZE - 1));

    uint64_t check = tt_checks[index];
    uint64_t data = tt_data[index];

    if ((check ^ data) == board.hash_key) {
        int tt_depth = unpack_depth(data);

        if (tt_depth >= depth) {
            int score = unpack_score(data);
            int flag = unpack_flag(data);

            if (score < -SearchConst::MATE_SCORE) score += ply;
            if (score > SearchConst::MATE_SCORE)  score -= ply;

            if (flag == HASH_FLAG_EXACT) return score;
            if (flag == HASH_FLAG_ALPHA && score <= alpha) return alpha;
            if (flag == HASH_FLAG_BETA && score >= beta) return beta;
        }
    }
    return NO_HASH_ENTRY;
}

bool probe_raw(const board& board, int ply, int& out_score, int& out_depth, int& out_flag) {
    if (TT_SIZE == 0) return false;

    int index = static_cast<int>(board.hash_key & (TT_SIZE - 1));

    uint64_t check = tt_checks[index];
    uint64_t data = tt_data[index];

    if ((check ^ data) != board.hash_key) {
        return false;
    }

    out_depth = unpack_depth(data);
    out_flag = unpack_flag(data);

    int score = unpack_score(data);
    if (score < -SearchConst::MATE_SCORE) score += ply;
    if (score > SearchConst::MATE_SCORE)  score -= ply;
    out_score = score;

    return true;
}

void write_hash_entry(const board& board, int score, int depth, int hash_flag, int best_move, int ply) {
    if (TT_SIZE == 0) return;

    int index = static_cast<int>(board.hash_key & (TT_SIZE - 1));

    uint64_t old_check = tt_checks[index];
    uint64_t old_data = tt_data[index];

    if ((old_check ^ old_data) == board.hash_key
        && unpack_depth(old_data) > depth
        && hash_flag != HASH_FLAG_EXACT) {
        return;
    }

    if (score < -SearchConst::MATE_SCORE) score -= ply;
    if (score > SearchConst::MATE_SCORE)  score += ply;

    uint64_t new_data = pack_data(depth, hash_flag, score, best_move);
    uint64_t new_check = board.hash_key ^ new_data;

    tt_data[index] = new_data;
    tt_checks[index] = new_check;
}

} // namespace TT