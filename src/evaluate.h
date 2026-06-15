#ifndef EVALUATE_H
#define EVALUATE_H

#include "constants.h"
#include "board.h"
#include "precalculated_move_tables.h"
#include "moves.h"
#include "../utilities/perft.h"

#define MG 0
#define EG 1

#ifdef TUNING_MODE
#define EVAL_PARAM
#else
#define EVAL_PARAM const
#endif

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

EVAL_PARAM int pieceValue[2][12] = {
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

const int ProximityToDarkCorner[64] = {
    7, 6, 5, 4, 3, 2, 1, 0,
    6, 5, 4, 3, 2, 1, 0, 1,
    5, 4, 3, 2, 1, 0, 1, 2,
    4, 3, 2, 1, 0, 1, 2, 3,
    3, 2, 1, 0, 1, 2, 3, 4,
    2, 1, 0, 1, 2, 3, 4, 5,
    1, 0, 1, 2, 3, 4, 5, 6,
    0, 1, 2, 3, 4, 5, 6, 7
};

const int ProximityToLightCorner[64] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    1, 0, 1, 2, 3, 4, 5, 6,
    2, 1, 0, 1, 2, 3, 4, 5,
    3, 2, 1, 0, 1, 2, 3, 4,
    4, 3, 2, 1, 0, 1, 2, 3,
    5, 4, 3, 2, 1, 0, 1, 2,
    6, 5, 4, 3, 2, 1, 0, 1,
    7, 6, 5, 4, 3, 2, 1, 0
};

U64 fileMasks[64];
U64 rankMasks[64];

U64 isolatedPawnMasks[64];
U64 passedPawnMasks[2][64];


EVAL_PARAM int doublePawnPenalty = -10;
EVAL_PARAM int isolatedPawnPenalty = -14;
EVAL_PARAM int passedPawnBonus[2][8] = {
    {0, 0, 0, 0, 48, 83, 93, 0},
    {0, 0, 0, 28, 49, 61, 35, 0}
};
EVAL_PARAM int passedPawnFriendlyKingBonus = 3;
EVAL_PARAM int passedPawnEnemyKingPenalty = 5;

EVAL_PARAM int bishopPairBonus[2] = {16, 81};
EVAL_PARAM int tempoBonus = 10;

EVAL_PARAM int semiOpenFileBonus = 10;
EVAL_PARAM int openFileBonus = 12;

EVAL_PARAM int pawnShieldBonus = 0;
EVAL_PARAM int pawnShieldMissingPenalty = -9;

EVAL_PARAM int backwardPawnPenalty[2] = {-10, -16};

EVAL_PARAM int rookOn7thBonus[2] = {5, 15};

EVAL_PARAM int knightOutpostBonus[2] = {21, 16};
EVAL_PARAM int rookBehindPassedBonus[2] = {0, 0};
EVAL_PARAM int connectedPassedBonus[2] = {0, 7};

EVAL_PARAM int badBishopPenalty[2] = {-2, -10};
EVAL_PARAM int blockedPasserPenalty[2] = {-20, -40};

EVAL_PARAM int mobilityMG = 3;
EVAL_PARAM int mobilityEG = 5;

EVAL_PARAM int threatByMinor[2] = {25, 30};
EVAL_PARAM int threatByRook[2] = {35, 40};
EVAL_PARAM int hangingPenalty[2] = {-20, -25};
EVAL_PARAM int pawnPushThreat = 15;

EVAL_PARAM int spaceBonus = 3;

EVAL_PARAM int pawnStormBonus[] = {0, 0, 0, 10, 20, 30, 0, 0};

EVAL_PARAM int centerControlBonus[2] = {4, 2};
EVAL_PARAM int connectedRookBonus[2] = {8, 12};
EVAL_PARAM int pawnDuoBonus[2] = {5, 3};
EVAL_PARAM int kingAdjacentOpenFile = -10;

EVAL_PARAM int kingAttackWeights[] = {0, 2, 2, 3, 5, 0, 0, 2, 2, 3, 5, 0};
EVAL_PARAM int kingSafetyTable[] = {
    0,   0,   2,   5,  10,  16,  23,  32,  42,  54,
   67,  82,  98, 115, 133, 153, 174, 196, 220, 245,
  271, 298, 327, 357, 388, 400, 400, 400, 400, 400
};

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
        bool whiteBishopOnLight = (board->bitboards[B] & LIGHT_SQUARES) != 0;
        bool blackBishopOnLight = (board->bitboards[b] & LIGHT_SQUARES) != 0;
        return whiteBishopOnLight == blackBishopOnLight;
    }

    return true;
}


