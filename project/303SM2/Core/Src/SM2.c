/*******************************************************************************
 ******     Copyright (c) 2014--2020 OSR.Co.Ltd. All rights reserved.     ****** 
 *******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "SM2.h"
#include "RNG.h"
#include "SM3.h"

#include "utility.h"


extern U32 ECCP_BITLEN;
extern U32 ECCP_BYTELEN;
extern U32 ECCP_WORDLEN;
extern U32 ECCP_N_BITLEN;
extern U32 ECCP_N_BYTELEN;
extern U32 ECCP_N_WORDLEN;
extern U32 *ECCP_p;
extern U32 *ECCP_a;
extern U32 *ECCP_b;
extern U32 *ECCP_Gx;
extern U32 *ECCP_Gy;
extern U32 *ECCP_n;
extern U32 ECCP_a_FLAG;        // 0: ECCP_a = -3; 1: else


#define SM2_P_BIT_LEN 256      //bit length of prime p
#define SM2_N_BIT_LEN 256      //bit length of order of point G
#define SM_MAX_ID_LEN 8191     // max ID length = 2^13 - 1

U32 * SM2P_n_1 = NULL;         // order of GF(p) minus 1; signature requires the private key to be in [1,n-2], not [1,n-1]


/* Function: C = A XOR B
 * Parameters:
 * Return: 
 * Caution:
 */
void SM2_XOR(U8 *A, U8 *B, U8 *C, U32 byteLen) 
{
	U32 i;

	for(i=0; i<byteLen; i++)
	{
		C[i] = A[i] ^ B[i];
	}
}

/* Function: SM2 GF(p) Initialize
 * Parameters:
 * Return: 
 *     0(all OK); else(error)
 * Caution:
 *     1. must be called first
 */
U8 SM2_Init()
{
	U32 b=1;

	//ECCP_Init(SM2P_p, SM2P_a, SM2P_b, SM2P_Gx, SM2P_Gy, SM2P_n, SM2_P_BIT_LEN, SM2_N_BIT_LEN, 5, SM2_COMB_iGR, SM2_COMB_iG);
	ECCP_Init_1(sm2p256v1_);

    // on the previous use OSR_SM2_Close() was not called to free memory; recompute n-1 here to prevent insufficient space if the ECC bit length changed
	if(NULL != SM2P_n_1)         
	{
		free(SM2P_n_1);
	}

	SM2P_n_1 = (U32 *)malloc(( ECCP_N_WORDLEN)*4);
	U32_Copy(SM2P_n_1, ECCP_n, ECCP_N_WORDLEN);
	Big_Sub(SM2P_n_1, ECCP_N_WORDLEN, &b, 1);

	return SM2Success;
}

/* Function: close SM2 GF(p) module
 * Parameters:
 * Return: 
 *     0(all OK); else(error)
 * Caution:
 *     1. must be called after SM2 operation
 */
U8 SM2_Close()
{
	ECCP_Close();

    // free the cached (n-1) buffer if it was allocated
	if(NULL != SM2P_n_1)    
	{
		free(SM2P_n_1);
		SM2P_n_1 = NULL;
	}

	return SM2Success;
}

/* Function: Generate Z value = SM3(bitLenofID||ID||a||b||Gx||Gy||Px||Py)
 * Parameters:
 *     ID     -------- User ID
 *     byteLenofID --- byte length of ID, less than 2^13
 *     pubKey -------- public key, 65 bytes
 *     Z      -------- Z value, 32 bytes
 * Return: 
 *     0(all OK); else(error)
 * Caution:
 *     1. bit length of ID must less than 2^16, thus byte lenth must less than 2^13
 */
