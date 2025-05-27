#ifndef MOVES_H
#define MOVES_H

#include "board.h"
#include "precalculated_move_tables.h"

/*
wk = 0001, wq = 0010, bk = 0100, bq = 1000

kings and rook dont move - 1111 & 1111 = 1111
white king moved - 1111 & 1100 = 1100
white K rook moved - 1111 & 1110 = 1110
white Q rook moved - 1111 & 1101 = 1101

black king moved - 1111 & 0011 = 0011
black K rook moved - 1111 & 1011 = 1011 
black Q rook moved - 1111 & 0111 = 0111
*/

const int castlingUpdate[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
     7, 15, 15, 15,  3, 15, 15, 11,
};

static inline int isSquareAttacked(const Board *board, int square, int side) {
    // Check if the square is attacked by any piece of the side given
    if ((side == white) && (pawnAttacks[black][square] & board->bitboards[P])) return 1;
    if ((side == black) && (pawnAttacks[white][square] & board->bitboards[p])) return 1;
    if (knightAttacks[square] & ((side == white) ? board->bitboards[N] : board->bitboards[n])) return 1;
    if (kingAttacks[square] & ((side == white) ? board->bitboards[K] : board->bitboards[k])) return 1;

    if (getRookAttacks(square, board->occupancies[both]) & ((side == white) ? board->bitboards[R] : board->bitboards[r])) return 1;
    if (getBishopAttacks(square, board->occupancies[both]) & ((side == white) ? board->bitboards[B] : board->bitboards[b])) return 1;
    if (getQueenAttacks(square, board->occupancies[both]) & ((side == white) ? board->bitboards[Q] : board->bitboards[q])) return 1;
    return 0;
}

void printAttackedSquares(const Board *board, int side) {
    std::cout << "Attacked squares by " << (side == white ? "White" : "Black") << ":\n";
    U64 attacked = 0ULL;
    for (int square = 0; square < 64; ++square) {
        if (isSquareAttacked(board, square, side)) setBit(attacked, square);
    }
    printBitboard(attacked);
    std::cout << "Total attacked squares: " << countBits(attacked) << std::endl << std::endl;
}

/*
    Move encoding scheme: 
    0000 0000 0000 0000 0011 1111    0x3f        source square  
    0000 0000 0000 1111 1100 0000    0xfc0       target square
    0000 0000 1111 0000 0000 0000    0xf000      piece
    0000 1111 0000 0000 0000 0000    0xf0000     promoted piece
    0001 0000 0000 0000 0000 0000    0x100000    capture flag
    0010 0000 0000 0000 0000 0000    0x200000    double pawn push flag
    0100 0000 0000 0000 0000 0000    0x400000    enpassant flag
    1000 0000 0000 0000 0000 0000    0x800000    castle flag
*/

#define encodeMove(source, target, piece, promoted, capture, doublePush, enPassant, castling) \
    ((source) | ((target) << 6) | ((piece) << 12) | ((promoted) << 16) | ((capture) << 20) | \
     ((doublePush) << 21) | ((enPassant) << 22) | ((castling) << 23))

#define decodeSource(move) ((move) & 0x3F)
#define decodeTarget(move) (((move) >> 6) & 0x3F)
#define decodePiece(move) (((move) >> 12) & 0xF)
#define decodePromoted(move) (((move) >> 16) & 0xF)
#define decodeCapture(move) (((move) >> 20) & 0x1)
#define decodeDoublePush(move) (((move) >> 21) & 0x1)
#define decodeEnPassant(move) (((move) >> 22) & 0x1)
#define decodeCastling(move) (((move) >> 23) & 0x1)

typedef struct{
    int moves[300];
    int count;
} MoveList;

static inline void addMove(MoveList *list, int move) {
    if (list->count < 300)
        list->moves[list->count++] = move;
    else printf("Move list is full, cannot add more moves.\nLast move: %d\n", move);
}

