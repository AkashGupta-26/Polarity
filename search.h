#ifndef SEARCH_H
#define SEARCH_H

#include "evaluate.h"
#include <algorithm>
#include <fstream>

// extern std::ofstream logFile;

struct ScoredMove {
    int move;
    int score;
};

struct SearchUCI {
    int depth;
    int timedGame;
    long long startTime;
    long long stopTime;
    int increment;
    int quit;
    int stop;

    SearchUCI() : depth(10), timedGame(0), startTime(0), stopTime(0), increment(0), quit(0), stop(0) {}
};

void read_input(SearchUCI *searchParams) 
{
    int bytes;
    char input[256] = "";
    char* endc;

    if (input_waiting())
    {
        searchParams->stop = 1;

        do {
            bytes = read(fileno(stdin), input, sizeof(input));
        } while (bytes < 0);

        endc = strchr(input, '\n');
        if (endc) *endc = '\0';

        if (std::strlen(input) > 0)
        {
            if (std::strncmp(input, "quit", 4) == 0)
            {
                searchParams->quit = 1;
            }
            else if (std::strncmp(input, "stop", 4) == 0)
            {
                searchParams->quit = 1;
            }
        }
    }
}

static void communicate(SearchUCI *searchParams) {
    if (searchParams->timedGame && TIME_IN_MILLISECONDS >= searchParams->stopTime) {
        searchParams->stop = 1; // Stop the search if time is up
    }
    read_input(searchParams);
}

// global initialization
SearchUCI searchParams[1];

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

// debug repetition detection
int foundRepetition = 0;

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

static inline int detectRepetition(Board *board) {
    for (int i = std::max(0, repetitionIndex - 100); i < repetitionIndex; ++i) {
        if (repetitionTable[i] == board->zobristHash) {
            return 1; // Repetition detected
        }
    }
    return 0; // No repetition
}

static inline int numLegalMovesInPosition(Board *board) {
    int legalMoves = 0;
    MoveList moveList;
    generateMoves(board, &moveList);
    copyBoard(board); // Backup the current board state
    for (int i = 0; i < moveList.count; ++i) {
        if (makeMove(board, moveList.moves[i]) == 0) continue; // Skip illegal moves
        legalMoves++;
        takeBack(board, backup); // Restore the board state
    }
    return legalMoves;
}

