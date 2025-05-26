#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <chrono>
#include <string>
#include <iostream>
#include <unordered_map>
#include <cstring>

// Data Types
#define U64 unsigned long long
#define U32 unsigned int

// bitboard macros
#define getBit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define setBit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define popBit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define countBits(bitboard) __builtin_popcountll(bitboard)
#define getLSBindex(bitboard) (bitboard)?__builtin_ctzll(bitboard):-1

// FEN debug positions
#define empty_board "8/8/8/8/8/8/8/8 w - -"
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"

enum {
    P, N, B, R, Q, K, p, n, b, r, q, k, none
};

enum {
    white, black, both, empty
};

enum {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, noSquare
};

enum {
    wk = 1, wq = 2, bk = 4, bq = 8,
};

enum {
    rook, bishop, queen
};

enum {
    pawn, knight, king
};

enum {
    allMoves, OnlyCaptures
};

char asciiPieces[13] = "PNBRQKpnbrqk";

const char *unicode_pieces[12] = {"♟︎", "♞", "♝", "♜", "♛", "♚", "♙", "♘", "♗", "♖", "♕", "♔"};

const std::unordered_map<char, int> pieceMap = {
    {'P', P}, {'B', B}, {'N', N}, {'R', R}, {'Q', Q}, {'K', K},
    {'p', p}, {'b', b}, {'n', n}, {'r', r}, {'q', q}, {'k', k}
};

const std::unordered_map<int, char> promotedPieceMap = {
    {0, ' '}, {N, 'n'}, {B, 'b'}, {R, 'r'}, {Q, 'q'}, 
    {none, ' '}, {n, 'n'}, {b, 'b'}, {r, 'r'}, {q, 'q'},
};


const std::string indexToSquare[64] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

void printBitboard(U64 bitboard) {
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << "  ";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            if (getBit(bitboard, square)) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "   a b c d e f g h\n" << std::endl;
    std::cout << "Bitboard: " << bitboard << std::endl << std::endl;
}

//file bitboard macros

#define A_FILE 0x0101010101010101ULL
#define B_FILE 0x0202020202020202ULL
#define C_FILE 0x0404040404040404ULL
#define D_FILE 0x0808080808080808ULL
#define E_FILE 0x1010101010101010ULL
#define F_FILE 0x2020202020202020ULL
#define G_FILE 0x4040404040404040ULL
#define H_FILE 0x8080808080808080ULL

#define AB_FILE (A_FILE | B_FILE)
#define GH_FILE (G_FILE | H_FILE)

// Rank bitboard macros
#define RANK_1 0x00000000000000FFULL
#define RANK_2 0x000000000000FF00ULL
#define RANK_3 0x0000000000FF0000ULL
#define RANK_4 0x00000000FF000000ULL
#define RANK_5 0x000000FF00000000ULL
#define RANK_6 0x0000FF0000000000ULL
#define RANK_7 0x00FF000000000000ULL
#define RANK_8 0xFF00000000000000ULL

// Time in milliseconds
#define TIME_IN_MILLISECONDS std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

// Time in microseconds
#define TIME_IN_MICROSECONDS std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

#endif // CONSTANTS_H;