static inline void generateMoves(Board *board, MoveList *moves) {
    moves->count = 0;
    
    int source, target;
    U64 bitboard, attacks;

    // Pawn moves
    if (board -> sideToMove == white){
        bitboard = board->bitboards[P];
        //Quiet Moves
        U64 singlePush = bitboard << 8;
        singlePush &= board->occupancies[3];
        U64 doublePush = (singlePush & RANK_3) << 8;
        doublePush &= board->occupancies[3];
        
        while (singlePush) {
            target = getLSBindex(singlePush);
            source = target - 8;
            if (target >= a8 && target <= h8) {
                // Promotion
                addMove(moves, encodeMove(source, target, P, Q, 0, 0, 0, 0));
                addMove(moves, encodeMove(source, target, P, R, 0, 0, 0, 0));
                addMove(moves, encodeMove(source, target, P, B, 0, 0, 0, 0));
                addMove(moves, encodeMove(source, target, P, N, 0, 0, 0, 0));
            } 
            else {
                addMove(moves, encodeMove(source, target, P, 0, 0, 0, 0, 0));
            }
            popBit(singlePush, target); // Clear the least significant bit
        }

        while (doublePush) {
            target = getLSBindex(doublePush);
            source = target - 16;
            addMove(moves, encodeMove(source, target, P, 0, 0, 1, 0, 0));
            popBit(doublePush, target); // Clear the least significant bit
        }
        // Capture Moves
        while(bitboard) {
            source = getLSBindex(bitboard);
            // enpassant square can also be treated as occupied
            attacks = pawnAttacks[white][source] & ((board->occupancies[black] | (1ULL << board->enPassantSquare)));
            while (attacks) {
                target = getLSBindex(attacks);
                if (target >= a8 && target <= h8) {
                    // Promotion
                    addMove(moves, encodeMove(source, target, P, Q, 1, 0, 0, 0));
                    addMove(moves, encodeMove(source, target, P, R, 1, 0, 0, 0));
                    addMove(moves, encodeMove(source, target, P, B, 1, 0, 0, 0));
                    addMove(moves, encodeMove(source, target, P, N, 1, 0, 0, 0));
                } 
                else if (target == board->enPassantSquare) {
                    addMove(moves, encodeMove(source, target, P, 0, 1, 0, 1, 0));
                } 
                else {
                    addMove(moves, encodeMove(source, target, P, 0, 1, 0, 0, 0));
                }
                popBit(attacks, target); // Clear the least significant bit
            }
            
            popBit(bitboard, source); // Clear the least significant bit
        }
    }
    else{
        bitboard = board->bitboards[p];
        //Quiet Moves
        U64 singlePush = bitboard >> 8;
        singlePush &= board->occupancies[3];
        U64 doublePush = (singlePush & RANK_6) >> 8;
        doublePush &= board->occupancies[3];

        while (singlePush) {
            target = getLSBindex(singlePush);
            source = target + 8;
            if (target >= a1 && target <= h1) {
                // Promotion
                addMove(moves, encodeMove(source, target, p, q, 0, 0, 0, 0));
                addMove(moves, encodeMove(source, target, p, r, 0, 0, 0, 0));
                addMove(moves, encodeMove(source, target, p, b, 0, 0, 0, 0));
                addMove(moves, encodeMove(source, target, p, n, 0, 0, 0, 0));
            } 
            else {
                addMove(moves, encodeMove(source, target, p, 0, 0, 0, 0, 0));
            }
            popBit(singlePush, target); // Clear the least significant bit
        }

        while (doublePush) {
            target = getLSBindex(doublePush);
            source = target + 16;
            addMove(moves, encodeMove(source, target, p, 0, 0, 1, 0, 0));
            popBit(doublePush, target); // Clear the least significant bit
        }
        // Capture Moves
        while(bitboard) {
            source = getLSBindex(bitboard);
            // enpassant square can also be treated as occupied
            attacks = pawnAttacks[black][source] & ((board->occupancies[white] | (1ULL << board->enPassantSquare)));
            while (attacks) {
                target = getLSBindex(attacks);
                if (target >= a1 && target <= h1) {
                    // Promotion
                    addMove(moves, encodeMove(source, target, p, q, 1, 0, 0 ,0));
                    addMove(moves, encodeMove(source, target, p ,r ,1 ,0 ,0 ,0));
                    addMove(moves, encodeMove(source, target, p, b, 1, 0, 0 ,0));
                    addMove(moves, encodeMove(source, target, p, n, 1, 0, 0 ,0));
                }
                else if (target == board->enPassantSquare) {
                    addMove(moves, encodeMove(source, target, p, 0, 1, 0, 1, 0));
                } 
                else {
                    addMove(moves, encodeMove(source, target, p, 0, 1, 0, 0, 0));
                }
                popBit(attacks, target); // Clear the least significant bit
            }
            popBit(bitboard, source); // Clear the least significant bit
        }
    }

    // Normal King Moves
    bitboard = board->bitboards[board->sideToMove == white ? K : k];
    while (bitboard){
        source = getLSBindex(bitboard);
        attacks = kingAttacks[source] & ((board->sideToMove == white) ? ~board->occupancies[white] : ~board->occupancies[black]);
        while (attacks) {
            target = getLSBindex(attacks);
            //quiet move
            if (!getBit((board->sideToMove == white)?board->occupancies[black]:board->occupancies[white], target)) {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? K : k, 0, 0, 0, 0, 0));
            }
            //capture move
            else {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? K : k, 0, 1, 0, 0, 0));
            }
            popBit(attacks, target); // Clear the least significant bit
        }
        popBit(bitboard, source); // Clear the least significant bit
    }

    // Castle Moves
    if (board->sideToMove == white){
        if (board->castlingRights & wk){
            if (!getBit(board->occupancies[both], f1) && !getBit(board->occupancies[both], g1)) {
                if (!isSquareAttacked(board, e1, black) && !isSquareAttacked(board, f1, black)) {
                    addMove(moves, encodeMove(e1, g1, K, 0, 0, 0, 0, 1)); // O-O
                }
            }
        }
        if (board->castlingRights & wq){
            if (!getBit(board->occupancies[both], d1) && !getBit(board->occupancies[both], c1) && !getBit(board->occupancies[both], b1)) {
                if (!isSquareAttacked(board, e1, black) && !isSquareAttacked(board, d1, black)) {
                    addMove(moves, encodeMove(e1, c1, K, 0, 0, 0, 0, 1)); // O-O-O
                }
            }
        }
    }
    else {
        if (board->castlingRights & bk){
            if (!getBit(board->occupancies[both], f8) && !getBit(board->occupancies[both], g8)) {
                if (!isSquareAttacked(board, e8, white) && !isSquareAttacked(board, f8, white)) {
                    addMove(moves, encodeMove(e8, g8, k, 0, 0, 0, 0, 1)); // O-O
                }
            }
        }
        if (board->castlingRights & bq){
            if (!getBit(board->occupancies[both], d8) && !getBit(board->occupancies[both], c8) && !getBit(board->occupancies[both], b8)) {
                if (!isSquareAttacked(board, e8, white) && !isSquareAttacked(board, d8, white)) {
                    addMove(moves, encodeMove(e8, c8, k, 0, 0, 0, 0, 1)); // O-O-O
                }
            }
        }
    }
    
    // Knight Moves
    bitboard = board->bitboards[(board->sideToMove == white) ? N : n];
    while (bitboard) {
        source = getLSBindex(bitboard);
        attacks = knightAttacks[source] & ((board->sideToMove == white) ? ~board->occupancies[white] : ~board->occupancies[black]);
        while (attacks) {
            target = getLSBindex(attacks);
            //quiet move
            if (!getBit((board->sideToMove == white)?board->occupancies[black]:board->occupancies[white], target)) {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? N : n, 0, 0, 0, 0, 0));
            }
            //capture move
            else {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? N : n, 0, 1, 0, 0, 0));
            }
            popBit(attacks, target); // Clear the least significant bit
        }
        popBit(bitboard, source); // Clear the least significant bit
    }

    // Bishop Moves
    bitboard = board->bitboards[(board->sideToMove == white) ? B : b];
    while (bitboard) {
        source = getLSBindex(bitboard);
        attacks = getBishopAttacks(source, board->occupancies[both]) & ((board->sideToMove == white) ? ~board->occupancies[white] : ~board->occupancies[black]);
        while (attacks) {
            target = getLSBindex(attacks);
            //quiet move
            if (!getBit((board->sideToMove == white)?board->occupancies[black]:board->occupancies[white], target)) {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? B : b, 0, 0, 0, 0, 0));
            }
            //capture move
            else {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? B : b, 0, 1, 0, 0, 0));
            }
            popBit(attacks, target); // Clear the least significant bit
        }
        popBit(bitboard, source); // Clear the least significant bit
    }

    // Rook Moves
    bitboard = board->bitboards[(board->sideToMove == white) ? R : r];
    while (bitboard) {
        source = getLSBindex(bitboard);
        attacks = getRookAttacks(source, board->occupancies[both]) & ((board->sideToMove == white) ? ~board->occupancies[white] : ~board->occupancies[black]);
        while (attacks) {
            target = getLSBindex(attacks);
            //quiet move
            if (!getBit((board->sideToMove == white)?board->occupancies[black]:board->occupancies[white], target)) {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? R : r, 0, 0, 0, 0, 0));
            }
            //capture move
            else {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? R : r, 0, 1, 0, 0, 0));
            }
            popBit(attacks, target); // Clear the least significant bit
        }
        popBit(bitboard, source); // Clear the least significant bit
    }

    // Queen Moves
    bitboard = board->bitboards[(board->sideToMove == white) ? Q : q];
    while (bitboard) {
        source = getLSBindex(bitboard);
        attacks = getQueenAttacks(source, board->occupancies[both]) & ((board->sideToMove == white) ? ~board->occupancies[white] : ~board->occupancies[black]);
        while (attacks) {
            target = getLSBindex(attacks);
            //quiet move
            if (!getBit((board->sideToMove == white)?board->occupancies[black]:board->occupancies[white], target)) {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? Q : q, 0, 0, 0, 0, 0));
            }
            //capture move
            else {
                addMove(moves, encodeMove(source, target, (board->sideToMove == white) ? Q : q, 0, 1, 0, 0, 0));
            }
            popBit(attacks, target); // Clear the least significant bit
        }
        popBit(bitboard, source); // Clear the least significant bit
    }
}

