//
// Created by PEPERO-LOVER on 26. 7. 21..
//

#include <sstream>
#include <vector>
#include <cctype>
#include <cstdio>

#include "header/bitboard/bb_utils.h"
#include "header/board/Board.h"
#include "header/board/castling.h"
#include "header/board/pieces.h"
#include "header/hash/zobrist.h"

void parse_fen(Board& board, const std::string& fen) {
    board.reset();

    std::istringstream iss(fen);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);

    const std::string& board_part = tokens[0];

    int rank = 7;
    int file = 0;

    for (char fenChar : board_part) {
        if (fenChar == '/') {
            rank--;
            file = 0;
        } else if (fenChar >= '1' && fenChar <= '8') {
            file += (fenChar - '0');
        } else if (std::isalpha((unsigned char)fenChar)) {
            int piece = char_to_piece(fenChar);
            if (piece != -1 && file < 8) {
                int square = rank * 8 + file;
                set_bit(board.bitboards[piece], square);
                file++;
            }
        }
    }

    // side to move
    board.side = (tokens.size() > 1 && tokens[1] == "b") ? black : white;

    // castling rights
    board.castle = 0;
    if (tokens.size() > 2 && tokens[2] != "-") {
        for (char c : tokens[2]) {
            switch (c) {
                case 'K': board.castle |= WK; break;
                case 'Q': board.castle |= WQ; break;
                case 'k': board.castle |= BK; break;
                case 'q': board.castle |= BQ; break;
            }
        }
    }

    // enpassant
    board.enpassant = no_sq;
    if (tokens.size() > 3 && tokens[3] != "-" && tokens[3].size() >= 2) {
        int fileInt = tokens[3][0] - 'a';
        int rankInt = tokens[3][1] - '1';
        if (fileInt >= 0 && fileInt <= 7 && rankInt >= 0 && rankInt <= 7) {
            board.enpassant = rankInt * 8 + fileInt;
        }
    }

    // occupancy
    for (int piece = P; piece <= K; piece++)
        board.occupancies[white] |= board.bitboards[piece];
    for (int piece = p; piece <= k; piece++)
        board.occupancies[black] |= board.bitboards[piece];

    board.occupancies[both] = board.occupancies[white] | board.occupancies[black];

    // half move / full move
    board.half_ply = (tokens.size() > 4) ? std::stoi(tokens[4]) : 0;
    int full_move = (tokens.size() > 5) ? std::stoi(tokens[5]) : 1;
    board.ply = (full_move - 1) * 2 + (board.side == white ? 0 : 1);

    board.hash_key = generate_hash_key(board);
}

void print_board(const Board& board) {
    char square_char[64];
    for (int i = 0; i < 64; i++) square_char[i] = '.';

    for (int piece = P; piece <= k; piece++) {
        U64 bb = board.bitboards[piece];
        while (bb) {
            int square = get_ls1b(bb);
            square_char[square] = ascii_pieces[piece];
            pop_bit(bb, square);
        }
    }

    printf("\n");

    for (int rank = 7; rank >= 0; rank--) {
        printf("  %d  ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            printf(" %c", square_char[square]);
        }
        printf("\n");
    }
    printf("\n      a b c d e f g h \n\n");

    printf("      Side:      %s\n", board.side == white ? "white" : "black");
    printf("      Enpassant: %s\n",
           board.enpassant != no_sq ? square_to_coordinates[board.enpassant] : "no");
    printf("      Castling:  %c%c%c%c\n",
           (board.castle & WK) ? 'K' : '-',
           (board.castle & WQ) ? 'Q' : '-',
           (board.castle & BK) ? 'k' : '-',
           (board.castle & BQ) ? 'q' : '-');
}