#include "constants.h"
#include "board.h"
#include "move.h"

using namespace std;

int main(){
    initializeLeaperAttacks();
    for (int square = 0; square < 64; ++square){
        cout << "Square: " << indexToSquare[square] << endl;
        cout << "Pawn Attacks (White): \n";
        printBitboard(pawnAttacks[white][square]);
        cout << "Pawn Attacks (Black): \n";
        printBitboard(pawnAttacks[black][square]);
        cout << "Knight Attacks: \n";
        printBitboard(knightAttacks[square]);
        cout << "King Attacks: \n";
        printBitboard(kingAttacks[square]);
    }
}