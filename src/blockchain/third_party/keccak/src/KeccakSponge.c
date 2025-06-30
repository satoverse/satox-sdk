#include "KeccakSponge.h"
#include <string.h>

void KeccakSponge_Initialize(KeccakSponge *sponge, unsigned int rate, unsigned int capacity) {
    sponge->rate = rate;
    sponge->capacity = capacity;
    sponge->bitsInQueue = 0;
    sponge->bitsAvailableForSqueezing = 0;
    KeccakF_1600_Initialize(&sponge->state);
}

void KeccakSponge_Absorb(KeccakSponge *sponge, const unsigned char *data, unsigned int dataByteLen) {
    unsigned int i, j, wholeBlocks;
    unsigned int partialBlock;
    unsigned int rateInBytes = sponge->rate / 8;

    // Process whole blocks
    wholeBlocks = dataByteLen / rateInBytes;
    for(i = 0; i < wholeBlocks; i++) {
        KeccakF_1600_StateXORBytes(&sponge->state, data + i * rateInBytes, 0, rateInBytes);
        KeccakF_1600_StatePermute(&sponge->state);
    }

    // Process remaining bytes
    partialBlock = dataByteLen - wholeBlocks * rateInBytes;
    if (partialBlock > 0) {
        KeccakF_1600_StateXORBytes(&sponge->state, data + wholeBlocks * rateInBytes, 0, partialBlock);
    }
}

void KeccakSponge_Squeeze(KeccakSponge *sponge, unsigned char *data, unsigned int dataByteLen) {
    unsigned int i, j, wholeBlocks;
    unsigned int partialBlock;
    unsigned int rateInBytes = sponge->rate / 8;

    // Process whole blocks
    wholeBlocks = dataByteLen / rateInBytes;
    for(i = 0; i < wholeBlocks; i++) {
        KeccakF_1600_StateExtractBytes(&sponge->state, data + i * rateInBytes, 0, rateInBytes);
        KeccakF_1600_StatePermute(&sponge->state);
    }

    // Process remaining bytes
    partialBlock = dataByteLen - wholeBlocks * rateInBytes;
    if (partialBlock > 0) {
        KeccakF_1600_StateExtractBytes(&sponge->state, data + wholeBlocks * rateInBytes, 0, partialBlock);
    }
} 