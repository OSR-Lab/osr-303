#ifndef __DES_H__
#define __DES_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

void des_subkey_gen(uint8_t keyIn[8], uint8_t Subkey[16][48]);
void des_encrypt(uint8_t pt[8], uint8_t subkeys[16][48], uint8_t ct[8]);
void des_decrypt(uint8_t ct[8], uint8_t subkeys[16][48], uint8_t pt[8]);


#ifdef __cplusplus
} // extern "C"
#endif

#endif
