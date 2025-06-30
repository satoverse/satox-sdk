#include "KeccakF-1600-reference.h"
#include <string.h>

#define ROL64(a, offset) ((((uint64_t)a) << offset) | (((uint64_t)a) >> (64-offset)))

static const uint64_t KeccakF_RoundConstants[KECCAK_F_1600_ROUNDS] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

void KeccakF_1600_Initialize(KeccakF_1600_State *state) {
    memset(state->A, 0, KECCAK_F_1600_LANES * sizeof(uint64_t));
}

void KeccakF_1600_StateXORBytes(KeccakF_1600_State *state, const unsigned char *data, unsigned int offset, unsigned int length) {
    unsigned int i;
    for(i = 0; i < length; i++) {
        ((unsigned char*)state->A)[offset + i] ^= data[i];
    }
}

void KeccakF_1600_StateExtractBytes(const KeccakF_1600_State *state, unsigned char *data, unsigned int offset, unsigned int length) {
    memcpy(data, ((unsigned char*)state->A) + offset, length);
}

void KeccakF_1600_StatePermute(KeccakF_1600_State *state) {
    uint64_t *A = state->A;
    uint64_t B[KECCAK_F_1600_LANES];
    uint64_t C[5];
    uint64_t D[5];
    unsigned int x, y, i;

    for(i = 0; i < KECCAK_F_1600_ROUNDS; i++) {
        // Theta
        for(x = 0; x < 5; x++) {
            C[x] = A[x] ^ A[x+5] ^ A[x+10] ^ A[x+15] ^ A[x+20];
        }
        for(x = 0; x < 5; x++) {
            D[x] = C[(x+4)%5] ^ ROL64(C[(x+1)%5], 1);
            for(y = 0; y < 5; y++) {
                A[x+5*y] ^= D[x];
            }
        }

        // Rho and Pi
        for(x = 0; x < 5; x++) {
            for(y = 0; y < 5; y++) {
                B[y+5*((2*x+3*y)%5)] = ROL64(A[x+5*y], ((x+1)*(x+2)/2)%64);
            }
        }

        // Chi
        for(x = 0; x < 5; x++) {
            for(y = 0; y < 5; y++) {
                A[x+5*y] = B[x+5*y] ^ ((~B[((x+1)%5)+5*y]) & B[((x+2)%5)+5*y]);
            }
        }

        // Iota
        A[0] ^= KeccakF_RoundConstants[i];
    }
} 