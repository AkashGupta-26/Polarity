# Polarity
A fully functional chess engine built in C/C++

The move generator is based heavily on the [Youtube Series](https://www.youtube.com/playlist?list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs) by Chess Programming. Excellent series, highly recommended. 

I have decided to use a more modular approach to the code rather to the didactic approach in the series, so the engine is split into several files. The main files are:
- `engine.cpp` - The main engine file.
- `moves.h` - Contains the move generation logic.
- `precalculated_move_tables.h` - Contains the precalculated move tables.
- `board.h` - Contains the board representation and logic.
- `constants.h` - Contains the constants used in the engine.
