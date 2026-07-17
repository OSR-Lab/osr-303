/*******************************************************************************
 ******     Copyright (c) 2014--2020 OSR.Co.Ltd. All rights reserved.     ****** 
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ECCP.h"
#include "utility.h"

/*******************************************************************************
 * all big-integer buffers in this file use a word length of ECCP_WORDLEN(+1)
 *******************************************************************************/


extern void ECCP_a2aR(U32 a[], U32 aR[]);


U32 ECCP_BITLEN;             // bit length of the modulus p of the ECC curve over GF(p)
U32 ECCP_BYTELEN;
U32 ECCP_WORDLEN;
U32 ECCP_N_BITLEN;           // bit length of the order n of the ECC curve over GF(p)
U32 ECCP_N_BYTELEN;
U32 ECCP_N_WORDLEN;
U32 ECCP_COMB_n;
U32 *ECCP_COMB_iGR;
U32 *ECCP_COMB_iG;
U32 *ECCP_p;
U32 *ECCP_a;
U32 *ECCP_b;
U32 *ECCP_Gx;
U32 *ECCP_Gy;
U32 *ECCP_n;

U32 ECCP_a_FLAG;             // 0: ECCP_a = -3; 1: else

#define _ECC_P_PLUS_1_DIV2_
#ifdef _ECC_P_PLUS_1_DIV2_
// to halve a mod p: if a is even use a/2, else (a-1)/2 + (p+1)/2 = (a+p)/2
#define ECCP_MAX_WORDLEN 64                  // supports curves up to ECC-2048
U32 ECC_P_plus_1_div2[ECCP_MAX_WORDLEN];     // store (p+1)/2
#endif

// NAF (non-adjacent form) digit: signed value and its bit index
typedef struct
{
	S32 value;
	U32 index;
}NAFKI;


#define ECCP_RESIDUE         // enable residue-based (Montgomery) modular arithmetic
#ifdef ECCP_RESIDUE
U32 * ECCP_R = NULL;         //R mod ECC_p
U32 * ECCP_aR = NULL;        //ECCP_a*R mod ECC_p
#endif

/* Function: ECC GF(p) Initialize
 * Parameters:
 *     p ----------- ECC parameter, prime p
 *     a ----------- ECC parameter, a
 *     b ----------- ECC parameter, b
 *     Gx ---------- ECC parameter, x of Generator G(x,y) 
 *     Gy ---------- ECC parameter, y of Generator G(x,y) 
 *     n ----------- ECC parameter, order of Generator G(x,y)
 *     PBitLen ----- ECC parameter, bit length of prime p
 *     NBitLen ----- ECC parameter, bit length of order n of point G
 * Return: 
 * Caution:
 *     1. make sure PBitLen >= 160
 */
void ECCP_Init(U32 *p, U32 *a, U32 *b, U32 *Gx, U32 *Gy, U32 *n, U32 PBitLen, U32 NBitLen, U32 COMB_n, U32 *COMB_iGR, U32 *COMB_iG)
{
	U32 temp =3, wordLen;
	U32 *A;

	ECCP_p = p;
	ECCP_a = a;
	ECCP_b = b;
	ECCP_Gx= Gx;
	ECCP_Gy= Gy;
	ECCP_n = n;
	ECCP_COMB_n  =COMB_n;
	ECCP_COMB_iGR=COMB_iGR;
	ECCP_COMB_iG =COMB_iG;
	ECCP_BITLEN = PBitLen;
	ECCP_BYTELEN= (PBitLen+0x07)>>3;
	ECCP_WORDLEN= (PBitLen+0x1F)>>5;
	ECCP_N_BITLEN = NBitLen;
	ECCP_N_BYTELEN= (NBitLen+0x07)>>3;    
	ECCP_N_WORDLEN= (NBitLen+0x1F)>>5; 

	//set a flag(whether ECC_a equals -3)
	A = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	U32_Copy(A, ECCP_a, ECCP_WORDLEN);
	wordLen = Big_Add(A, Get_WordLen(A, ECCP_WORDLEN), &temp, 1);
	if(Big_Compare(A, wordLen, ECCP_p, ECCP_WORDLEN)==0) 
		ECCP_a_FLAG=0;
	else
		ECCP_a_FLAG=1; 

#ifdef _ECC_P_PLUS_1_DIV2_
	//get (p+1)/2
	U32_Copy(ECC_P_plus_1_div2, ECCP_p, ECCP_WORDLEN);
	temp = 1;
	Big_Add(ECC_P_plus_1_div2, ECCP_WORDLEN, &temp, 1);
	Big_Div2(ECC_P_plus_1_div2, ECCP_WORDLEN);
#endif

#ifdef ECCP_RESIDUE
	if(NULL != ECCP_R)         // on the previous exit ECCP_Close was not called to free memory; recompute here to prevent insufficient space if the ECC bit length changed
	{
		free(ECCP_R);
	}
	ECCP_R = (U32 *)malloc((ECCP_WORDLEN)*4);
	U32_Clear(A, ECCP_WORDLEN);
	A[0] = 1;
	ECCP_a2aR(A, ECCP_R);      // ECCP_R = R mod ECCP_p

	if(ECCP_a_FLAG != 0)  // ECCP_a is not -3
	{
		if(NULL != ECCP_aR)    // free ECCP_aR if it was allocated
		{
			free(ECCP_aR);
		}
		ECCP_aR = (U32 *)malloc((ECCP_WORDLEN)*4);
		ECCP_a2aR(ECCP_a, ECCP_aR);    // ECCP_aR = ECCP_a*R mod ECCP_p
	}
#endif

	free(A);
}

/* Function: ECC GF(p) Initialize
 * Parameters:
 *     curve ------- ECC curve structure
 * Return: 
 * Caution:
 *     1. make sure PBitLen of the curve >= 160
 */
void ECCP_Init_1(ECCP_CURVE *curve)
{
	ECCP_Init(curve->ECCP_p, curve->ECCP_a, curve->ECCP_b, curve->ECCP_Gx, curve->ECCP_Gy, 
			  curve->ECCP_n, curve->ECCP_BITLEN, curve->ECCP_N_BITLEN, curve->ECCP_COMB_n,
			  curve->ECCP_COMB_iGR, curve->ECCP_COMB_iG);

	curve->ECCP_BYTELEN= ECCP_BYTELEN;
	curve->ECCP_WORDLEN= ECCP_WORDLEN;
	curve->ECCP_N_BYTELEN= ECCP_N_BYTELEN;    
	curve->ECCP_N_WORDLEN= ECCP_N_WORDLEN; 
	curve->ECCP_a_FLAG=ECCP_a_FLAG;
}

/* Function: ECC GF(p) Close
 * Parameters:
 * Return: 
 * Caution:
 */
void ECCP_Close()
{
	ECCP_p = NULL;
	ECCP_a = NULL;
	ECCP_b = NULL;
	ECCP_Gx= NULL;
	ECCP_Gy= NULL;
	ECCP_n = NULL;

	//U32_Clear(ECC_P_plus_1_div2, ECCP_WORDLEN);    // (optional) clear the (p+1)/2 table

	ECCP_BITLEN = 0;
	ECCP_BYTELEN= 0;
	ECCP_WORDLEN= 0;
	ECCP_N_BITLEN = 0;
	ECCP_N_BYTELEN= 0;
	ECCP_N_WORDLEN= 0;

	ECCP_a_FLAG=0;

#ifdef ECCP_RESIDUE
	if(NULL != ECCP_R)         // if ECCP_R was allocated (not NULL), free it
	{
		free(ECCP_R);
		ECCP_R = NULL;
	}

	if(ECCP_a_FLAG != 0)  // ECCP_a is not -3    
	{
		if(NULL != ECCP_aR)    // free ECCP_aR if it was allocated
		{
			free(ECCP_aR);
			ECCP_aR = NULL;
		}
	}
#endif
}

/* Function: Convert ECCp Point to ECCp Point Char buffer
 * Parameters:
 * Return: 
 * Caution:
 */
void ECCP_Point2Char(ECCP_POINT *P, U8 *c)
{
	c[0] = 0x04;
	U8_BigInt_reverse((U8 *)P->x, c + 1, ECCP_BYTELEN);  
	U8_BigInt_reverse((U8 *)P->y, c + 1 + ECCP_BYTELEN, ECCP_BYTELEN);  
}

/* Function: Convert ECCp Point Char buffer to ECCp Point
 * Parameters:
 * Return: 
 * Caution: 1. bytes beyond ECCP_BYTELEN are zero-padded to a 4-byte boundary
 */
void ECCP_Char2Point(U8 *c, ECCP_POINT *P)
{
	U8_BigInt_reverse_with_padding0(c + 1, (U8 *)P->x, ECCP_BYTELEN);        
	U8_BigInt_reverse_with_padding0(c + 1 + ECCP_BYTELEN, (U8 *)P->y, ECCP_BYTELEN);  
}

/* Function: check big integer k valid or not
 * Parameters:
 *     k      -------- input big integer
 *     kWORDLEN ------ word length of k
 *     n      -------- upper bound of input integer
 *     nWORDLEN ------ word length of n
 * Return: 
 *     0(k is in [1,n-1]); other(k is 0 or k >= n)
 * Caution:
 */
U8 ECCP_IntegerCheck(U32 k[], U32 kWORDLEN, U32 n[], U32 nWORDLEN)      // typically kWORDLEN==nWORDLEN; for ECC, n is the group order (or p)
{
	if(Bigint_Check_NULL(k, kWORDLEN) == 1) 
	{
		return ECCPZeroALL;
	}
	else if(Big_Compare(k, kWORDLEN, n, nWORDLEN) != 2) 
	{
		return ECCPIntegerTooBig;
	}

	return ECCPSuccess;
}

// scratch variables for carry propagation
U32 ECCP_carry;
U32 ECCP_temp_U32;
U32 ECCP_temp_a;

/* Function: c = (a+b) without carry
 * Parameters:
 *     a   --------- input, big integer a, buffer word length is ECCP_WORDLEN
 *     b   --------- input, big integer b, buffer word length is ECCP_WORDLEN
 *     c   --------- output, big integer c=(a+b), buffer word length is ECCP_WORDLEN
 * Return: 1(a+b overflows ECCP_WORDLEN words), 0(a+b fits in ECCP_WORDLEN words)
 * Caution:
 *     1. the buffers a, b, c all have word length ECCP_WORDLEN
 *     2. a and c, or a and b, may share a buffer; likewise b and c, or a, b and c together
 */
U32 ECCP_Add_Without_Carry(U32 a[], U32 b[], U32 c[])
{
	U32 i;

	ECCP_carry = 0;
	for(i=0; i<ECCP_WORDLEN; i++)
	{
		ECCP_temp_U32 = a[i] + b[i];
		c[i] = ECCP_temp_U32 + ECCP_carry;
		if((ECCP_temp_U32 < b[i]) || (c[i] < ECCP_carry))
		{
			ECCP_carry = 1;
		}
		else
		{
			ECCP_carry = 0;
		}
	}

	return ECCP_carry;
}

/* Function: c = (a-b) without carry
 * Parameters:
 *     a   --------- input, big integer a, buffer word length is ECCP_WORDLEN
 *     b   --------- input, big integer b, buffer word length is ECCP_WORDLEN
 *     c   --------- output, big integer c=(a-b), buffer word length is ECCP_WORDLEN
 * Return: 1(a<b, so c = 2^(32*ECCP_WORDLEN) + a - b, i.e. a borrow occurred), 0(a>=b)
 * Caution:
 *     1. the buffers a, b, c all have word length ECCP_WORDLEN
 *     2. a, b, c may share the same buffer
 */
