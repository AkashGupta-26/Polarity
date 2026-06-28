#ifndef SEARCH_H
#define SEARCH_H

#include "evaluate.h"
#include <algorithm>

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

static inline void read_input(SearchUCI *searchParams) 
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

static inline void communicate(SearchUCI *searchParams) {
    if (searchParams->timedGame && TIME_IN_MILLISECONDS >= searchParams->stopTime) {
        searchParams->stop = 1; // Stop the search if time is up
    }
    read_input(searchParams);
}

// global initialization
static SearchUCI searchParams[1];

// Maximum ply depth for search
const int maxPly = 64;
const int HISTORY_MAX = 8192;

// Search parameters
static int ply; 
static U64 searchedNodes;

// Late Move Reduction (LMR) parameters
const int FullDepthMoves = 4;
const int ReductionLimit = 3;

// Null Move pruning parameters
static inline int nullMoveReduction(int depth) {
    return 3 + depth / 6;
}

// Futility / late-move pruning margins (indexed by remaining depth)
static const int futilityMargins[5] = {0, 100, 160, 250, 350};
static const int lmpThreshold[6] = {0, 4, 7, 12, 20, 30};

// Middlegame piece values for capture/delta pruning
static const int captureValue[12] = {
    82, 337, 365, 477, 1025, 0,
    82, 337, 365, 477, 1025, 0
};

// SEE piece values for static exchange evaluation
static const int seeValues[12] = {
    100, 300, 300, 500, 900, 20000,
    100, 300, 300, 500, 900, 20000
};

static inline U64 getAttackersToSquare(const Board *board, int square, U64 occ) {
    U64 attackers = 0ULL;
    attackers |= pawnAttacks[black][square] & board->bitboards[P];
    attackers |= pawnAttacks[white][square] & board->bitboards[p];
    attackers |= knightAttacks[square] & (board->bitboards[N] | board->bitboards[n]);
    attackers |= getBishopAttacks(square, occ) & (board->bitboards[B] | board->bitboards[b] | board->bitboards[Q] | board->bitboards[q]);
    attackers |= getRookAttacks(square, occ) & (board->bitboards[R] | board->bitboards[r] | board->bitboards[Q] | board->bitboards[q]);
    attackers |= kingAttacks[square] & (board->bitboards[K] | board->bitboards[k]);
    return attackers & occ;
}

static inline int getLeastValuableAttacker(const Board *board, U64 attackers, int side, int &piece) {
    int start = (side == white) ? P : p;
    int end = (side == white) ? K : k;
    for (int p = start; p <= end; ++p) {
        U64 bb = attackers & board->bitboards[p];
        if (bb) {
            piece = p;
            return __builtin_ctzll(bb);
        }
    }
    return -1;
}

static inline int see(const Board *board, int move) {
    int target = decodeTarget(move);
    int from = decodeSource(move);
    int movePiece = decodePiece(move);

    int gain[32];
    int depth_see = 0;

    int capturedPiece = none;
    if (decodeCapture(move)) {
        if (decodeEnPassant(move)) {
            capturedPiece = (board->sideToMove == white) ? p : P;
        } else {
            int startPiece = (board->sideToMove == white) ? p : P;
            int endPiece = (board->sideToMove == white) ? k : K;
            for (int bbPiece = startPiece; bbPiece <= endPiece; ++bbPiece) {
                if (getBit(board->bitboards[bbPiece], target)) {
                    capturedPiece = bbPiece;
                    break;
                }
            }
        }
    }

    gain[0] = (capturedPiece != none) ? seeValues[capturedPiece] : 0;
    if (decodePromoted(move))
        gain[0] += seeValues[(board->sideToMove == white) ? Q : q] - seeValues[P];

    U64 occ = board->occupancies[both];
    occ ^= (1ULL << from);
    occ |= (1ULL << target);

    U64 attackers = getAttackersToSquare(board, target, occ);
    int side = board->sideToMove ^ 1;
    int lastPieceValue = seeValues[movePiece];

    while (1) {
        depth_see++;
        int piece;
        U64 sideAttackers = attackers & board->occupancies[side];
        if (!sideAttackers) break;

        int sq = getLeastValuableAttacker(board, sideAttackers, side, piece);
        if (sq == -1) break;

        gain[depth_see] = lastPieceValue - gain[depth_see - 1];
        lastPieceValue = seeValues[piece];

        if (std::max(-gain[depth_see - 1], gain[depth_see]) < 0) break;

        occ ^= (1ULL << sq);
        attackers = getAttackersToSquare(board, target, occ);
        side ^= 1;

        if (depth_see >= 31) break;
    }

    while (--depth_see > 0) {
        gain[depth_see - 1] = -std::max(-gain[depth_see - 1], gain[depth_see]);
    }

    return gain[0];
}