static inline bool isMinorPieceEndgame(const Board* board) {
    U64 whiteMinor = board->bitboards[N] | board->bitboards[B];
    U64 blackMinor = board->bitboards[n] | board->bitboards[b];

    U64 whiteOther = board->bitboards[P] | board->bitboards[R] | board->bitboards[Q];
    U64 blackOther = board->bitboards[p] | board->bitboards[r] | board->bitboards[q];

    return whiteOther == 0 && blackOther == 0 &&
           (whiteMinor || blackMinor); // at least one minor piece on the board
}

static inline int MinorPieceEvaluation(Board *board, int perspective){
    int distance = 0;

    int piece = (perspective == white) ? B : b;
    int opponentKingSquare = getLSBindex(board->bitboards[(perspective == white) ? k : K]);
    U64 pieceBitboard = board->bitboards[piece];
    while (pieceBitboard) {
        int square = getLSBindex(pieceBitboard);
        bool bishopOnLight = ((1ULL << square) & LIGHT_SQUARES) != 0;
        distance += ((bishopOnLight) ? ProximityToLightCorner[opponentKingSquare] : ProximityToDarkCorner[opponentKingSquare]);
        popBit(pieceBitboard, square);
    }
    if (distance == 0)
        distance = DistanceFromCentre[opponentKingSquare];

    return distance;
}

static inline int MopUpEvaluation(Board *board, int perspective, int friendlymaterial, int opponentmaterial, float endgameWeight) {
    int score = 0;

    if (friendlymaterial < opponentmaterial || endgameWeight == 0) return 0;

    int opponentKingSquare = getLSBindex(board->bitboards[(perspective == white) ? k : K]);
    int friendlyKingSquare = getLSBindex(board->bitboards[(perspective== white) ? K : k]);

    if (isMinorPieceEndgame(board))
        score += MinorPieceEvaluation(board, perspective) * 10;
    else
        score += DistanceFromCentre[opponentKingSquare] * 15;

    int distbetweenKings = std::abs((opponentKingSquare / 8) - (friendlyKingSquare / 8)) +
                        std::abs((opponentKingSquare % 8) - (friendlyKingSquare % 8));

    score += (14 - distbetweenKings) * 8;

    int matAdvantage = std::min(friendlymaterial - opponentmaterial, 500);
    score = score * matAdvantage / 100;

    return score * endgameWeight;
}


static inline int endgameScaleFactor(const Board *board, int score) {
    int strongSide = (score > 0) ? white : black;
    int strongPawns = countBits(board->bitboards[(strongSide == white) ? P : p]);

    if (strongPawns == 0) {
        int strongKnights = countBits(board->bitboards[(strongSide == white) ? N : n]);
        int strongBishops = countBits(board->bitboards[(strongSide == white) ? B : b]);
        int strongRooks = countBits(board->bitboards[(strongSide == white) ? R : r]);
        int strongQueens = countBits(board->bitboards[(strongSide == white) ? Q : q]);
        int strongMajor = strongRooks + strongQueens;

        if (strongMajor == 0 && (strongKnights + strongBishops) <= 2) {
            if (strongKnights + strongBishops <= 1) return 0;
            if (strongKnights == 2) return 0;
        }
    }

    int wb = countBits(board->bitboards[B]);
    int bb = countBits(board->bitboards[b]);
    if (wb == 1 && bb == 1 && strongPawns <= 2) {
        int wbSq = getLSBindex(board->bitboards[B]);
        int bbSq = getLSBindex(board->bitboards[b]);
        bool wbLight = ((wbSq / 8) + (wbSq % 8)) % 2;
        bool bbLight = ((bbSq / 8) + (bbSq % 8)) % 2;
        if (wbLight != bbLight) return 50;
    }

    return 100;
}

