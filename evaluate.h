#ifndef EVALUATE_H
#define EVALUATE_H

#include "constants.h"
#include "board.h"
#include "precalculated_move_tables.h"
#include "moves.h"
#include "perft.h"

#define MG 0
#define EG 1

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

const int pieceValue[2][12] = {
    {82, 337, 365, 477, 1025,  0, -82, -337, -365, -477, -1025,  0},
    {94, 281, 297, 512,  936,  0, -94, -281, -297, -512,  -936,  0}
};

const int phaseScore[12] = {0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};
const int endGamePhaseMaterialScore = 2 * phaseScore[R] + phaseScore[B] + phaseScore[N];

const int pawnSquareTable[2][64] = {
    { // MGpawnSquareTable
         0,    0,   0,   0,   0,   0,   0,   0,
       -35,  -1, -20, -23, -15,  24,  38, -22,
       -26,  -4,  -4, -10,   3,   3,  33, -12,
       -27,  -2,  -5,  12,  17,   6,  10, -25,
       -14,  13,   6,  21,  23,  12,  17, -23,
        -6,   7,  26,  31,  65,  56,  25, -20,
        98, 134,  61,  95,  68, 126,  34, -11,
         0,   0,   0,   0,   0,   0,   0,   0
    },
    { // EGpawnSquareTable
         0,   0,   0,   0,   0,   0,   0,   0,
        13,   8,   8,  10,  13,   0,   2,  -7,
         4,   7,  -6,   1,   0,  -5,  -1,  -8,
        13,   9,  -3,  -7,  -7,  -8,   3,  -1,
        32,  24,  13,   5,  -2,   4,  17,  17,
        94, 100,  85,  67,  56,  53,  82,  84,
       178, 173, 158, 134, 147, 132, 165, 187,
         0,   0,   0,   0,   0,   0,   0,   0
    }
};

const int knightSquareTable[2][64] = {
    { // MGknightSquareTable
       -105, -21, -58, -33, -17, -28, -19,  -23,
        -29, -53, -12,  -3,  -1,  18, -14,  -19,
        -23,  -9,  12,  10,  19,  17,  25,  -16,
        -13,   4,  16,  13,  28,  19,  21,   -8,
         -9,  17,  19,  53,  37,  69,  18,   22,
        -47,  60,  37,  65,  84, 129,  73,   44,
        -73, -41,  72,  36,  23,  62,   7,  -17,
       -167, -89, -34, -49,  61, -97, -15, -107
    },
    { // EGknightSquareTable
        -29, -51, -23, -15, -22, -18, -50, -64,
        -42, -20, -10,  -5,  -2, -20, -23, -44,
        -23,  -3,  -1,  15,  10,  -3, -20, -22,
        -18,  -6,  16,  25,  16,  17,   4, -18,
        -17,   3,  22,  22,  22,  11,   8, -18,
        -24, -20,  10,   9,  -1,  -9, -19, -41,
        -25,  -8, -25,  -2,  -9, -25, -24, -52,
        -58, -38, -13, -28, -31, -27, -63, -99
    }
};

const int bishopSquareTable[2][64] = {
    { // MGbishopSquareTable
        -33,  -3, -14, -21, -13, -12, -39, -21,
          4,  15,  16,   0,   7,  21,  33,   1,
          0,  15,  15,  15,  14,  27,  18,  10,
         -6,  13,  13,  26,  34,  12,  10,   4,
         -4,   5,  19,  50,  37,  37,   7,  -2,
        -16,  37,  43,  40,  35,  50,  37,  -2,
        -26,  16, -18, -13,  30,  59,  18, -47,
        -29,   4, -82, -37, -25, -42,   7,  -8
    },
    { // EGbishopSquareTable
        -23,  -9, -23,  -5, -9, -16,  -5, -17,
        -14, -18,  -7,  -1,  4,  -9, -15, -27,
        -12,  -3,   8,  10, 13,   3,  -7, -15,
         -6,   3,  13,  19,  7,  10,  -3,  -9,
         -3,   9,  12,   9, 14,  10,   3,   2,
          2,  -8,   0,  -1, -2,   6,   0,   4,
         -8,  -4,   7, -12, -3, -13,  -4, -14,
        -14, -21, -11,  -8, -7,  -9, -17, -24
    }
};