// Quiescence search to handle captures
static inline int quiescienceSearch(Board *board, int alpha, int beta) {

    if ((searchedNodes & 2047) == 0) 
        communicate(searchParams); // Communicate with the engine every 2048 nodes

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
        repetitionTable[repetitionIndex++] = board->zobristHash; // Add current position to repetition table
        int score = -quiescienceSearch(board, -beta, -alpha);

        ply--;
        repetitionIndex--; // Remove the position from the repetition table after searching
        takeBack(board, backup); // Restore the board state
        
        if (searchParams->stop) {
            return alpha;
        }
        
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
static inline int negamax(Board *board, int alpha, int beta, int depth) {

    if ((searchedNodes & 2047) == 0) 
        communicate(searchParams); // Communicate with the engine every 2048 nodes

    //static int currentLine[maxPly]; // Track the line of moves searched
    int score;
    int hashFlag = hashAlpha;

    PrincipalVariationLength[ply] = ply;

    if (ply && detectRepetition(board)) {

        return 0;
    }

    int inCheck = isBoardInCheck(board);

    if (board->halfMoveClock >= 100) {
        if (inCheck && numLegalMovesInPosition(board) == 0) {
            return -MATESCORE + ply; // Checkmate
        }
        return 0; // Draw by fifty-move rule
    }

    int PVnode = (beta - alpha > 1);

    if (!PVnode && ply && (score = readHashEntry(board, alpha, beta, depth, ply)) != noHashEntry) {
        return score;
    }

    if (depth == 0)
        return quiescienceSearch(board, alpha, beta);

    if (ply > maxPly - 1)
        return evaluate(board);

    searchedNodes++;

    if (inCheck) depth++;

    int legalMoves = 0;
    int movesSearched = 0;

    int OnlyPawnsOnBoard = 1;
    for (int piece = P; piece <= k; piece++) {
        if (piece == p || piece == P || piece == K || piece == k) continue;
        if (board->bitboards[piece] != 0) {
            OnlyPawnsOnBoard = 0;
            break;
        }
    }

    if (depth >= 3 && !inCheck && ply && !OnlyPawnsOnBoard) {
        copyBoard(board);
        ply++;
        repetitionTable[repetitionIndex++] = board->zobristHash;

        board->zobristHash ^= enpassantZobristKeys[board->enPassantSquare];
        board->enPassantSquare = noSquare;
        board->zobristHash ^= sideZobristKey;
        board->sideToMove ^= 1;

        score = -negamax(board, -beta, -beta + 1, depth - 1 - NullMoveReduction);

        ply--;
        repetitionIndex--;
        takeBack(board, backup);

        if (searchParams->stop) {
            return alpha;
        }

        if (score >= beta) {
            return beta;
        }
    }

    MoveList moveList;
    generateMoves(board, &moveList);

    if (followPrincipalVariation)
        enablePrincipalVariationScoring(board, &moveList);

    sortMoves(board, &moveList);
    copyBoard(board);

    for (int count = 0; count < moveList.count; ++count) {
        repetitionTable[repetitionIndex++] = board->zobristHash;
        if (makeMove(board, moveList.moves[count]) == 0) {
            repetitionIndex--;
            continue;
        }

        //currentLine[ply] = moveList.moves[count]; // Store move in current line

        ply++;
        legalMoves++;

        if (movesSearched == 0) {
            score = -negamax(board, -beta, -alpha, depth - 1);
        } else {
            if (movesSearched >= FullDepthMoves && depth >= ReductionLimit &&
                !inCheck && !decodeCapture(moveList.moves[count]) && !decodePromoted(moveList.moves[count])) {
                score = -negamax(board, -alpha - 1, -alpha, depth - 2);
            } else {
                score = alpha + 1;
            }

            if (score > alpha) {
                score = -negamax(board, -alpha - 1, -alpha, depth - 1);
                if ((score > alpha) && (score < beta)) {
                    score = -negamax(board, -beta, -alpha, depth - 1);
                }
            }
        }

        movesSearched++;
        ply--;
        repetitionIndex--;
        takeBack(board, backup);

        if (searchParams->stop) {
            return alpha; // Stop search if requested
        }

        // Alpha raise logging
        if (score > alpha) {
            hashFlag = hashExact;

            if (!decodeCapture(moveList.moves[count])) {
                historyMoves[decodePiece(moveList.moves[count])][decodeTarget(moveList.moves[count])] += depth;
            }

            alpha = score;

            PrincipalVariationTable[ply][ply] = moveList.moves[count];
            for (int nextPly = ply + 1; nextPly < PrincipalVariationLength[ply + 1]; nextPly++) {
                PrincipalVariationTable[ply][nextPly] = PrincipalVariationTable[ply + 1][nextPly];
            }
            PrincipalVariationLength[ply] = PrincipalVariationLength[ply + 1];

            // 📝 Log Alpha Raise
            // logFile << "Alpha raised at ply " << ply << " depth " << depth
            //         << " score " << score << " move " << moveToUCI(moveList.moves[count])
            //         << " line: ";
            // for (int i = 0; i <= ply; ++i)
            //     logFile << moveToUCI(currentLine[i]) << " ";
            // logFile << std::endl;

            if (score >= beta) {
                writeHashEntry(board, beta, depth, hashBeta, ply);

                if (!decodeCapture(moveList.moves[count])) {
                    killerMoves[1][ply] = killerMoves[0][ply];
                    killerMoves[0][ply] = moveList.moves[count];
                }

                // 📝 Log Beta Cutoff
                // logFile << "Beta cutoff at ply " << ply << " depth " << depth
                //         << " score " << score << " move " << moveToUCI(moveList.moves[count])
                //         << " line: ";
                // for (int i = 0; i <= ply; ++i)
                //     logFile << moveToUCI(currentLine[i]) << " ";
                // logFile << std::endl;

                return beta;
            }
        }
    }

    if (legalMoves == 0) {
        alpha = inCheck ? -MATEVALUE + ply : 0;
    }

    writeHashEntry(board, alpha, depth, hashFlag, ply);
    return alpha;
}


void searchPosition(Board *board, SearchUCI *searchparams) {

    searchParams[0] = *searchparams;
    searchParams->stop = 0; // Reset stop flag
    searchParams->quit = 0; // Reset quit flag

    int depth = searchParams->depth;

    // logFile << "\n\nStarting search with depth: " << depth << "\n";

    ply = 0;
    searchedNodes = 0;
    followPrincipalVariation = 0;
    scorePrincipalVariation = 0;

    int alpha = -INFINITY;
    int beta = INFINITY;

    int PrincipalVariationLastIteration[maxPly];
    int bestEvaluationPreviousIteration = 0;
    
    memset(PrincipalVariationLength, 0, sizeof(PrincipalVariationLength)); 
    memset(PrincipalVariationTable, 0, sizeof(PrincipalVariationTable)); 
    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyMoves, 0, sizeof(historyMoves));
    //clearTranspositionTable(); // Clear the transposition table before starting the search

    for (int curDepth = 1; curDepth <= depth; curDepth++){

        if (searchParams->stop or searchParams->quit) {
            std::cout << "info Search Time Over" << std::endl;
            break; // Stop search if requested
        }

        followPrincipalVariation = 1;

        int score = negamax(board, alpha, beta, curDepth);

        if ((score <= alpha) || (score >= beta)) {
            alpha = -INFINITY; // Reset alpha if score is out of bounds
            beta = INFINITY; // Reset beta if score is out of bounds
            curDepth--; // reset depth to re-search with a wider score bandwith
            //std:: cout << "info Full Search Re-Start at depth " << curDepth + 1 << std::endl;
            continue;
        }
        // aspiration window
        alpha = score - 50;
        beta = score + 50; // Narrow the search window for the next iteration

        if (searchParams->stop || searchParams->quit) {
            if (abs(bestEvaluationPreviousIteration - score) > 100 || MATEVALUE - abs(score) < 20){
                std::cout << "info unfinished search instability" << std::endl;
                break; // Dont use this search result if the evaluation changed too much or Mate found on board on stopping search forcefully
            }
        }

        bestEvaluationPreviousIteration = score;

        if (score > 10000 || score < -10000) {
            std::cout << "info score mate " << (score > 0 ? (MATEVALUE - score)/2 + 1 : -(MATEVALUE + score)/2 - 1) 
                << " depth " << curDepth << " nodes " << searchedNodes << std::endl;
        }
        else
            std::cout << "info score cp " << score << " depth " << curDepth
                << " nodes " << searchedNodes << " pv ";

        for (int i = 0; i < PrincipalVariationLength[0]; i++) {
            if (PrincipalVariationTable[0][i] == 0) break; // Stop at the end of the principal variation
            std::cout << moveToUCI(PrincipalVariationTable[0][i]) << " ";
            PrincipalVariationLastIteration[i] = PrincipalVariationTable[0][i];
        }
        std::cout << std::endl;
    }
    std::cout << "bestmove " << moveToUCI(PrincipalVariationLastIteration[0]) << std::endl;
}

#endif // SEARCH_H;