std::string moveToUCI(int move) {
    int source = decodeSource(move);
    int target = decodeTarget(move);
    int promoted = decodePromoted(move);

    std::string result;
    result += 'a' + (source % 8);
    result += '1' + (source / 8);
    result += 'a' + (target % 8);
    result += '1' + (target / 8);

    if (promoted) {
        char promoChar = 'q'; // default
        switch (promoted) {
            case N:
            case n: promoChar = 'n'; break;
            case B:
            case b: promoChar = 'b'; break;
            case R:
            case r: promoChar = 'r'; break;
            case Q:
            case q: promoChar = 'q'; break;
        }
        result += promoChar;
    }

    return result;
}


void printMove(int move){
    std::cout << "Move: " << moveToUCI(move) << std::endl;
}

void printMoveList(const MoveList *list) {
    int move, source, target, piece, promotedPiece, capture, doublePush, enPassant, castling;
    
    std::cout << "Move: " << "Piece " << "Capture " << "Double " << "Enpass " << "Castle" << std::endl;
    
    for (int moveIndex = 0; moveIndex < list->count; ++moveIndex) {
        move = list->moves[moveIndex];
        source = decodeSource(move);
        target = decodeTarget(move);
        piece = decodePiece(move);
        promotedPiece = decodePromoted(move);
        capture = decodeCapture(move);
        doublePush = decodeDoublePush(move);
        enPassant = decodeEnPassant(move);
        castling = decodeCastling(move);

        std::cout << indexToSquare[source] 
                  << indexToSquare[target]
                  << promotedPieceMap.at(promotedPiece) << " "
                  << asciiPieces[piece] << "     "
                  << (capture ? "1" : "0") << "       "
                  << (doublePush ? "1" : "0") << "      "
                  << (enPassant ? "1" : "0") << "      "
                  << (castling ? "1" : "0") << std::endl;
    }

    std::cout << "Total Moves: " << list->count << std::endl << std::endl;
}