static inline int getCapturedPiece(const Board *board, int target) {
    int startPiece = (board->sideToMove == white) ? p : P;
    int endPiece = (board->sideToMove == white) ? k : K;
    for (int bbPiece = startPiece; bbPiece <= endPiece; ++bbPiece) {
        if (getBit(board->bitboards[bbPiece], target))
            return bbPiece;
    }
    return none;
}

static inline int lmrReduction(int depth, int movesSearched, bool improving) {
    int reduction = 1 + movesSearched / 5 + depth / 3;
    if (!improving) reduction++;
    if (reduction > depth - 1) reduction = depth - 1;
    if (reduction < 1) reduction = 1;
    return reduction;
}

static int killerMoves[2][maxPly];
static int historyMoves[12][64];
static int counterMoves[12][64];
static int prevMovePiece[maxPly];
static int prevMoveTarget[maxPly];
static int staticEvalHistory[maxPly];

static inline void updateHistory(int piece, int target, int bonus) {
    int clamped = std::max(-HISTORY_MAX, std::min(HISTORY_MAX, bonus));
    historyMoves[piece][target] += clamped - historyMoves[piece][target] * abs(clamped) / HISTORY_MAX;
}

// Table to store principal variation moves
static int PrincipalVariationLength[maxPly]; 
static int PrincipalVariationTable[maxPly][maxPly];

// follow PV flags
static int followPrincipalVariation, scorePrincipalVariation;

static int gameHistoryPly = 0;

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
        if (decodeEnPassant(move)) return 10105;
        int startPiece = (board->sideToMove == white) ? p : P;
        int endPiece = (board->sideToMove == white) ? k : K;
        int capturedPiece = P;
        for (int bbPiece = startPiece; bbPiece <= endPiece; ++bbPiece) {
            if (getBit(board->bitboards[bbPiece], target)) {
                capturedPiece = bbPiece;
                break;
            }
        }
        int mvvlva = seeValues[capturedPiece] - seeValues[piece] / 100;
        if (seeValues[capturedPiece] >= seeValues[piece])
            return 10000 + mvvlva;
        if (see(board, move) >= 0)
            return 10000 + mvvlva;
        return mvvlva - 1000;
    }
    else{
        if (killerMoves[0][ply] == move) return 9000;
        if (killerMoves[1][ply] == move) return 8000;
        if (ply > 0 && counterMoves[prevMovePiece[ply - 1]][prevMoveTarget[ply - 1]] == move)
            return 7000;
        return historyMoves[piece][target];
    }
    return 0;
}

// Print move scores for debugging
static inline void printMoveScores(Board *board, MoveList *list) {
    std::cout << "Move Scores:" << std::endl;
    for (int i = 0; i < list->count; ++i) {
        int move = list->moves[i];
        int score = scoreMove(board, move);
        std::cout << "Move: " << moveToUCI(move) << " Score: " << score << std::endl;
    }
}

static int moveScores[maxPly + 1][300];

static inline void scoreMoves(Board *board, MoveList *list, int bestMove, int searchPly) {
    for (int i = 0; i < list->count; ++i) {
        if (bestMove != 0 && ttMoveMatch(list->moves[i], (uint16_t)bestMove))
            moveScores[searchPly][i] = 30000;
        else
            moveScores[searchPly][i] = scoreMove(board, list->moves[i]);
    }
}

