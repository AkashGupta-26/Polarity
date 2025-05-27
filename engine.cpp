// #include "constants.h"
// #include "board.h"
// #include "precalculated_move_tables.h"
// #include "moves.h"
// #include "perft.h"
// #include "evaluate.h"
#include "search.h"

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
                makeMove(board, move);
            } else {
                cerr << "Invalid move: " << moveStr << endl;
            }
        }
    }
}


void parseGo(Board *board, const string &input) {
    // Handles "go depth 10"
    int depth = 6; // Default depth
    size_t pos = input.find("depth ");
    if (pos != string::npos) {
        depth = stoi(input.substr(pos + 6));
    }

    //cout << "info depth " << depth << endl;
    
    searchPosition(board, depth);
}

void uci(Board *board) {
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
            break;
        } else if (input.rfind("go", 0) == 0) {
            parseGo(board, input);
        } else if (input.rfind("position", 0) == 0) {
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
        } else {
            cout << "Unknown command: " << input << endl;
        }
    }
}


int main(){
    //cout << "Welcome to Polarity Chess Engine!" << endl;
    initializeMoveTables();
    Board board;
    //MoveList list;
    uci(&board);
    // parseFEN(&board, tricky_position);
    // searchPosition(&board, 6);
}