const int rookSquareTable[2][64] = {
    { // MGrookSquareTable
        -19, -13,   1,  17, 16,  7, -37, -26,
        -44, -16, -20,  -9, -1, 11,  -6, -71,
        -45, -25, -16, -17,  3,  0,  -5, -33,
        -36, -26, -12,  -1,  9, -7,   6, -23,
        -24, -11,   7,  26, 24, 35,  -8, -20,
         -5,  19,  26,  36, 17, 45,  61,  16,
         27,  32,  58,  62, 80, 67,  26,  44,
         32,  42,  32,  51, 63,  9,  31,  43
    },
    { // EGrookSquareTable
        -9,  2,  3, -1, -5, -13,   4, -20,
        -6, -6,  0,  2, -9,  -9, -11,  -3,
        -4,  0, -5, -1, -7, -12,  -8, -16,
         3,  5,  8,  4, -5,  -6,  -8, -11,
         4,  3, 13,  1,  2,   1,  -1,   2,
         7,  7,  7,  5,  4,  -3,  -5,  -3,
        11, 13, 13, 11, -3,   3,   8,   3,
        13, 10, 18, 15, 12,  12,   8,   5
    }
};

const int queenSquareTable[2][64] = {
    { // MGqueenSquareTable
         -1, -18,  -9,  10, -15, -25, -31, -50,  
        -35,  -8,  11,   2,   8,  15,  -3,   1,  
        -14,   2, -11,  -2,  -5,   2,  14,   5,  
         -9, -26,  -9, -10,  -2,  -4,   3,  -3,  
        -27, -27, -16, -16,  -1,  17,  -2,   1,  
        -13, -17,   7,   8,  29,  56,  47,  57,  
        -24, -39,  -5,   1, -16,  57,  28,  54,  
        -28,   0,  29,  12,  59,  44,  43,  45,  
    },
    { // EGqueenSquareTable
        -33, -28, -22, -43,  -5, -32, -20, -41,  
        -22, -23, -30, -16, -16, -23, -36, -32,  
        -16, -27,  15,   6,   9,  17,  10,   5,  
        -18,  28,  19,  47,  31,  34,  39,  23,  
          3,  22,  24,  45,  57,  40,  57,  36,  
        -20,   6,   9,  49,  47,  35,  19,   9,  
        -17,  20,  32,  41,  58,  25,  30,   0,  
         -9,  22,  22,  27,  27,  19,  10,  20,  
    }
};

const int kingSquareTable[2][64] = {
    { // mg_king_table
        -15,  36,  12, -54,   8, -28,  24,  14,
          1,   7,  -8, -64, -43, -16,   9,   8,
        -14, -14, -22, -46, -44, -30, -15, -27,
        -49,  -1, -27, -39, -46, -44, -33, -51,
        -17, -20, -12, -27, -30, -25, -14, -36,
         -9,  24,   2, -16, -20,   6,  22, -22,
         29,  -1, -20,  -7,  -8,  -4, -38, -29,
        -65,  23,  16, -15, -56, -34,   2,  13
    },
    { // eg_king_table
        -53, -34, -21, -11, -28, -14, -24, -43,
        -27, -11,   4,  13,  14,   4,  -5, -17,
        -19,  -3,  11,  21,  23,  16,   7,  -9,
        -18,  -4,  21,  24,  27,  23,   9, -11,
         -8,  22,  24,  27,  26,  33,  26,   3,
         10,  17,  23,  15,  20,  45,  44,  13,
        -12,  17,  14,  17,  17,  38,  23,  11,
        -74, -35, -18, -18, -11,  15,   4, -17
    }
};

