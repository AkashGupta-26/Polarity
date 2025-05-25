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

const int bishop_relevant_bits[] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6,
};

const int rook_relevant_bits[] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12,
};

// Magic Numbers for slider pieces
U64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL,
};

U64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL,
};

// Bishop attack masks
U64 bishop_masks[64];
U64 bishop_attacks[64][512];

// Rook attack masks
U64 rook_masks[64];
U64 rook_attacks[64][4096];

// mask bishop attacks
U64 mask_bishop_attacks(int square){
    U64 attacks = 0ULL;
    int r, f;
    int tr, tf;
    tr = square / 8, tf = square % 8;
    for (r = tr + 1, f = tf + 1; r < 7 && f < 7; r++, f++)
        setBit(attacks, r * 8 + f);
    for (r = tr + 1, f = tf - 1; r < 7 && f > 0; r++, f--)
        setBit(attacks, r * 8 + f);
    for (r = tr - 1, f = tf + 1; r > 0 && f < 7; r--, f++)
        setBit(attacks, r * 8 + f);
    for (r = tr - 1, f = tf - 1; r > 0 && f > 0; r--, f--)
        setBit(attacks, r * 8 + f);
    return attacks;
}

//mask rook attacks

U64 mask_rook_attacks(int square){
    U64 attacks = 0ULL;
    int r, f;
    int tr, tf;
    tr = square / 8, tf = square % 8;
    for (r = tr + 1; r < 7; r++)
        setBit(attacks, r * 8 + tf);
    for (r = tr - 1; r > 0; r--)
        setBit(attacks, r * 8 + tf);
    for (f = tf + 1; f < 7; f++)
        setBit(attacks, tr * 8 + f);
    for (f = tf - 1; f > 0; f--)
        setBit(attacks, tr * 8 + f);
    return attacks;
}

// generate bishop attacks on the fly
U64 bishop_attacks_on_the_fly(int square, U64 blockers){
    U64 attacks = 0ULL;
    int r, f;
    int tr, tf;
    tr = square / 8, tf = square % 8;
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++){
        setBit(attacks, r * 8 + f);
        if (getBit(blockers, r * 8 + f)) break;
    }
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--){
        setBit(attacks, r * 8 + f);
        if (getBit(blockers, r * 8 + f)) break;
    }
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++){
        setBit(attacks, r * 8 + f);
        if (getBit(blockers, r * 8 + f)) break;
    }
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--){
        setBit(attacks, r * 8 + f);
        if (getBit(blockers, r * 8 + f)) break;
    }
    return attacks;
}

// generate rook attacks on the fly
U64 rook_attacks_on_the_fly(int square, U64 blockers){
    U64 attacks = 0ULL;
    int r, f;
    int tr, tf;
    tr = square / 8, tf = square % 8;
    for (r = tr + 1; r <= 7; r++){
        setBit(attacks, r * 8 + tf);
        if (getBit(blockers, r * 8 + tf)) break;
    }
    for (r = tr - 1; r >= 0; r--){
        setBit(attacks, r * 8 + tf);
        if (getBit(blockers, r * 8 + tf)) break;
    }
    for (f = tf + 1; f <= 7; f++){
        setBit(attacks, tr * 8 + f);
        if (getBit(blockers, tr * 8 + f)) break;
    }
    for (f = tf - 1; f >= 0; f--){
        setBit(attacks, tr * 8 + f);
        if (getBit(blockers, tr * 8 + f)) break;
    }
    return attacks;
}

//set occupancies
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask){
    U64 occupancy = 0ULL;
    for (int i = 0; i < bits_in_mask; i++){
        int square = getLSBindex(attack_mask);
        popBit(attack_mask, square);
        if (index & (1 << i)) setBit(occupancy, square);
    }
    return occupancy;
}

/*  =========================
        Magic Numbers
    =========================
*/

U64 find_magic_number(int square, int relevant_bits, int bishop){
    U64 occupancies[4096];
    U64 attacks[4096];
    U64 used_attacks[4096];

    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
    
    int occupancy_indicies = 1 << relevant_bits;

    for (int index = 0; index < occupancy_indicies; index ++){
        occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);
        attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index]) : 
                                    rook_attacks_on_the_fly(square, occupancies[index]);
    }

    for (int random_count = 0; random_count < 100000000; random_count++){
        U64 magic_number = generateMagicNumber();
        if (countBits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

        memset(used_attacks, 0ULL, sizeof(used_attacks));

        int index, fail;

        for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++){
            int magic_index = occupancies[index] * magic_number >> (64 - relevant_bits);
            if (used_attacks[magic_index] == 0ULL) used_attacks[magic_index] = attacks[index];
            else if (used_attacks[magic_index] != attacks[index]) fail = 1;
        }
        if (!fail) return magic_number;
    }
    printf(" Magic Number not found");
    return 0ULL;
}

void init_magic_numbers(){
    for (int square = 0; square < 64; square++){
        //printf(" 0x%llxULL,\n", find_magic_number(square, rook_relevant_bits[square], rook));
        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);
    }
    //printf("\n");
    for (int square = 0; square < 64; square++){
        //printf(" 0x%llxULL,\n", find_magic_number(square, bishop_relevant_bits[square], bishop));
        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
    }
}

// slider piece attack tables

void init_slider_attacks(int bishop){

    for (int square = 0; square < 64; square++){
        // init bishop & rook masks
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];
        int relevant_bits = countBits(attack_mask);
        int occupancy_indicies = (1 << relevant_bits);
        
        for (int index = 0; index < occupancy_indicies; index++){
            if (bishop){
                U64 occupancy = set_occupancy(index, relevant_bits, attack_mask);
                int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
                bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
            }
            else{
                U64 occupancy = set_occupancy(index, relevant_bits, attack_mask);
                int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
                rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
            }
        }
    }
}   

// get bishop attacks
static inline U64 get_bishop_attacks(int square, U64 occupancy){
    // get bishop attacks assuming occupancy represents current board state
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= (64 - bishop_relevant_bits[square]);
    return bishop_attacks[square][occupancy]; //attacks
}

static inline U64 get_rook_attacks(int square, U64 occupancy){
    // get rook attacks assuming occupancy represents current board state
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= (64 - rook_relevant_bits[square]);
    return rook_attacks[square][occupancy]; //attacks
}

static inline U64 get_queen_attacks(int square, U64 occupancy){
    return get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
}



#endif // MOVE_H
