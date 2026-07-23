//
// Created by PEPERO-LOVER on 26. 7. 22..
//

#ifndef PEPEROBOT_CPP_NNUE_ADAPTER_H
#define PEPEROBOT_CPP_NNUE_ADAPTER_H

#include <cstring>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "header/bitboard/bb_utils.h"
#include "header/bitboard/constants.h"
#include "header/bitboard/squares.h"
#include "header/board/Board.h"
#include "header/board/pieces.h"
#include "header/engine/nnue/nnue_adapter.h"

#include "stockfish/nnue/nnue_common.h"
#include "stockfish/nnue/layers/affine_transform.h"
#include "stockfish/nnue/layers/affine_transform_sparse_input.h"
#include "stockfish/nnue/layers/clipped_relu.h"
#include "stockfish/nnue/layers/sqr_clipped_relu.h"

namespace NNUEAdapter {

    namespace Layers = Stockfish::Eval::NNUE::Layers;
    using Stockfish::Eval::NNUE::ceil_to_multiple;
    using Stockfish::Eval::NNUE::IndexType;
    using Stockfish::Eval::NNUE::read_leb_128;

    using BiasType = int16_t;
    using WeightType = int16_t;
    using PSQTWeightType = int32_t;
    using TransformedFeatureType = uint8_t;

    constexpr int HalfDimensions = 1536;
    constexpr int InputDimensions = 22528;
    constexpr int PSQTBuckets = 8;
    constexpr int LayerStacks = 8;
    constexpr int OutputScale = 16;
    constexpr int RegWidth = 256 / 16;                    // 16
    constexpr int NumChunks = HalfDimensions / RegWidth;  // 96

    // =====================================================================
    // Features (HalfKAv2_hm 대응)
    // =====================================================================
    namespace Features {

        constexpr int SQUARE_NB = 64;

        enum {
            PS_W_PAWN   = 0,
            PS_B_PAWN   = 1 * SQUARE_NB,
            PS_W_KNIGHT = 2 * SQUARE_NB,
            PS_B_KNIGHT = 3 * SQUARE_NB,
            PS_W_BISHOP = 4 * SQUARE_NB,
            PS_B_BISHOP = 5 * SQUARE_NB,
            PS_W_ROOK   = 6 * SQUARE_NB,
            PS_B_ROOK   = 7 * SQUARE_NB,
            PS_W_QUEEN  = 8 * SQUARE_NB,
            PS_B_QUEEN  = 9 * SQUARE_NB,
            PS_KING     = 10 * SQUARE_NB,
            PS_NB       = 11 * SQUARE_NB
        };

        // 피스 순서: P,N,B,R,Q,K,p,n,b,r,q,k (0~11)
        static constexpr int PieceSquareIndex[2][12] = {
            { PS_W_PAWN, PS_W_KNIGHT, PS_W_BISHOP, PS_W_ROOK, PS_W_QUEEN, PS_KING,
              PS_B_PAWN, PS_B_KNIGHT, PS_B_BISHOP, PS_B_ROOK, PS_B_QUEEN, PS_KING },
            { PS_B_PAWN, PS_B_KNIGHT, PS_B_BISHOP, PS_B_ROOK, PS_B_QUEEN, PS_KING,
              PS_W_PAWN, PS_W_KNIGHT, PS_W_BISHOP, PS_W_ROOK, PS_W_QUEEN, PS_KING }
        };

#define B(v) ((v) * PS_NB)
        static constexpr int KingBuckets[SQUARE_NB] = {
            B(28), B(29), B(30), B(31), B(31), B(30), B(29), B(28),
            B(24), B(25), B(26), B(27), B(27), B(26), B(25), B(24),
            B(20), B(21), B(22), B(23), B(23), B(22), B(21), B(20),
            B(16), B(17), B(18), B(19), B(19), B(18), B(17), B(16),
            B(12), B(13), B(14), B(15), B(15), B(14), B(13), B(12),
            B( 8), B( 9), B(10), B(11), B(11), B(10), B( 9), B( 8),
            B( 4), B( 5), B( 6), B( 7), B( 7), B( 6), B( 5), B( 4),
            B( 0), B( 1), B( 2), B( 3), B( 3), B( 2), B( 1), B( 0),
        };
#undef B

        static constexpr int OrientTBL[SQUARE_NB] = {
            7,7,7,7, 0,0,0,0,   7,7,7,7, 0,0,0,0,
            7,7,7,7, 0,0,0,0,   7,7,7,7, 0,0,0,0,
            7,7,7,7, 0,0,0,0,   7,7,7,7, 0,0,0,0,
            7,7,7,7, 0,0,0,0,   7,7,7,7, 0,0,0,0,
        };

        inline int orient(int perspective, int s) {
            return perspective == black ? (s ^ 56) : s;
        }

        inline int make_index(int perspective, int s, int piece, int ksq) {
            int os   = orient(perspective, s);
            int oksq = orient(perspective, ksq);
            return (os ^ OrientTBL[oksq]) + PieceSquareIndex[perspective][piece] + KingBuckets[oksq];
        }

    } // namespace Features

