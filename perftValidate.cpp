#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include "constants.h"
#include "board.h"
#include "precalculated_move_tables.h"
#include "moves.h"
#include "perft.h"

using namespace std;

int main() {
    initializeMoveTables();

    ifstream infile("perftSuite.txt");
    if (!infile.is_open()) {
        cerr << "Error: could not open perftSuite.txt\n";
        return 1;
    }

    string line;
    int totalTests = 0, failedTests = 0;

    while (getline(infile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string segment;
        getline(ss, segment, ';'); // FEN is before the first semicolon
        string fen = segment;

        Board board;
        parseFEN(board, fen);
        cout << "\nFEN: " << fen << "\n";

        while (getline(ss, segment, ';')) {
            if (segment.empty()) continue;

            size_t d_pos = segment.find('D');
            if (d_pos == string::npos) continue;

            stringstream pair_ss(segment);
            string depth_token, val_token;
            pair_ss >> depth_token >> val_token;

            if (depth_token.size() < 2 || val_token.empty()) continue;

            int depth = stoi(depth_token.substr(1));
            U64 expected = stoull(val_token);
            totalTests++;

            U64 result = perftTest(board, depth);
            if (result != expected) {
                cout << "  X Depth " << depth << ": expected " << expected << ", got " << result << "\n";
                failedTests++;
            } else {
                cout << " >> Depth " << depth << ": " << result << "\n";
            }
        }
    }

    cout << "\nSummary: " << (totalTests - failedTests) << "/" << totalTests << " tests passed.\n";
    return 0;
}
