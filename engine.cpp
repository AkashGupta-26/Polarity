#include "constants.h"
#include "board.h"
#include "move.h"

using namespace std;

int main(){
    initializeMoveTables();
    Board board;
    parseFEN(board, tricky_position);
    cout << "Initial Board State:" << endl;
    printBoard(board);

    printBitboard(getBishopAttacks(e2, board.occupancies[both]));
}