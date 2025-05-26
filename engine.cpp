#include "constants.h"
#include "board.h"
#include "precalculated_move_tables.h"
#include "moves.h"
#include "perft.h"

using namespace std;

int main(){
    initializeMoveTables();
    Board board;
    parseFEN(board, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ");
    printBoard(board);
    for (int depth = 1; depth <= 6; ++depth) {
        cout << "Perft at depth " << depth << ":" << endl;
        perftTest(board, depth);
        cout << endl;
    }
}