static inline int evaluate(Board *board) {
    int mgScore = 0;
    int egScore = 0;
    int score = 0;

    int phase = 0;
    int whitePhase = 0;
    int blackPhase = 0;
    int whiteMaterial = 0;
    int blackMaterial = 0;
    U64 bitboard;
    int square;
    int numPawnsOnFile;
    int mobility;

    int whiteKingSq = getLSBindex(board->bitboards[K]);
    int blackKingSq = getLSBindex(board->bitboards[k]);
    U64 whiteKingZone = kingAttacks[whiteKingSq] | (1ULL << whiteKingSq);
    U64 blackKingZone = kingAttacks[blackKingSq] | (1ULL << blackKingSq);
    int whiteKingAttackWeight = 0, blackKingAttackWeight = 0;
    int whiteKingAttackerCount = 0, blackKingAttackerCount = 0;

    U64 whiteAttacks = 0ULL, blackAttacks = 0ULL;
    U64 whitePawnAttacks = 0ULL, blackPawnAttacks = 0ULL;
    U64 whiteKnightAttacks = 0ULL, blackKnightAttacks = 0ULL;
    U64 whiteBishopAttacks = 0ULL, blackBishopAttacks = 0ULL;
    U64 whiteRookAttacks = 0ULL, blackRookAttacks = 0ULL;

    U64 wp = board->bitboards[P];
    while (wp) { int sq = getLSBindex(wp); whitePawnAttacks |= pawnAttacks[white][sq]; popBit(wp, sq); }
    U64 bp = board->bitboards[p];
    while (bp) { int sq = getLSBindex(bp); blackPawnAttacks |= pawnAttacks[black][sq]; popBit(bp, sq); }
    whiteAttacks |= whitePawnAttacks;
    blackAttacks |= blackPawnAttacks;
    whiteAttacks |= kingAttacks[whiteKingSq];
    blackAttacks |= kingAttacks[blackKingSq];

    for (int piece = P; piece <= k; ++piece) {
        bitboard = board->bitboards[piece];
        while (bitboard) {
            square = getLSBindex(bitboard);

            // Add mg and eg piece values
            mgScore += pieceValue[0][piece];
            egScore += pieceValue[1][piece];

            if (piece < 6)
                whiteMaterial += pieceValue[EG][piece];
            else
                blackMaterial -= pieceValue[EG][piece];

            phase += phaseScore[piece];
            whitePhase += (piece < 6) ? phaseScore[piece] : 0;
            blackPhase += (piece >= 6) ? phaseScore[piece] : 0;

            switch (piece) {
                case P:
                    mgScore += pawnSquareTable[0][square];
                    egScore += pawnSquareTable[1][square];

                    numPawnsOnFile = countBits(board->bitboards[P] & fileMasks[square]);
                    if (numPawnsOnFile > 1) {
                        for (int r = square / 8 - 1; r >= 0; --r) {
                            if (board->bitboards[P] & (1ULL << (r * 8 + square % 8))) {
                                mgScore += doublePawnPenalty;
                                egScore += doublePawnPenalty;
                                break;
                            }
                        }
                    }

                    {
                        int pFile = square % 8;
                        int pRank = square / 8;
                        if (pFile < 7 && (board->bitboards[P] & (1ULL << (pRank * 8 + pFile + 1)))) {
                            mgScore += pawnDuoBonus[0];
                            egScore += pawnDuoBonus[1];
                        }
                    }

                    if ((board->bitboards[P] & isolatedPawnMasks[square]) == 0) {
                        mgScore += isolatedPawnPenalty;
                        egScore += isolatedPawnPenalty;
                    } else if (square + 8 < 64 && (pawnAttacks[white][square + 8] & board->bitboards[p]) &&
                               (board->bitboards[P] & isolatedPawnMasks[square] & ~passedPawnMasks[white][square]) == 0) {
                        mgScore += backwardPawnPenalty[0];
                        egScore += backwardPawnPenalty[1];
                    }

                    if (((board->bitboards[p] | (board->bitboards[P] & fileMasks[square])) & passedPawnMasks[white][square]) == 0) {
                        int rank = square / 8;
                        mgScore += passedPawnBonus[0][rank];
                        egScore += passedPawnBonus[1][rank];

                        int friendlyKingSq = getLSBindex(board->bitboards[K]);
                        int enemyKingSq = getLSBindex(board->bitboards[k]);
                        int friendlyDist = std::abs(friendlyKingSq / 8 - rank) + std::abs(friendlyKingSq % 8 - square % 8);
                        int enemyDist = std::abs(enemyKingSq / 8 - rank) + std::abs(enemyKingSq % 8 - square % 8);
                        egScore += (7 - friendlyDist) * passedPawnFriendlyKingBonus;
                        egScore += enemyDist * passedPawnEnemyKingPenalty;

                        if (square + 8 < 64 && (board->occupancies[black] & (1ULL << (square + 8)))) {
                            mgScore += blockedPasserPenalty[0];
                            egScore += blockedPasserPenalty[1];
                        } else if (rank >= 5 && square + 8 < 64 &&
                                   !(board->occupancies[both] & (1ULL << (square + 8)))) {
                            static const int freePasserBonus[] = {0, 0, 0, 0, 0, 20, 60, 0};
                            int file = square % 8;
                            int bonus = freePasserBonus[rank];
                            if (file == 0 || file == 7) bonus /= 2;
                            egScore += bonus;
                        }

                        U64 adjacentPassedFriendly = board->bitboards[P] & isolatedPawnMasks[square];
                        while (adjacentPassedFriendly) {
                            int adjSq = getLSBindex(adjacentPassedFriendly);
                            if (((board->bitboards[p] | (board->bitboards[P] & fileMasks[adjSq])) & passedPawnMasks[white][adjSq]) == 0) {
                                mgScore += connectedPassedBonus[0];
                                egScore += connectedPassedBonus[1];
                                break;
                            }
                            popBit(adjacentPassedFriendly, adjSq);
                        }
                    }
                    break;

                case N:
                    {
                        U64 attacks = knightAttacks[square];
                        whiteKnightAttacks |= attacks;
                        whiteAttacks |= attacks;
                        mobility = countBits(attacks & ~board->occupancies[white] & ~blackPawnAttacks);
                        mgScore += knightSquareTable[0][square] + mobility * mobilityMG;
                        egScore += knightSquareTable[1][square] + mobility * mobilityEG;

                        if (attacks & blackKingZone) {
                            whiteKingAttackWeight += 2;
                            whiteKingAttackerCount++;
                        }

                        int knightRank = square / 8;
                        bool onOutpost = knightRank >= 3 && knightRank <= 5;
                        bool defendedByPawn = whitePawnAttacks & (1ULL << square);
                        bool canBeKicked = (isolatedPawnMasks[square] & passedPawnMasks[white][square] & board->bitboards[p]) != 0;
                        if (onOutpost && defendedByPawn && !canBeKicked) {
                            mgScore += knightOutpostBonus[0];
                            egScore += knightOutpostBonus[1];
                        }
                    }
                    break;

                case B:
                    {
                        U64 attacks = getBishopAttacks(square, board->occupancies[both]);
                        whiteBishopAttacks |= attacks;
                        whiteAttacks |= attacks;
                        mobility = countBits(attacks & ~board->occupancies[white] & ~blackPawnAttacks);
                        mgScore += bishopSquareTable[0][square] + mobility * mobilityMG;
                        egScore += bishopSquareTable[1][square] + mobility * mobilityEG;
                        if (attacks & blackKingZone) {
                            whiteKingAttackWeight += 2;
                            whiteKingAttackerCount++;
                        }
                        U64 sameColorSquares = ((1ULL << square) & LIGHT_SQUARES) ? LIGHT_SQUARES : DARK_SQUARES;
                        int pawnsOnColor = countBits(board->bitboards[P] & sameColorSquares);
                        mgScore += pawnsOnColor * badBishopPenalty[0];
                        egScore += pawnsOnColor * badBishopPenalty[1];
                    }
                    break;

                case R:
                    {
                        U64 attacks = getRookAttacks(square, board->occupancies[both]);
                        whiteRookAttacks |= attacks;
                        whiteAttacks |= attacks;
                        mobility = countBits(attacks & ~board->occupancies[white] & ~blackPawnAttacks);
                        mgScore += rookSquareTable[0][square] + mobility * mobilityMG;
                        egScore += rookSquareTable[1][square] + mobility * mobilityEG;
                        if (attacks & blackKingZone) {
                            whiteKingAttackWeight += 3;
                            whiteKingAttackerCount++;
                        }
                    }

                    if ((board->bitboards[P] & fileMasks[square]) == 0) {
                        mgScore += semiOpenFileBonus;
                        egScore += semiOpenFileBonus;
                    } 
                    if (((board->bitboards[p] | board->bitboards[P]) & fileMasks[square]) == 0) {
                        mgScore += openFileBonus;
                        egScore += openFileBonus;
                    }

                    if (square / 8 == 6) {
                        int enemyKingRank = getLSBindex(board->bitboards[k]) / 8;
                        if (enemyKingRank == 7 || (board->bitboards[p] & rankMasks[48])) {
                            mgScore += rookOn7thBonus[0];
                            egScore += rookOn7thBonus[1];
                        }
                    }

                    {
                        U64 friendlyPawnsOnFile = board->bitboards[P] & fileMasks[square];
                        while (friendlyPawnsOnFile) {
                            int pawnSq = getLSBindex(friendlyPawnsOnFile);
                            if (((board->bitboards[p] | (board->bitboards[P] & fileMasks[pawnSq])) & passedPawnMasks[white][pawnSq]) == 0 &&
                                square / 8 < pawnSq / 8) {
                                mgScore += rookBehindPassedBonus[0];
                                egScore += rookBehindPassedBonus[1];
                                break;
                            }
                            popBit(friendlyPawnsOnFile, pawnSq);
                        }
                    }
                    break;

                case Q:
                    {
                        U64 attacks = getQueenAttacks(square, board->occupancies[both]);
                        whiteAttacks |= attacks;
                        mobility = countBits(attacks & ~board->occupancies[white] & ~blackPawnAttacks);
                        mgScore += queenSquareTable[0][square] + mobility * mobilityMG;
                        egScore += queenSquareTable[1][square] + mobility * mobilityEG;
                        if (attacks & blackKingZone) {
                            whiteKingAttackWeight += 5;
                            whiteKingAttackerCount++;
                        }
                    }
                    break;

                case K:
                    mgScore += kingSquareTable[0][square];
                    egScore += kingSquareTable[1][square];

                    if ((board->bitboards[P] & fileMasks[square]) == 0)
                        mgScore -= semiOpenFileBonus;
                    if (((board->bitboards[p] | board->bitboards[P]) & fileMasks[square]) == 0)
                        mgScore -= openFileBonus;

                    {
                        int pawnsShielding = countBits(board->bitboards[P] & kingAttacks[square]);
                        int expectedShield = (square % 8 == 0 || square % 8 == 7) ? 2 : 3;
                        mgScore += pawnsShielding * pawnShieldBonus;
                        mgScore += (expectedShield - pawnsShielding) * pawnShieldMissingPenalty;
                    }
                    break;

                case p:
                    mgScore -= pawnSquareTable[0][mirrorSquare[square]];
                    egScore -= pawnSquareTable[1][mirrorSquare[square]];

                    numPawnsOnFile = countBits(board->bitboards[p] & fileMasks[square]);
                    if (numPawnsOnFile > 1) {
                        for (int r = square / 8 + 1; r < 8; ++r) {
                            if (board->bitboards[p] & (1ULL << (r * 8 + square % 8))) {
                                mgScore -= doublePawnPenalty;
                                egScore -= doublePawnPenalty;
                                break;
                            }
                        }
                    }

                    {
                        int pFile = square % 8;
                        int pRank = square / 8;
                        if (pFile < 7 && (board->bitboards[p] & (1ULL << (pRank * 8 + pFile + 1)))) {
                            mgScore -= pawnDuoBonus[0];
                            egScore -= pawnDuoBonus[1];
                        }
                    }

                    if ((board->bitboards[p] & isolatedPawnMasks[square]) == 0) {
                        mgScore -= isolatedPawnPenalty;
                        egScore -= isolatedPawnPenalty;
                    } else if (square - 8 >= 0 && (pawnAttacks[black][square - 8] & board->bitboards[P]) &&
                               (board->bitboards[p] & isolatedPawnMasks[square] & ~passedPawnMasks[black][square]) == 0) {
                        mgScore -= backwardPawnPenalty[0];
                        egScore -= backwardPawnPenalty[1];
                    }

                    if (((board->bitboards[P] | (board->bitboards[p] & fileMasks[square])) & passedPawnMasks[black][square]) == 0) {
                        int rank = mirrorSquare[square] / 8;
                        mgScore -= passedPawnBonus[0][rank];
                        egScore -= passedPawnBonus[1][rank];

                        int friendlyKingSq = getLSBindex(board->bitboards[k]);
                        int enemyKingSq = getLSBindex(board->bitboards[K]);
                        int friendlyDist = std::abs(friendlyKingSq / 8 - (square / 8)) + std::abs(friendlyKingSq % 8 - square % 8);
                        int enemyDist = std::abs(enemyKingSq / 8 - (square / 8)) + std::abs(enemyKingSq % 8 - square % 8);
                        egScore -= (7 - friendlyDist) * passedPawnFriendlyKingBonus;
                        egScore -= enemyDist * passedPawnEnemyKingPenalty;

                        if (square - 8 >= 0 && (board->occupancies[white] & (1ULL << (square - 8)))) {
                            mgScore -= blockedPasserPenalty[0];
                            egScore -= blockedPasserPenalty[1];
                        } else if (rank >= 5 && square - 8 >= 0 &&
                                   !(board->occupancies[both] & (1ULL << (square - 8)))) {
                            static const int freePasserBonus[] = {0, 0, 0, 0, 0, 20, 60, 0};
                            int file = square % 8;
                            int bonus = freePasserBonus[rank];
                            if (file == 0 || file == 7) bonus /= 2;
                            egScore -= bonus;
                        }

                        U64 adjacentPassedFriendly = board->bitboards[p] & isolatedPawnMasks[square];
                        while (adjacentPassedFriendly) {
                            int adjSq = getLSBindex(adjacentPassedFriendly);
                            if (((board->bitboards[P] | (board->bitboards[p] & fileMasks[adjSq])) & passedPawnMasks[black][adjSq]) == 0) {
                                mgScore -= connectedPassedBonus[0];
                                egScore -= connectedPassedBonus[1];
                                break;
                            }
                            popBit(adjacentPassedFriendly, adjSq);
                        }
                    }
                    break;

                case n:
                    {
                        U64 attacks = knightAttacks[square];
                        blackKnightAttacks |= attacks;
                        blackAttacks |= attacks;
                        mobility = countBits(attacks & ~board->occupancies[black] & ~whitePawnAttacks);
                        mgScore -= (knightSquareTable[0][mirrorSquare[square]] + mobility * mobilityMG);
                        egScore -= (knightSquareTable[1][mirrorSquare[square]] + mobility * mobilityEG);

                        if (attacks & whiteKingZone) {
                            blackKingAttackWeight += 2;
                            blackKingAttackerCount++;
                        }

                        int knightRank = square / 8;
                        bool onOutpost = knightRank >= 2 && knightRank <= 4;
                        bool defendedByPawn = blackPawnAttacks & (1ULL << square);
                        bool canBeKicked = (isolatedPawnMasks[square] & passedPawnMasks[black][square] & board->bitboards[P]) != 0;
                        if (onOutpost && defendedByPawn && !canBeKicked) {
                            mgScore -= knightOutpostBonus[0];
                            egScore -= knightOutpostBonus[1];
                        }
                    }
                    break;

                case b:
                    {
                        U64 attacks = getBishopAttacks(square, board->occupancies[both]);
                        blackBishopAttacks |= attacks;
                        blackAttacks |= attacks;
                        mobility = countBits(attacks & ~board->occupancies[black] & ~whitePawnAttacks);
                        mgScore -= (bishopSquareTable[0][mirrorSquare[square]] + mobility * mobilityMG);
                        egScore -= (bishopSquareTable[1][mirrorSquare[square]] + mobility * mobilityEG);
                        if (attacks & whiteKingZone) {
                            blackKingAttackWeight += 2;
                            blackKingAttackerCount++;
                        }
                        U64 sameColorSquares = ((1ULL << square) & LIGHT_SQUARES) ? LIGHT_SQUARES : DARK_SQUARES;
                        int pawnsOnColor = countBits(board->bitboards[p] & sameColorSquares);
                        mgScore -= pawnsOnColor * badBishopPenalty[0];
                        egScore -= pawnsOnColor * badBishopPenalty[1];
                    }
                    break;

                case r:
                    {
                        U64 attacks = getRookAttacks(square, board->occupancies[both]);
                        blackRookAttacks |= attacks;
                        blackAttacks |= attacks;
                        mobility = countBits(attacks & ~board->occupancies[black] & ~whitePawnAttacks);
                        mgScore -= (rookSquareTable[0][mirrorSquare[square]] + mobility * mobilityMG);
                        egScore -= (rookSquareTable[1][mirrorSquare[square]] + mobility * mobilityEG);
                        if (attacks & whiteKingZone) {
                            blackKingAttackWeight += 3;
                            blackKingAttackerCount++;
                        }
                    }

                    if ((board->bitboards[p] & fileMasks[square]) == 0) {
                        mgScore -= semiOpenFileBonus;
                        egScore -= semiOpenFileBonus;
                    } 
                    if (((board->bitboards[p] | board->bitboards[P]) & fileMasks[square]) == 0) {
                        mgScore -= openFileBonus;
                        egScore -= openFileBonus;
                    }

                    if (square / 8 == 1) {
                        int enemyKingRank = getLSBindex(board->bitboards[K]) / 8;
                        if (enemyKingRank == 0 || (board->bitboards[P] & rankMasks[8])) {
                            mgScore -= rookOn7thBonus[0];
                            egScore -= rookOn7thBonus[1];
                        }
                    }

                    {
                        U64 friendlyPawnsOnFile = board->bitboards[p] & fileMasks[square];
                        while (friendlyPawnsOnFile) {
                            int pawnSq = getLSBindex(friendlyPawnsOnFile);
                            if (((board->bitboards[P] | (board->bitboards[p] & fileMasks[pawnSq])) & passedPawnMasks[black][pawnSq]) == 0 &&
                                square / 8 > pawnSq / 8) {
                                mgScore -= rookBehindPassedBonus[0];
                                egScore -= rookBehindPassedBonus[1];
                                break;
                            }
                            popBit(friendlyPawnsOnFile, pawnSq);
                        }
                    }
                    break;
                
                case q:
                    {
                        U64 attacks = getQueenAttacks(square, board->occupancies[both]);
                        blackAttacks |= attacks;
                        mobility = countBits(attacks & ~board->occupancies[black] & ~whitePawnAttacks);
                        mgScore -= (queenSquareTable[0][mirrorSquare[square]] + mobility * mobilityMG);
                        egScore -= (queenSquareTable[1][mirrorSquare[square]] + mobility * mobilityEG);
                        if (attacks & whiteKingZone) {
                            blackKingAttackWeight += 5;
                            blackKingAttackerCount++;
                        }
                    }
                    break;

                case k:
                    mgScore -= kingSquareTable[0][mirrorSquare[square]];
                    egScore -= kingSquareTable[1][mirrorSquare[square]];

                    if ((board->bitboards[p] & fileMasks[square]) == 0)
                        mgScore += semiOpenFileBonus;
                    if (((board->bitboards[p] | board->bitboards[P]) & fileMasks[square]) == 0)
                        mgScore += openFileBonus;

                    {
                        int pawnsShielding = countBits(board->bitboards[p] & kingAttacks[square]);
                        int expectedShield = (square % 8 == 0 || square % 8 == 7) ? 2 : 3;
                        mgScore -= pawnsShielding * pawnShieldBonus;
                        mgScore -= (expectedShield - pawnsShielding) * pawnShieldMissingPenalty;
                    }
                    break;
            }
            popBit(bitboard, square);
        }
    }

    if (countBits(board->bitboards[B]) >= 2) {
        mgScore += bishopPairBonus[0];
        egScore += bishopPairBonus[1];
    }
    if (countBits(board->bitboards[b]) >= 2) {
        mgScore -= bishopPairBonus[0];
        egScore -= bishopPairBonus[1];
    }

    if (whiteKingAttackerCount >= 2) {
        int idx = std::min(whiteKingAttackWeight, 29);
        mgScore += kingSafetyTable[idx];
    }
    if (blackKingAttackerCount >= 2) {
        int idx = std::min(blackKingAttackWeight, 29);
        mgScore -= kingSafetyTable[idx];
    }

    // --- Threats: hanging pieces and minor/rook threats ---
    {
        U64 whiteMinorAttacks = whiteKnightAttacks | whiteBishopAttacks;
        U64 blackMinorAttacks = blackKnightAttacks | blackBishopAttacks;

        U64 blackPieces = board->occupancies[black] & ~board->bitboards[k];
        U64 whitePieces = board->occupancies[white] & ~board->bitboards[K];

        U64 minorThreatsW = whiteMinorAttacks & (board->bitboards[r] | board->bitboards[q]);
        int threatCountW = countBits(minorThreatsW);
        mgScore += threatCountW * threatByMinor[0];
        egScore += threatCountW * threatByMinor[1];

        U64 rookThreatsW = whiteRookAttacks & board->bitboards[q];
        int rookThreatCountW = countBits(rookThreatsW);
        mgScore += rookThreatCountW * threatByRook[0];
        egScore += rookThreatCountW * threatByRook[1];

        U64 minorThreatsB = blackMinorAttacks & (board->bitboards[R] | board->bitboards[Q]);
        int threatCountB = countBits(minorThreatsB);
        mgScore -= threatCountB * threatByMinor[0];
        egScore -= threatCountB * threatByMinor[1];

        U64 rookThreatsB = blackRookAttacks & board->bitboards[Q];
        int rookThreatCountB = countBits(rookThreatsB);
        mgScore -= rookThreatCountB * threatByRook[0];
        egScore -= rookThreatCountB * threatByRook[1];

        U64 whiteHanging = whitePieces & blackAttacks & ~whiteAttacks;
        U64 blackHanging = blackPieces & whiteAttacks & ~blackAttacks;
        mgScore += countBits(whiteHanging) * hangingPenalty[0];
        egScore += countBits(whiteHanging) * hangingPenalty[1];
        mgScore -= countBits(blackHanging) * hangingPenalty[0];
        egScore -= countBits(blackHanging) * hangingPenalty[1];

        // Pawn push threats: pawns that can advance to attack enemy pieces
        U64 whitePawnPush = ((board->bitboards[P] << 8) & ~board->occupancies[both]);
        U64 whitePushAttacks = (((whitePawnPush & ~0xFF00000000000000ULL) << 7) & ~fileMasks[7]) |
                               (((whitePawnPush & ~0xFF00000000000000ULL) << 9) & ~fileMasks[0]);
        U64 whitePushThreats = whitePushAttacks & blackPieces & ~blackPawnAttacks;
        mgScore += countBits(whitePushThreats) * pawnPushThreat;

        U64 blackPawnPush = ((board->bitboards[p] >> 8) & ~board->occupancies[both]);
        U64 blackPushAttacks = (((blackPawnPush & ~0x00000000000000FFULL) >> 7) & ~fileMasks[0]) |
                               (((blackPawnPush & ~0x00000000000000FFULL) >> 9) & ~fileMasks[7]);
        U64 blackPushThreats = blackPushAttacks & whitePieces & ~whitePawnAttacks;
        mgScore -= countBits(blackPushThreats) * pawnPushThreat;
    }

    // --- Space evaluation (MG only) ---
    {
        const U64 whiteSpaceArea = 0x00003C3C3C3C0000ULL;
        const U64 blackSpaceArea = 0x0000003C3C3C3C00ULL;

        U64 whiteSafe = whiteSpaceArea & ~blackPawnAttacks & ~board->bitboards[p];
        U64 blackSafe = blackSpaceArea & ~whitePawnAttacks & ~board->bitboards[P];

        int whiteSpace = countBits(whiteSafe & (whiteAttacks | ~blackAttacks));
        int blackSpace = countBits(blackSafe & (blackAttacks | ~whiteAttacks));
        mgScore += (whiteSpace - blackSpace) * spaceBonus;
    }

    // --- Pawn storm on enemy king ---
    {
        int bkFile = blackKingSq % 8;
        U64 stormPawns = board->bitboards[P];
        while (stormPawns) {
            int sq = getLSBindex(stormPawns);
            int file = sq % 8;
            int rank = sq / 8;
            if (std::abs(file - bkFile) <= 1) {
                mgScore += pawnStormBonus[rank];
            }
            popBit(stormPawns, sq);
        }

        int wkFile = whiteKingSq % 8;
        stormPawns = board->bitboards[p];
        while (stormPawns) {
            int sq = getLSBindex(stormPawns);
            int file = sq % 8;
            int rank = 7 - sq / 8;
            if (std::abs(file - wkFile) <= 1) {
                mgScore -= pawnStormBonus[rank];
            }
            popBit(stormPawns, sq);
        }
    }

    // --- Center control ---
    {
        const U64 CENTER = (1ULL << e4) | (1ULL << d4) | (1ULL << e5) | (1ULL << d5);
        int whiteCenterCtrl = countBits(whiteAttacks & CENTER);
        int blackCenterCtrl = countBits(blackAttacks & CENTER);
        mgScore += (whiteCenterCtrl - blackCenterCtrl) * centerControlBonus[0];
        egScore += (whiteCenterCtrl - blackCenterCtrl) * centerControlBonus[1];
    }

    // --- Connected rooks (using cached rook attacks) ---
    {
        U64 whiteRooks = board->bitboards[R];
        if (countBits(whiteRooks) >= 2) {
            int r2 = getLSBindex(whiteRooks);
            if (whiteRookAttacks & (1ULL << r2)) {
                mgScore += connectedRookBonus[0];
                egScore += connectedRookBonus[1];
            }
        }
        U64 blackRooks = board->bitboards[r];
        if (countBits(blackRooks) >= 2) {
            int r2 = getLSBindex(blackRooks);
            if (blackRookAttacks & (1ULL << r2)) {
                mgScore -= connectedRookBonus[0];
                egScore -= connectedRookBonus[1];
            }
        }
    }

    // --- King adjacent open file penalty ---
    {
        int wkFile = whiteKingSq % 8;
        for (int f = std::max(0, wkFile - 1); f <= std::min(7, wkFile + 1); f++) {
            if (f == wkFile) continue;
            U64 adjFileMask = fileMasks[f];
            if (((board->bitboards[P] | board->bitboards[p]) & adjFileMask) == 0) {
                mgScore += kingAdjacentOpenFile;
            }
        }
        int bkFile = blackKingSq % 8;
        for (int f = std::max(0, bkFile - 1); f <= std::min(7, bkFile + 1); f++) {
            if (f == bkFile) continue;
            U64 adjFileMask = fileMasks[f];
            if (((board->bitboards[P] | board->bitboards[p]) & adjFileMask) == 0) {
                mgScore -= kingAdjacentOpenFile;
            }
        }
    }

    if (phase <= 3 && insufficientMaterial(board)) {
        return 0; // Draw by insufficient material
    }

    float whiteEndGamePhase = 1 - std::min(1.0f, float(whitePhase) / float(endGamePhaseMaterialScore));
    float blackEndGamePhase = 1 - std::min(1.0f, float(blackPhase) / float(endGamePhaseMaterialScore));

    int mopUpScore = MopUpEvaluation(board, white, whiteMaterial, blackMaterial, blackEndGamePhase) 
                    - MopUpEvaluation(board, black, blackMaterial, whiteMaterial, whiteEndGamePhase); 

    egScore += mopUpScore;
    phase = std::min(24, phase);
    score = (mgScore * phase + egScore * (24 - phase)) / 24;

    int scaleFactor = endgameScaleFactor(board, score);
    if (scaleFactor < 100)
        score = score * scaleFactor / 100;

    score += (board->sideToMove == white) ? tempoBonus : -tempoBonus;
    return board->sideToMove == white ? score : -score;
}


#endif // EVALUATE_H;