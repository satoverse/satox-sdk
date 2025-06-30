#ifndef KECCAKSPONGE_H
#define KECCAKSPONGE_H

#include <stdint.h>
#include "KeccakF-1600-reference.h"

#define KECCAK_SPONGE_BITRATE 1088
#define KECCAK_SPONGE_CAPACITY 512
#define KECCAK_SPONGE_BLOCKSIZE (KECCAK_SPONGE_BITRATE / 8)

typedef struct {
    KeccakF_1600_State state;
    unsigned int rate;
    unsigned int capacity;
    unsigned int bitsInQueue;
    unsigned int bitsAvailableForSqueezing;
    unsigned char queue[KECCAK_SPONGE_BLOCKSIZE];
} KeccakSponge;

#ifdef __cplusplus
extern "C" {
#endif

void KeccakSponge_Initialize(KeccakSponge *sponge, unsigned int rate, unsigned int capacity);
void KeccakSponge_Absorb(KeccakSponge *sponge, const unsigned char *data, unsigned int dataByteLen);
void KeccakSponge_Squeeze(KeccakSponge *sponge, unsigned char *data, unsigned int dataByteLen);

#ifdef __cplusplus
}
#endif

#endif // KECCAKSPONGE_H 