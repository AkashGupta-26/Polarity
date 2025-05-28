#ifndef SEARCH_H
#define SEARCH_H

#include "evaluate.h"
#include <algorithm>

struct ScoredMove {
    int move;
    int score;
};

// Least Valuable Aggressor Most Valuable Victim (MVV-LVA) table
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

// Maximum ply depth for search
const int maxPly = 64;

// Search parameters
int ply; 
int searchedNodes; 

// Late Move Reduction (LMR) parameters
const int FullDepthMoves = 4; // Number of moves to search at full depth
const int ReductionLimit = 3; // Maximum reduction depth

// Null Move pruning parameters
const int NullMoveReduction = 2; // Reduction depth for null move pruning

int killerMoves[2][maxPly];
int historyMoves[12][64];

// Table to store principal variation moves
int PrincipalVariationLength[maxPly]; 
int PrincipalVariationTable[maxPly][maxPly];

// follow PV flags
int followPrincipalVariation, scorePrincipalVariation;

// Allows Principal Variation to be evaluated first
static inline void enablePrincipalVariationScoring(Board *board, MoveList *list) {
    followPrincipalVariation = 0;
    for (int i = 0; i < list->count; i++){
        if (PrincipalVariationTable[0][ply] == list->moves[i]) {
            scorePrincipalVariation = 1;
            followPrincipalVariation = 1;
        }
    }
}