U32 ECCP_Sub_Without_Carry(U32 a[], U32 b[], U32 c[])
{
	U32 i;

	ECCP_carry = 0;
	for(i=0; i<ECCP_WORDLEN; i++)
	{
		ECCP_temp_a = a[i];
		ECCP_temp_U32 = a[i] - b[i];
		c[i] = ECCP_temp_U32 - ECCP_carry;
		if((ECCP_temp_a < ECCP_temp_U32) || (ECCP_temp_U32 < ECCP_carry))
		{
			ECCP_carry = 1;
		}
		else
		{
			ECCP_carry = 0;
		}
	}

	return ECCP_carry;
}

/* Function: a = a+b mod N
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 * Return: word length of a+b mod N
 * Caution:
 *     1. make sure a[aWordLen] available
 */
U32 ECCP_ModAdd(U32 *a, U32 aWordLen, U32 *b, U32 bWordLen, U32 *N)
{
	aWordLen = Get_WordLen(a, aWordLen);    
	bWordLen = Get_WordLen(b, bWordLen);

	if(!bWordLen)
		return aWordLen;

	aWordLen = Big_Add(a, aWordLen, b, bWordLen);
	if(Big_Compare(a, aWordLen, N, ECCP_WORDLEN)!=2)  //if a >= ECCP_p
		aWordLen = Big_Sub(a, aWordLen, N, ECCP_WORDLEN); 

	return aWordLen;
}

/* Function: c = a+b mod ECCP_p in ECC GF(p)
 * Parameters:
 *     a ----------- input, big integer a, less than ECCP_p
 *     b ----------- input, big integer b, less than ECCP_p
 *     c ----------- output, c = a+b mod ECCP_p
 * Return: none
 * Caution:
 *     1. unlike ECCP_ModAdd, a[aWordLen] is not needed; a, b, c have buffer word length ECCP_WORDLEN
 *     2. as in ECCP_Add_Without_Carry, a, b, c may share a buffer
 */
void ECCP_ModAdd_(U32 *a, U32 *b, U32 *c)
{
	U32 ret;

	ret = ECCP_Add_Without_Carry(a,b,c);

	// if ret==1, then a+b overflowed and exceeds ECCP_p, so c - ECCP_p = 2^n + c - ECCP_p
	// otherwise ret==0
	if(ret || Big_Compare(c, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN)!=2)   //if a+b >= ECCP_p
		ECCP_Sub_Without_Carry(c, ECCP_p, c);
}

/* Function: a = a-b mod N
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 * Return: word length of a-b mod N
 * Caution:
 *     1. make sure a[aWordLen] available
 */
U32 ECCP_ModSub(U32 *a, U32 aWordLen, U32 *b, U32 bWordLen, U32 *N)
{
	
	aWordLen = Get_WordLen(a, aWordLen);    
	bWordLen = Get_WordLen(b, bWordLen);

	if(!bWordLen)
		return aWordLen;

	if(Big_Compare(a, aWordLen, b, bWordLen)==2)           //if a < b
		aWordLen = Big_Add(a, aWordLen, N, ECCP_WORDLEN);
	aWordLen = Big_Sub(a, aWordLen, b, bWordLen); 

	return aWordLen;
}

/* Function: c = a-b mod ECCP_p in ECC GF(p)
 * Parameters:
 *     a ----------- input, big integer a, less than ECCP_p
 *     b ----------- input, big integer b, less than ECCP_p
 *     c ----------- output, big integer c = (a-b) mod ECCP_p
 * Return: none
 * Caution:
 *     1. unlike ECCP_ModSub, a[aWordLen] is not needed; a, b, c have buffer word length ECCP_WORDLEN
 *     2. a, b, c may share the same buffer
 */
void ECCP_ModSub_(U32 *a, U32 *b, U32 *c)
{
	U32 ret;

	ret = ECCP_Sub_Without_Carry(a, b, c);
	if(ret)
	{
		// a borrow occurred (a<b), so c = 2^n + a - b; adding ECCP_p yields a + ECCP_p - b
		ECCP_Add_Without_Carry(c,ECCP_p,c);
	}

/*  alternative version (a, b, c may share a buffer):
	if(Big_Compare(a, ECCP_WORDLEN, b, ECCP_WORDLEN)==2)               //if a < b
	{//printf("AA ");
		ECCP_Add_Without_Carry(a,ECCP_p,c);
		ECCP_Sub_Without_Carry(c, b, c);
	}
	else
	{
		ECCP_Sub_Without_Carry(a, b, c);
	}
*/
}

/* Function: a = 2*a mod ECCP_p in ECC GF(p)
 * Parameters:
 *     a ----------- big integer a, less than ECCP_p
 *     aWordLen ---- word length of a
 * Return: word length of 2*a mod ECCP_p
 * Caution:
 *     1. make sure a[aWordLen] available
 */
U32 ECCP_ModMul_two(U32 *a, U32 aWordLen)
{
	aWordLen = Get_WordLen(a, aWordLen);

	if(aWordLen==0)
	{
		return 0;
	}

	aWordLen = Big_Mul2(a, aWordLen);
	if(Big_Compare(a, aWordLen, ECCP_p, ECCP_WORDLEN)!=2)  //if a >= ECCP_p
		aWordLen = Big_Sub(a, aWordLen, ECCP_p, ECCP_WORDLEN);  

	return aWordLen;
}

/* Function: out = 2*a mod ECCP_p in ECC GF(p)
 * Parameters:
 *     a ----------- input, big integer a, less than ECCP_p
 *     out --------- output, big integer out = 2*a mod ECCP_p
 * Return: none
 * Caution:
 *     1. unlike ECCP_ModMul_two, a[aWordLen] is not needed; a and out have buffer word length ECCP_WORDLEN
 *     2. a and out may share a buffer
 */
void ECCP_ModMul_two_(U32 *a, U32 *out)
{
	U8 flag=0;
	S32 i;
	
	if(a[ECCP_WORDLEN-1]&0x80000000)
	{
		flag=1;
	}

	for(i=ECCP_WORDLEN-1; i>0; i--)
	{
		out[i] = a[i]<<1;
		out[i] |= (a[i-1]>>31);
	}
	out[i] = a[i]<<1;

	// see the comments for ECCP_ModAdd_
	if(flag || Big_Compare(out, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN)!=2)
	{
		ECCP_Sub_Without_Carry(out, ECCP_p, out);
	}
}

/* Function: a = 3*a mod ECCP_p in ECC GF(p)
 * Parameters:
 *     a ----------- big integer a, less than ECCP_p
 *     aWordLen ---- word length of a
 * Return: word length of 3*a mod ECCP_p
 * Caution:
 *     1. make sure a[aWordLen] available
 */
U32 ECCP_ModMul_three(U32 *a, U32 aWordLen)
{
	U32 *A, AWordLen;

	aWordLen = Get_WordLen(a, aWordLen);

	if(aWordLen==0)
	{
		return 0;
	}

	A = (U32 *)malloc((AWordLen=aWordLen)*4);
	U32_Copy(A, a, aWordLen);
	aWordLen = ECCP_ModMul_two(a, aWordLen);
	aWordLen = Big_Add(a, aWordLen, A, AWordLen);
	if(Big_Compare(a, aWordLen, ECCP_p, ECCP_WORDLEN)!=2)  //if a >= ECCP_p
		aWordLen = Big_Sub(a, aWordLen, ECCP_p, ECCP_WORDLEN);  

	free(A);
	return aWordLen;
}

/* Function: out = 3*a mod ECCP_p in ECC GF(p)
 * Parameters:
 *     a ----------- input, big integer a, less than ECCP_p
 *     out --------- output, big integer out = 3*a mod ECCP_p
 * Return: none
 * Caution:
 *     1. unlike ECCP_ModMul_three, a[aWordLen] is not needed; a and out have buffer word length ECCP_WORDLEN
 *     2. a and out may share a buffer
 */
void ECCP_ModMul_three_(U32 *a, U32 *out)
{
	U32 *A;

	A = (U32 *)malloc((ECCP_WORDLEN)*4);
	ECCP_ModMul_two_(a, A);
	ECCP_ModAdd_(a, A, out);  

	free(A);
}

/* Function: out = a/2 mod ECCP_p in ECC GF(p)
 * Parameters:
 *     a ----------- input, big integer a, less than ECCP_p
 *     out --------- output, big integer out = a/2 mod ECCP_p
 * Return: none
 * Caution:
 *     1. a and out have buffer word length ECCP_WORDLEN
 *     2. a and out may share a buffer
 */
void ECCP_ModDiv_two_(U32 *a, U32 *out)
{
	U8 flag=0;
	
#ifdef _ECC_P_PLUS_1_DIV2_
	if(a[0] & 1)
		flag = 1;

	Big_Div2_(a, ECCP_WORDLEN, out);
	if(flag)
	{
		ECCP_ModAdd_(out, ECC_P_plus_1_div2, out);                                               //y/2=(y-1)/2+(p+1)/2
	}
#else
	if(a[0] & 1)
	{
		flag = ECCP_Add_Without_Carry(a, ECCP_p, out);                                           //y=y+p
		Big_Div2(out, ECCP_WORDLEN);  
		if(flag)
			out[ECCP_WORDLEN-1] |= 0x80000000;                                                   //y/2=(y+p)/2
	}
	else
	{
		Big_Div2_(a, ECCP_WORDLEN, out);
	}
#endif
}

/* Function: a -> aR mod ECCP_p (both a and aR buffer word length are ECCP_WORDLEN, and their value both less than ECCP_p)
 * Parameters:
 *     a   --------- input, big integer a mod ECCP_p
 *     aR   -------- output, aR mod ECCP_p
 * Return: none
 * Caution:
 *     1. a and aR can be the same buffer
 */
void ECCP_a2aR(U32 a[], U32 aR[])
{
	U32 *R;
	U32 wordLen;

	R = (U32 *)malloc((ECCP_WORDLEN<<1)*4);
	U32_Clear(R, ECCP_WORDLEN);
	U32_Copy(R+ECCP_WORDLEN, a, ECCP_WORDLEN);
	Big_Mod_2(R, ECCP_WORDLEN<<1, ECCP_p, ECCP_WORDLEN, aR, &wordLen);

	free(R);
}

/* Function: aR -> a mod ECCP_p (both a and aR buffer word length are ECCP_WORDLEN, and their value both less than ECCP_p)
 * Parameters:
 *     aR  --------- input, big integer aR mod ECCP_p
 *     a    -------- output, a mod ECCP_p
 * Return: none
 * Caution:
 *     1. a and aR can be the same buffer
 */
void ECCP_aR2a(U32 aR[], U32 out[])
{
	U32 b=1;
	U32 wordLen = Get_WordLen(aR, ECCP_WORDLEN);

	MontMulFIPS_AB_(aR, wordLen, &b, 1, ECCP_p, ECCP_WORDLEN, out, &wordLen);
}

/* Function: ECC GF(p) affine point to Jacobi point
 * Parameters:
 *     Pin --------- input, pointer to affine point
 *     Pout -------- output, pointer to Jacobi point
 * Return: 
 * Caution:
 */
void ECCP_Point2JacobiPoint(ECCP_POINT *Pin, ECCP_JACOBIPOINT *Pout)
{
	U32_Copy(Pout->x, Pin->x, ECCP_WORDLEN);
	U32_Copy(Pout->y, Pin->y, ECCP_WORDLEN);
	U32_Clear(Pout->z, ECCP_WORDLEN);
	Pout->z[0] = 1;
} 

/* Function: ECC GF(p) affine point to Jacobi point (Residue domain)
 * Parameters:
 *     Pin --------- input, pointer to affine point
 *     Pout -------- output, pointer to Jacobi point
 * Return: 
 * Caution:
 */
void ECCP_Point2JacobiPoint_Residue(ECCP_POINT *Pin, ECCP_JACOBIPOINT *Pout)
{
	U32_Copy(Pout->x, Pin->x, ECCP_WORDLEN);
	U32_Copy(Pout->y, Pin->y, ECCP_WORDLEN);
	U32_Copy(Pout->z, ECCP_R, ECCP_WORDLEN);
} 

