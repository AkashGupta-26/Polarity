#ifndef PERFT_H
#define PERFT_H

#include "../src/constants.h"
#include "../src/board.h"
#include "../src/precalculated_move_tables.h"
#include "../src/moves.h"

static U64 nodes = 0;

static inline void perft(Board *board, int depth) {
    if (depth == 0) {
        nodes++;
        return;
    }

    MoveList moves[1];
    generateMoves(board, moves);
    copyBoard(board);

    for (int i = 0; i < moves->count; ++i) {
        if (!makeMove(board, moves->moves[i])) { takeBack(board, backup); continue; }

        perft(board, depth - 1);
        takeBack(board, backup);
    }
}

static inline int perftTest(Board *board, int depth, int verbose = 0) {
    nodes = 0;
    MoveList moves[1];
    generateMoves(board, moves);
    copyBoard(board);
    U64 cumNodes = 0;
    auto startTime = TIME_IN_MICROSECONDS;
    for (int i = 0; i < moves->count; ++i) {
        int move = moves->moves[i];
        if (!makeMove(board, move)) { takeBack(board, backup); continue; }
        perft(board, depth - 1);
        takeBack(board, backup);
        if (verbose){
            std::cout << "Move: ";
            printMove(move);
            std::cout << " - Nodes: " << nodes - cumNodes << std::endl;
            cumNodes = nodes;
        }
    }
    auto endTime = TIME_IN_MICROSECONDS;
    if (verbose) {
        std::cout << "Time taken: " << (endTime - startTime) << " microseconds" << std::endl;
        std::cout << "Total Nodes at depth " << depth << ": " << nodes << std::endl;
        std::cout << "Average Nodes per second: " << (nodes * 1000000 / (endTime - startTime)) << std::endl;
    }
    return nodes;
}

#endif // PERFT_H;