// Assign score to moves for sorting
static inline int scoreMove(Board *board, int move) {
    int piece = decodePiece(move); 
    int target = decodeTarget(move);

    if (scorePrincipalVariation){
        if (PrincipalVariationTable[0][ply] == move) {
            scorePrincipalVariation = 0;
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

// Print move scores for debugging
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

// Quiescence search to handle captures
static inline int quiescienceSearch(Board *board, int alpha, int beta) {
    // evaluate position
    searchedNodes++;

    if (ply > maxPly - 1) 
        return evaluate(board); // Return evaluation if maximum ply is reached

    int eval = evaluate(board);

    if (eval >= beta) 
        return beta; // Beta cutoff fails high

    if (eval > alpha) 
        alpha = eval; // PV node

    MoveList moveList;
    generateMoves(board, &moveList);
    sortMoves(board, &moveList); // Sort moves by score

    copyBoard(board); // Backup the current board state

    for (int count = 0; count < moveList.count; ++count) {
        if (makeMove(board, moveList.moves[count], OnlyCaptures) == 0)
            continue; // Skip illegal moves
        
        ply++;
        int score = -quiescienceSearch(board, -beta, -alpha);

        ply--;
        takeBack(board, backup); // Restore the board state
        
        
        if (score > alpha){
            alpha = score; // Update alpha for PV node
            // Fail-hard beta cutoff
            if (score >= beta) 
                return beta; // Beta cutoff fails high
        }
    }

    return alpha; // Return the best score found
}

// main negamax search function
static inline int negamax(Board *board, int alpha, int beta, int depth){

    int score;
    int hashFlag = hashAlpha; // Default hash flag

    PrincipalVariationLength[ply] = ply;

    if(ply && (score = readHashEntry(board, alpha, beta, depth)) != noHashEntry) {
        return score;
    }

    if (depth == 0) 
        return quiescienceSearch(board, alpha, beta); // Quiescence search at leaf nodes
    
    if (ply > maxPly - 1) 
        return evaluate(board); // Return evaluation if maximum ply is reached
    
    searchedNodes++;
    
    // checks if the king of the side to move is in check
    int inCheck = isBoardInCheck(board);
    
    // increase search depth for checks;
    if (inCheck)
        depth++;
    
    int legalMoves = 0;
    int movesSearched = 0;

    int OnlyPawnsOnBoard = 0; // Check if only pawns are on the board
    for (int piece = P; piece <= k; piece++){
        if (piece == p || piece == P || piece == K || piece == k) continue; // Skip pawns and kings
        if (board->bitboards[piece] != 0) {
            OnlyPawnsOnBoard = 0; // Found a non-pawn piece
            break;
        }
        OnlyPawnsOnBoard = 1; // Only pawns are on the board
    }

    // Null Move Pruning
    if (depth >= 3 && !inCheck && ply && !OnlyPawnsOnBoard) {
        copyBoard(board); 
        ply++;
        
        board->zobristHash ^= enpassantZobristKeys[board->enPassantSquare]; // Reset en passant hash
        board->enPassantSquare = noSquare; // Reset en passant square

        board->zobristHash ^= sideZobristKey; // Update Zobrist hash for side change
        board->sideToMove ^= 1; // Switch sides
        
        score = -negamax(board, -beta, -beta + 1, depth - 1 - NullMoveReduction); 
        
        ply--;
        takeBack(board, backup); // Restore the board state

        if (score >= beta) 
            return beta; // Beta cutoff fails high
        
    }

    MoveList moveList;
    generateMoves(board, &moveList);

    if (followPrincipalVariation) 
        enablePrincipalVariationScoring(board, &moveList); // Enable PV scoring if following principal variation

    sortMoves(board, &moveList); // Sort moves by score

    copyBoard(board); // Backup the current board state

    for (int count = 0; count < moveList.count; ++count) {
        if (makeMove(board, moveList.moves[count]) == 0)
            continue; // Skip illegal moves

        ply++;
        legalMoves++;
        
        if (movesSearched == 0) {
            score = -negamax(board, -beta, -alpha, depth - 1);
        }
        else{
            // Late Move Reduction (LMR)
            if(movesSearched >= FullDepthMoves && depth >= ReductionLimit && 
                !inCheck && !decodeCapture(moveList.moves[count]) && !decodePromoted(moveList.moves[count]))
                    score = -negamax(board, -alpha - 1, -alpha, depth - 2);
            
            else score = alpha + 1; // Ensure full-depth search
            
            // if found a better move during LMR
            if(score > alpha)
            {
                // re-search at full depth but with narrowed score bandwith
                score = -negamax(board, -alpha - 1, -alpha, depth-1);
            
                // if LMR fails re-search at full depth and full score bandwith
                if((score > alpha) && (score < beta))
                    score = -negamax(board, -beta, -alpha, depth-1);
            }
        }

        movesSearched++; //comment this line to disable Late Move Reduction (LMR)
        ply--;
        takeBack(board, backup); // Restore the board state
        
        // PV node
        if (score > alpha) {

            hashFlag = hashExact; // Exact match

            if (!decodeCapture(moveList.moves[count])){
                historyMoves[decodePiece(moveList.moves[count])][decodeTarget(moveList.moves[count])] += depth; // Update history move
            }
            alpha = score; // Update alpha for PV node
            
            PrincipalVariationTable[ply][ply] = moveList.moves[count]; // Store the best move in the principal variation table
            
            for (int nextPly = ply + 1; nextPly < PrincipalVariationLength[ply + 1]; nextPly++) {
                PrincipalVariationTable[ply][nextPly] = PrincipalVariationTable[ply + 1][nextPly]; // Extend the principal variation
            }
            PrincipalVariationLength[ply] = PrincipalVariationLength[ply + 1]; // Update the length of the principal variation

            // fail-hard beta cutoff
            if (score >= beta) {

                writeHashEntry(board, beta, depth, hashBeta); // Store beta cutoff in transposition table

                if (!decodeCapture(moveList.moves[count])){
                    killerMoves[1][ply] = killerMoves[0][ply]; 
                    killerMoves[0][ply] = moveList.moves[count];
                }
                return beta; // Beta cutoff fails high
            }
        }
    }

    if (legalMoves == 0) {
        // If no legal moves, check for checkmate or stalemate
        if (inCheck)
            return -49000 + ply; // Checkmate
        else
            return 0; // Stalemate
    }

    writeHashEntry(board, alpha, depth, hashFlag); // Store the best score in the transposition table

    // node fails low
    return alpha; // Return the best score found
}

void searchPosition(Board *board, int depth) {
    ply = 0;
    searchedNodes = 0;
    followPrincipalVariation = 0;
    scorePrincipalVariation = 0;

    int alpha = -50000;
    int beta = 50000;
    
    memset(PrincipalVariationLength, 0, sizeof(PrincipalVariationLength)); 
    memset(PrincipalVariationTable, 0, sizeof(PrincipalVariationTable)); 
    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyMoves, 0, sizeof(historyMoves));
    //clearTranspositionTable(); // Clear the transposition table before starting the search

    for (int curDepth = 1; curDepth <= depth; curDepth++){
        followPrincipalVariation = 1;

        int score = negamax(board, alpha, beta, curDepth);

        if ((score <= alpha) || (score >= beta)) {
            alpha = -50000; // Reset alpha if score is out of bounds
            beta = 50000; // Reset beta if score is out of bounds
            curDepth--; // reset depth to re-search with a wider score bandwith
            continue;
        }
        // aspiration window
        alpha = score - 50;
        beta = score + 50; // Narrow the search window for the next iteration

        std::cout << "info score cp " << score * ((board->sideToMove == white)? 1 : 1)  << " depth " << curDepth
            << " nodes " << searchedNodes << " pv ";

        for (int i = 0; i <= PrincipalVariationLength[0]; i++) {
            if (PrincipalVariationTable[0][i] == 0) break; // Stop at the end of the principal variation
            std::cout << moveToUCI(PrincipalVariationTable[0][i]) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "bestmove " << moveToUCI(PrincipalVariationTable[0][0]) << std::endl;
}

#endif // SEARCH_H;