/* Function: ECC GF(p) Jacobi point to affine point
 * Parameters:
 *     Pin --------- input, pointer to Jacobi point
 *     Pout -------- output, pointer to affine point
 * Return: 0(all OK), 1(Pin is zero point)
 * Caution:
 *     1. Pin can not be zero point (1,1,0)
 *     2. Pin, Pout can not be the same
 */
U8 ECCP_JacobiPoint2Point(ECCP_JACOBIPOINT *Pin, ECCP_POINT *Pout)
{
	U32 *z_inv, *z_inv2;
	U32 wordLen;

	if(Bigint_Check_NULL(Pin->z, ECCP_WORDLEN))                                      // if z=0, thus Pin is Zero Point
		return 1;

	z_inv = (U32 *)malloc((ECCP_WORDLEN)*4);
	z_inv2 = (U32 *)malloc((ECCP_WORDLEN)*4);
	
	Big_ModInv_Odd_1(Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, z_inv, &wordLen);   // z_inv = z^(-1) mod ECCP_p
	//U32_Clear(z_inv+wordLen, ECCP_WORDLEN-wordLen);

#if 0
	Big_ModMul_2(Pin->x, ECCP_WORDLEN, z_inv, wordLen, ECCP_p, ECCP_WORDLEN, z_inv2, &wordLen);
	Big_ModMul_2(z_inv2, wordLen, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->x, &wordLen);
	Big_ModMul_2(Pin->y, ECCP_WORDLEN, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, z_inv2, &wordLen);
	Big_ModMul_2(z_inv2, wordLen, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);
	Big_ModMul_2(Pout->y, wordLen, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);
#else
	Big_ModMul_2(z_inv, wordLen, z_inv, wordLen, ECCP_p, ECCP_WORDLEN, z_inv2, &wordLen);
	Big_ModMul_2(Pin->x, ECCP_WORDLEN, z_inv2, wordLen, ECCP_p, ECCP_WORDLEN, Pout->x, &wordLen);
	Big_ModMul_2(z_inv, ECCP_WORDLEN, z_inv2, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);
	Big_ModMul_2(Pout->y, wordLen, Pin->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);  
#endif

	free(z_inv);
	free(z_inv2);
	return 0;
}

/* Function: ECC GF(p) Jacobi point to affine point (Residue domain)
 * Parameters:
 *     Pin --------- input, pointer to Jacobi point
 *     Pout -------- output, pointer to affine point
 * Return: 0(all OK), 1(Pin is zero point)
 * Caution:
 *     1. Pin can not be zero point (1,1,0)
 *     2. Pin, Pout can not be the same
 */
U8 ECCP_JacobiPoint2Point_Residue(ECCP_JACOBIPOINT *Pin, ECCP_POINT *Pout)
{
	U32 *z_inv, *z_inv2;
	U32 wordLen;

	if(Bigint_Check_NULL(Pin->z, ECCP_WORDLEN))                                      // if z=0, thus Pin is Zero Point
		return 1;

	z_inv = (U32 *)malloc((ECCP_WORDLEN)*4);
	z_inv2 = (U32 *)malloc((ECCP_WORDLEN)*4);
	
	ECCP_aR2a(Pin->z,z_inv2);
	Big_ModInv_Odd_1(z_inv2, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, z_inv, &wordLen);   // z_inv = z^(-1) mod ECCP_p	
	ECCP_a2aR(z_inv,z_inv);                                                          // z_inv =  z^(-1)R mod ECCP_p	 
	//U32_Clear(z_inv+wordLen, ECCP_WORDLEN-wordLen);

#if 0
	MontMulFIPS_AB_(Pin->x, ECCP_WORDLEN, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, z_inv2, &wordLen);
	MontMulFIPS_AB_(z_inv2, wordLen, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->x, &wordLen);
	MontMulFIPS_AB_(Pin->y, ECCP_WORDLEN, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, z_inv2, &wordLen);
	MontMulFIPS_AB_(z_inv2, wordLen, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);
	MontMulFIPS_AB_(Pout->y, wordLen, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);
#else
	MontMulFIPS_AB_(z_inv, ECCP_WORDLEN, z_inv, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, z_inv2, &wordLen);
	MontMulFIPS_AB_(Pin->x, ECCP_WORDLEN, z_inv2, wordLen, ECCP_p, ECCP_WORDLEN, Pout->x, &wordLen);
	MontMulFIPS_AB_(z_inv, ECCP_WORDLEN, z_inv2, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);
	MontMulFIPS_AB_(Pout->y, wordLen, Pin->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);
#endif

	free(z_inv);
	free(z_inv2);
	return 0;
}

/* Function:  ECC GF(p) Pout = 2*Pin in Jacobi Coordinate
 * Parameters:
 *     Pin --------- input Jacobi point
 *     Pout -------- output Jacobi point
 * Return: 
 * Caution:
 *     1. Pin and Pout can be the same
 *     2. all buffers have word length ECCP_WORDLEN+1
 */
void ECCP_JacobiPoint_Double(ECCP_JACOBIPOINT *Pin, ECCP_JACOBIPOINT *Pout)
{
	U32 wordLen;
#if 1
	U32 *A, *B, *C, *D;
#else
	U32 A[8+1], B[8+1], C[8+1], D[8+1];
#endif

    // if z=0, thus Pin is Zero Point, so be Pout; or if Pin is on x-axis, thus Pout is Zero Point too.
	if(Bigint_Check_NULL(Pin->z, ECCP_WORDLEN) || Bigint_Check_NULL(Pin->y, ECCP_WORDLEN))
	{
		U32_Clear(Pout->x, ECCP_WORDLEN);
		U32_Clear(Pout->y, ECCP_WORDLEN);
		U32_Clear(Pout->z, ECCP_WORDLEN);
		Pout->x[0] = Pout->y[0] = 1;
		return;
	}
#if 1
	A = (U32 *)malloc((ECCP_WORDLEN+1)*4); // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	B = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	C = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	D = (U32 *)malloc((ECCP_WORDLEN+1)*4);
#endif
	if(ECCP_a_FLAG==0)  // if ECCP_a == -3
	{
		Big_ModMul_2(Pin->z, ECCP_WORDLEN, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);   //B=z^2
		ECCP_ModSub_(Pin->x, B, C);                                                                    //C=x-z^2
		ECCP_ModAdd_(Pin->x, B, D);                                                                    //D=x+z^2
		Big_ModMul_2(C, ECCP_WORDLEN, D, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);             //A=x^2-z^4
		ECCP_ModMul_three_(A, A);                                                                      //A=3(x^2-z^4)
	}
	else                // if ECCP_a != -3
	{
		Big_ModMul_2(Pin->x, ECCP_WORDLEN, Pin->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);   //C=x^2
		ECCP_ModMul_three_(C, C);                                                                      //C=3*x^2
		Big_ModMul_2(Pin->z, ECCP_WORDLEN, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, D, &wordLen);   //D=z^2
		Big_ModMul_2(D, wordLen, D, wordLen, ECCP_p, ECCP_WORDLEN, B, &wordLen);                       //B=z^4
		Big_ModMul_2(B, wordLen, ECCP_a, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);             //A=a*z^4	
		ECCP_ModAdd_(A, C, A);                                                                         //A=3*x^2+a*z^4  
	}

	ECCP_ModMul_two_(Pin->y, B);                                                                       //B=2y
	Big_ModMul_2(Pin->z, ECCP_WORDLEN, B, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->z, &wordLen);      //Z3=2yz                                                     
	Big_ModMul_2(B, ECCP_WORDLEN, B, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);                 //C=B^2=4*y^2
	Big_ModMul_2(C, wordLen, Pin->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);           //Y3=C*x
	Big_ModMul_2(A, ECCP_WORDLEN, A, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);                 //B=A^2
	ECCP_ModMul_two_(Pout->y, D);                                                                      //D=2*Y3
	ECCP_ModSub_(B, D, Pout->x);                                                                       //X3=A^2-2D
	Big_ModMul_2(C, ECCP_WORDLEN, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);                 //B=C^2
	ECCP_ModDiv_two_(B, B);                                                                            //B=(B^2)/2

	ECCP_ModSub_(Pout->y, Pout->x, C);                                                                 //C=4*x*y^2-x3
	Big_ModMul_2(C, ECCP_WORDLEN, A, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);           //C=A(4*x*y^2-x3)
	ECCP_ModSub_(Pout->y, B, Pout->y);                                                                 //Y3=A(4*x*y^2-x3)-B

#if 1
	free(A);
	free(B);
	free(C);
	free(D);
#endif
}

/* Function:  ECC GF(p) Pout = 2*Pin in Jacobi Coordinate (Residue domain)
 * Parameters:
 *     Pin --------- input Jacobi point
 *     Pout -------- output Jacobi point
 * Return: 
 * Caution:
 *     1. Pin and Pout can be the same
 *     2. all buffers have word length ECCP_WORDLEN+1
 *     3. inputs and outputs are in Montgomery form (R mod ECC_p)
 */
void ECCP_JacobiPoint_Double_Residue(ECCP_JACOBIPOINT *Pin, ECCP_JACOBIPOINT *Pout)
{
	U32 wordLen;
#if 1
	U32 *A, *B, *C, *D;
#else
	U32 A[8+1], B[8+1], C[8+1], D[8+1];
#endif

    // if z=0, thus Pin is Zero Point, so be Pout; or if Pin is on x-axis, thus Pout is Zero Point too.
	if(Bigint_Check_NULL(Pin->z, ECCP_WORDLEN) || Bigint_Check_NULL(Pin->y, ECCP_WORDLEN))
	{
		U32_Clear(Pout->x, ECCP_WORDLEN);
		U32_Clear(Pout->y, ECCP_WORDLEN);
		U32_Clear(Pout->z, ECCP_WORDLEN);
		Pout->x[0] = Pout->y[0] = 1;
		return;
	}
#if 1
	A = (U32 *)malloc((ECCP_WORDLEN+1)*4); // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	B = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	C = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	D = (U32 *)malloc((ECCP_WORDLEN+1)*4);
#endif
	if(ECCP_a_FLAG==0)  // if ECCP_a == -3
	{
		MontMulFIPS_AB_(Pin->z, ECCP_WORDLEN, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);  //B=z^2		
		ECCP_ModSub_(Pin->x, B, C);                                                                      //C=x-z^2
		ECCP_ModAdd_(Pin->x, B, D);                                                                      //D=x+z^2
		MontMulFIPS_AB_(C, ECCP_WORDLEN, D, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);            //A=x^2-z^4		 
		ECCP_ModMul_three_(A, A);                                                                        //A=3(x^2-z^4)
	}
	else                // if ECCP_a != -3
	{
		MontMulFIPS_AB_(Pin->x, ECCP_WORDLEN, Pin->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);  //C=x^2
		ECCP_ModMul_three_(C, C);                                                                        //C=3*x^2
		MontMulFIPS_AB_(Pin->z, ECCP_WORDLEN, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, D, &wordLen);  //D=z^2		
		MontMulFIPS_AB_(D, wordLen, D, wordLen, ECCP_p, ECCP_WORDLEN, B, &wordLen);                      //B=z^4		     
		MontMulFIPS_AB_(B, wordLen, ECCP_aR, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);           //A=a*z^4	-----------------------ECCP_a in Montgomery form is ECCP_aR		 
		ECCP_ModAdd_(A, C, A);                                                                           //A=3*x^2+a*z^4  
	}

	ECCP_ModMul_two_(Pin->y, B);                                                                         //B=2y
	MontMulFIPS_AB_(B, ECCP_WORDLEN, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->z, &wordLen);     //Z3=2yz 
	MontMulFIPS_AB_(B, ECCP_WORDLEN, B, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);                //C=B^2=4*y^2
	MontMulFIPS_AB_(C, wordLen, Pin->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);          //Y3=C*x                                                 
	MontMulFIPS_AB_(A, ECCP_WORDLEN, A, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);                //B=A^2	    
	ECCP_ModMul_two_(Pout->y, D);                                                                        //D=2*Y3
	ECCP_ModSub_(B, D, Pout->x);                                                                         //X3=A^2-2D
    MontMulFIPS_AB_(C, ECCP_WORDLEN, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);                //B=C^2
	ECCP_ModDiv_two_(B, B);                                                                              //B=(B^2)/2

	ECCP_ModSub_(Pout->y, Pout->x, C);  
	MontMulFIPS_AB_(C, ECCP_WORDLEN, A, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);
	ECCP_ModSub_(Pout->y, B, Pout->y); 

#if 1
	free(A);
	free(B);
	free(C);
	free(D);
#endif
}