static inline void pickMove(MoveList *list, int startIdx, int searchPly) {
    int bestIdx = startIdx;
    int bestScore = moveScores[searchPly][startIdx];
    for (int i = startIdx + 1; i < list->count; ++i) {
        if (moveScores[searchPly][i] > bestScore) {
            bestScore = moveScores[searchPly][i];
            bestIdx = i;
        }
    }
    if (bestIdx != startIdx) {
        std::swap(list->moves[startIdx], list->moves[bestIdx]);
        std::swap(moveScores[searchPly][startIdx], moveScores[searchPly][bestIdx]);
    }
}

static inline int detectRepetition(Board *board) {
    int limit = std::max(0, repetitionIndex - board->halfMoveClock);
    if (repetitionIndex - limit < 4)
        return 0;
    for (int i = repetitionIndex - 2; i >= limit; i -= 2) {
        if (repetitionTable[i] == board->zobristHash) {
            return 1;
        }
    }
    return 0;
}

static inline bool isGameHistoryRepetition(Board *board) {
    for (int i = std::max(0, gameHistoryPly - 100); i < gameHistoryPly; ++i) {
        if (repetitionTable[i] == board->zobristHash) {
            return true;
        }
    }
    return false;
}

static inline int numLegalMovesInPosition(Board *board) {
    int legalMoves = 0;
    MoveList moveList;
    generateMoves(board, &moveList);
    copyBoard(board);
    for (int i = 0; i < moveList.count; ++i) {
        if (makeMove(board, moveList.moves[i]) == 0) { takeBack(board, backup); continue; }
        legalMoves++;
        takeBack(board, backup); // Restore the board state
    }
    return legalMoves;
}

// Quiescence search to handle captures and captures that lead to checks
static inline int quiescenceSearch(Board *board, int alpha, int beta, int qDepth = 0) {

    if ((searchedNodes & 1023) == 0) 
        communicate(searchParams);

    searchedNodes++;

    if (ply > maxPly - 1) 
        return evaluate(board);

    TTProbeResult ttProbe = probeHashEntry(board, alpha, beta, 0, ply);
    int ttMove = ttProbe.ttMove;
    if (ttProbe.score != noHashEntry && ply)
        return ttProbe.score;

    int inCheck = isBoardInCheck(board);

    int eval = 0;
    if (!inCheck) {
        eval = evaluate(board);

        if (eval >= beta) 
            return beta;

        if (eval > alpha) 
            alpha = eval;
    } else {
        if (qDepth >= 3)
            return evaluate(board);
        eval = -INFINITY;
    }

    MoveList moveList;
    if (inCheck)
        generateMoves(board, &moveList);
    else
        generateCaptures(board, &moveList);
    int qsPly = ply;
    scoreMoves(board, &moveList, ttMove, qsPly);

    copyBoard(board);

    int legalMoves = 0;

    for (int count = 0; count < moveList.count; ++count) {
        pickMove(&moveList, count, qsPly);
        int move = moveList.moves[count];

        bool isCapture = decodeCapture(move);

        if (!inCheck && !isCapture)
            continue;

        if (!inCheck && isCapture) {
            if (moveScores[qsPly][count] < 0)
                continue;
            int victim = getCapturedPiece(board, decodeTarget(move));
            int gain = (victim != none) ? captureValue[victim] : 82;
            if (decodePromoted(move))
                gain = captureValue[(board->sideToMove == white) ? Q : q];
            if (eval + gain + 200 < alpha)
                continue;
        }

        if (makeMove(board, move) == 0) {
            takeBack(board, backup);
            continue;
        }

        ttPrefetch(board->zobristHash);
        legalMoves++;
        ply++;
        repetitionTable[repetitionIndex++] = board->zobristHash;
        int score = -quiescenceSearch(board, -beta, -alpha, qDepth + 1);

        ply--;
        repetitionIndex--;
        takeBack(board, backup);
        
        if (searchParams->stop) {
            return alpha;
        }
        
        if (score > alpha){
            alpha = score;
            if (score >= beta) 
                return beta;
        }
    }

    if (inCheck && legalMoves == 0)
        return -MATEVALUE + ply;

    return alpha;
}

