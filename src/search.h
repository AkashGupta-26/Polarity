#ifndef SEARCH_H
#define SEARCH_H

#include "evaluate.h"
#include <algorithm>
#include <fstream>
#include <iomanip>

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
                searchParams->stop = 1;
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
U64 searchedNodes;

U64 hashHits = 0;
U64 hashExactHits = 0;
U64 hashAlphaHits = 0;
U64 hashBetaHits = 0;
U64 hashMoveOrderHits = 0;

// Late Move Reduction (LMR) parameters
const int FullDepthMoves = 4; // Number of moves to search at full depth
const int ReductionLimit = 3; // Maximum reduction depth

// Null Move pruning parameters
const int NullMoveReduction = 2;

// Futility / late-move pruning margins (indexed by remaining depth)
static const int futilityMargins[5] = {0, 100, 160, 250, 350};
static const int lmpThreshold[6] = {0, 4, 7, 12, 20, 30};

// Middlegame piece values for capture/delta pruning
static const int captureValue[12] = {
    82, 337, 365, 477, 1025, 0,
    82, 337, 365, 477, 1025, 0
};

static inline int getCapturedPiece(const Board *board, int target) {
    int startPiece = (board->sideToMove == white) ? p : P;
    int endPiece = (board->sideToMove == white) ? k : K;
    for (int bbPiece = startPiece; bbPiece <= endPiece; ++bbPiece) {
        if (getBit(board->bitboards[bbPiece], target))
            return bbPiece;
    }
    return none;
}

static inline int lmrReduction(int depth, int movesSearched) {
    int reduction = 1 + movesSearched / 6 + depth / 4;
    if (reduction > depth - 1) reduction = depth - 1;
    if (reduction > 3) reduction = 3;
    return reduction;
}

int killerMoves[2][maxPly];
int historyMoves[12][64];
int counterMoves[12][64];
int prevMovePiece[maxPly];
int prevMoveTarget[maxPly];
int staticEvalHistory[maxPly];

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
        int capturedPiece = P;
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
        if (killerMoves[0][ply] == move) return 9000;
        if (killerMoves[1][ply] == move) return 8000;
        if (ply > 0 && counterMoves[prevMovePiece[ply - 1]][prevMoveTarget[ply - 1]] == move)
            return 7000;
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

