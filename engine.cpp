#include "constants.h"
#include "board.h"
#include "precalculated_move_tables.h"
#include "moves.h"

using namespace std;

int main(){
    initializeMoveTables();
    Board board;
    parseFEN(board, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBqPPP/R3K2R w KQkq - 0 1");
    printBoard(board);

    // copyBoard(board);
    // parseFEN(board, start_position);
    // printBoard(board);
    // takeBack(board, backup)
    // printBoard(board);

    MoveList moves[1];
    generateMoves(&board, moves);
    copyBoard(board);
    for (int i = 0; i < moves->count; ++i) {
        printMove(moves->moves[i]);
        cout << endl;
        if (!makeMove(board, moves->moves[i])) continue;;
        printBoard(board);
        takeBack(board, backup);
        cout << endl;
    }
}