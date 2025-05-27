#ifndef EVALUATE_H
#define EVALUATE_H

#include "constants.h"
#include "board.h"
#include "precalculated_move_tables.h"
#include "moves.h"
#include "perft.h"

#define isBoardInCheck(board) \
    isSquareAttacked(board, getLSBindex(board->bitboards[(board->sideToMove == white) ? K : k]), board->sideToMove ^ 1)

const int mirrorSquare[] = {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

const int pieceValue[] = {
    100, // White Pawn
    300, // White Knight
    350, // White Bishop
    500, // White Rook
    900, // White Queen
    10000, // White King
    -100, // Black Pawn
    -300, // Black Knight
    -350, // Black Bishop
    -500, // Black Rook
    -900, // Black Queen
    -10000 // Black King
};

const int pawnSquareTable[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0, -10, -10,   0,   0,   0,
    0,   0,   0,   5,   5,   0,   0,   0,
    5,   5,  10,  20,  20,   5,   5,   5,
    10,  10,  10,  20,  20,  10,  10,  10,
    20,  20,  20,  30,  30,  30,  20,  20,
    30,  30,  30,  40,  40,  30,  30,  30,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int knightSquareTable[64] = {
    -5, -10,   0,   0,   0,   0, -10,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   5,  20,  10,  10,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5
};

const int bishopSquareTable[64] = {
    0,   0, -10,   0,   0, -10,   0,   0,
    0,  30,   0,   0,   0,   0,  30,   0,
    0,  10,   0,   0,   0,   0,  10,   0,
    0,   0,  10,  20,  20,  10,   0,   0,
    0,   0,  10,  20,  20,  10,   0,   0,
    0,   0,   0,  10,  10,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

static int rookSquareTable[64] = {
    0,    0,   0,  20,  20,   0,   0,   0,
    0,    0,  10,  20,  20,  10,   0,   0,
    0,    0,  10,  20,  20,  10,   0,   0,
    0,    0,  10,  20,  20,  10,   0,   0,
    0,    0,  10,  20,  20,  10,   0,   0,
    0,    0,  10,  20,  20,  10,   0,   0,
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50
};

static int kingSquaretable[64] = {
    0,   0,   5,   0, -15,   0,  10,   0,
    0,   5,   5,  -5,  -5,   0,   5,   0,
    0,   0,   5,  10,  10,   5,   0,   0,
    0,   5,  10,  20,  20,  10,   5,   0,
    0,   5,  10,  20,  20,  10,   5,   0,
    0,   5,   5,  10,  10,   5,   5,   0,
    0,   0,   5,   5,   5,   5,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

static inline int evaluate(Board *board) {
    int score = 0;

    U64 bitboard;
    int square;

    for (int piece = P; piece <= k; ++piece) {
        bitboard = board->bitboards[piece];
        while (bitboard) {
            square = getLSBindex(bitboard);
            score += pieceValue[piece];
            switch (piece) {
                case P: score += pawnSquareTable[square]; break;
                case N: score += knightSquareTable[square]; break;
                case B: score += bishopSquareTable[square]; break;
                case R: score += rookSquareTable[square]; break;
                case K: score += kingSquaretable[square]; break;

                case p: score -= pawnSquareTable[mirrorSquare[square]]; break;
                case n: score -= knightSquareTable[mirrorSquare[square]]; break;
                case b: score -= bishopSquareTable[mirrorSquare[square]]; break;
                case r: score -= rookSquareTable[mirrorSquare[square]]; break;
                case k: score -= kingSquaretable[mirrorSquare[square]]; break;
            }
            popBit(bitboard, square);
        }
    }
    
    return board->sideToMove == white ? score : -score;
}




#endif // EVALUATE_H;