static inline void sortMoves(Board *board, MoveList *list, int bestMove = 0) {
    int n = list->count;
    if (n <= 1) return;

    ScoredMove scoredMoves[300];
    for (int i = 0; i < n; ++i) {
        if (list->moves[i] == bestMove) {
            scoredMoves[i] = { list->moves[i], 30000 }; // Highest score for principal variation move
            continue;
        }
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

// Quiescence search to handle captures and captures that lead to checks
static inline int quiescenceSearch(Board *board, int alpha, int beta) {

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
        int move = moveList.moves[count];

        if (decodeCapture(move)) {
            int victim = getCapturedPiece(board, decodeTarget(move));
            int gain = (victim != none) ? captureValue[victim] : 82;
            if (decodePromoted(move))
                gain = captureValue[(board->sideToMove == white) ? Q : q];
            if (eval + gain + 200 < alpha)
                continue;
        }

        if (makeMove(board, move, OnlyCaptures) == 0)
            continue;
        
        ply++;
        repetitionTable[repetitionIndex++] = board->zobristHash; // Add current position to repetition table
        int score = -quiescenceSearch(board, -beta, -alpha);

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

    int bestMove = 0;

    PrincipalVariationLength[ply] = ply;

    if (ply && detectRepetition(board)) 
        return 0;

    int inCheck = isBoardInCheck(board);

    if (board->halfMoveClock >= 100) {
        if (inCheck && numLegalMovesInPosition(board) == 0)
            return -MATEVALUE + ply; // Checkmate

        return 0; // Draw by fifty-move rule
    }

    int PVnode = (beta - alpha > 1);

    if (inCheck) depth++;

    if (ply && (score = readHashEntry(board, &bestMove, alpha, beta, depth, ply)) != noHashEntry) {
        if (!PVnode)
            return score;
        if (abs(score) >= MATEVALUE - maxPly)
            return score;
        if (score <= alpha - 200 || score >= beta + 200)
            return score;
    }

    if (depth == 0)
        return quiescenceSearch(board, alpha, beta);

    if (ply > maxPly - 1)
        return evaluate(board);

    searchedNodes++;

    int legalMoves = 0;
    int movesSearched = 0;

    int staticEval = evaluate(board);
    staticEvalHistory[ply] = staticEval;
    bool improving = (ply >= 2 && staticEval > staticEvalHistory[ply - 2]);

    // Internal iterative deepening when no hash move is available
    if (depth >= 5 && bestMove == 0 && !inCheck) {
        negamax(board, alpha, beta, depth - 2);
        readHashEntry(board, &bestMove, -INFINITY, INFINITY, 1, ply);
    }

    // Reverse Futility Pruning / Static Null Move Pruning
    if (depth <= 6 && !PVnode && !inCheck && abs(beta) < MATEVALUE - maxPly){
        int evalMargin = 80 * depth;
        if (staticEval - evalMargin >= beta)
            return staticEval - evalMargin;
    }

    int OnlyPawnsOnBoard = 1;
    for (int piece = P; piece <= k; piece++) {
        if (piece == p || piece == P || piece == K || piece == k) continue;
        if (board->bitboards[piece] != 0) {
            OnlyPawnsOnBoard = 0;
            break;
        }
    }

    // Null move pruning
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

        if (searchParams->stop) 
            return alpha;

        if (score >= beta)
            return beta;
    }

    // Razoring
    if (depth <= 3 && !PVnode && !inCheck){
        score = staticEval + 125;
        int newScore;

        if (score < alpha){
            if (depth == 1){
                newScore = quiescenceSearch(board, alpha, beta);
                return (newScore > score) ? newScore : score;
            }
            score += 175;
            if (score < alpha && depth <= 2) {
                newScore = quiescenceSearch(board, alpha, beta);
                if (newScore < alpha) 
                    return (newScore > score) ? newScore : score;
            }
        }
    }

    MoveList moveList;
    generateMoves(board, &moveList);

    if (followPrincipalVariation)
        enablePrincipalVariationScoring(board, &moveList);

    sortMoves(board, &moveList, bestMove);
    copyBoard(board);

    for (int count = 0; count < moveList.count; ++count) {
        int move = moveList.moves[count];
        bool isCapture = decodeCapture(move);
        bool isPromotion = decodePromoted(move);

        int capturedVictim = none;
        if (isCapture && !decodeEnPassant(move))
            capturedVictim = getCapturedPiece(board, decodeTarget(move));

        repetitionTable[repetitionIndex++] = board->zobristHash;
        if (makeMove(board, move) == 0) {
            repetitionIndex--;
            continue;
        }

        ply++;
        legalMoves++;

        prevMovePiece[ply] = decodePiece(move);
        prevMoveTarget[ply] = decodeTarget(move);

        bool givesCheck = isBoardInCheck(board);

        if (!PVnode && !inCheck && !givesCheck && movesSearched > 0 && move != bestMove) {
            if (depth <= 4 && !isCapture && !isPromotion &&
                movesSearched >= lmpThreshold[depth] + (improving ? depth : 0)) {
                ply--;
                repetitionIndex--;
                takeBack(board, backup);
                continue;
            }

            if (depth <= 3 && !isCapture && !isPromotion &&
                staticEval + futilityMargins[depth] + (improving ? 80 : 0) <= alpha) {
                ply--;
                repetitionIndex--;
                takeBack(board, backup);
                continue;
            }

            if (depth <= 2 && isCapture && capturedVictim != none &&
                captureValue[capturedVictim] + 400 < captureValue[decodePiece(move)]) {
                ply--;
                repetitionIndex--;
                takeBack(board, backup);
                continue;
            }
        }

        if (movesSearched == 0) {
            score = -negamax(board, -beta, -alpha, depth - 1);
        } else {
            if (movesSearched >= FullDepthMoves && depth >= ReductionLimit &&
                !inCheck && !givesCheck && !isCapture && !isPromotion) {
                score = -negamax(board, -alpha - 1, -alpha, depth - 1 - lmrReduction(depth, movesSearched));
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

        if (score > alpha) {
            hashFlag = hashExact;

            bestMove = move;

            if (!isCapture) {
                historyMoves[decodePiece(move)][decodeTarget(move)] += depth;
            }

            alpha = score;

            PrincipalVariationTable[ply][ply] = move;
            for (int nextPly = ply + 1; nextPly < PrincipalVariationLength[ply + 1]; nextPly++) {
                PrincipalVariationTable[ply][nextPly] = PrincipalVariationTable[ply + 1][nextPly];
            }
            PrincipalVariationLength[ply] = PrincipalVariationLength[ply + 1];

            if (score >= beta) {
                writeHashEntry(board, bestMove, beta, depth, hashBeta, ply);

                if (!isCapture) {
                    killerMoves[1][ply] = killerMoves[0][ply];
                    killerMoves[0][ply] = move;

                    if (ply > 0)
                        counterMoves[prevMovePiece[ply - 1]][prevMoveTarget[ply - 1]] = move;
                }

                return beta;
            }
        }
        if (searchParams->stop) {
            return alpha;
        }
    }

    if (legalMoves == 0) {
        alpha = inCheck ? -MATEVALUE + ply : 0;
    }

    writeHashEntry(board, bestMove, alpha, depth, hashFlag, ply);
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

    hashHits = 0;
    hashExactHits = 0;
    hashAlphaHits = 0;
    hashBetaHits = 0;
    hashMoveOrderHits = 0;

    int alpha = -INFINITY;
    int beta = INFINITY;

    int PrincipalVariationLastIteration[maxPly];
    int PrincipalVariationLastIterationLength = 0;
    int bestEvaluationPreviousIteration = 0;
    
    memset(PrincipalVariationLength, 0, sizeof(PrincipalVariationLength)); 
    memset(PrincipalVariationTable, 0, sizeof(PrincipalVariationTable)); 
    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyMoves, 0, sizeof(historyMoves));
    memset(counterMoves, 0, sizeof(counterMoves));
    memset(prevMovePiece, 0, sizeof(prevMovePiece));
    memset(prevMoveTarget, 0, sizeof(prevMoveTarget));
    memset(staticEvalHistory, 0, sizeof(staticEvalHistory));
    //clearTranspositionTable(); // Clear the transposition table before starting the search

    int delta = 25;

    for (int curDepth = 1; curDepth <= depth; curDepth++){

        if (searchParams->stop or searchParams->quit) {
            std::cout << "info Search Time Over" << std::endl;
            break;
        }

        followPrincipalVariation = 1;

        int score = negamax(board, alpha, beta, curDepth);

        if ((searchParams->stop || searchParams->quit) &&
            ((score <= alpha) || (score >= beta))) {
            break;
        }

        if (score <= alpha) {
            alpha = score - delta;
            delta *= 2;
            if (delta > 500) { alpha = -INFINITY; beta = INFINITY; }
            curDepth--;
            continue;
        }

        if (score >= beta) {
            beta = score + delta;
            delta *= 2;
            if (delta > 500) { alpha = -INFINITY; beta = INFINITY; }
            curDepth--;
            continue;
        }

        alpha = score - 25;
        beta = score + 25;
        delta = 25;

        if (searchParams->stop || searchParams->quit) {
            if (PrincipalVariationLastIterationLength > 0 &&
                (abs(bestEvaluationPreviousIteration - score) > 100 || MATEVALUE - abs(score) < 20)){
                std::cout << "info unfinished search instability" << std::endl;
                break;
            }
        }

        bestEvaluationPreviousIteration = score;

        if (score > 10000 || score < -10000) {
            std::cout << "info score mate " << (score > 0 ? (MATEVALUE - score)/2 + 1 : -(MATEVALUE + score)/2 - 1) 
                << " depth " << curDepth << " nodes " << searchedNodes << " time " << TIME_IN_MILLISECONDS - searchParams->startTime << " pv ";
        }
        else
            std::cout << "info score cp " << score << " depth " << curDepth
                << " nodes " << searchedNodes << " time " << TIME_IN_MILLISECONDS - searchParams->startTime << " pv ";

        for (int i = 0; i < PrincipalVariationLength[0]; i++) {
            if (PrincipalVariationTable[0][i] == 0) break;
            std::cout << moveToUCI(PrincipalVariationTable[0][i]) << " ";
            PrincipalVariationLastIteration[i] = PrincipalVariationTable[0][i];
        }
        PrincipalVariationLastIterationLength = PrincipalVariationLength[0];
        std::cout << std::endl;
    }

    U64 totalCutoffs = hashExactHits + hashAlphaHits + hashBetaHits;
    double hashHitRate = searchedNodes > 0 ? (100.0 * hashHits / searchedNodes) : 0.0;
    double cutoffRate = hashHits > 0 ? (100.0 * totalCutoffs / hashHits) : 0.0;
    double moveOrderRate = hashHits > 0 ? (100.0 * hashMoveOrderHits / hashHits) : 0.0;

    std::cout << "info string Hash Stats: "
              << "Hits=" << hashHits
              << " (" << std::fixed << std::setprecision(1) << hashHitRate << "%) "
              << "Exact=" << hashExactHits
              << " Alpha=" << hashAlphaHits
              << " Beta=" << hashBetaHits
              << " Cutoffs=" << totalCutoffs
              << " (" << cutoffRate << "%) "
              << "Moves=" << hashMoveOrderHits
              << " (" << moveOrderRate << "%)" << std::endl;

    if (PrincipalVariationLastIterationLength > 0 && PrincipalVariationLastIteration[0] != 0) {
        std::cout << "bestmove " << moveToUCI(PrincipalVariationLastIteration[0]) << std::endl;
    } else {
        int fallbackMove = 0;

        int ttMove = 0;
        readHashEntry(board, &ttMove, -INFINITY, INFINITY, 0);
        if (ttMove != 0) {
            copyBoard(board);
            if (makeMove(board, ttMove) != 0)
                fallbackMove = ttMove;
            takeBack(board, backup);
        }

        if (fallbackMove == 0) {
            MoveList fallbackList;
            generateMoves(board, &fallbackList);
            sortMoves(board, &fallbackList);
            copyBoard(board);
            for (int i = 0; i < fallbackList.count; i++) {
                if (makeMove(board, fallbackList.moves[i]) == 0) {
                    takeBack(board, backup);
                    continue;
                }
                takeBack(board, backup);
                fallbackMove = fallbackList.moves[i];
                break;
            }
        }

        if (fallbackMove != 0)
            std::cout << "bestmove " << moveToUCI(fallbackMove) << std::endl;
        else
            std::cout << "bestmove 0000" << std::endl;
    }
}

#endif // SEARCH_H;