static inline int makeMove(Board *board, int move, int onlyCaptures = 0) {
    if (onlyCaptures && !decodeCapture(move)) {
        return 0;
    }

    copyBoard(board); // Backup the current board state

    int source = decodeSource(move);
    int target = decodeTarget(move);
    int piece = decodePiece(move);
    int promotedPiece = decodePromoted(move);
    int capture = decodeCapture(move);
    int doublePush = decodeDoublePush(move);
    int enPass = decodeEnPassant(move);
    int castling = decodeCastling(move);

    popBit(board->bitboards[piece], source);
    setBit(board->bitboards[piece], target);

    if (capture) {
        int startPiece = (board->sideToMove == white) ? p : P;
        int endPiece = (board->sideToMove == white) ? k : K;
        for (int bbPiece = startPiece; bbPiece <= endPiece; ++bbPiece) {
            if (getBit(board->bitboards[bbPiece], target)) {
                popBit(board->bitboards[bbPiece], target);
                break;
            }
        }
    }

    if (promotedPiece) {
        popBit(board->bitboards[piece], target);
        setBit(board->bitboards[promotedPiece], target);
    }

    if (enPass) {
        popBit(board->bitboards[(board->sideToMove == white) ? p : P], target + (board->sideToMove == white ? -8 : 8));
    }

    board->enPassantSquare = noSquare;
    if (doublePush) {
        board->enPassantSquare = target + (board->sideToMove == white ? -8 : 8);
    }

    if (castling) {
        switch (target) {
            case g1:
                popBit(board->bitboards[R], h1);
                setBit(board->bitboards[R], f1);
                break;
            case c1:
                popBit(board->bitboards[R], a1);
                setBit(board->bitboards[R], d1);
                break;
            case g8:
                popBit(board->bitboards[r], h8);
                setBit(board->bitboards[r], f8);
                break;
            case c8:
                popBit(board->bitboards[r], a8);
                setBit(board->bitboards[r], d8);
                break;
        }
    }

    board->castlingRights &= castlingUpdate[source];
    board->castlingRights &= castlingUpdate[target];

    memset(board->occupancies, 0ULL, sizeof(board->occupancies));
    board->occupancies[white] = board->bitboards[P] | board->bitboards[N] | board->bitboards[B] | board->bitboards[R] | board->bitboards[Q] | board->bitboards[K];
    board->occupancies[black] = board->bitboards[p] | board->bitboards[n] | board->bitboards[b] | board->bitboards[r] | board->bitboards[q] | board->bitboards[k];
    board->occupancies[both] = board->occupancies[white] | board->occupancies[black];
    board->occupancies[3] = ~board->occupancies[both]; // Empty squares

    board->sideToMove ^= 1;

    int kingSquare = (board->sideToMove == white)
        ? getLSBindex(board->bitboards[k])
        : getLSBindex(board->bitboards[K]);

    if (isSquareAttacked(board, kingSquare, board->sideToMove)) {
        takeBack(board, backup); // Restore the board
        return 0; // Illegal move
    }

    return 1;
}


#endif // MOVES_H;