    // =====================================================================
    // Feature Transformer (가중치 보관 + 리프레시/증분 업데이트에서 사용)
    // =====================================================================
    struct MyFeatureTransformer {
        alignas(64) BiasType biases[HalfDimensions];
        alignas(64) WeightType weights[(size_t)HalfDimensions * InputDimensions];
        alignas(64) PSQTWeightType psqtWeights[(size_t)InputDimensions * PSQTBuckets];

        bool read_parameters(std::istream& stream) {
            read_leb_128<BiasType>(stream, biases, HalfDimensions);
            read_leb_128<WeightType>(stream, weights, HalfDimensions * InputDimensions);
            read_leb_128<PSQTWeightType>(stream, psqtWeights, PSQTBuckets * InputDimensions);
            return !stream.fail();
        }
    };

    // =====================================================================
    // Network (fc_0 -> ac_sqr_0/ac_0 -> fc_1 -> ac_1 -> fc_2)
    // =====================================================================
    struct Network {
        static constexpr int FC_0_OUTPUTS = 15;
        static constexpr int FC_1_OUTPUTS = 32;

        using FC0    = Layers::AffineTransformSparseInput<HalfDimensions, FC_0_OUTPUTS + 1>;
        using ACSqr0 = Layers::SqrClippedReLU<FC_0_OUTPUTS + 1>;
        using AC0    = Layers::ClippedReLU<FC_0_OUTPUTS + 1>;
        using FC1    = Layers::AffineTransform<FC_0_OUTPUTS * 2, FC_1_OUTPUTS>;
        using AC1    = Layers::ClippedReLU<FC_1_OUTPUTS>;
        using FC2    = Layers::AffineTransform<FC_1_OUTPUTS, 1>;

        FC0 fc_0;
        ACSqr0 ac_sqr_0;
        AC0 ac_0;
        FC1 fc_1;
        AC1 ac_1;
        FC2 fc_2;

        bool read_parameters(std::istream& stream) {
            return fc_0.read_parameters(stream)
                && ac_0.read_parameters(stream)
                && fc_1.read_parameters(stream)
                && ac_1.read_parameters(stream)
                && fc_2.read_parameters(stream);
        }

        std::int32_t propagate(const TransformedFeatureType* transformedFeatures) {
            alignas(64) FC0::OutputBuffer fc_0_out;
            alignas(64) ACSqr0::OutputType ac_sqr_0_out[
                ceil_to_multiple<IndexType>(FC_0_OUTPUTS * 2, 32)];
            alignas(64) AC0::OutputBuffer ac_0_out;
            alignas(64) FC1::OutputBuffer fc_1_out;
            alignas(64) AC1::OutputBuffer ac_1_out;
            alignas(64) FC2::OutputBuffer fc_2_out;

            fc_0.propagate(transformedFeatures, fc_0_out);
            ac_sqr_0.propagate(fc_0_out, ac_sqr_0_out);
            ac_0.propagate(fc_0_out, ac_0_out);
            std::memcpy(ac_sqr_0_out + FC_0_OUTPUTS, ac_0_out, FC_0_OUTPUTS * sizeof(AC0::OutputType));
            fc_1.propagate(ac_sqr_0_out, fc_1_out);
            ac_1.propagate(fc_1_out, ac_1_out);
            fc_2.propagate(ac_1_out, fc_2_out);

            std::int32_t fwdOut = int(fc_0_out[FC_0_OUTPUTS]) * (600 * OutputScale) / (127 * (1 << 6));
            return fc_2_out[0] + fwdOut;
        }
    };

    // =====================================================================
    // Accumulator: ply 스택에 보관되는 캐시된 feature-transformer 출력
    // =====================================================================
    struct Accumulator {
        alignas(64) int16_t accumulation[2][HalfDimensions]; // [white, black]
        int32_t psqtAccumulation[2][PSQTBuckets];
        bool computed[2] = { false, false };
    };

    // ply별 스택 (스레드마다 하나)
    inline thread_local Accumulator g_accStack[Board::MAX_DEPTH + 8];

    // color 관점을 처음부터 완전히 재계산 (킹 이동/루트 초기화 시 사용)
    inline void refresh_accumulator(const Board& b, int color, const MyFeatureTransformer& ft, Accumulator& acc) {
        int ksq = __builtin_ctzll(color == white ? b.bitboards[K] : b.bitboards[k]);

        std::memcpy(acc.accumulation[color], ft.biases, sizeof(ft.biases));
        std::memset(acc.psqtAccumulation[color], 0, sizeof(acc.psqtAccumulation[color]));

        __m256i* accVec = reinterpret_cast<__m256i*>(acc.accumulation[color]);

        for (int piece = P; piece <= k; ++piece) {
            U64 bb = b.bitboards[piece];
            while (bb) {
                int sq = __builtin_ctzll(bb);
                bb &= bb - 1;
                int idx = Features::make_index(color, sq, piece, ksq);

                const __m256i* w = reinterpret_cast<const __m256i*>(&ft.weights[(size_t)HalfDimensions * idx]);
                for (int i = 0; i < NumChunks; ++i)
                    accVec[i] = _mm256_add_epi16(accVec[i], w[i]);

                const int32_t* pw = &ft.psqtWeights[(size_t)idx * PSQTBuckets];
                for (int k2 = 0; k2 < PSQTBuckets; ++k2)
                    acc.psqtAccumulation[color][k2] += pw[k2];
            }
        }
        acc.computed[color] = true;
    }

