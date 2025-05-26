#ifndef SEARCH_H
#define SEARCH_H

#include "evaluate.h"

int ply = 0; // half-move counter
int searchedNodes = 0; // total nodes searched
int bestMove = 0;

static inline int negamax(Board *board, int alpha, int beta, int depth){
    if (depth == 0) {
        return evaluate(board);
    }
    searchedNodes++;

    int bestSoFar; 

    // old alpha 
    int oldAlpha = alpha;

    MoveList moveList;
    generateMoves(board, &moveList);

    for (int count = 0; count < moveList.count; ++count) {
        copyBoard(board); // Backup the current board state
        ply++;
        if (makeMove(board, moveList.moves[count]) == 0){
            ply--;
            takeBack(board, backup); // Restore the board state
            continue; // Skip illegal moves
        }
        int score = -negamax(board, -beta, -alpha, depth - 1);

        ply--;
        takeBack(board, backup); // Restore the board state
        
        // fail-hard beta cutoff
        if (score >= beta) {
            return beta; // Beta cutoff fails high
        }

        if (score > alpha) {
            // PV node
            alpha = score;
            if (ply == 0){
                bestSoFar = moveList.moves[count]; // Store the best move at root
            }
        }
    }
    if (alpha != oldAlpha){
        bestMove = bestSoFar;
    }
    // node fails low
    return alpha; // Return the best score found
}

void searchPosition(Board *board, int depth) {
    int score = negamax(board, -50000, 50000, depth);
    std::cout << "BestMove: ";
    printMove(bestMove);
    std::cout << " Score: " << score << std::endl;
    std::cout << "Searched Nodes: " << searchedNodes << std::endl;
}

#endif // EVALUATE_H;