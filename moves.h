#ifndef MOVES_H
#define MOVES_H

#include "board.h"
#include "precalculated_move_tables.h"

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

void generateMoves(Board *board, MoveList *moves) {
    moves->count = 0;
    // Pawn moves
    int move;
    int source, target;
    U64 bitboard, attacks;
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
}

void printMove(int move){
    int source = decodeSource(move);
    int target = decodeTarget(move);
    int promotedPiece = decodePromoted(move);
    //std:: cout << "Promoted Piece " << promotedPiece << std::endl;

    std::cout << indexToSquare[source]
              << indexToSquare[target]
              << promotedPieceMap.at(promotedPiece);
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
}

#endif // MOVES_H;