// main negamax search function
static inline int negamax(Board *board, int alpha, int beta, int depth) {

    if ((searchedNodes & 1023) == 0) 
        communicate(searchParams);

    //static int currentLine[maxPly]; // Track the line of moves searched
    int score;
    int hashFlag = hashAlpha;

    int bestMove = 0;

    PrincipalVariationLength[ply] = ply;

    if (ply && detectRepetition(board)) {
        return 0;
    }

    int inCheck = isBoardInCheck(board);

    if (board->halfMoveClock >= 100) {
        if (inCheck && numLegalMovesInPosition(board) == 0)
            return -MATEVALUE + ply;

        return 0;
    }

    int PVnode = (beta - alpha > 1);

    if (inCheck) depth++;

    TTProbeResult ttProbe = probeHashEntry(board, alpha, beta, depth, ply);
    if (ttProbe.ttMove != 0) bestMove = ttProbe.ttMove;
    if (ply && ttProbe.score != noHashEntry) {
        if (!PVnode)
            return ttProbe.score;
    }

    uint16_t hashMove = (uint16_t)bestMove;

    if (depth == 0)
        return quiescenceSearch(board, alpha, beta);

    if (ply > maxPly - 1)
        return evaluate(board);

    searchedNodes++;

    int legalMoves = 0;
    int movesSearched = 0;

    int staticEval = (ttProbe.ttEval != -32768) ? ttProbe.ttEval : evaluate(board);
    staticEvalHistory[ply] = staticEval;
    bool improving = (ply >= 2 && staticEval > staticEvalHistory[ply - 2]);

    // Internal iterative deepening when no hash move is available
    if (depth >= 5 && bestMove == 0 && !inCheck) {
        negamax(board, alpha, beta, depth - 2);
        TTProbeResult iidProbe = probeHashEntry(board, -INFINITY, INFINITY, 1, ply);
        if (iidProbe.ttMove != 0) bestMove = iidProbe.ttMove;
        hashMove = (uint16_t)bestMove;
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
    if (depth >= 3 && !inCheck && ply && !OnlyPawnsOnBoard && staticEval >= beta) {
        copyBoard(board);
        ply++;
        repetitionTable[repetitionIndex++] = board->zobristHash;

        board->zobristHash ^= enpassantZobristKeys[board->enPassantSquare];
        board->enPassantSquare = noSquare;
        board->zobristHash ^= sideZobristKey;
        board->sideToMove ^= 1;

        int R = nullMoveReduction(depth);
        if (R >= depth) R = depth - 1;
        score = -negamax(board, -beta, -beta + 1, depth - 1 - R);

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

    int nmPly = ply;
    scoreMoves(board, &moveList, bestMove, nmPly);
    copyBoard(board);

    for (int count = 0; count < moveList.count; ++count) {
        pickMove(&moveList, count, nmPly);
        int move = moveList.moves[count];
        bool isCapture = decodeCapture(move);
        bool isPromotion = decodePromoted(move);

        if (!PVnode && !inCheck && movesSearched > 0 && !ttMoveMatch(move, hashMove) &&
            depth <= 2 && isCapture && !isPromotion && moveScores[nmPly][count] < 0) {
            continue;
        }

        repetitionTable[repetitionIndex++] = board->zobristHash;
        if (makeMove(board, move) == 0) {
            repetitionIndex--;
            takeBack(board, backup);
            continue;
        }

        ttPrefetch(board->zobristHash);
        ply++;
        legalMoves++;

        prevMovePiece[ply] = decodePiece(move);
        prevMoveTarget[ply] = decodeTarget(move);

        int givesCheck = isBoardInCheck(board);

        if (!PVnode && !inCheck && movesSearched > 0 && !ttMoveMatch(move, hashMove)) {
            if (depth <= 4 && !isCapture && !isPromotion) {
                if (!givesCheck &&
                    movesSearched >= lmpThreshold[depth] + (improving ? depth : 0)) {
                    ply--;
                    repetitionIndex--;
                    takeBack(board, backup);
                    continue;
                }
            }

            if (depth <= 3 && !isCapture && !isPromotion) {
                if (!givesCheck &&
                    staticEval + futilityMargins[depth] + (improving ? 80 : 0) <= alpha) {
                    ply--;
                    repetitionIndex--;
                    takeBack(board, backup);
                    continue;
                }
            }
        }

        if (movesSearched == 0) {
            score = -negamax(board, -beta, -alpha, depth - 1);
        } else {
            if (movesSearched >= FullDepthMoves && depth >= ReductionLimit &&
                !inCheck && !givesCheck && !isCapture && !isPromotion) {
                int reduction = lmrReduction(depth, movesSearched, improving);
                if (killerMoves[0][ply] == move || killerMoves[1][ply] == move)
                    reduction--;
                if (PVnode)
                    reduction--;
                if (reduction < 1) reduction = 1;
                score = -negamax(board, -alpha - 1, -alpha, depth - 1 - reduction);
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
                int bonus = depth * depth;
                updateHistory(decodePiece(move), decodeTarget(move), bonus);
            }

            alpha = score;

            PrincipalVariationTable[ply][ply] = move;
            for (int nextPly = ply + 1; nextPly < PrincipalVariationLength[ply + 1]; nextPly++) {
                PrincipalVariationTable[ply][nextPly] = PrincipalVariationTable[ply + 1][nextPly];
            }
            PrincipalVariationLength[ply] = PrincipalVariationLength[ply + 1];

            if (score >= beta) {
                writeHashEntry(board, bestMove, beta, depth, hashBeta, ply, staticEval);

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

    writeHashEntry(board, bestMove, alpha, depth, hashFlag, ply, staticEval);
    return alpha;
}


static inline void searchPosition(Board *board, SearchUCI *searchparams) {

    searchParams[0] = *searchparams;
    searchParams->stop = 0; // Reset stop flag
    searchParams->quit = 0; // Reset quit flag

    int depth = searchParams->depth;

    incrementTTGeneration();

    ply = 0;
    searchedNodes = 0;
    followPrincipalVariation = 0;
    scorePrincipalVariation = 0;
    gameHistoryPly = repetitionIndex;

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

        if (searchParams->timedGame && TIME_IN_MILLISECONDS >= searchParams->stopTime) {
            searchParams->stop = 1;
        }
        
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

        U64 elapsedMs = TIME_IN_MILLISECONDS - searchParams->startTime;
        U64 nps = elapsedMs > 0 ? (searchedNodes * 1000) / elapsedMs : 0;

        if (score > 10000 || score < -10000) {
            std::cout << "info score mate " << (score > 0 ? (MATEVALUE - score)/2 + 1 : -(MATEVALUE + score)/2 - 1) 
                << " depth " << curDepth << " nodes " << searchedNodes << " time " << elapsedMs << " nps " << nps << " hashfull " << hashfull() << " pv ";
        }
        else
            std::cout << "info score cp " << score << " depth " << curDepth
                << " nodes " << searchedNodes << " time " << elapsedMs << " nps " << nps << " hashfull " << hashfull() << " pv ";

        for (int i = 0; i < PrincipalVariationLength[0]; i++) {
            if (PrincipalVariationTable[0][i] == 0) break;
            std::cout << moveToUCI(PrincipalVariationTable[0][i]) << " ";
            PrincipalVariationLastIteration[i] = PrincipalVariationTable[0][i];
        }
        PrincipalVariationLastIterationLength = PrincipalVariationLength[0];
        std::cout << std::endl;
    }

    if (PrincipalVariationLastIterationLength > 0 && PrincipalVariationLastIteration[0] != 0) {
        std::cout << "bestmove " << moveToUCI(PrincipalVariationLastIteration[0]) << std::endl;
    } else {
        int fallbackMove = 0;

        TTProbeResult fallbackProbe = probeHashEntry(board, -INFINITY, INFINITY, 0);
        int ttMove = fallbackProbe.ttMove;
        if (ttMove != 0) {
            MoveList ttList;
            generateMoves(board, &ttList);
            for (int i = 0; i < ttList.count; i++) {
                if (ttMoveMatch(ttList.moves[i], (uint16_t)ttMove)) {
                    copyBoard(board);
                    if (makeMove(board, ttList.moves[i]) != 0)
                        fallbackMove = ttList.moves[i];
                    takeBack(board, backup);
                    break;
                }
            }
        }

        if (fallbackMove == 0) {
            MoveList fallbackList;
            generateMoves(board, &fallbackList);
            scoreMoves(board, &fallbackList, 0, 0);
            copyBoard(board);
            for (int i = 0; i < fallbackList.count; i++) {
                pickMove(&fallbackList, i, 0);
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