    // 부모 accumulator + dirty piece로 증분 업데이트 (킹이 안 움직였을 때만 호출)
    inline void update_accumulator(const Accumulator& prev, Accumulator& cur, int color,
                                    const NNUEDirtyPiece& dirty, int ksq, const MyFeatureTransformer& ft) {
        std::memcpy(cur.accumulation[color], prev.accumulation[color], sizeof(prev.accumulation[color]));
        std::memcpy(cur.psqtAccumulation[color], prev.psqtAccumulation[color], sizeof(prev.psqtAccumulation[color]));

        __m256i* accVec = reinterpret_cast<__m256i*>(cur.accumulation[color]);

        for (int i = 0; i < dirty.nRemoved; ++i) {
            int idx = Features::make_index(color, dirty.removedSquare[i], dirty.removedPiece[i], ksq);
            const __m256i* w = reinterpret_cast<const __m256i*>(&ft.weights[(size_t)HalfDimensions * idx]);
            for (int c = 0; c < NumChunks; ++c)
                accVec[c] = _mm256_sub_epi16(accVec[c], w[c]);
            const int32_t* pw = &ft.psqtWeights[(size_t)idx * PSQTBuckets];
            for (int k2 = 0; k2 < PSQTBuckets; ++k2)
                cur.psqtAccumulation[color][k2] -= pw[k2];
        }
        for (int i = 0; i < dirty.nAdded; ++i) {
            int idx = Features::make_index(color, dirty.addedSquare[i], dirty.addedPiece[i], ksq);
            const __m256i* w = reinterpret_cast<const __m256i*>(&ft.weights[(size_t)HalfDimensions * idx]);
            for (int c = 0; c < NumChunks; ++c)
                accVec[c] = _mm256_add_epi16(accVec[c], w[c]);
            const int32_t* pw = &ft.psqtWeights[(size_t)idx * PSQTBuckets];
            for (int k2 = 0; k2 < PSQTBuckets; ++k2)
                cur.psqtAccumulation[color][k2] += pw[k2];
        }
        cur.computed[color] = true;
    }

    inline void nnue_do_move(const Board& b, const MyFeatureTransformer& ft) {
        int ply = b.ply;
        const NNUEDirtyPiece& dirty = b.nnue_dirty_history[ply - 1];
        Accumulator& prev = g_accStack[ply - 1];
        Accumulator& cur  = g_accStack[ply];

        for (int color = white; color <= black; ++color) {
            if (dirty.kingMoved[color]) {
                refresh_accumulator(b, color, ft, cur);
            } else {
                int ksq = __builtin_ctzll(color == white ? b.bitboards[K] : b.bitboards[k]);
                update_accumulator(prev, cur, color, dirty, ksq, ft);
            }
        }
    }

    // 탐색 시작(루트) 시 1회 호출
    inline void nnue_refresh_root(const Board& b, const MyFeatureTransformer& ft) {
        refresh_accumulator(b, white, ft, g_accStack[b.ply]);
        refresh_accumulator(b, black, ft, g_accStack[b.ply]);
    }

    // =====================================================================
    // 최종 평가: accumulator 스택에서 읽기만 함 (재계산 없음)
    // =====================================================================
    inline std::int32_t evaluate(const Board& b, Network networks[LayerStacks]) {
        Accumulator& acc = g_accStack[b.ply];

        int stm = b.side, opp = 1 - b.side;
        int pieceCount = __builtin_popcountll(b.occupancies[both]);
        int bucket = std::min((pieceCount - 1) / 4, LayerStacks - 1);

        std::int32_t psqt = (acc.psqtAccumulation[stm][bucket] - acc.psqtAccumulation[opp][bucket]) / 2;

        alignas(64) TransformedFeatureType transformed[HalfDimensions];
        int sides[2] = { stm, opp };
        for (int p = 0; p < 2; ++p) {
            int side = sides[p];
            int offset = (HalfDimensions / 2) * p;
            for (int j = 0; j < HalfDimensions / 2; j++) {
                int sum0 = std::clamp<int>(acc.accumulation[side][j], 0, 127);
                int sum1 = std::clamp<int>(acc.accumulation[side][j + HalfDimensions/2], 0, 127);
                transformed[offset + j] = static_cast<TransformedFeatureType>(sum0 * sum1 / 128);
            }
        }

        std::int32_t positional = networks[bucket].propagate(transformed);
        return (psqt + positional) / OutputScale;
    }

} // namespace NNUEAdapter

#endif //PEPEROBOT_CPP_NNUE_ADAPTER_H