const int DistanceFromCentre[64] = {
    6, 5, 4, 3, 3, 4, 5, 6, 
    5, 4, 3, 2, 2, 3, 4, 5, 
    4, 3, 2, 1, 1, 2, 3, 4, 
    3, 2, 1, 0, 0, 1, 2, 3, 
    3, 2, 1, 0, 0, 1, 2, 3, 
    4, 3, 2, 1, 1, 2, 3, 4, 
    5, 4, 3, 2, 2, 3, 4, 5, 
    6, 5, 4, 3, 3, 4, 5, 6
};

U64 fileMasks[64];
U64 rankMasks[64];

U64 isolatedPawnMasks[64];
U64 passedPawnMasks[2][64];


const int doublePawnPenalty = -10;
const int isolatedPawnPenalty = -10;
const int passedPawnBonus[8] = {0, 5, 10, 20, 30, 45, 60, 0};

const int semiOpenFileBonus = 15;
const int openFileBonus = 20;

const int kingShieldBonus = 10;

U64 setFileRankMasks(int file, int rank) {
    U64 mask = 0ULL;
    
    for (int r = 0; r < 8; r++){
        for (int f = 0; f < 8; f++){
            if (file != -1) {
                if (f == file) {
                    setBit(mask, r * 8 + f);
                }
            }
            else if (rank != -1) {
                if (r == rank) {
                    setBit(mask, r * 8 + f);
                }
            }
        }
    }
    return mask;
}

void initializeEvaluationMasks() {
    for (int file = 0; file < 8; ++file) {
        for (int rank = 0; rank < 8; ++rank) {
            fileMasks[rank * 8 + file] = setFileRankMasks(file, -1);
            rankMasks[rank * 8 + file] = setFileRankMasks(-1, rank);
        }
    }
    for (int rank = 0; rank < 8; rank++){
        for (int file = 0; file < 8; file++){
            isolatedPawnMasks[rank * 8 + file] = setFileRankMasks(file + 1, -1) |
                                                 setFileRankMasks(file - 1, -1);

            U64 ppMask = isolatedPawnMasks[rank * 8 + file] | fileMasks[rank * 8 + file];
            passedPawnMasks[white][rank * 8 + file] = (rank < 7) ? ppMask << 8 * (rank + 1) : 0ULL;
            passedPawnMasks[black][rank * 8 + file] = (rank > 0) ? ppMask >> 8 * (8 - rank) : 0ULL;
        }
    }
}

static inline bool insufficientMaterial(Board *board) {
    if (board->bitboards[P] | board->bitboards[p]) return false;
    if (board->bitboards[Q] | board->bitboards[q]) return false;
    if (board->bitboards[R] | board->bitboards[r]) return false;

    int whiteBishops = countBits(board->bitboards[B]);
    int blackBishops = countBits(board->bitboards[b]);
    if (whiteBishops > 1 || blackBishops > 1) return false;

    int whiteKnights = countBits(board->bitboards[N]);
    int blackKnights = countBits(board->bitboards[n]);
    if (whiteKnights > 1 || blackKnights > 1) return false;

    if (whiteKnights + whiteBishops > 1 || blackKnights + blackBishops > 1) return false;

    if (whiteBishops == 1 && blackBishops == 1) {
        int whiteSq = getLSBindex(board->bitboards[B]);
        int blackSq = getLSBindex(board->bitboards[b]);
        // If bishops are on same-colored squares, it's insufficient
        bool sameColor = ((whiteSq ^ blackSq) & 1) == 0;
        return sameColor;
    }

    return true;
}


static inline int MopUpEvaluation(Board *board, int perspective, int friendlymaterial, int opponentmaterial, float endgameWeight) {
    int score = 0;

    if (friendlymaterial < opponentmaterial || endgameWeight == 0) return 0;

    int opponentKingSquare = getLSBindex(board->bitboards[(perspective == white) ? k : K]);
    int friendlyKingSquare = getLSBindex(board->bitboards[(perspective== white) ? K : k]);

    score += DistanceFromCentre[opponentKingSquare] * 10;
    int distbetweenKings = std::abs((opponentKingSquare / 8) - (friendlyKingSquare / 8)) +
                        std::abs((opponentKingSquare % 8) - (friendlyKingSquare % 8));
    
    score += (14 - distbetweenKings) * 4; // Encourage keeping kings close
    return score * endgameWeight; // Scale score by endgame weight
}