/* Function:  ECC GF(p) Pout = (2^m)*Pin in Jacobi Coordinate
 * Parameters:
 *     Pin --------- input Jacobi point
 *     m   --------- count of point double
 *     Pout -------- output Jacobi point
 * Return: 
 * Caution:
 *     1. Pin and Pout can be the same
 *     2. make sure m>0
 *     3. all buffers have word length ECCP_WORDLEN+1
 */
void ECCP_JacobiPoint_Double_m(ECCP_JACOBIPOINT *Pin, U32 m, ECCP_JACOBIPOINT *Pout)
{
	//U32 wordLen, flag=0;
	U32 wordLen;
#if 1
	U32 *A, *B, *C, *W;
#else
	U32 A[8+1], B[8+1], C[8+1], W[8+1];
#endif

    // if z=0, thus Pin is Zero Point, so be Pout; or if Pin is on x-axis, thus Pout is Zero Point too.
	if(Bigint_Check_NULL(Pin->z, ECCP_WORDLEN) || Bigint_Check_NULL(Pin->y, ECCP_WORDLEN))
	{
		U32_Clear(Pout->x, ECCP_WORDLEN);
		U32_Clear(Pout->y, ECCP_WORDLEN);
		U32_Clear(Pout->z, ECCP_WORDLEN);
		Pout->x[0] = Pout->y[0] = 1;
		return;
	}
#if 1
	A = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	B = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	C = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	W= (U32 *)malloc((ECCP_WORDLEN+1)*4);
#endif

	ECCP_ModMul_two_(Pin->y, Pout->y);                                                                 //y=2y
	Big_ModMul_2(Pin->z, ECCP_WORDLEN, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);
	Big_ModMul_2(A, wordLen, A, wordLen, ECCP_p, ECCP_WORDLEN, W, &wordLen);                           //W=z^4

	while(m>0)
	{
		Big_ModMul_2(Pin->x, ECCP_WORDLEN, Pin->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);   //A=x^2
		if(ECCP_a_FLAG==0)  // if ECCP_a == -3
		{			
			ECCP_ModSub_(A, W, A);                                                                     //A=x^2-W			
			ECCP_ModMul_three_(A, A);                                                                  //A=3(x^2-W)
		}
		else                // if ECCP_a != -3
		{
			ECCP_ModMul_three_(A, A);                                                                  //A=3(x^2)
			Big_ModMul_2(ECCP_a, ECCP_WORDLEN, W, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);    //B=a(z^4)
			ECCP_ModAdd_(A, B, A);                                                                     //A=3(x^2)+a(z^4)
		}

		Big_ModMul_2(Pin->z, ECCP_WORDLEN, Pout->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->z, &wordLen);    //z=zy
		Big_ModMul_2(Pout->y, ECCP_WORDLEN, Pout->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);         //C=y^2
		Big_ModMul_2(C, ECCP_WORDLEN, Pin->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);                //B=xy^2
		Big_ModMul_2(A, ECCP_WORDLEN, A, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->x, &wordLen);               //x=A^2
		ECCP_ModMul_two_(B, Pout->y);
		ECCP_ModSub_(Pout->x, Pout->y, Pout->x);                                                               //x=(A^2)-2B
		Big_ModMul_2(C, ECCP_WORDLEN, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);               //y=y^4
		ECCP_ModSub_(B, Pout->x, B);                                                                           //B=B-x
		Big_ModMul_2(A, ECCP_WORDLEN, B, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);                     //A=A(B-x)

		m-=1;
		if(m)
		{
			Big_ModMul_2(W, ECCP_WORDLEN, Pout->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, W, &wordLen);           //W=W(y^4)
			ECCP_ModMul_two_(A, A);                                                                            //A=2A(B-x)
			ECCP_ModSub_(A, Pout->y, Pout->y);                                                                 //A=2A(B-x)-y^4
		}
		else
		{
			ECCP_ModDiv_two_(Pout->y, Pout->y);                                                                //y=(y^4)/2
			ECCP_ModSub_(A, Pout->y, Pout->y);                                                                 //y=A(B-x)-(y^4)/2	
		}
	}

#if 1
	free(A);
	free(B);
	free(C);
	free(W);
#endif
}

/* Function:  ECC GF(p) Pout = (2^m)*Pin in Jacobi Coordinate (Residue domain)
 * Parameters:
 *     Pin --------- input Jacobi point
 *     m   --------- count of point double
 *     Pout -------- output Jacobi point
 * Return: 
 * Caution:
 *     1. Pin and Pout can be the same
 *     2. make sure m>0
 *     3. all buffers have word length ECCP_WORDLEN+1
 *     4. inputs and outputs are in Montgomery form (R mod ECC_p)
 */
void ECCP_JacobiPoint_Double_m_Residue(ECCP_JACOBIPOINT *Pin, U32 m, ECCP_JACOBIPOINT *Pout)
{
	U32 wordLen, flag=0;
#if 1
	U32 *A, *B, *C, *W;
#else
	U32 A[8+1], B[8+1], C[8+1], W[8+1];
#endif

    // if z=0, thus Pin is Zero Point, so be Pout; or if Pin is on x-axis, thus Pout is Zero Point too.
	if(Bigint_Check_NULL(Pin->z, ECCP_WORDLEN) || Bigint_Check_NULL(Pin->y, ECCP_WORDLEN))
	{
		U32_Clear(Pout->x, ECCP_WORDLEN);
		U32_Clear(Pout->y, ECCP_WORDLEN);
		U32_Clear(Pout->z, ECCP_WORDLEN);
		Pout->x[0] = Pout->y[0] = 1;
		return;
	}
#if 1
	A = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	B = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	C = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	W = (U32 *)malloc((ECCP_WORDLEN+1)*4);
#endif

	ECCP_ModMul_two_(Pin->y, Pout->y);                                                                   //y=2y
	MontMulFIPS_AB_(Pin->z, ECCP_WORDLEN, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);
	MontMulFIPS_AB_(A, wordLen, A, wordLen, ECCP_p, ECCP_WORDLEN, W, &wordLen);                          //W=z^4	   

	while(m>0)
	{
		MontMulFIPS_AB_(Pin->x, ECCP_WORDLEN, Pin->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);  //A=x^2		
		if(ECCP_a_FLAG==0)  // if ECCP_a == -3
		{			
			ECCP_ModSub_(A, W, A);                                                                       //A=x^2-W
			ECCP_ModMul_three_(A, A);                                                                    //A=3(x^2-W)
		}
		else                // if ECCP_a != -3
		{
			ECCP_ModMul_three_(A, A);                                                                    //A=3(x^2)
			MontMulFIPS_AB_(ECCP_aR, ECCP_WORDLEN, W, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);  //B=a(z^4)			
			ECCP_ModAdd_(A, B, A);                                                                       //A=3(x^2)+a(z^4)
		}

		MontMulFIPS_AB_(Pin->z, ECCP_WORDLEN, Pout->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->z, &wordLen);   //z=zy		
		MontMulFIPS_AB_(Pout->y, ECCP_WORDLEN, Pout->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);        //C=y^2		
		MontMulFIPS_AB_(C, ECCP_WORDLEN, Pin->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);               //B=xy^2		     
		MontMulFIPS_AB_(A, ECCP_WORDLEN, A, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->x, &wordLen);              //x=A^2
		ECCP_ModMul_two_(B, Pout->y);
		ECCP_ModSub_(Pout->x, Pout->y, Pout->x);                                                                 //x=(A^2)-2B
		MontMulFIPS_AB_(C, ECCP_WORDLEN, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->y, &wordLen);              //y=y^4		      
		ECCP_ModSub_(B, Pout->x, B);                                                                             //B=B-x
		MontMulFIPS_AB_(A, ECCP_WORDLEN, B, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);                    //A=A(B-x)
		
		m-=1;
		if(m)
		{
			MontMulFIPS_AB_(W, ECCP_WORDLEN, Pout->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, W, &wordLen);          //W=W(y^4)			
			ECCP_ModMul_two_(A, A);                                                                              //A=2A(B-x)
			ECCP_ModSub_(A, Pout->y, Pout->y);                                                                   //A=2A(B-x)-y^4
		}
		else
		{
			ECCP_ModDiv_two_(Pout->y, Pout->y);                                                                  //y=(y^4)/2
			ECCP_ModSub_(A, Pout->y, Pout->y);                                                                   //y=A(B-x)-(y^4)/2
		}
	}

#if 1
	free(A);
	free(B);
	free(C);
	free(W);
#endif
}

/* Function:  ECC GF(p) Pout = Pin + Pin2
 * Parameters:
 *     Pin --------- input Jacobi point
 *     Pin2 -------- input affine point
 *     Pout -------- output Jacobi point
 * Return: 
 * Caution:
 *     1. Pin and Pout can be the same
 *     2. make sure Pin and Pin2 not the same Point in E(GF(p))
 *     3. all buffers have word length ECCP_WORDLEN+1
 */
void ECCP_Jacobi_Affine_Point_Add(ECCP_JACOBIPOINT *Pin, ECCP_POINT *Pin2, ECCP_JACOBIPOINT *Pout)
{
	U32 wordLen;
	U32 *A=NULL, *B=NULL, *C=NULL, *D=NULL, *E=NULL;

    // if Pin->z=0, thus Pin is Zero Point, so Pout is Pin2; 
	if(Bigint_Check_NULL(Pin->z, ECCP_WORDLEN))
	{
		ECCP_Point2JacobiPoint(Pin2, Pout);
		return;
	}

	A = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	B = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	C = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	D = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	E = (U32 *)malloc((ECCP_WORDLEN+1)*4);

	Big_ModMul_2(Pin->z, ECCP_WORDLEN, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);       //A=z1^2
	Big_ModMul_2(A, wordLen, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);                 //B=z1^3
	Big_ModMul_2(A, ECCP_WORDLEN, Pin2->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);           //C=x2*z1^2
	Big_ModMul_2(B, ECCP_WORDLEN, Pin2->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, D, &wordLen);           //D=y2*z1^3
	ECCP_ModSub_(C, Pin->x, C);                                                                        //C=C-x1  -- E
	ECCP_ModSub_(D, Pin->y, D);                                                                        //D=D-y1  -- F

	if(Bigint_Check_NULL(C, ECCP_WORDLEN))
	{
		if(Bigint_Check_NULL(D, ECCP_WORDLEN))     //Pin and Pin2 are the same Point in E(GF(p))
		{
			ECCP_JacobiPoint_Double(Pin, Pout);
		}
		else
		{
			U32_Clear(Pout->x, ECCP_WORDLEN);
			U32_Clear(Pout->y, ECCP_WORDLEN);
			U32_Clear(Pout->z, ECCP_WORDLEN);
			Pout->x[0] = Pout->y[0] = 1;
		}
		goto end;
	}

	Big_ModMul_2(Pin->z, ECCP_WORDLEN, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->z, &wordLen);      //z=z1*C -- z1*E
	
	Big_ModMul_2(C, ECCP_WORDLEN, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);                 //A=C^2  -- E^2 -- G
	Big_ModMul_2(A, wordLen, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);                      //B=C^3  -- E^3 -- H
	Big_ModMul_2(Pin->x, ECCP_WORDLEN, A, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, E, &wordLen);            //E=x1*A -- x1*G - I
	Big_ModMul_2(D, ECCP_WORDLEN, D, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);                 //A=D^2  -- F^2 
	
	ECCP_ModMul_two_(E, C);                                                                            //C=2E   -- 2I
	ECCP_ModAdd_(C, B, C);                                                                             //C=2I+H
	ECCP_ModSub_(A, C, Pout->x);                                                                       //x=F^2-(H+2I)

	ECCP_ModSub_(E, Pout->x, E);                                                                       //E=E-x3 -- I-x3
	Big_ModMul_2(D, ECCP_WORDLEN, E, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);                 //A=F*(I-x3) 
	Big_ModMul_2(B, ECCP_WORDLEN, Pin->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);            //C=H*y1 
	ECCP_ModSub_(A, C, Pout->y);

end:
	if(NULL != A)
		free(A);

	if(NULL != B)
		free(B);

	if(NULL != C)
		free(C);

	if(NULL != D)
		free(D);

	if(NULL != E)
		free(E);
}

