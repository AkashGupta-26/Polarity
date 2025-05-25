#include "constants.h"
#include "board.h"
#include "move.h"

using namespace std;

int main(){
    initializeLeaperAttacks();
    init_magic_numbers();
    init_slider_attacks(bishop);
    init_slider_attacks(rook);
    for (int square = 0; square < 64; square++){
        cout << "0x" 
             << hex << bishop_magic_numbers[square] << "ULL," << endl;
    }
    cout << endl;
    for (int square = 0; square < 64; square++){
        cout << "0x" 
             << hex << rook_magic_numbers[square] << "ULL," << endl;
    }


    // initializeSlidingAttacks();

    U64 board = 0ULL;
    setBit(board, g5);
    setBit(board, d3);
    setBit(board, b4);
    setBit(board, a5);
    setBit(board, d7);

    printBitboard(board);
    printBitboard(get_bishop_attacks(f4, board));
}