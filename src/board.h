#ifndef BOARD_H
#define BOARD_H

#include "constants.h"
#include "random.h"
#include <sstream>
#include <cstring>
#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

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

static U64 repetitionTable[1024];
static int repetitionIndex = 0;

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
static U64 pieceZobristKeys[12][64];
static U64 enpassantZobristKeys[65];
static U64 castlingZobristKeys[16];
static U64 sideZobristKey;

static inline void initializeRandomKeys() {
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

static inline U64 computeZobristHash(const Board *board) {
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

static inline void clearBoard(Board* board) {
    memset(board->bitboards, 0, sizeof(board->bitboards));
    memset(board->occupancies, 0, sizeof(board->occupancies));

    board->sideToMove = white; // Default to white
    board->castlingRights = 0; // No castling rights
    board->enPassantSquare = noSquare; // No en passant square
    board->zobristHash = 0ULL; // Reset Zobrist hash
    board->halfMoveClock = 0; // Reset half-move clock
}

static inline std::string boardToFEN(Board* board) {
    std::string fen;
    for (int rank = 7; rank >= 0; --rank) {
        int emptyCount = 0;
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            int piece = none;
            for (int bbIndex = 0; bbIndex < 12; ++bbIndex) {
                if (getBit(board->bitboards[bbIndex], square)) {
                    piece = bbIndex;
                    break;
                }
            }
            if (piece == none) {
                emptyCount++;
            } else {
                if (emptyCount > 0) { fen += std::to_string(emptyCount); emptyCount = 0; }
                fen += asciiPieces[piece];
            }
        }
        if (emptyCount > 0) fen += std::to_string(emptyCount);
        if (rank > 0) fen += '/';
    }

    fen += (board->sideToMove == white) ? " w " : " b ";

    if (board->castlingRights) {
        if (board->castlingRights & wk) fen += 'K';
        if (board->castlingRights & wq) fen += 'Q';
        if (board->castlingRights & bk) fen += 'k';
        if (board->castlingRights & bq) fen += 'q';
    } else {
        fen += '-';
    }

    fen += ' ';
    if (board->enPassantSquare != noSquare)
        fen += indexToSquare[board->enPassantSquare];
    else
        fen += '-';

    fen += ' ' + std::to_string(board->halfMoveClock) + " 1";
    return fen;
}

static inline void printBoard(Board* board) {
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

    std::cout << "    Hash: 0x" << std::hex << computeZobristHash(board) << std::dec << std::endl;
    std::cout << "     FEN: " << boardToFEN(board) << std::endl;

    std::cout << std::endl << std::endl;
}

static inline void parseFEN(Board* board, const std::string& fen) {
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

#define noHashEntry 32001

// TT move encoding: store only source(6) + target(6) + promoted(4) = 16 bits
static inline uint16_t moveToTTMove(int move) {
    if (move == 0) return 0;
    return (uint16_t)((move & 0x3F) | (((move >> 6) & 0x3F) << 6) | (((move >> 16) & 0xF) << 12));
}

static inline bool ttMoveMatch(int move, uint16_t ttMove) {
    if (ttMove == 0) return false;
    uint16_t compressed = moveToTTMove(move);
    return compressed == ttMove;
}

struct TTEntry {
    uint16_t key16;
    int16_t  value;
    int16_t  staticEval;
    uint16_t move;
    int8_t   depth;
    uint8_t  genBound;
};

static_assert(sizeof(TTEntry) == 10, "TTEntry must be 10 bytes");

struct TTBucket {
    TTEntry entries[4];
    uint8_t padding[24];
};

static_assert(sizeof(TTBucket) == 64, "TTBucket must be 64 bytes (one cache line)");

static TTBucket *TranspositionTable = NULL;
static uint32_t ttNumBuckets = 0;
static uint32_t ttBucketMask = 0;
static uint8_t ttGeneration = 0;
static uint32_t ttUsedEntries = 0;

static inline TTBucket* getTTBucket(U64 key) {
    return &TranspositionTable[(key >> 32) & ttBucketMask];
}

static inline void ttPrefetch(U64 key) {
#if defined(_MSC_VER)
    _mm_prefetch((const char*)getTTBucket(key), _MM_HINT_T0);
#else
    __builtin_prefetch(getTTBucket(key));
#endif
}

static inline void incrementTTGeneration() {
    ttGeneration += 4; // lower 2 bits reserved for bound type
}

static inline int ttEntryQuality(const TTEntry &e) {
    int ageDelta = ((ttGeneration & 0xFC) - (e.genBound & 0xFC)) & 0xFC;
    return (int)e.depth - (ageDelta >> 2) * 4 + ((e.genBound & 3) == hashExact ? 2 : 0);
}

static inline void clearTranspositionTable() {
    if (TranspositionTable != NULL)
        memset(TranspositionTable, 0, (size_t)ttNumBuckets * sizeof(TTBucket));
    ttUsedEntries = 0;
}

static inline uint32_t roundDownPow2(uint32_t v) {
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return (v >> 1) + 1;
}

static inline void initializeTranspositionSize(int MB) {
    if (MB <= 0) {
        std::cout << "Invalid size for transposition table, must be greater than 0 MB" << std::endl;
        return;
    }
    uint32_t totalBuckets = (uint32_t)((uint64_t)MB * 1024 * 1024 / sizeof(TTBucket));
    ttNumBuckets = roundDownPow2(totalBuckets);
    ttBucketMask = ttNumBuckets - 1;

    if (TranspositionTable != NULL) {
        delete[] TranspositionTable;
    }
    TranspositionTable = new TTBucket[ttNumBuckets];
    if (TranspositionTable == NULL) {
        std::cout << "Failed to allocate memory for transposition table, trying " << MB / 2 << " MB" << std::endl;
        initializeTranspositionSize(MB / 2);
    } else {
        clearTranspositionTable();
        std::cout << "Transposition table initialized with " << ttNumBuckets
                  << " buckets (" << ttNumBuckets * 4 << " entries)." << std::endl;
    }
}

static inline int hashfull() {
    uint32_t totalEntries = ttNumBuckets * 4;
    if (totalEntries == 0) return 0;
    return (int)((uint64_t)ttUsedEntries * 1000 / totalEntries);
}

struct TTProbeResult {
    int score;
    int ttMove;
    int ttEval;
    bool hit;
};

static inline TTProbeResult probeHashEntry(Board *board, int alpha, int beta, int depth, int ply = 0) {
    TTProbeResult result;
    result.score = noHashEntry;
    result.ttMove = 0;
    result.ttEval = -32768;
    result.hit = false;

    U64 key = board->zobristHash;
    uint16_t key16 = (uint16_t)(key & 0xFFFF);
    TTBucket *bucket = getTTBucket(key);

    for (int i = 0; i < 4; i++) {
        TTEntry *entry = &bucket->entries[i];
        if (entry->key16 == key16 && entry->genBound != 0) {
            result.hit = true;

            if (entry->move != 0)
                result.ttMove = (int)entry->move;

            if (entry->staticEval != (int16_t)-32768)
                result.ttEval = (int)entry->staticEval;

            if (entry->depth >= depth) {
                int value = (int)entry->value;

                if (value < -MATESCORE) value += ply;
                if (value > MATESCORE) value -= ply;

                int bound = entry->genBound & 3;
                if (bound == hashExact) {
                    result.score = value;
                    return result;
                }
                if (bound == hashAlpha && value <= alpha) {
                    result.score = alpha;
                    return result;
                }
                if (bound == hashBeta && value >= beta) {
                    result.score = beta;
                    return result;
                }
            }
            return result;
        }
    }
    return result;
}

static inline void writeHashEntry(Board *board, int bestMove, int value, int depth, int flag, int ply = 0, int staticEval = -32768) {
    U64 key = board->zobristHash;
    uint16_t key16 = (uint16_t)(key & 0xFFFF);
    TTBucket *bucket = getTTBucket(key);

    if (value < -MATESCORE) value -= ply;
    if (value > MATESCORE) value += ply;

    int16_t storedValue = (int16_t)std::max(-32767, std::min(32767, value));
    int16_t storedEval = (staticEval == -32768) ? (int16_t)-32768 : (int16_t)std::max(-32767, std::min(32767, staticEval));
    uint16_t storedMove = moveToTTMove(bestMove);
    uint8_t storedGenBound = (ttGeneration & 0xFC) | (flag & 3);

    TTEntry *replace = &bucket->entries[0];
    int worstQuality = ttEntryQuality(bucket->entries[0]);

    for (int i = 0; i < 4; i++) {
        TTEntry *entry = &bucket->entries[i];

        if (entry->key16 == key16 || entry->genBound == 0) {
            if (entry->genBound == 0)
                ttUsedEntries++;
            if (entry->key16 == key16 && storedMove == 0)
                storedMove = entry->move;
            if (entry->key16 == key16 && storedEval == (int16_t)-32768)
                storedEval = entry->staticEval;

            replace = entry;
            break;
        }

        int q = ttEntryQuality(*entry);
        if (q < worstQuality) {
            worstQuality = q;
            replace = entry;
        }
    }

    if (replace->key16 == key16 && replace->depth > depth + 2 &&
        flag != hashExact && (replace->genBound & 3) == hashExact)
        return;

    replace->key16 = key16;
    replace->value = storedValue;
    replace->staticEval = storedEval;
    replace->move = storedMove;
    replace->depth = (int8_t)depth;
    replace->genBound = storedGenBound;
}

#endif // BOARD_H;