/* Function:  ECC GF(p) Pout = Pin + Pin2 (Residue domain)
 * Parameters:
 *     Pin --------- input Jacobi point
 *     Pin2 -------- input affine point
 *     Pout -------- output Jacobi point
 * Return: 
 * Caution:
 *     1. Pin and Pout can be the same
 *     2. make sure Pin and Pin2 not the same Point in E(GF(p))
 *     3. all buffers have word length ECCP_WORDLEN+1
 *     4. inputs and outputs are in Montgomery form (R mod ECC_p)
 */
void ECCP_Jacobi_Affine_Point_Add_Residue(ECCP_JACOBIPOINT *Pin, ECCP_POINT *Pin2, ECCP_JACOBIPOINT *Pout)
{
	U32 wordLen;
	U32 *A=NULL, *B=NULL, *C=NULL, *D=NULL, *E=NULL;

    // if Pin->z=0, thus Pin is Zero Point, so Pout is Pin2; 
	if(Bigint_Check_NULL(Pin->z, ECCP_WORDLEN))
	{
		ECCP_Point2JacobiPoint(Pin2, Pout);
		return;
	}

	A = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	B = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	C = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	D = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	E = (U32 *)malloc((ECCP_WORDLEN+1)*4);

	MontMulFIPS_AB_(Pin->z, ECCP_WORDLEN, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);      //A=z1^2	
	MontMulFIPS_AB_(A, wordLen, Pin->z, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);                //B=z1^3	 
	MontMulFIPS_AB_(A, ECCP_WORDLEN, Pin2->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);          //C=x2*z1^2	  
	MontMulFIPS_AB_(B, ECCP_WORDLEN, Pin2->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, D, &wordLen);          //D=y2*z1^3	
	ECCP_ModSub_(C, Pin->x, C);                                                                          //C=C-x1  -- E
	ECCP_ModSub_(D, Pin->y, D);                                                                          //D=D-y1  -- F

	if(Bigint_Check_NULL(C, ECCP_WORDLEN))
	{
		if(Bigint_Check_NULL(D, ECCP_WORDLEN))     //Pin and Pin2 are the same Point in E(GF(p))
		{
			ECCP_JacobiPoint_Double(Pin, Pout);
		}
		else
		{
			U32_Clear(Pout->x, ECCP_WORDLEN);
			U32_Clear(Pout->y, ECCP_WORDLEN);
			U32_Clear(Pout->z, ECCP_WORDLEN);
			Pout->x[0] = Pout->y[0] = 1;
		}
		goto end;
	}

	MontMulFIPS_AB_(Pin->z, ECCP_WORDLEN, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, Pout->z, &wordLen);     //z=z1*C -- z1*E	   
	
	MontMulFIPS_AB_(C, ECCP_WORDLEN, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);                //A=C^2  -- E^2 -- G	  
	MontMulFIPS_AB_(A, wordLen, C, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, B, &wordLen);                     //B=C^3  -- E^3 -- H	   
	MontMulFIPS_AB_(Pin->x, ECCP_WORDLEN, A, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, E, &wordLen);           //E=x1*A -- x1*G - I	     
	MontMulFIPS_AB_(D, ECCP_WORDLEN, D, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);                //A=D^2  -- F^2 	   

	ECCP_ModMul_two_(E, C);                                                                              //C=2E   -- 2I
	ECCP_ModAdd_(C, B, C);                                                                               //C=2I+H
	ECCP_ModSub_(A, C, Pout->x);                                                                         //A=F^2-(H+2I)

	ECCP_ModSub_(E, Pout->x, E);                                                                         //E=E-x3 -- I-x3
	MontMulFIPS_AB_(D, ECCP_WORDLEN, E, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, A, &wordLen);                //A=F*(I-x3) 	    
	MontMulFIPS_AB_(B, ECCP_WORDLEN, Pin->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, C, &wordLen);           //C=H*y1 	      
	ECCP_ModSub_(A, C, Pout->y);

end:
	if(NULL != A)
		free(A);

	if(NULL != B)
		free(B);

	if(NULL != C)
		free(C);

	if(NULL != D)
		free(D);

	if(NULL != E)
		free(E);
}

/* Function:  ECC GF(p) Pout = Pin + Pin2
 * Parameters:
 *     Pin --------- input, affine point
 *     Pin2 -------- input, affine point
 *     Pout -------- output, affine point
 * Return: 0(all OK); 1(Q is O)
 * Caution:
 *     1. Pin, Pin2 and Pout can be the same
 *     2. make sure Pin, Pin2 in the curve E(GF(p))
 */
U8 ECCP_PointAdd(ECCP_POINT *Pin, ECCP_POINT *Pin2, ECCP_POINT *Pout)
{
	U8 i;
	U32 *xx, *yy, *zz;
	ECCP_JACOBIPOINT Jcb_Point;

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	ECCP_Point2JacobiPoint(Pin, &Jcb_Point); 
	ECCP_Jacobi_Affine_Point_Add(&Jcb_Point, Pin2, &Jcb_Point);
	if(ECCP_JacobiPoint2Point(&Jcb_Point, Pout))   //if Jcb_Point is O
		i=1;
	else
		i=0;

	free(xx);
	free(yy);
	free(zz);
	return i;
}

/* Function:  ECC GF(p) test point P in or not in the curve
 * Parameters:
 *     P ----------- input, affine point
 * Return: 1(in the curve); 0(not in the curve)
 * Caution:
 */
U8 ECCP_TestPoint(ECCP_POINT *P)
{
	U32 wordLen;
	U32 *tmp, *tmp2;

	tmp = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	tmp2= (U32 *)malloc((ECCP_WORDLEN+1)*4);

	Big_ModMul_2(P->x, ECCP_WORDLEN, P->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, tmp, &wordLen);         //tmp=x^2 
	Big_ModMul_2(tmp, wordLen, P->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, tmp2, &wordLen);              //tmp2=x^3
	Big_ModMul_2(ECCP_a, ECCP_WORDLEN, P->x, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, tmp, &wordLen);       //tmp=a*x
	ECCP_ModAdd_(tmp, tmp2, tmp);                                                                      //tmp=a*x+x^3
	ECCP_ModAdd_(tmp, ECCP_b, tmp);                                                                    //tmp=a*x+x^3+b
	Big_ModMul_2(P->y, ECCP_WORDLEN, P->y, ECCP_WORDLEN, ECCP_p, ECCP_WORDLEN, tmp2, &wordLen);        //tmp2=y^2

	wordLen = Big_Compare(tmp, ECCP_WORDLEN, tmp2, ECCP_WORDLEN);
	free(tmp);
	free(tmp2);

	if((U8)wordLen == 0)
		return 1;
	else
		return 0;
}

/* Function:  ECC GF(p) Q = k*P (binary method)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     P ----------- input, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(Q is 0)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure k is neither 0 nor multiple of n
 *     3. make sure P in the curve E(GF(p))
 */
U8 ECCP_PointMul_Bin(U32 *k, U32 kWordLen, ECCP_POINT *P, ECCP_POINT *Q)
{
	U32 i;
	U32 *xx, *yy, *zz;
	ECCP_JACOBIPOINT Jcb_Point;
	
	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	ECCP_Point2JacobiPoint(P, &Jcb_Point);  
	i = Get_BitLen(k, kWordLen);
	while(--i)
	{
#if 0
		ECCP_JacobiPoint_Double_m(&Jcb_Point, 1, &Jcb_Point);
#else
		ECCP_JacobiPoint_Double(&Jcb_Point, &Jcb_Point);
#endif
		if(Get_BitValue(k, i))
		{
			ECCP_Jacobi_Affine_Point_Add(&Jcb_Point, P, &Jcb_Point);
		}
	}

	if(ECCP_JacobiPoint2Point(&Jcb_Point, Q))   //if Jcb_Point is O
		i=1;
	else
		i=0;

	free(xx);
	free(yy);
	free(zz);
	return (U8)i;
}

/* Function:  ECC GF(p) Q = k*P (binary method) (internal Residue domain)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     P ----------- input, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(Q is 0)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure k is neither 0 nor multiple of n
 *     3. make sure P in the curve E(GF(p))
 */
U8 ECCP_PointMul_Bin_Residue(U32 *k, U32 kWordLen, ECCP_POINT *P, ECCP_POINT *Q)
{
	U32 i;
	U32 *xx, *yy, *zz, *x1, *y1;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT tmp_P;
	
	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	x1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	y1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	tmp_P.x = x1;
	tmp_P.y = y1;

	//ECCP_Point2JacobiPoint(P, &Jcb_Point);  
	ECCP_a2aR(P->x, Jcb_Point.x);
	ECCP_a2aR(P->y, Jcb_Point.y);
	U32_Copy(Jcb_Point.z, ECCP_R, ECCP_WORDLEN);

	ECCP_a2aR(P->x, tmp_P.x);
	ECCP_a2aR(P->y, tmp_P.y);

	i = Get_BitLen(k, kWordLen);
	while(--i)
	{
#if 0
		ECCP_JacobiPoint_Double_m_Residue(&Jcb_Point, 1, &Jcb_Point);
#else
		ECCP_JacobiPoint_Double_Residue(&Jcb_Point, &Jcb_Point);
#endif
		if(Get_BitValue(k, i))
		{
			ECCP_Jacobi_Affine_Point_Add_Residue(&Jcb_Point, &tmp_P, &Jcb_Point);
		}
	}

	if(ECCP_JacobiPoint2Point_Residue(&Jcb_Point, Q))   //if Jcb_Point is O
	{
		i=1;
	}
	else
	{
		i=0;
		ECCP_aR2a(Q->x, Q->x);
		ECCP_aR2a(Q->y, Q->y);
	}

	free(xx);
	free(yy);
	free(zz);
	free(x1);
	free(y1);

	return (U8)i;
}

/* Function:  ECC GF(p) Q = k*P (fixed window method)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     w ----------- bit length of the fixed window
 *     P ----------- input, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(Q is O)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure k is neither 0 nor multiple of n
 *     3. make sure P in the curve E(GF(p))
 */
