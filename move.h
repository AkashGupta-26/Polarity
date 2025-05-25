#ifndef MOVE_H
#define MOVE_H

#include "board.h"
#include "random.h"

/*
    reference directions
    North = leftshift 8
    South = rightshift 8
    East = rightshift 1
    West = leftshift 1

    NE = Right Up diagonal = leftshift 7
    Nw = Left Up diagonal = leftshift 9

    SW = Right Down diagonal ::: Left Up diagoanl => rightshift 7
    SE = Left Down diagonal ::: Right Up diagonal => rightshift 9
*/


// Leaping pieces attack tables
U64 pawnAttacks[2][64];
U64 knightAttacks[64];
U64 kingAttacks[64];

U64 maskPawnAttacks(int side, int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    setBit(bitboard, square);
    if (side == white) {
        if (bitboard & ~H_FILE) attacks |= (bitboard << 9); // Left up diagonal attack
        if (bitboard & ~A_FILE) attacks |= (bitboard << 7); // Right up diagonal attack
    } 
    else {
        if (bitboard & ~A_FILE) attacks |= (bitboard >> 9); // left down diagonal attack
        if (bitboard & ~H_FILE) attacks |= (bitboard >> 7); // Right down diagonal attack
    }
    return attacks;
}

U64 maskKightAttacks(int square){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    setBit(bitboard, square);

    if ((bitboard) & ~H_FILE) attacks |= (bitboard << 17); //NNW
    if ((bitboard) & ~A_FILE) attacks |= (bitboard << 15); //NNE
    if ((bitboard) & ~GH_FILE) attacks |= (bitboard << 10); //NWW
    if ((bitboard) & ~AB_FILE) attacks |= (bitboard << 6); //NEE

    if ((bitboard) & ~GH_FILE) attacks |= (bitboard >> 6); //SWW
    if ((bitboard) & ~AB_FILE) attacks |= (bitboard >> 10); //SEE
    if ((bitboard) & ~H_FILE) attacks |= (bitboard >> 15); //SSW
    if ((bitboard) & ~A_FILE) attacks |= (bitboard >> 17); //SSE
    return attacks;
}

U64 maskKingAttacks(int square){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    setBit(bitboard, square);
    attacks |= (bitboard << 8); // North
    attacks |= (bitboard >> 8); // South

    if ((bitboard << 1) & ~A_FILE) attacks |= (bitboard << 1); // East
    if ((bitboard >> 1) & ~H_FILE) attacks |= (bitboard >> 1); // West
    if ((bitboard << 9) & ~A_FILE) attacks |= (bitboard << 9); // North East
    if ((bitboard << 7) & ~H_FILE) attacks |= (bitboard << 7); // North West
    if ((bitboard >> 9) & ~H_FILE) attacks |= (bitboard >> 9); // South East
    if ((bitboard >> 7) & ~A_FILE) attacks |= (bitboard >> 7); // South West
    return attacks;
}

void initializeLeaperAttacks(){
    for (int square = 0; square < 64; ++square){
        pawnAttacks[white][square] = maskPawnAttacks(white, square);
        pawnAttacks[black][square] = maskPawnAttacks(black, square);
        knightAttacks[square] = maskKightAttacks(square);
        kingAttacks[square] = maskKingAttacks(square);
    }
}

#endif // MOVE_H