static inline int evaluate(Board *board) {
    int mgScore = 0;
    int egScore = 0;
    int score = 0;

    int phase = 0;
    int whitePhase = 0;
    int blackPhase = 0;
    U64 bitboard;
    int square;
    int numPawnsOnFile;
    int mobility;
    int piecesAroundKing;

    for (int piece = P; piece <= k; ++piece) {
        bitboard = board->bitboards[piece];
        while (bitboard) {
            square = getLSBindex(bitboard);

            // Add mg and eg piece values
            mgScore += pieceValue[0][piece];
            egScore += pieceValue[1][piece];

            phase += phaseScore[piece];
            whitePhase += (piece < 6) ? phaseScore[piece] : 0;
            blackPhase += (piece >= 6) ? phaseScore[piece] : 0;

            switch (piece) {
                case P:
                    mgScore += pawnSquareTable[0][square];
                    egScore += pawnSquareTable[1][square];

                    numPawnsOnFile = countBits(board->bitboards[P] & fileMasks[square]);
                    mgScore += (numPawnsOnFile - 1) * doublePawnPenalty;
                    egScore += (numPawnsOnFile - 1) * doublePawnPenalty;

                    if ((board->bitboards[P] & isolatedPawnMasks[square]) == 0) {
                        mgScore += isolatedPawnPenalty;
                        egScore += isolatedPawnPenalty;
                    }

                    if (((board->bitboards[p] | (board->bitboards[P] & fileMasks[square])) & passedPawnMasks[white][square]) == 0) {
                        mgScore += passedPawnBonus[square / 8];
                        egScore += passedPawnBonus[square / 8];
                    }
                    break;

                case N:
                    mobility = countBits(knightAttacks[square] & ~board->occupancies[white]);
                    mgScore += knightSquareTable[0][square] + mobility * 3;
                    egScore += knightSquareTable[1][square] + mobility * 5; // encourage more mobility in endgame
                    break;

                case B:
                    mobility = countBits(getBishopAttacks(square, board->occupancies[both]));
                    mgScore += bishopSquareTable[0][square] + mobility * 3;
                    egScore += bishopSquareTable[1][square] + mobility * 5;
                    break;

                case R:
                    mobility = countBits(getRookAttacks(square, board->occupancies[both]));
                    mgScore += rookSquareTable[0][square] + mobility * 3;
                    egScore += rookSquareTable[1][square] + mobility * 5;

                    if ((board->bitboards[P] & fileMasks[square]) == 0) {
                        mgScore += semiOpenFileBonus;
                        egScore += semiOpenFileBonus;
                    } 
                    if (((board->bitboards[p] | board->bitboards[P]) & fileMasks[square]) == 0) {
                        mgScore += openFileBonus;
                        egScore += openFileBonus;
                    }
                    break;

                case Q:
                    mobility = countBits(getQueenAttacks(square, board->occupancies[both]));
                    mgScore += queenSquareTable[0][square] + mobility * 3;
                    egScore += queenSquareTable[1][square] + mobility * 5; // encourage more mobility in endgame
                    break;

                case K:
                    mgScore += kingSquareTable[0][square];
                    egScore += kingSquareTable[1][square];

                    // Encourage king safety by penalizing if the king is on a file with no pawns
                    if ((board->bitboards[P] & fileMasks[square]) == 0) {
                        mgScore -= semiOpenFileBonus;
                        egScore -= semiOpenFileBonus;
                    } 
                    if (((board->bitboards[p] | board->bitboards[P]) & fileMasks[square]) == 0) {
                        mgScore -= openFileBonus;
                        egScore -= openFileBonus;
                    }

                    piecesAroundKing = countBits(board->occupancies[white] & kingAttacks[square]);
                    mgScore += piecesAroundKing * kingShieldBonus;
                    egScore += piecesAroundKing * (kingShieldBonus / 2); // Less important in endgame
                    break;

                case p:
                    mgScore -= pawnSquareTable[0][mirrorSquare[square]];
                    egScore -= pawnSquareTable[1][mirrorSquare[square]];

                    numPawnsOnFile = countBits(board->bitboards[p] & fileMasks[square]);
                    mgScore -= (numPawnsOnFile - 1) * doublePawnPenalty;
                    egScore -= (numPawnsOnFile - 1) * doublePawnPenalty;

                    if ((board->bitboards[p] & isolatedPawnMasks[square]) == 0) {
                        mgScore -= isolatedPawnPenalty;
                        egScore -= isolatedPawnPenalty;
                    }

                    if (((board->bitboards[P] | (board->bitboards[p] & fileMasks[square])) & passedPawnMasks[black][square]) == 0) {
                        mgScore -= passedPawnBonus[mirrorSquare[square] / 8];
                        egScore -= passedPawnBonus[mirrorSquare[square] / 8];
                    }
                    break;

                case n:
                    mobility = countBits(knightAttacks[square] & ~board->occupancies[black]);
                    mgScore -= (knightSquareTable[0][mirrorSquare[square]] + mobility * 3);
                    egScore -= (knightSquareTable[1][mirrorSquare[square]] + mobility * 5); // encourage more mobility in endgame
                    break;

                case b:
                    mobility = countBits(getBishopAttacks(square, board->occupancies[both]));
                    mgScore -= (bishopSquareTable[0][mirrorSquare[square]] + mobility * 3);
                    egScore -= (bishopSquareTable[1][mirrorSquare[square]] + mobility * 5);
                    break;

                case r:
                    mobility = countBits(getRookAttacks(square, board->occupancies[both]));
                    mgScore -= (rookSquareTable[0][mirrorSquare[square]] + mobility * 3);
                    egScore -= (rookSquareTable[1][mirrorSquare[square]] + mobility * 5);

                    if ((board->bitboards[p] & fileMasks[square]) == 0) {
                        mgScore -= semiOpenFileBonus;
                        egScore -= semiOpenFileBonus;
                    } 
                    if (((board->bitboards[p] | board->bitboards[P]) & fileMasks[square]) == 0) {
                        mgScore -= openFileBonus;
                        egScore -= openFileBonus;
                    }
                    break;
                
                case q:
                    mobility = countBits(getQueenAttacks(square, board->occupancies[both]));
                    mgScore -= (queenSquareTable[0][mirrorSquare[square]] + mobility * 3);
                    egScore -= (queenSquareTable[1][mirrorSquare[square]] + mobility * 5); // encourage more mobility in endgame
                    break;

                case k:
                    mgScore -= kingSquareTable[0][mirrorSquare[square]];
                    egScore -= kingSquareTable[1][mirrorSquare[square]];

                    // Encourage king safety by penalizing(incrementing for black) if the king is on a file with no pawns
                    if ((board->bitboards[p] & fileMasks[square]) == 0) {
                        mgScore += semiOpenFileBonus;
                        egScore += semiOpenFileBonus;
                    } 
                    if (((board->bitboards[p] | board->bitboards[P]) & fileMasks[square]) == 0) {
                        mgScore += openFileBonus;
                        egScore += openFileBonus;
                    }
                    piecesAroundKing = countBits(board->occupancies[black] & kingAttacks[square]);
                    mgScore -= piecesAroundKing * kingShieldBonus;
                    egScore -= piecesAroundKing * (kingShieldBonus / 2); // Less important in endgame
                    break;
            }
            popBit(bitboard, square);
        }
    }

    float whiteEndGamePhase = 1 - std::min(1.0f, float(whitePhase) / float(endGamePhaseMaterialScore));
    float blackEndGamePhase = 1 - std::min(1.0f, float(blackPhase) / float(endGamePhaseMaterialScore));

    int mopUpScore = MopUpEvaluation(board, white, whitePhase, blackPhase, blackEndGamePhase) 
                    - MopUpEvaluation(board, black, blackPhase, whitePhase, whiteEndGamePhase); 

    egScore += mopUpScore;
    phase = std::min(24, phase);
    score = (mgScore * phase + egScore * (24 - phase)) / 24;
    return board->sideToMove == white ? score : -score;
}


#endif // EVALUATE_H;