U8 ECCP_PointMul_W(U32 *k, U32 kWordLen, U8 w, ECCP_POINT *P, ECCP_POINT *Q)
{
	U32 i,j,m,len;
	U32 *xx, *yy, *zz;
	U32 *iP_buf;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT *iP;

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	/////////////////////////////////////
	len = (2<<(w-1))-1;                   //need so many pre-calculated points
	iP = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));
	iP[0].x = P->x;
	iP[0].y = P->y;

	iP_buf = (U32 *)malloc(((len-1)*ECCP_WORDLEN*2)*4);
	for(i=1; i<len; i++)
	{
		iP[i].x = iP_buf+ECCP_WORDLEN*(i-1)*2;
		iP[i].y = iP[i].x+ECCP_WORDLEN;
	}

	//2P
	ECCP_Point2JacobiPoint(P, &Jcb_Point); 
	ECCP_JacobiPoint_Double(&Jcb_Point, &Jcb_Point);
	ECCP_JacobiPoint2Point(&Jcb_Point, &iP[1]);

	//other points
	for(i=2; i<len; i++)
	{
		ECCP_PointAdd(&iP[0], &iP[i-1], &iP[i]);	//P+iP=(i+1)P
	}
	//////////////////////////////////

	i = Get_BitLen(k, kWordLen);

	m=0;
	for(j=0;j<w&&i>0;j++)
	{
		m <<= 1;
		m |= Get_BitValue(k, i);
		i--;
	}
	ECCP_Point2JacobiPoint(&iP[m-1], &Jcb_Point); 

	while(i>0)
	{
		m=0;
		for(j=0;j<w&&i>0;j++)
		{
			m <<= 1;
			m |= Get_BitValue(k, i);
			i--;
		}

		ECCP_JacobiPoint_Double_m(&Jcb_Point, j, &Jcb_Point);
		if(m)
		{
			ECCP_Jacobi_Affine_Point_Add(&Jcb_Point, &iP[m-1], &Jcb_Point);
		}
	}

	if(ECCP_JacobiPoint2Point(&Jcb_Point, Q))   //if Jcb_Point is O
		i=1;
	else
		i=0;

	free(iP);
	free(iP_buf);
	free(xx);
	free(yy);
	free(zz);

	return (U8)i;
}

/* Function:  ECC GF(p) Q = k*P (fixed window method) (internal Residue domain)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     w ----------- bit length of the fixed window
 *     P ----------- input, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(Q is O)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure k is neither 0 nor multiple of n
 *     3. make sure P in the curve E(GF(p))
 */
U8 ECCP_PointMul_W_Residue(U32 *k, U32 kWordLen, U8 w, ECCP_POINT *P, ECCP_POINT *Q)
{
	U32 i,j,m,len;
	U32 *xx, *yy, *zz, *x1, *y1, *z1;
	U32 *iP_buf;
	ECCP_JACOBIPOINT Jcb_Point,Jcb_Point2;
	ECCP_POINT *iP;

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4);// allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	x1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	y1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	z1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point2.x = x1;
	Jcb_Point2.y = y1;
	Jcb_Point2.z = z1;

	/////////////////////////////////////
	len = (2<<(w-1))-1;                   //need so many pre-calculated points
	iP = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));
	iP_buf = (U32 *)malloc((len*ECCP_WORDLEN*2));
	for(i=0; i<len; i++)
	{
		iP[i].x = iP_buf+ECCP_WORDLEN*i*2;
		iP[i].y = iP[i].x+ECCP_WORDLEN;
	}

	//1P
	ECCP_a2aR(P->x, iP[0].x);
	ECCP_a2aR(P->y, iP[0].y);

	//2P
	ECCP_Point2JacobiPoint_Residue(&iP[0], &Jcb_Point); 
	ECCP_JacobiPoint_Double_Residue(&Jcb_Point, &Jcb_Point);
	ECCP_JacobiPoint2Point_Residue(&Jcb_Point, &iP[1]);

	//other points
	ECCP_Point2JacobiPoint_Residue(&iP[0], &Jcb_Point);
	for(i=2; i<len; i++)
	{
		ECCP_Jacobi_Affine_Point_Add_Residue(&Jcb_Point, &iP[i-1], &Jcb_Point2);	//P+iP=(i+1)P		
		ECCP_JacobiPoint2Point_Residue(&Jcb_Point2, &iP[i]); 
	}
	//////////////////////////////////

	i = Get_BitLen(k, kWordLen);

	m=0;
	for(j=0;j<w&&i>0;j++)
	{
		m <<= 1;
		m |= Get_BitValue(k, i);
		i--;
	}
	ECCP_Point2JacobiPoint_Residue(&iP[m-1], &Jcb_Point); 

	while(i>0)
	{
		m=0;
		for(j=0;j<w&&i>0;j++)
		{
			m <<= 1;
			m |= Get_BitValue(k, i);
			i--;
		}

		ECCP_JacobiPoint_Double_m_Residue(&Jcb_Point, j, &Jcb_Point);
		if(m)
		{
			ECCP_Jacobi_Affine_Point_Add_Residue(&Jcb_Point, &iP[m-1], &Jcb_Point);
		}
	}
	
	if(ECCP_JacobiPoint2Point_Residue(&Jcb_Point, Q))   //if Jcb_Point is O
	{
		i=1;
	}
	else
	{
		i=0;
		ECCP_aR2a(Q->x, Q->x);
		ECCP_aR2a(Q->y, Q->y);
	}

	free(iP);
	free(iP_buf);
	free(xx);
	free(yy);
	free(zz);
	free(x1);
	free(y1);
	free(z1);
	
	return (U8)i;
}

/* Function:  ECC GF(p) Q = k*P (slide window method)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     w ----------- max bit length of the slide window
 *     P ----------- input, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(error)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure k is neither 0 nor multiple of n
 *     3. make sure P in the curve E(GF(p))
 */
U8 ECCP_PointMul_SW(U32 *k, U32 kWordLen, U8 w, ECCP_POINT *P, ECCP_POINT *Q)
{
	U32 i,j,len,m;
	U32 value, index;
	U32 *xx, *yy, *zz, *x1, *y1;
	U32 *iP_buf;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT *iP, _2P_Point;

	if(w<2)
		return 1;

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4);  // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	x1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	y1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	_2P_Point.x = x1;
	_2P_Point.y = y1;

	/////////////////////////////////////
	len = (1<<(w-1));                   //need so many pre-calculated points
	iP = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));
	iP[0].x = P->x;
	iP[0].y = P->y;

	iP_buf = (U32 *)malloc(((len-1)*ECCP_WORDLEN*2)*4);
	for(i=1; i<len; i++)
	{
		iP[i].x = iP_buf+ECCP_WORDLEN*(i-1)*2;
		iP[i].y = iP[i].x+ECCP_WORDLEN;
	}

	//2P
	ECCP_Point2JacobiPoint(P, &Jcb_Point); 
	ECCP_JacobiPoint_Double(&Jcb_Point, &Jcb_Point);
	ECCP_JacobiPoint2Point(&Jcb_Point, &_2P_Point);

	//other points
	ECCP_Point2JacobiPoint(P, &Jcb_Point); 
	for(i=1; i<len; i++)
	{
		ECCP_Jacobi_Affine_Point_Add(&Jcb_Point, &_2P_Point, &Jcb_Point); //P+2P=3P,3P+2P=5P,5P+2P=7P,...
		ECCP_JacobiPoint2Point(&Jcb_Point, &iP[i]); 
	}
	//////////////////////////////////

	i = Get_BitLen(k, kWordLen);
	ECCP_Point2JacobiPoint(P, &Jcb_Point); 
	i--;
	m=0;

	while(i)
	{
		value = 0; // window value
		index = 0; // number of bits in the window
		while(i>0)
		{
			if(0 == Get_BitValue(k, i))
			{
				i--;
				m++;  // count consecutive zero bits
			}
			else
			{
				i--;
				value=1;
				for(index=1; index<w && i>0; index++) // read up to (w-1) more bits into the window
				{
					value <<=1;
					value |= Get_BitValue(k, i);
					i--;
				}
				break;
			}
		}
	
		j=0;    // count trailing zero bits of value
		if(value)
		{
			while(0 == (value&1))
			{
				j++;
				value = value>>1;
			}
		}

		// double the accumulator m+(index-j) times
		ECCP_JacobiPoint_Double_m(&Jcb_Point, m+(index-j), &Jcb_Point);

		// if value is nonzero, add the precomputed point for value
		if(value)
			ECCP_Jacobi_Affine_Point_Add(&Jcb_Point, &iP[value>>1], &Jcb_Point); 

		// at the start of the next loop scan, the preceding m bits have already been scanned and are all 0
		m=j;
	}

	if(m>1)
		ECCP_JacobiPoint_Double_m(&Jcb_Point, m, &Jcb_Point);
	else if(1==m)
		ECCP_JacobiPoint_Double(&Jcb_Point, &Jcb_Point);

	if(ECCP_JacobiPoint2Point(&Jcb_Point, Q))   //if Jcb_Point is O
		i=1;
	else
		i=0;

	free(xx);
	free(yy);
	free(zz);
	free(x1);
	free(y1);
	free(iP);
	free(iP_buf);

	return (U8)i;
}

/* Function:  ECC GF(p) Q = k*P (slide window method) (internal Residue domain)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     w ----------- max bit length of the slide window
 *     P ----------- input, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(error)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure k is neither 0 nor multiple of n
 *     3. make sure P in the curve E(GF(p))
 */
U8 ECCP_PointMul_SW_Residue(U32 *k, U32 kWordLen, U8 w, ECCP_POINT *P, ECCP_POINT *Q)
{
	U32 i,j,len,m;
	U32 value, index;
	U32 *xx, *yy, *zz, *x1, *y1;
	U32 *iP_buf;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT *iP, _2P_Point;

	if(w<2)
		return 1;

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4);  // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	x1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	y1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	_2P_Point.x = x1;
	_2P_Point.y = y1;

	/////////////////////////////////////
	len = (1<<(w-1));                   //need so many pre-calculated points
	iP = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));
	iP_buf = (U32 *)malloc((len*ECCP_WORDLEN*2)*4);
	for(i=0; i<len; i++)
	{
		iP[i].x = iP_buf+ECCP_WORDLEN*i*2;
		iP[i].y = iP[i].x+ECCP_WORDLEN;
	}

	//1P
	ECCP_a2aR(P->x, iP[0].x);
	ECCP_a2aR(P->y, iP[0].y);

	//2P
	ECCP_Point2JacobiPoint_Residue(&iP[0], &Jcb_Point);
	ECCP_JacobiPoint_Double_Residue(&Jcb_Point, &Jcb_Point);
	ECCP_JacobiPoint2Point_Residue(&Jcb_Point, &_2P_Point);

	//other points
	ECCP_Point2JacobiPoint_Residue(&iP[0], &Jcb_Point); 
	for(i=1; i<len; i++)
	{
		ECCP_Jacobi_Affine_Point_Add_Residue(&Jcb_Point, &_2P_Point, &Jcb_Point); //P+2P=3P,3P+2P=5P,5P+2P=7P,...
		ECCP_JacobiPoint2Point_Residue(&Jcb_Point, &iP[i]); 
	}
	//////////////////////////////////

	i = Get_BitLen(k, kWordLen);
	ECCP_Point2JacobiPoint_Residue(&iP[0], &Jcb_Point); 
	i--;
	m=0;

	while(i)
	{
		value = 0; // window value
		index = 0; // number of bits in the window
		while(i>0)
		{
			if(0 == Get_BitValue(k, i))
			{
				i--;
				m++;  // count consecutive zero bits
			}
			else
			{
				i--;
				value=1;
				for(index=1; index<w && i>0; index++) // read up to (w-1) more bits into the window
				{
					value <<=1;
					value |= Get_BitValue(k, i);
					i--;
				}
				break;
			}
		}
	
		j=0;    // count trailing zero bits of value
		if(value)
		{
			while(0 == (value&1))
			{
				j++;
				value = value>>1;
			}
		}

		// double the accumulator m+(index-j) times
		ECCP_JacobiPoint_Double_m_Residue(&Jcb_Point, m+(index-j), &Jcb_Point);

		// if value is nonzero, add the precomputed point for value
		if(value)
			ECCP_Jacobi_Affine_Point_Add_Residue(&Jcb_Point, &iP[value>>1], &Jcb_Point); 

		// at the start of the next loop scan, the preceding m bits have already been scanned and are all 0
		m=j;
	}

	if(m>1)
		ECCP_JacobiPoint_Double_m_Residue(&Jcb_Point, m, &Jcb_Point);
	else if(1==m)
		ECCP_JacobiPoint_Double_Residue(&Jcb_Point, &Jcb_Point);

	if(ECCP_JacobiPoint2Point_Residue(&Jcb_Point, Q))   //if Jcb_Point is O
	{
		i=1;
	}
	else
	{
		i=0;
		ECCP_aR2a(Q->x, Q->x);
		ECCP_aR2a(Q->y, Q->y);
	}

	free(xx);
	free(yy);
	free(zz);
	free(x1);
	free(y1);
	free(iP);
	free(iP_buf);

	return (U8)i;
}

