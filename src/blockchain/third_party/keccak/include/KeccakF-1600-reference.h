#ifndef KECCAKF_1600_REFERENCE_H
#define KECCAKF_1600_REFERENCE_H

#include <stdint.h>

#define KECCAK_F_1600_WIDTH 1600
#define KECCAK_F_1600_LANES 25
#define KECCAK_F_1600_ROUNDS 24

typedef struct {
    uint64_t A[KECCAK_F_1600_LANES];
} KeccakF_1600_State;

#ifdef __cplusplus
extern "C" {
#endif

void KeccakF_1600_Initialize(KeccakF_1600_State *state);
void KeccakF_1600_StateXORBytes(KeccakF_1600_State *state, const unsigned char *data, unsigned int offset, unsigned int length);
void KeccakF_1600_StateExtractBytes(const KeccakF_1600_State *state, unsigned char *data, unsigned int offset, unsigned int length);
void KeccakF_1600_StatePermute(KeccakF_1600_State *state);

#ifdef __cplusplus
}
#endif

#endif // KECCAKF_1600_REFERENCE_H 