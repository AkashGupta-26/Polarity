#ifndef SEARCH_H
#define SEARCH_H

#include "evaluate.h"
#include <algorithm>

struct ScoredMove {
    int move;
    int score;
};

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

const int maxPly = 64;

int killerMoves[2][maxPly];

int historyMoves[12][maxPly];

int pvLength[maxPly]; 

int pvTable[maxPly][maxPly]; // principal variation table

// follow PV flags
int followPV, scorePV;

int ply; // half-move counter
int searchedNodes; // total nodes searched

static inline void enablePVscoring(Board *board, MoveList *list) {
    
    followPV = 0;
    for (int i = 0; i < list->count; i++){
        if (pvTable[0][ply] == list->moves[i]) {
            scorePV = 1;
            followPV = 1;
        }
    }
}

static inline int scoreMove(Board *board, int move) {
    int piece = decodePiece(move); 
    int target = decodeTarget(move);

    if (scorePV){
        if (pvTable[0][ply] == move) {
            scorePV = 0;
            return 20000; // Highest score for principal variation moves
        }
    }

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
        return MvvLva[piece][capturedPiece] + 10000;
    }
    else{
        // score killer moves
        if (killerMoves[0][ply] == move) return 9000;
        if (killerMoves[1][ply] == move) return 8000;
        // score history move
        return historyMoves[piece][target] + 1000;
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

    int foundPV = 0;
    pvLength[ply] = ply;

    if (depth == 0) {
        return quiescienceSearch(board, alpha, beta); // Quiescence search at leaf nodes
    }

    if (ply > maxPly - 1) {
        return evaluate(board); // Return evaluation if maximum ply is reached
    }

    searchedNodes++;
    
    // checks if the king of the side to move is in check
    int inCheck = isSquareAttacked(board, getLSBindex(board->bitboards[(board->sideToMove == white) ? K : k]), board->sideToMove ^ 1);
    
    // increase search depth for checks;
    if (inCheck) {
        depth++;
    }
    
    int legalMoves = 0;

    MoveList moveList;
    generateMoves(board, &moveList);

    if (followPV) enablePVscoring(board, &moveList); // Enable PV scoring if following principal variation

    sortMoves(board, &moveList); // Sort moves by score

    for (int count = 0; count < moveList.count; ++count) {
        copyBoard(board); // Backup the current board state
        ply++;
        if (makeMove(board, moveList.moves[count]) == 0){
            ply--;
            continue; // Skip illegal moves
        }
        legalMoves++;
        int score;
        
        if (foundPV){
            score = -negamax(board, -alpha - 1, -alpha, depth - 1);
            if (score > alpha && score < beta) {
                score = -negamax(board, -beta, -alpha, depth - 1); // Re-search with a full window
            }
        }
        else
            score = -negamax(board, -beta, -alpha, depth - 1);

        ply--;
        takeBack(board, backup); // Restore the board state
        
        // fail-hard beta cutoff
        if (score >= beta) {
            if (!decodeCapture(moveList.moves[count])){
                killerMoves[1][ply] = killerMoves[0][ply]; 
                killerMoves[0][ply] = moveList.moves[count];
            }
            return beta; // Beta cutoff fails high
        }

        if (score > alpha) {
            // PV node
            if (!decodeCapture(moveList.moves[count])){
                historyMoves[decodePiece(moveList.moves[count])][decodeTarget(moveList.moves[count])] += depth; // Update history move
            }
            alpha = score;
            foundPV = 1;
            pvTable[ply][ply] = moveList.moves[count]; // Store the best move in the principal variation table
            
            for (int nextPly = ply + 1; nextPly < pvLength[ply + 1]; nextPly++) {
                pvTable[ply][nextPly] = pvTable[ply + 1][nextPly]; // Extend the principal variation
            }
            pvLength[ply] = pvLength[ply + 1]; // Update the length of the principal variation
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
    // node fails low
    return alpha; // Return the best score found
}

void searchPosition(Board *board, int depth) {
    /*
    I actually don't know if I need to reset in case of uci as it 
    always loads everything from start at each move but yeah I feel like
    it is better to reset everything here so that I don't have to worry about it later.

    Edit: I was right, it is better to reset everything here, because these tables are 
    not reset when the position is changed, so if I don't reset them here, the search 
    will become more and more slow.
    */

    ply = 0;
    searchedNodes = 0;
    followPV = 0;
    scorePV = 0;
    
    memset(pvLength, 0, sizeof(pvLength)); 
    memset(pvTable, 0, sizeof(pvTable)); 
    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyMoves, 0, sizeof(historyMoves));

    for (int curDepth = 1; curDepth <= depth; curDepth++){
        followPV = 1;
        int score = negamax(board, -50000, 50000, curDepth);
        
        std::cout << "info score cp " << score << " depth " << curDepth
            << " nodes " << searchedNodes << " pv ";

        for (int i = 0; i <= pvLength[0]; i++) {
            if (pvTable[0][i] == 0) break; // Stop at the end of the principal variation
            std::cout << moveToUCI(pvTable[0][i]) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "bestmove " << moveToUCI(pvTable[0][0]) << std::endl;
}

#endif // SEARCH_H;