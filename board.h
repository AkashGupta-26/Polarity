#ifndef BOARD_H
#define BOARD_H

#include "constants.h"
#include "random.h"
#include <sstream>
#include <cstring>

/*
    This is our board representation 
    8 | a8 b8 c8 d8 e8 f8 g8 h8
    7 | a7 b7 c7 d7 e7 f7 g7 h7
    6 | a6 b6 c6 d6 e6 f6 g6 h6
    5 | a5 b5 c5 d5 e5 f5 g5 h5
    4 | a4 b4 c4 d4 e4 f4 g4 h4
    3 | a3 b3 c3 d3 e3 f3 g3 h3
    2 | a2 b2 c2 d2 e2 f2 g2 h2
    1 | a1 b1 c1 d1 e1 f1 g1 h1
      +------------------------
        a  b  c  d  e  f  g  h
    
    LSB is a1, MSB is h8

*/

#define copyBoard(board) \
    Board backup; \
    memcpy(&backup, board, sizeof(Board));

#define takeBack(board, backup) \
    memcpy(board, &backup, sizeof(Board));

U64 repetitionTable[1024];
int repetitionIndex = 0;

struct Board{
    U64 bitboards[12]; // 12 types of pieces (6 white, 6 black)
    U64 occupancies[4]; // 0: white, 1: black, 2: all, 3: empty
    int sideToMove;
    int castlingRights; // Bitmask for castling rights
    int enPassantSquare; // Square for en passant capture
    int halfMoveClock; // Half-move clock for the fifty-move rule
    U64 zobristHash; // Zobrist hash for the board state
};


// Zobrist Hashing Constants
U64 pieceZobristKeys[12][64];
U64 enpassantZobristKeys[65];
U64 castlingZobristKeys[16];
U64 sideZobristKey;

void initializeRandomKeys() {
    randomState = 1804289383; // reset random state
    for (int piece = P; piece <= k; ++piece){
        for (int square = 0; square < 64; ++square) {
            pieceZobristKeys[piece][square] = generateRandomU64();
        }
    }

    for (int square = 0; square < 64; ++square) {
        enpassantZobristKeys[square] = generateRandomU64();
    }

    enpassantZobristKeys[noSquare] = 0ULL; // No en passant square

    sideZobristKey = generateRandomU64();

    for (int i = 0; i < 16; ++i) {
        castlingZobristKeys[i] = generateRandomU64();
    }
}

U64 computeZobristHash(const Board *board) {
    U64 hash = 0ULL;
    U64 bitboard;
    for (int piece = P; piece <= k; piece++){
        bitboard = board->bitboards[piece];
        while (bitboard) {
            int square = getLSBindex(bitboard);
            hash ^= pieceZobristKeys[piece][square];
            popBit(bitboard, square);
        }
    }

    hash ^= enpassantZobristKeys[board->enPassantSquare];
    
    hash ^= castlingZobristKeys[board->castlingRights];

    if (board->sideToMove == black) 
        hash ^= sideZobristKey;

    return hash;
}

void clearBoard(Board* board) {
    memset(board->bitboards, 0, sizeof(board->bitboards));
    memset(board->occupancies, 0, sizeof(board->occupancies));

    board->sideToMove = white; // Default to white
    board->castlingRights = 0; // No castling rights
    board->enPassantSquare = noSquare; // No en passant square
    board->zobristHash = 0ULL; // Reset Zobrist hash
    board->halfMoveClock = 0; // Reset half-move clock
}

void printBoard(Board* board) {
    U64* bitboards = board->bitboards;
    int sideToMove = board->sideToMove;
    int castlingRights = board->castlingRights;
    int enPassantSquare = board->enPassantSquare;

    std::cout << "\n";
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << "  ";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            int piece = none;
            for (int bbIndex = 0; bbIndex < 12; ++bbIndex) {
                if (getBit(bitboards[bbIndex], square)) {
                    piece = bbIndex;
                    break;
                }
            }
            if (piece != none) std::cout << asciiPieces[piece] << " ";
            else std::cout << ". ";
        }
        std::cout << std::endl;
    }
    std::cout << "\n   a b c d e f g h\n\n";
    std::cout << "    Side: " << (sideToMove == white ? "white" : "black") << std::endl;
    std::cout << "Castling: ";
    if (castlingRights & wk) std::cout << "K";
    if (castlingRights & wq) std::cout << "Q";
    if (castlingRights & bk) std::cout << "k";
    if (castlingRights & bq) std::cout << "q";
    std::cout << std::endl;
    std::cout << "  Enpass: ";
    if (enPassantSquare != noSquare)
        std::cout << indexToSquare[enPassantSquare];
    else
        std::cout << "None";

    std::cout << std::endl;

    std::cout << "Hash: 0x" << std::hex << computeZobristHash(board) << std::dec << std::endl;

    std::cout << std::endl << std::endl;
}