U8 SM2_GetZ(U8 *ID, U16 byteLenofID, U8 pubKey[65], U8 Z[32])
{
	U8 tmp32[32];
	SM3_Ctx md[1];

	if(NULL == ID || NULL == pubKey || NULL == Z)
	{
		return SM2BufferNull;
	}

	if(byteLenofID > SM_MAX_ID_LEN)
	{
		return SM2InputLenInvalid;
	}

	if(pubKey[0]!=0x04)
	{
		return SM2PointHeadNot04;
	}

	byteLenofID <<= 3;
	tmp32[1] = byteLenofID & 0xFF;
	tmp32[0] = (byteLenofID >> 8) & 0xFF;
	byteLenofID >>= 3;

	SM3_Init(md);	
	SM3_Process(md, tmp32, 2);
	SM3_Process(md, ID, byteLenofID);

	U8_BigInt_reverse((U8 *)ECCP_a, tmp32, ECCP_BYTELEN);        //a
	SM3_Process(md, tmp32, ECCP_BYTELEN);

	U8_BigInt_reverse((U8 *)ECCP_b, tmp32, ECCP_BYTELEN);        //b
	SM3_Process(md, tmp32, ECCP_BYTELEN);

	U8_BigInt_reverse((U8 *)ECCP_Gx, tmp32, ECCP_BYTELEN);       //Gx
	SM3_Process(md, tmp32, ECCP_BYTELEN);

	U8_BigInt_reverse((U8 *)ECCP_Gy, tmp32, ECCP_BYTELEN);       //Gy
	SM3_Process(md, tmp32, ECCP_BYTELEN);

	SM3_Process(md, pubKey + 1, ECCP_BYTELEN<<1);            //Px & Py
	SM3_Done(md, Z);	

	return SM2Success;
}

/* Function: Generate E value = SM3(Z||M)
 * Parameters:
 *     M      -------- Message
 *     byteLen ------- byte length of M
 *     Z      -------- Z value, 32 bytes
 *     E      -------- E value, 32 bytes
 * Return: 
 *     0(all OK); else(error)
 * Caution:
 */
U8 SM2_GetE(U8 *M, U32 byteLen, U8 Z[32], U8 E[32])
{
	SM3_Ctx md[1];

	if(NULL == M || NULL == Z || NULL == E)
	{
		return SM2BufferNull;
	}

	SM3_Init(md);
	SM3_Process(md, Z, 32);
	SM3_Process(md, M, byteLen);
	SM3_Done(md, E);

	return SM2Success;
}

/* Function: SM2 KDF(Key Derivation Function)
 * Parameters:
 *     Z      -------- input key
 *     zByteLen --- -- byte length of Z
 *     K      -------- output key
 *     kByteLen --- -- byte length of K
 * Return: 
 * Caution:
 */
void SM2_KDF(U8 *Z , U32 zByteLen, U8 *K, U32 kByteLen)
{
	SM3_Ctx md[1];
	U8 Hash[32];
	U32 i, t, count=1;                               // count = 1;
	U8 *p = (U8 *)(&count);

	t = kByteLen>>5;                                 // t = kByteLen/32;
	for(i=0; i<t; i++)
	{
		SM3_Init(md);
		SM3_Process(md, Z, zByteLen);
		SM3_Process(md, p+3, 1); 
		SM3_Process(md, p+2, 1);
		SM3_Process(md, p+1, 1);
		SM3_Process(md, p, 1);
		SM3_Done(md, K+(i<<5));
		count++;
	}

	if(kByteLen & 0x1F)
	{
		SM3_Init(md);
		SM3_Process(md, Z, zByteLen);
		SM3_Process(md, p+3, 1); 
		SM3_Process(md, p+2, 1);
		SM3_Process(md, p+1, 1);
		SM3_Process(md, p, 1);
		SM3_Done(md, Hash);
		memcpy(K + (t<<5), Hash, kByteLen & 0x1F);
	}
}

/* Function: Get SM2 Public Key from Private Key
 * Parameters:
 *     priKey ------ private key, 32 bytes, big-endian
 *     pubKey ------ public key(0x04 + x + y), 65 bytes, big-endian
 * Return: 
 *     0(all OK); else(error)
 * Caution:
 *     1. must be called after SM2_Init();
 */
U8 SM2_GetPubKey_from_PriKey(U8 priKey[32], U8 pubKey[65])
{
	U32 k[8], Qx[8], Qy[8];
	ECCP_POINT Q[1];
	U8 ret;

	if(NULL == priKey || NULL == pubKey)
	{
		return SM2BufferNull;
	}

	Q->x = Qx;
	Q->y = Qy;

	//little-endian to big-endian
	U8_BigInt_reverse(priKey, (U8 *)k, ECCP_BYTELEN);

	// make sure 0 < k < n-1, namely k in [1,n-2]
	ret = ECCP_IntegerCheck(k, ECCP_WORDLEN, SM2P_n_1, ECCP_WORDLEN);
	if(ret)
		return ret;

	//get public key Q = kG
	ECCP_PointMul_G(k, ECCP_WORDLEN, Q);
	//printf("%d", ECCP_TestPoint(Q));

	ECCP_Point2Char(Q, pubKey);

	return SM2Success;
}

