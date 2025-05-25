#ifndef RANDOM_H
#define RANDOM_H

#define U64 unsigned long long
#define U32 unsigned int

U32 randomState = 1804289383;

U32 generateRandomU32() {
    randomState ^= randomState << 13;
    randomState ^= randomState >> 17;
    randomState ^= randomState << 5;
    return randomState;
}

U64 generateRandomU64() {
    U64 int n1, n2, n3, n4;
    n1 = (U64) (generateRandomU32()) & 0xFFFF;
    n2 = (U64) (generateRandomU32()) & 0xFFFF;
    n3 = (U64) (generateRandomU32()) & 0xFFFF;
    n4 = (U64) (generateRandomU32()) & 0xFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 generateMagicNumber(){
    return generateRandomU64() & generateRandomU64() & generateRandomU64();
}

#endif // RANDOM_H;