/* Function:  get a mod 2^w, here -2^(w-1)<a<2^(w-1) (for NAF method)
 * Parameters:
 *     a ----------- integer a
 *     w ----------- bit length of windows for NAF method
 * Return: a mod 2^w, and -2^(w-1)<a<2^(w-1)
 * Caution:
 *     1. make sure w not too big
 */
S32 kMod2w(U32 a, U32 w)
{
	a = a & ((1<<w)-1);
	if(a > (U32)(1<<(w-1)))
		return ((S32)a) - (1<<w);

	return (S32)a;
}

/* Function:  get NAF of big integer k (for NAF method)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     w ----------- bit length of windows for NAF method
 *     ki ---------- output, buffer of NAF digits
 * Return: number of NAF digits
 * Caution:
 *     1. make sure kWordLen is actual, not bigger or smaller
 *     2. make sure w not too big
 *     3. make sure buffer ki sufficient
 */
U32 ECCP_GetNAF(U32 *k, U32 kWordLen, U32 w, NAFKI *ki)
{
	S32 i = 0, j = 0;  // i scans bits from 0; j indexes the NAF buffer
	U32 temp;
	U32 *data;

	data = (U32 *)malloc((kWordLen+1)*4); // working copy of k (one extra word)
	U32_Copy(data, k, kWordLen);

	while(kWordLen)
	{
		if(data[0] & 1)
		{
			ki[j].value = kMod2w(data[0], w);
			ki[j].index = i;

			if(ki[j].value > 0)
			{
				kWordLen = Big_Sub(data, kWordLen, (U32 *)(&ki[j].value), 1);
			}
			else
			{
				temp = (U32)(-ki[j].value);   
				kWordLen = Big_Add(data, kWordLen, &temp, 1);
			}

			j++;
			i = i + w;
			kWordLen = Big_Div2n(data, kWordLen, w);
		}
		else
		{
			kWordLen = Big_Div2(data, kWordLen);
			i++;
		}
	}

	free(data);

	return j;
}

/* Function:  get pre-caculated points (for NAF method)
 * Parameters:
  *     len --------- the number of words to preallocate for the storage area
 *     iP  --------- output, buffer for the precomputed points iP
 *     DiP --------- output, buffer for the precomputed points -iP
 * Return: 
 * Caution:
 *     1. make sure buffers sufficient
 */
void ECCP_GetNAF_prePoints(U32 len, ECCP_POINT *iP, ECCP_POINT *DiP)
{
	U32 i;
	U32 *xx, *yy, *zz, *x1, *y1;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT _2P_Point;

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4); // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	x1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	y1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	_2P_Point.x = x1;
	_2P_Point.y = y1;

	//2P
	ECCP_Point2JacobiPoint(&iP[0], &Jcb_Point); 
	ECCP_JacobiPoint_Double(&Jcb_Point, &Jcb_Point);
	ECCP_JacobiPoint2Point(&Jcb_Point, &_2P_Point);

	//-P
	U32_Copy(DiP[0].y, iP[0].y, ECCP_WORDLEN);
	Big_Subb(ECCP_p, ECCP_WORDLEN, DiP[0].y, ECCP_WORDLEN);

	//other points
	ECCP_Point2JacobiPoint(&iP[0], &Jcb_Point); 
	for(i=1; i<len; i++)
	{
		//3P=2P+P,5P=2P+3P,7P=2P+5P, ...
		ECCP_Jacobi_Affine_Point_Add(&Jcb_Point, &_2P_Point, &Jcb_Point);
		ECCP_JacobiPoint2Point(&Jcb_Point, &iP[i]);

		// compute the next precomputed odd multiple
		U32_Copy(DiP[i].y, ECCP_p, ECCP_WORDLEN);
		Big_Sub(DiP[i].y, ECCP_WORDLEN, iP[i].y, ECCP_WORDLEN); 
	}

	free(xx);
	free(yy);
	free(zz);
	free(x1);
	free(y1);
}

/* Function:  get pre-caculated points (for NAF method) (internal Residue domain)
 * Parameters:
  *     len --------- the number of words to preallocate for the storage area
 *     iP  --------- output, buffer for the precomputed points iP
 *     DiP --------- output, buffer for the precomputed points -iP
 * Return: 
 * Caution:
 *     1. make sure buffers sufficient
 */
void ECCP_GetNAF_prePoints_Residue(U32 len, ECCP_POINT *iP, ECCP_POINT *DiP)
{
	U32 i;
	U32 *xx, *yy, *zz, *x1, *y1;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT _2P_Point;

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4); // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	x1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	y1 = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	_2P_Point.x = x1;
	_2P_Point.y = y1;

	//2P
	ECCP_Point2JacobiPoint_Residue(&iP[0], &Jcb_Point); 
	ECCP_JacobiPoint_Double_Residue(&Jcb_Point, &Jcb_Point);
	ECCP_JacobiPoint2Point_Residue(&Jcb_Point, &_2P_Point);

	//-P
	U32_Copy(DiP[0].y, iP[0].y, ECCP_WORDLEN);
	Big_Subb(ECCP_p, ECCP_WORDLEN, DiP[0].y, ECCP_WORDLEN);

	//other points
	ECCP_Point2JacobiPoint_Residue(&iP[0], &Jcb_Point);
	for(i=1; i<len; i++)
	{
		//3P=2P+P,5P=2P+3P,7P=2P+5P, ...
		ECCP_Jacobi_Affine_Point_Add_Residue(&Jcb_Point, &_2P_Point, &Jcb_Point);
		ECCP_JacobiPoint2Point_Residue(&Jcb_Point, &iP[i]);

		// compute the next precomputed odd multiple
		U32_Copy(DiP[i].y, ECCP_p, ECCP_WORDLEN);
		Big_Sub(DiP[i].y, ECCP_WORDLEN, iP[i].y, ECCP_WORDLEN); 
	}

	free(xx);
	free(yy);
	free(zz);
	free(x1);
	free(y1);
}

/* Function:  ECC GF(p) Q = k*P (fixed window NAF method)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     w ----------- bit length of the fixed window
 *     P ----------- input, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(error)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure k is neither 0 nor multiple of n
 *     3. make sure w not too big(but bigger than 1)
 *     4. make sure P in the curve E(GF(p))
 */
U8 ECCP_PointMul_NAF_W(U32 *k, U32 kWordLen, U8 w, ECCP_POINT *P, ECCP_POINT *Q)
{
	U32 i,j,len,t,bitLen;
	U32 *xx, *yy, *zz;
	U32 *iP_buf, *DiP_buf;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT *iP,*DiP;
	NAFKI *naf_ki;
	
	if(w<2)
		return 1;

	kWordLen = Get_WordLen(k, kWordLen);
	bitLen = Get_BitLen(k, kWordLen);

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4); // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	// max number of width-w NAF digits: (bitLen + w-1)/w + 1
	len = (bitLen+(w-1))/w + 1;  

	naf_ki = (NAFKI *)malloc((sizeof(NAFKI)*len));
	memset(naf_ki, 0, sizeof(NAFKI)*len);
	
	len = (1<<(w-2));                         // number of precomputed odd multiples: 2^(w-2)
	iP = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));
	DiP = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));

	iP_buf = (U32 *)malloc(((len-1)*2*ECCP_WORDLEN)*4);  // storage for iP[1..len-1] (x and y each)
	DiP_buf = (U32 *)malloc((len*ECCP_WORDLEN)*4);       // storage for the DiP y-coordinates

	iP[0].x = P->x;
	iP[0].y = P->y;
	DiP[0].x = iP[0].x;
	DiP[0].y = DiP_buf;
	for(i=1; i<len; i++)
	{
		iP[i].x = iP_buf+(i-1)*2*ECCP_WORDLEN;
		iP[i].y = iP[i].x+ECCP_WORDLEN;
		DiP[i].x = iP[i].x;
		DiP[i].y = DiP_buf+(i*ECCP_WORDLEN);
	}

	ECCP_GetNAF_prePoints(len, iP, DiP);             // precompute the odd multiples iP and their negations DiP

	j = ECCP_GetNAF(k, kWordLen, w, naf_ki);         // compute the width-w NAF of k
	t = naf_ki[j-1].value;                           // most significant NAF digit
	ECCP_Point2JacobiPoint(&iP[t>>1], &Jcb_Point);   // initialize the accumulator from the top digit's point

	j--;
	while(j>0)
	{
		ECCP_JacobiPoint_Double_m(&Jcb_Point, naf_ki[j].index - naf_ki[j-1].index, &Jcb_Point);

		if(naf_ki[j-1].value > 0)
		{
			ECCP_Jacobi_Affine_Point_Add(&Jcb_Point, &iP[(naf_ki[j-1].value)>>1], &Jcb_Point);
		}
		else
		{
			ECCP_Jacobi_Affine_Point_Add(&Jcb_Point, &DiP[(~(naf_ki[j-1].value))>>1], &Jcb_Point);   // negative NAF digit: (~value)>>1 selects the -iP point
		}
		j--;	
	}

	if(naf_ki[0].index > 1)
	{
		ECCP_JacobiPoint_Double_m(&Jcb_Point, naf_ki[0].index, &Jcb_Point);
	}
	else if(naf_ki[0].index == 1)
	{
		ECCP_JacobiPoint_Double(&Jcb_Point, &Jcb_Point);
	}

	if(ECCP_JacobiPoint2Point(&Jcb_Point, Q))   //if Jcb_Point is O
		i=1;
	else
		i=0;

	free(iP_buf);
	free(DiP_buf);
	free(iP);
	free(DiP);
	free(naf_ki);
	free(xx);
	free(yy);
	free(zz);

	return (U8)i;
}

/* Function:  ECC GF(p) Q = k*P (fixed window NAF method) (internal Residue domain)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     w ----------- bit length of the fixed window
 *     P ----------- input, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(error)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure k is neither 0 nor multiple of n
 *     3. make sure w not too big(but bigger than 1)
 *     4. make sure P in the curve E(GF(p))
 */
