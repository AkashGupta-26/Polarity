#ifndef BOARD_H
#define BOARD_H

#include "constants.h"
#include <sstream>

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

struct Board{
    U64 bitboards[12]; // 12 types of pieces (6 white, 6 black)
    U64 occupancies[4]; // 0: white, 1: black, 2: all, 3: empty
    int sideToMove;
    int castlingRights; // Bitmask for castling rights
    int enPassantSquare; // Square for en passant capture
};

void clearBoard(Board& board) {
    memset(board.bitboards, 0, sizeof(board.bitboards));
    memset(board.occupancies, 0, sizeof(board.occupancies));

    board.sideToMove = white; // Default to white
    board.castlingRights = 0; // No castling rights
    board.enPassantSquare = noSquare; // No en passant square
}

const void printBoard(Board& board){
    U64* bitboards = board.bitboards;
    int sideToMove = board.sideToMove;
    int castlingRights = board.castlingRights;
    int enPassantSquare = board.enPassantSquare;
    
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << " ";
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
    std::cout << "  a b c d e f g h\n\n";
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
}

void parseFEN(Board& board, const std::string& fen) {
    clearBoard(board);
    std::istringstream iss(fen);
    std::string boardPart, side, castling, enPassant;
    iss >> boardPart >> side >> castling >> enPassant;

    int square = 56; // Start at a8

    // Parse board pieces
    for (char c : boardPart) {
        if (c == '/') {
            square -= 16; // Move to the next rank below
        } else if (isdigit(c)) {
            square += c - '0'; // Skip empty squares
        } else {
            int piece = pieceMap.at(c);
            setBit(board.bitboards[piece], square);
            square++;
        }
    }

    // Set occupancies
    for (int piece = P; piece <= K; ++piece) board.occupancies[white] |= board.bitboards[piece];
    for (int piece = p; piece <= k; ++piece) board.occupancies[black] |= board.bitboards[piece];
    board.occupancies[2] = board.occupancies[white] | board.occupancies[black];
    board.occupancies[3] = ~board.occupancies[2];

    // Set side to move
    board.sideToMove = (side == "w") ? white : black;

    // Set castling rights
    board.castlingRights = 0;
    for (char c : castling) {
        switch (c) {
            case 'K': board.castlingRights |= wk; break;
            case 'Q': board.castlingRights |= wq; break;
            case 'k': board.castlingRights |= bk; break;
            case 'q': board.castlingRights |= bq; break;
        }
    }

    // Set en passant square
    if (enPassant != "-") {
        int file = enPassant[0] - 'a';
        int rank = enPassant[1] - '1';
        board.enPassantSquare = rank * 8 + file;
    } else {
        board.enPassantSquare = noSquare;
    }
}

#endif
