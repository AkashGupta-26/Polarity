#ifndef SEARCH_H
#define SEARCH_H

#include "evaluate.h"
#include <algorithm>

// Most Valuable Victim - Least Valuable Aggressor (MVV-LVA) table
// to avoid runtime calculations we are using a 12x12 array instead of 6x6
// MVV LVA [attacker][victim]

static int MvvLva[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

int ply = 0; // half-move counter
int searchedNodes = 0; // total nodes searched
int bestMove = 0;


static inline int scoreMove(Board *board, int move) {
    int piece = decodePiece(move); 
    int target = decodeTarget(move);

    if (decodeCapture(move)){
        int capturedPiece;
        if (decodeEnPassant(move)) return MvvLva[P][p];
        int startPiece = (board->sideToMove == white) ? p : P;
        int endPiece = (board->sideToMove == white) ? k : K;
        for (int bbPiece = startPiece; bbPiece <= endPiece; ++bbPiece) {
            if (getBit(board->bitboards[bbPiece], target)) {
                capturedPiece = bbPiece;
                break;
            }
        }
        return MvvLva[piece][capturedPiece];
    }
    else{
        return 0;
    }
    return 0;
}

void printMoveScores(Board *board, MoveList *list) {
    std::cout << "Move Scores:" << std::endl;
    for (int i = 0; i < list->count; ++i) {
        int move = list->moves[i];
        int score = scoreMove(board, move);
        std::cout << "Move: " << moveToUCI(move) << " Score: " << score << std::endl;
    }
}

struct ScoredMove {
    int move;
    int score;
};

static inline void sortMoves(Board *board, MoveList *list) {
    int n = list->count;
    if (n <= 1) return;

    ScoredMove scoredMoves[300];
    for (int i = 0; i < n; ++i) {
        scoredMoves[i] = { list->moves[i], scoreMove(board, list->moves[i]) };
    }

    std::sort(scoredMoves, scoredMoves + n, [](const ScoredMove &a, const ScoredMove &b) {
        return a.score > b.score; // Descending
    });

    for (int i = 0; i < n; ++i) {
        list->moves[i] = scoredMoves[i].move;
    }
}


static inline int quiescienceSearch(Board *board, int alpha, int beta) {
    // evaluate position
    searchedNodes++;

    int eval = evaluate(board);

    if (eval >= beta) 
        return beta; // Beta cutoff fails high

    if (eval > alpha) 
        alpha = eval; // PV node

    MoveList moveList;
    generateMoves(board, &moveList);
    sortMoves(board, &moveList); // Sort moves by score

    for (int count = 0; count < moveList.count; ++count) {
        copyBoard(board); // Backup the current board state
        ply++;
        if (makeMove(board, moveList.moves[count], OnlyCaptures) == 0){
            ply--;
            continue; // Skip illegal moves
        }

        int score = -quiescienceSearch(board, -beta, -alpha);

        ply--;
        takeBack(board, backup); // Restore the board state
        
        // fail-hard beta cutoff
        if (score >= beta) {
            return beta; // Beta cutoff fails high
        }

        if (score > alpha) {
            // PV node
            alpha = score;
        }
    }
    // node fails low
    return alpha;
}

static inline int negamax(Board *board, int alpha, int beta, int depth){
    if (depth == 0) {
        return quiescienceSearch(board, alpha, beta); // Quiescence search at leaf nodes
    }
    searchedNodes++;
    
    // checks if the king of the side to move is in check
    int inCheck = isSquareAttacked(board, getLSBindex(board->bitboards[(board->sideToMove == white) ? K : k]), board->sideToMove ^ 1);
    
    // increase search depth for checks;
    if (inCheck) {
        depth++;
    }
    
    int legalMoves = 0;
    int bestSoFar; 

    // old alpha 
    int oldAlpha = alpha;

    MoveList moveList;
    generateMoves(board, &moveList);

    sortMoves(board, &moveList); // Sort moves by score

    for (int count = 0; count < moveList.count; ++count) {
        copyBoard(board); // Backup the current board state
        ply++;
        if (makeMove(board, moveList.moves[count]) == 0){
            ply--;
            continue; // Skip illegal moves
        }
        legalMoves++;
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

    if (legalMoves == 0) {
        // If no legal moves, check for checkmate or stalemate
        if (inCheck) {
            return -49000 + ply; // Checkmate
        } else {
            return 0; // Stalemate
        }
    }

    if (alpha != oldAlpha){
        bestMove = bestSoFar;
    }
    // node fails low
    return alpha; // Return the best score found
}

void searchPosition(Board *board, int depth) {
    ply = 0; // Reset half-move counter
    searchedNodes = 0; // Reset total nodes searched
    int score = negamax(board, -50000, 50000, depth);
    std::cout << "info score cp " << score << " depth " << depth << " nodes " << searchedNodes << std::endl;
    std::cout << "bestmove " << moveToUCI(bestMove) << std::endl;
}

#endif // SEARCH_H;