/* Function: SM2 Encryption
 * Parameters:
 *     M           ------ plaintext, MByteLen bytes, big-endian
 *     MByteLen    ------ byte length of M
 *     k[8]        ------ random number k, 32 bytes, little-endian
 *     pubKey[65]  ------ public key, 65 bytes, big-endian
 *     tag         ------ 0(the output C will be C1||C2||C3); 1(the output C will be C1||C3||C2)
 *     C           ------ ciphertext, CByteLen bytes, big-endian
 *     CByteLen    ------ byte length of C, thus MByteLen+97
 * Return: 
 *     0(all OK); else(error)
 * Caution:
 *     1. must be called after SM2_Init();
 *     2. M and C can not be the same buffer
 */
U8 SM2_encrypt_U32(U8 *M, U32 MByteLen, U32 k[8], U8 pubKey[65], U8 tag, U8 *C, U32 *CByteLen) 
{
	ECCP_POINT Ps[1];
	U32 Psx[8], Psy[8]; 
	U8 *C2, tmp[64];
	U8 flag;
	SM3_Ctx md[1];

	if(NULL == M || NULL == k || NULL == pubKey || NULL == C || NULL == CByteLen)
	{
		return SM2BufferNull;
	}

	if(MByteLen == 0) 
	{
		return SM2InputLenInvalid;
	}
	
	if(pubKey[0]!=0x04)
	{
		return SM2PointHeadNot04;
	}

	if(M == C)
	{
		return SM2InOutSameBuffer;
	}

	// make sure k in [1, n-1]
	flag = ECCP_IntegerCheck(k, ECCP_WORDLEN, ECCP_n, ECCP_WORDLEN);
	if(flag)
	{
		return flag;
	}

	Ps->x = Psx;
	Ps->y = Psy;
                                     
	ECCP_PointMul_G(k, ECCP_WORDLEN, Ps);                  // Ps = kG
	ECCP_Point2Char(Ps, C);                                                 // get C1

	ECCP_Char2Point(pubKey, Ps);
	ECCP_PointMul(k, ECCP_WORDLEN, Ps, Ps);                                              // Ps = k(Pb)
	if(ECCP_TestPoint(Ps) == 0)
	{	
		return SM2NotInCurve;
	}

	C2 = C+2*ECCP_BYTELEN + 1 + (tag==0?0:ECCP_BYTELEN);                   // 65 = 2*ECCP_BYTELEN + 1;
	
	U8_BigInt_reverse((U8 *)Ps->x, tmp, ECCP_BYTELEN);
	U8_BigInt_reverse((U8 *)Ps->y, tmp + ECCP_BYTELEN, ECCP_BYTELEN);
	
	SM2_KDF(tmp, 2*ECCP_BYTELEN, C2, MByteLen);
	if(BigintU8_Check_NULL(C2, MByteLen) == 1)
	{
		return SM2ZeroALL;
	}

	SM2_XOR(M, C2, C2, MByteLen);                                          // get C2

	SM3_Init(md);
	SM3_Process(md, tmp, ECCP_BYTELEN);
	SM3_Process(md, M, MByteLen);
	SM3_Process(md, tmp + ECCP_BYTELEN, ECCP_BYTELEN);
	SM3_Done(md, C+2*ECCP_BYTELEN + 1 +(tag==0?MByteLen:0));           // get C3

	*CByteLen = MByteLen + 3*ECCP_BYTELEN + 1; //65 + MByteLen + 32;

	return SM2Success;
}

/* Function: SM2 Encryption
 * Parameters:
 *     M           ------ plaintext, MByteLen bytes, big-endian
 *     MByteLen    ------ byte length of M
 *     pubKey[65]  ------ public key, 65 bytes, big-endian
 *     tag         ------ 0(the output C will be C1||C2||C3); 1(the output C will be C1||C3||C2)
 *     C           ------ ciphertext, CByteLen bytes, big-endian
 *     CByteLen    ------ byte length of C, thus MByteLen+97
 * Return: 
 *     0(all OK); other(error)
 * Caution:
 *     1. must be called after SM2_Init();
 */
