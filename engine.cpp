// #include "constants.h"
// #include "board.h"
// #include "precalculated_move_tables.h"
// #include "moves.h"
#include "perft.h"
// #include "evaluate.h"
#include "search.h"

//std::ofstream logFile("search_log.txt");

using namespace std;

int parseMove(Board *board, const string &moveStr) {
    // Convert UCI move string like "e2e4", "e7e8q" into move integer
    MoveList moveList[1];
    generateMoves(board, moveList);

    if (moveStr.length() < 4) return 0;

    int source = (moveStr[0] - 'a') + (moveStr[1] - '1') * 8;
    int target = (moveStr[2] - 'a') + (moveStr[3] - '1') * 8;
    char promoChar = moveStr.length() == 5 ? moveStr[4] : ' ';

    for (int i = 0; i < moveList->count; ++i) {
        int move = moveList->moves[i];
        if (decodeSource(move) == source && decodeTarget(move) == target) {
            int promo = decodePromoted(move);
            if (promo) {
                if ((promoChar == 'q' && (promo == Q || promo == q)) ||
                    (promoChar == 'r' && (promo == R || promo == r)) ||
                    (promoChar == 'b' && (promo == B || promo == b)) ||
                    (promoChar == 'n' && (promo == N || promo == n))) {
                    return move;
                }
                continue; // promotion mismatch
            }
            return move;
        }
    }
    return 0; // invalid move
}


void parsePosition(Board *board, const string &input) {
    // Supports "position startpos moves ..." and "position fen ..."

    if (input.find("startpos") != string::npos) {
        parseFEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else if (input.find("fen ") != string::npos) {
        size_t fenStart = input.find("fen ") + 4;
        size_t movesStart = input.find(" moves");
        string fen = (movesStart == string::npos)
                     ? input.substr(fenStart)
                     : input.substr(fenStart, movesStart - fenStart);
        parseFEN(board, fen);
    }

    size_t movesPos = input.find("moves ");
    if (movesPos != string::npos) {
        stringstream ss(input.substr(movesPos + 6));
        string moveStr;
        while (ss >> moveStr) {
            int move = parseMove(board, moveStr);
            if (move) {
                repetitionTable[repetitionIndex++] = board->zobristHash; // Store the Zobrist hash for repetition detection
                makeMove(board, move);
            } else {
                cerr << "Invalid move: " << moveStr << endl;
            }
        }
    }
}

// do gepth 6 wtime 180000 ms btime 180000 ms binc 1000 winc 1000 movetime 1000 movestogo 40

void parseGo(Board *board, const string &input, SearchUCI *searchParams) {
    // Handles "go depth 10"
    int depth = -1, movestogo = 30, movetime = -1;
    int time = 1800000, inc = 0; // default time per move set to 60 seconds
    size_t pos = input.find("depth ");
    if (pos != string::npos) {
        depth = stoi(input.substr(pos + 6));
    }

    size_t wtimePos = input.find("wtime ");
    size_t btimePos = input.find("btime ");
    size_t bincPos = input.find("binc ");
    size_t wincPos = input.find("winc ");

    if (board->sideToMove == white) {
        if (wtimePos != string::npos) {
            time = stoi(input.substr(wtimePos + 6));
        }
        if (wincPos != string::npos) {
            inc = stoi(input.substr(bincPos + 5));
        }
    } 
    else {
        if (btimePos != string::npos) {
            time = stoi(input.substr(btimePos + 6));
        }
        if (bincPos != string::npos) {
            inc = stoi(input.substr(bincPos + 5));
        }
    }

    if (input.find("movetime ") != string::npos) {
        movetime = stoi(input.substr(input.find("movetime ") + 9));
    }

    if (input.find("movestogo ") != string::npos) {
        movestogo = stoi(input.substr(input.find("movestogo ") + 10));
    }

    if (movetime != -1) {
        time = movetime; // Use movetime if specified
        movestogo = 1; // Set movestogo to 1 for single move
    }

    if (time != -1){
        searchParams->timedGame = 1;
        time /= movestogo; // Calculate time per move
        time -= 10;
    }

    if (depth == -1){
        depth = 40;
    }

    searchParams->depth = depth;
    searchParams->startTime = TIME_IN_MILLISECONDS;
    searchParams->stopTime = searchParams->startTime + time;
    searchParams->increment = inc;
    searchParams->quit = 0;
    searchParams->stop = 0;

    searchPosition(board, searchParams);
}

void uci(Board *board, SearchUCI *searchParams) {
    cout << "id name Polarity" << endl;
    cout << "id author Magnet" << endl;
    cout << "uciok" << endl;
    string input;
    while (getline(cin, input)) {
        if (input.empty()) continue; // Skip empty lines
        if (input == "uci") {
            cout << "id name Polarity" << endl;
            cout << "id author Magnet" << endl;
            cout << "uciok" << endl;
        } else if (input == "isready") {
            cout << "readyok" << endl;
        } else if (input == "quit") {
            searchParams->quit = 1;
            break;
        } else if (input == "stop") {
            searchParams->stop = 1;
        }else if (input.rfind("go", 0) == 0) {
            parseGo(board, input, searchParams);
        } else if (input.rfind("position", 0) == 0) {
            clearTranspositionTable();
            parsePosition(board, input);
        } else if (input == "d") {
            printBoard(board);
        } else if (input.rfind("perft", 0) == 0) {
            int depth = 6;
            size_t pos = input.find("depth");
            if (pos != string::npos) {
                depth = stoi(input.substr(pos + 6));
            }
            perftTest(board, depth, 1);
        } else if (input == "ucinewgame") {
            parsePosition(board, "position startpos");
            clearTranspositionTable();
        } else {
            cout << "Unknown command: " << input << endl;
        }
    }
}

void initializeAll() {
    initializeMoveTables();
    initializeRandomKeys();
    clearTranspositionTable();
    initializeEvaluationMasks();
}

int main(){
    //cout << "Welcome to Polarity Chess Engine!" << endl;
    initializeAll();
    Board board;
    SearchUCI searchParams;
    searchParams.depth = 10; // Default search depth
    int uciMode = 1;
    MoveList list;
    if (uciMode) {
        uci(&board, &searchParams);
        return 0; // Exit after UCI initialization
    }

    parseFEN(&board, "8/8/1R6/3k4/6P1/8/1p5K/8 b - - 0 48");
    generateMoves(&board, &list);
    printMoveList(&list);
    printBoard(&board);
    cout << evaluate(&board); // Initial evaluation
    // printBoard(&board);
    // searchPosition(&board, &searchParams); // Search with the initial position

    // makeMove(&board, PrincipalVariationTable[0][0]); // Make the first move from the principal variation
    // searchPosition(&board, &searchParams); // Search again after making the move
    //logFile.close();

}