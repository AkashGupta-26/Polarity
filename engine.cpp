#include "constants.h"
#include "board.h"
#include "precalculated_move_tables.h"
#include "moves.h"

using namespace std;

int main(){
    initializeMoveTables();
    Board board;
    parseFEN(board, "r3k2r/p2pqpb1/bn2pnp1/2pPN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R b KQkq a3 0 1");
    printBoard(board);

    MoveList moves[1];
    generateMoves(&board, moves);
    printMoveList(moves);
}