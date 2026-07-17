#ifndef _ECCP_CURVE_
#define _ECCP_CURVE_


#include "Type.h"



typedef struct 
{
    U32 ECCP_BITLEN;             // bit length of the modulus p of the ECC curve over GF(p)
    U32 ECCP_BYTELEN;
    U32 ECCP_WORDLEN;
    U32 ECCP_N_BITLEN;           // bit length of the order n of the ECC curve over GF(p)
    U32 ECCP_N_BYTELEN;
    U32 ECCP_N_WORDLEN;
	U32 ECCP_a_FLAG;             // 0: ECCP_a = -3; 1: else
	U32 ECCP_COMB_n;
	U32 *ECCP_COMB_iGR;
	U32 *ECCP_COMB_iG;
	U32 *ECCP_p;
    U32 *ECCP_a;
    U32 *ECCP_b;
    U32 *ECCP_Gx;
    U32 *ECCP_Gy;
    U32 *ECCP_n;
} ECCP_CURVE;

extern ECCP_CURVE iso_15946_4_p160_1[1];
extern ECCP_CURVE secp192r1[1];
extern ECCP_CURVE brainpoolP224r1[1];
extern ECCP_CURVE eccp302_1[1];
extern ECCP_CURVE sm2p256v1_[1];
extern ECCP_CURVE X9_62_prime256v1[1];
extern ECCP_CURVE eccp256_1[1];
extern ECCP_CURVE brainpoolP320r1[1];
extern ECCP_CURVE secp384r1[1];
extern ECCP_CURVE brainpoolP512r1[1];
extern ECCP_CURVE secp521r1[1];

#endif