U8 SM2_Encrypt(U8 *M, U32 MByteLen, U8 pubKey[65], U8 tag, U8 *C, U32 *CByteLen) 
{
	U32 k[8];
	U8 flag;

LOOP:
	GetRandU32(k, ECCP_WORDLEN);
//	U32 k[] = {0x614e4fe4, 0x7907ac4d, 0x2f520302, 0x78621413,
//			   0x7d928650, 0x4e2fc049, 0x18614d4e, 0x1f4fde6f};

	flag = SM2_encrypt_U32(M, MByteLen, k, pubKey, tag, C, CByteLen);
	if((flag == SM2ZeroALL)||(flag == SM2IntegerTooBig))
	{
		goto LOOP;
	}
	else
	{
		return flag;
	}
}


/* Function: SM2 Decryption
 * Parameters:
 *     C           ------ ciphertext, CByteLen bytes, big-endian
 *     CByteLen    ------ byte length of C, make sure MByteLen>97
 *     priKey[32]  ------ private key, 32 bytes, big-endian
 *     M           ------ plaintext, MByteLen bytes, big-endian
 *     MByteLen    ------ byte length of M, thus CByteLen-97
 * Return:
 *     0(all OK); else(error)
 * Caution:
 *     1. must be called after SM2_Init();
 *     2. M and C can not be the same buffer
 */
U8 SM2_Decrypt(U8 *C, U32 CByteLen, U8 priKey[32], U8 tag, U8 *M, U32 *MByteLen)
{
	SM3_Ctx md[1];
	ECCP_POINT Ps[1];
	U32 Psx[8], Psy[8], dB[8];
	U8 Hash[32], tmp[64];
	U8 flag;
	U32 byteLen;

	if(NULL == C || NULL == priKey || NULL == M || NULL == MByteLen)
	{
		return SM2BufferNull;
	}

	if(C[0]!=0x04)
	{
		return SM2PointHeadNot04;
	}

	if(CByteLen <= 3*ECCP_BYTELEN + 1)                                     //97 = 3*ECCP_BYTELEN + 1
	{
		return SM2InputLenInvalid;
	}

	if(M == C)
	{
		return SM2InOutSameBuffer;
	}

	byteLen = CByteLen-(1+2*ECCP_BYTELEN+ECCP_BYTELEN);                    //CByteLen - 97;

	Ps->x = Psx;
	Ps->y = Psy;
	ECCP_Char2Point(C, Ps);
	if(ECCP_TestPoint(Ps) != 1)                                            // make sure C1 in Curve
	{
		return SM2NotInCurve;
	}

	U8_BigInt_reverse(priKey, (U8 *)dB, ECCP_BYTELEN);
	flag = ECCP_IntegerCheck(dB, ECCP_WORDLEN, SM2P_n_1, ECCP_WORDLEN);
	if(flag)
	{
		return flag;
	}

	ECCP_PointMul(dB, ECCP_WORDLEN, Ps, Ps);                                             // Ps = dB(C1)=dB(kG)=k(dB(G))=k(PB)
	if(ECCP_TestPoint(Ps) != 1)
	{
		return SM2NotInCurve;
	}

    U8_BigInt_reverse((U8 *)Ps->x, tmp, ECCP_BYTELEN);
    U8_BigInt_reverse((U8 *)Ps->y, tmp + ECCP_BYTELEN, ECCP_BYTELEN);
	SM2_KDF(tmp, 2*ECCP_BYTELEN, M, byteLen);
	if(BigintU8_Check_NULL(M, byteLen))
	{
		return SM2ZeroALL;
	}
	SM2_XOR(C + 2*ECCP_BYTELEN+1 + (tag==0?0:ECCP_BYTELEN), M, M, byteLen);

	SM3_Init(md);
	SM3_Process(md, tmp, ECCP_BYTELEN);
	SM3_Process(md, M, byteLen);
	SM3_Process(md, tmp + ECCP_BYTELEN, ECCP_BYTELEN);
	SM3_Done(md, Hash);

	if(memcmp(Hash, C + (1+2*ECCP_BYTELEN) + (tag==0?byteLen:0), ECCP_BYTELEN)!= 0)
	{
		return SM2DecryVerifyFailed;
	}
	else
	{
		*MByteLen = byteLen;
		return SM2Success;
	}
}
