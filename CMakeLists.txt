cmake_minimum_required(VERSION 4.2)
project(Peperobot_Cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options($<$<CONFIG:Release>:/O2>)
    add_compile_options(/arch:AVX2)
    add_compile_definitions(USE_AVX2)
else()
    add_compile_options($<$<CONFIG:Release>:-O3>)
    add_compile_options(-mavx2 -mbmi2 -mfma)
    add_compile_definitions(USE_AVX2)
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_executable(
        Peperobot_Cpp include/main.cpp
        header/bitboard/bb_utils.h
        header/bitboard/squares.h
        header/bitboard/constants.h
        header/bitboard/attacks.h
        include/bitboard/attacks.cpp
        header/bitboard/attacks.h
        include/bitboard/magics.cpp
        header/bitboard/magics.h
        include/bitboard/random.cpp
        header/bitboard/random.h
        include/bitboard/magic_numbers.cpp
        header/bitboard/magic_numbers.h
        include/bitboard/init.cpp
        header/bitboard/init.h
        header/board/pieces.h
        include/board/board.cpp
        header/board/board.h
        header/board/castling.h
        header/board/fen.h
        include/board/fen.cpp
        include/hash/zobrist.cpp
        header/hash/zobrist.h
        include/movegen/move.cpp
        header/movegen/move.h
        include/movegen/movegen.cpp
        header/movegen/movegen.h
        include/board/castling.cpp
        header/movegen/perft.h
        include/engine/uci/time_control.cpp
        header/engine/uci/time_control.h
        include/time/time_utils.cpp
        header/time/time_utils.h
        include/engine/search.cpp
        header/engine/search.h
        include/engine/uci/uci_parse.cpp
        header/engine/uci/uci_parse.h
        include/engine/uci/uci_manager.cpp
        header/engine/uci/uci_manager.h
        header/engine/search_constants.h
        include/engine/tt.cpp
        header/engine/tt.h
        header/engine/evaluate/mvv_lva.h
        include/engine/evaluate/see.cpp
        header/engine/evaluate/see.h
        include/engine/evaluate/score_move.cpp
        header/engine/evaluate/score_move.h
        include/engine/evaluate/evaluate.cpp
        header/engine/evaluate/evaluate.h
        header/engine/nnue/nnue_adapter.h
        stockfish/nnue/nnue_accumulator.h
        stockfish/nnue/evaluate_nnue.h
        header/engine/nnue/nnue_dirty.h
        header/engine/uci/bench_fens.h
        include/engine/uci/bench.cpp
        header/engine/uci/bench.h
        header/incbin/incbin.h
        header/engine/nnue/embedded_nnue.h
)

if(MINGW)
    target_link_options(Peperobot_Cpp PRIVATE -static-libgcc -static-libstdc++ -static)
endif()

target_compile_options(Peperobot_Cpp PRIVATE -mavx2 -mbmi -mbmi2 -mssse3 -msse4.1 -mpopcnt)
target_compile_definitions(Peperobot_Cpp PRIVATE
        USE_AVX2 USE_SSSE3 USE_SSE41 USE_SSE2
        USE_BMI2
        USE_POPCNT
        USE_PEXT
        IS_64BIT
)

if(NOT DEFINED EVALFILE)
    set(EVALFILE "${CMAKE_SOURCE_DIR}/networks/nn-5af11540bbfe.nnue")
endif()

target_compile_definitions(Peperobot_Cpp PRIVATE EVALFILE="${EVALFILE}")
target_include_directories(Peperobot_Cpp PRIVATE ${CMAKE_SOURCE_DIR}/third_party)