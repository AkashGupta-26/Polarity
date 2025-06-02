#ifndef PERFT_H
#define PERFT_H

#include "constants.h"
#include "board.h"
#include "precalculated_move_tables.h"
#include "moves.h"

U64 nodes = 0;

void perft(Board *board, int depth) {
    if (depth == 0) {
        nodes++;
        return;
    }

    MoveList moves[1];
    generateMoves(board, moves);
    copyBoard(board);

    for (int i = 0; i < moves->count; ++i) {
        if (!makeMove(board, moves->moves[i])) continue;

        perft(board, depth - 1);
        takeBack(board, backup);
    }
}

int perftTest(Board *board, int depth, int verbose = 0) {
    nodes = 0; // Reset nodes count
    MoveList moves[1];
    generateMoves(board, moves);
    copyBoard(board);
    U64 cumNodes = 0;
    auto startTime = TIME_IN_MICROSECONDS;
    for (int i = 0; i < moves->count; ++i) {
        int move = moves->moves[i];
        if (!makeMove(board, move)) continue;
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