U8 ECCP_PointMul_NAF_W_Residue(U32 *k, U32 kWordLen, U8 w, ECCP_POINT *P, ECCP_POINT *Q)
{
	U32 i,j,len,t,bitLen;
	U32 *xx, *yy, *zz;
	U32 *iP_buf, *DiP_buf;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT *iP,*DiP;
	NAFKI *naf_ki;
	
	if(w<2)
		return 1;

	kWordLen = Get_WordLen(k, kWordLen);
	bitLen = Get_BitLen(k, kWordLen);

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4); // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz;

	// max number of width-w NAF digits: (bitLen + w-1)/w + 1
	len = (bitLen+(w-1))/w + 1;  

	naf_ki = (NAFKI *)malloc((sizeof(NAFKI)*len));
	memset(naf_ki, 0, sizeof(NAFKI)*len);
	
	len = (1<<(w-2));                         // number of precomputed odd multiples: 2^(w-2)
	iP = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));
	DiP = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));

	iP_buf = (U32 *)malloc((len*2*ECCP_WORDLEN)*4);      // storage for the iP points (x and y each)
	DiP_buf = (U32 *)malloc((len*ECCP_WORDLEN)*4);       // storage for the DiP y-coordinates

	for(i=0; i<len; i++)
	{
		iP[i].x = iP_buf+(i)*2*ECCP_WORDLEN;
		iP[i].y = iP[i].x+ECCP_WORDLEN;
		DiP[i].x = iP[i].x;
		DiP[i].y = DiP_buf+(i*ECCP_WORDLEN);
	}

	ECCP_a2aR(P->x, iP[0].x);
	ECCP_a2aR(P->y, iP[0].y);
	ECCP_GetNAF_prePoints_Residue(len, iP, DiP);             // precompute the odd multiples iP and their negations DiP

	j = ECCP_GetNAF(k, kWordLen, w, naf_ki);                 // compute the width-w NAF of k
	t = naf_ki[j-1].value;                                   // most significant NAF digit
	ECCP_Point2JacobiPoint_Residue(&iP[t>>1], &Jcb_Point);   // initialize the accumulator from the top digit's point

	j--;
	while(j>0)
	{
		ECCP_JacobiPoint_Double_m_Residue(&Jcb_Point, naf_ki[j].index - naf_ki[j-1].index, &Jcb_Point);

		if(naf_ki[j-1].value > 0)
		{
			ECCP_Jacobi_Affine_Point_Add_Residue(&Jcb_Point, &iP[(naf_ki[j-1].value)>>1], &Jcb_Point);
		}
		else
		{
			ECCP_Jacobi_Affine_Point_Add_Residue(&Jcb_Point, &DiP[(~(naf_ki[j-1].value))>>1], &Jcb_Point);   // negative NAF digit: (~value)>>1 selects the -iP point
		}
		j--;	
	}

	if(naf_ki[0].index > 1)
	{
		ECCP_JacobiPoint_Double_m_Residue(&Jcb_Point, naf_ki[0].index, &Jcb_Point);
	}
	else if(naf_ki[0].index == 1)
	{
		ECCP_JacobiPoint_Double_Residue(&Jcb_Point, &Jcb_Point);
	}

	if(ECCP_JacobiPoint2Point_Residue(&Jcb_Point, Q))   //if Jcb_Point is O
	{
		i=1;
	}
	else
	{
		i=0;
		ECCP_aR2a(Q->x, Q->x);
		ECCP_aR2a(Q->y, Q->y);
	}

	free(iP_buf);
	free(DiP_buf);
	free(iP);
	free(DiP);
	free(naf_ki);
	free(xx);
	free(yy);
	free(zz);

	return (U8)i;
}

/* Function:  get bit value of U32 big integer for COMB method
 * Parameters:
 *     a ----------- U32 big integer
 *     aBitLen ----- bit length of a
 *     bitLen ------ bit location, begins from 1.
 * Return: 0(the bit value is 0); 1(the bit value is 1)
 * Caution:
 */
U8 ECCP_Comb_Get_BitValue(U32 a[], U32 aBitLen, U32 bitLen)
{
	if(bitLen > ECCP_N_BITLEN)
		return 0;
	else if(bitLen > aBitLen)
		return 0;
	else
		return Get_BitValue(a, bitLen);
}

/* Function:  ECC GF(p) Q = k*G (COMB method)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length of k
 *     n ----------- divide k into n parts bit string
 *     iG ---------- Comb method requires pre-caculated points, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(error)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure P in the curve E(GF(p))
 *     3. if k>ECCP_n, then k = k mod ECCP_n
 */
U8 ECCP_PointMul_Comb(U32 *k, U32 kWordLen, U8 n, U32 *iG, ECCP_POINT *Q)
{
	U32 i,j,len,index,kBitLen;
	U32 *xx, *yy, *zz;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT *P;

	if(Bigint_Check_NULL(k, kWordLen))    //k is 0 
		return 1;

	//get k = k mod n
	Big_Mod_1(k, kWordLen, ECCP_n, ECCP_N_WORDLEN, k, &kWordLen); 

	if(kWordLen==0)                       //k is a multiple of ECCP_n
		return 1;

	kBitLen = Get_BitLen(k, kWordLen);    //get kBitLen

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4);  // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz; 
	
	len = (2<<(n-1))-1;                   //need so many pre-calculated points
	P = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));

	P[0].x = ECCP_Gx;
	P[0].y = ECCP_Gy;
	for(i=1; i<len; i++)
	{
		P[i].x = iG+ECCP_WORDLEN*(i-1)*2;
		P[i].y = iG+ECCP_WORDLEN*(i-1)*2+ECCP_WORDLEN;
	}

	len = (ECCP_N_BITLEN+(n-1))/n;        //divide k into n parts, each part len bits

	for(i=0;i<len;i++)
	{
		index = 0;
		for(j=0;j<n;j++)
		{
			index <<=1;
			index |= ECCP_Comb_Get_BitValue(k, kBitLen, (n-j)*len-i);		
		}

		if(index)
		{
			break;
		}
	}

	ECCP_Point2JacobiPoint(&P[index-1], &Jcb_Point); 

	i++;
	for(;i<len;i++)
	{
		ECCP_JacobiPoint_Double(&Jcb_Point, &Jcb_Point);

		index = 0;
		for(j=0;j<n;j++)
		{
			index <<=1;
			index |= ECCP_Comb_Get_BitValue(k, kBitLen, (n-j)*len-i);		
		}

		if(index)
		{
			ECCP_Jacobi_Affine_Point_Add(&Jcb_Point, &P[index-1], &Jcb_Point);
		}
	}

	if(ECCP_JacobiPoint2Point(&Jcb_Point, Q))   //if Jcb_Point is O
		i=1;
	else
		i=0;

	free(P);
	free(xx);
	free(yy);
	free(zz);

	return (U8)i;
}

/* Function:  ECC GF(p) Q = k*G (COMB method) (internal Residue domain)
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length ok k
 *     n ----------- divide k into n parts bit string
 *     iG ---------- Comb method requires pre-caculated points, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(error)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure P in the curve E(GF(p))
 *     3. if k>ECCP_n, then k = k mod ECCP_n
 */
U8 ECCP_PointMul_Comb_Residue(U32 *k, U32 kWordLen, U8 n, U32 *iGR, ECCP_POINT *Q)
{
	U32 i,j,len,index,kBitLen;
	U32 *xx, *yy, *zz;
	ECCP_JACOBIPOINT Jcb_Point;
	ECCP_POINT *P;

	if(Bigint_Check_NULL(k, kWordLen))    //k is 0 
		return 1;

	//get k = k mod n
	Big_Mod_1(k, kWordLen, ECCP_n, ECCP_N_WORDLEN, k, &kWordLen); 

	if(kWordLen==0)                       //k is a multiple of ECCP_n
		return 1;

	kBitLen = Get_BitLen(k, kWordLen);    //get kBitLen

	xx = (U32 *)malloc((ECCP_WORDLEN+1)*4);  // allocate ECCP_WORDLEN+1 words (extra room for intermediate results)
	yy = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	zz = (U32 *)malloc((ECCP_WORDLEN+1)*4);
	Jcb_Point.x = xx;
	Jcb_Point.y = yy;
	Jcb_Point.z = zz; 
	
	len = (2<<(n-1))-1;                   //need so many pre-calculated points
	P = (ECCP_POINT *)malloc((sizeof(ECCP_POINT)*len));

	for(i=0; i<len; i++)
	{
		P[i].x = iGR+ECCP_WORDLEN*i*2;
		P[i].y = iGR+ECCP_WORDLEN*i*2+ECCP_WORDLEN;
	}

	len = (ECCP_N_BITLEN+(n-1))/n;        //divide k into n parts, each part len bits

	for(i=0;i<len;i++)
	{
		index = 0;
		for(j=0;j<n;j++)
		{
			index <<=1;
			index |= ECCP_Comb_Get_BitValue(k, kBitLen, (n-j)*len-i);		
		}

		if(index)
		{
			break;
		}
	}

	ECCP_Point2JacobiPoint_Residue(&P[index-1], &Jcb_Point); 

	i++;
	for(;i<len;i++)
	{
		ECCP_JacobiPoint_Double_Residue(&Jcb_Point, &Jcb_Point);

		index = 0;
		for(j=0;j<n;j++)
		{
			index <<=1;
			index |= ECCP_Comb_Get_BitValue(k, kBitLen, (n-j)*len-i);		
		}

		if(index)
		{
			ECCP_Jacobi_Affine_Point_Add_Residue(&Jcb_Point, &P[index-1], &Jcb_Point);
		}
	}

	if(ECCP_JacobiPoint2Point_Residue(&Jcb_Point, Q))   //if Jcb_Point is O
	{
		i=1;
	}
	else
	{
		i=0;
		ECCP_aR2a(Q->x, Q->x);
		ECCP_aR2a(Q->y, Q->y);
	}

	free(P);
	free(xx);
	free(yy);
	free(zz);

	return (U8)i;
}

/* Function:  ECC GF(p) Q = k*P
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length of k
 *     P ----------- input, affine point
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(Q is O)
 * Caution:
 *     1. P and Q can be the same
 *     2. make sure k is neither 0 nor multiple of n
 *     3. make sure P in the curve E(GF(p))
 *     4. if k>ECCP_n, then k = k mod ECCP_n
 */
U8 ECCP_PointMul(U32 *k, U32 kWordLen, ECCP_POINT *P, ECCP_POINT *Q)
{
	if(Bigint_Check_NULL(k, kWordLen))
		return 1;                         //k is 0 

	Big_Mod_1(k, kWordLen, ECCP_n, ECCP_N_WORDLEN, k, &kWordLen); 
	if(kWordLen==0)                       //k is a multiple of ECCP_n
		return 1;

	// if k>=ECCP_n (kWordLen>=ECCP_N_WORDLEN), Big_Mod_1 reduces k in place
	// if k<ECCP_n, Big_Mod_1 leaves k unchanged (kWordLen stays reduced)

    // select the scalar-multiplication implementation
#define ECCP_PointMul_OPTION   2

#if (1==ECCP_PointMul_OPTION)

	return ECCP_PointMul_Bin(k, kWordLen, P, Q);

#elif (2==ECCP_PointMul_OPTION)

	return ECCP_PointMul_Bin_Residue(k, kWordLen, P, Q);

#elif (3==ECCP_PointMul_OPTION)

	return ECCP_PointMul_W(k, kWordLen, 4, P, Q);

#elif (4==ECCP_PointMul_OPTION)

	return ECCP_PointMul_W_Residue(k, kWordLen, 4, P, Q);

#elif (5==ECCP_PointMul_OPTION)

	return ECCP_PointMul_SW(k, kWordLen, 4, P, Q);	

#elif (6==ECCP_PointMul_OPTION)
	
	return ECCP_PointMul_SW_Residue(k, kWordLen, 4, P, Q);

#elif (7==ECCP_PointMul_OPTION)

	return ECCP_PointMul_NAF_W(k, kWordLen, 4, P, Q);
	
#elif (8==ECCP_PointMul_OPTION)

	return ECCP_PointMul_NAF_W_Residue(k, kWordLen, 4, P, Q);

#endif
}

/* Function:  ECC GF(p) Q = k*G
 * Parameters:
 *     k ----------- U32 big integer
 *     kWordLen ---- word length of k
 *     Q ----------- output, affine point
 * Return: 0(all OK); 1(Q is O)
 * Caution:
 *     1. make sure k is neither 0 nor multiple of n
 *     2. if k>ECCP_n, then k = k mod ECCP_n
 */
U8 ECCP_PointMul_G(U32 *k, U32 kWordLen, ECCP_POINT *Q)
{
	ECCP_POINT G[1];

	G->x = ECCP_Gx;
	G->y = ECCP_Gy;

	if(Bigint_Check_NULL(k, kWordLen))
		return 1;                         //k is 0 

	Big_Mod_1(k, kWordLen, ECCP_n, ECCP_N_WORDLEN, k, &kWordLen); 
	if(kWordLen==0)                       //k is a multiple of ECCP_n
		return 1;

	if(NULL != ECCP_COMB_iGR)
	{
		return ECCP_PointMul_Comb_Residue(k, kWordLen, ECCP_COMB_n, ECCP_COMB_iGR, Q); 
	}
	else if(NULL != ECCP_COMB_iG)
	{
		return ECCP_PointMul_Comb(k, kWordLen, ECCP_COMB_n, ECCP_COMB_iG, Q);
	}
	else
	{
		return ECCP_PointMul(k, kWordLen, G, Q); 
	}
}
