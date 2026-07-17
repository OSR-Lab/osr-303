#ifndef __RSA_H__
#define __RSA_H__
#include "bignum.h"
#include "bn_mul.h"
#ifdef __cplusplus
extern "C" {
#endif

// out = message^exponent mod modulus
int rsa(char* message,char* exponent, char* modulus, char* out);


// out = message^exponent mod modulus with CRT
// dp = d mod p
// dq = d mod q
// q_inv = q^(-1) mod p 
int rsa_crt(char* message, char* p,char* q, char* dp, char* dq, char* q_inv, char* out);



#ifdef __cplusplus
} // extern "C"
#endif

#endif