void parseFEN(Board* board, const std::string& fen) {
    clearBoard(board);
    repetitionIndex = 0;
    std::istringstream iss(fen);
    std::string boardPart, side, castling, enPassant, halfMoveClock;;
    iss >> boardPart >> side >> castling >> enPassant >> halfMoveClock;

    int square = 56; // Start at a8

    // Parse board pieces
    for (char c : boardPart) {
        if (c == '/') {
            square -= 16; // Move to the next rank below
        } else if (isdigit(c)) {
            square += c - '0'; // Skip empty squares
        } else {
            int piece = pieceMap.at(c);
            setBit(board->bitboards[piece], square);
            square++;
        }
    }

    // Set occupancies
    for (int piece = P; piece <= K; ++piece) board->occupancies[white] |= board->bitboards[piece];
    for (int piece = p; piece <= k; ++piece) board->occupancies[black] |= board->bitboards[piece];
    board->occupancies[both] = board->occupancies[white] | board->occupancies[black];
    board->occupancies[3] = ~board->occupancies[2];

    // Set side to move
    board->sideToMove = (side == "w") ? white : black;

    // Set castling rights
    board->castlingRights = 0;
    for (char c : castling) {
        switch (c) {
            case 'K': board->castlingRights |= wk; break;
            case 'Q': board->castlingRights |= wq; break;
            case 'k': board->castlingRights |= bk; break;
            case 'q': board->castlingRights |= bq; break;
        }
    }

    // Set en passant square
    if (enPassant != "-") {
        int file = enPassant[0] - 'a';
        int rank = enPassant[1] - '1';
        board->enPassantSquare = rank * 8 + file;
    } else {
        board->enPassantSquare = noSquare;
    }

    // Set half-move clock
    board->halfMoveClock = std::stoi(halfMoveClock);

    // Update the Zobrist hash
    board -> zobristHash = computeZobristHash(board);
}

// Hash Flags
#define hashExact 0
#define hashAlpha 1
#define hashBeta 2

#define TranspositionTableEntries 0x400000 // 4 million entries
#define getTTIndex(key) ((key) % TranspositionTableEntries)

#define noHashEntry 100000 // outside alpha-beta bounds

typedef struct {
    U64 key;
    int depth;
    int flag; // 0: exact, 1: alpha, 2: beta
    int value;
} HashEntry;

// Transposition Table
HashEntry TranspositionTable[TranspositionTableEntries];

void clearTranspositionTable() {
    for (int i = 0; i < TranspositionTableEntries; ++i) {
        TranspositionTable[i].key = 0ULL;
        TranspositionTable[i].depth = 0;
        TranspositionTable[i].flag = 0;
        TranspositionTable[i].value = 0;
    }
}

static inline int readHashEntry(Board *board, int alpha, int beta, int depth, int ply = 0) {
    HashEntry *entry = &TranspositionTable[getTTIndex(board->zobristHash)];

    if (entry->key == board->zobristHash && entry->depth >= depth){
        int value = entry->value;

        if (value < -MATESCORE) value += ply; // Adjust for mate scores
        if (value > MATESCORE) value -= ply; // Adjust for mate scores

        if (entry->flag == hashExact) return value; // PV node
        if (entry->flag == hashAlpha && value <= alpha) return alpha; // fails low
        if (entry->flag == hashBeta && value >= beta) return beta; // fails high
    }

    return noHashEntry; // No valid entry found
}

static inline void writeHashEntry(Board *board, int value, int depth, int flag, int ply = 0) {
    HashEntry *entry = &TranspositionTable[getTTIndex(board->zobristHash)];

    if (value < -MATESCORE) value -= ply;
    if (value > MATESCORE) value += ply;

    entry->key = board->zobristHash;
    entry->depth = depth;
    entry->flag = flag;
    entry->value = value;
}

#endif // BOARD_H;
