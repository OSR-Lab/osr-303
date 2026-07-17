#ifndef _ECCP_H_
#define _ECCP_H_

#include "Type.h"
#include "PAElib.h"
#include "ECCP_curve.h"


// define the return-value type
enum ECCP_RET_CODE
{
	ECCPSuccess = 0, 
	ECCPBufferNull,            // input buffer pointer is null
	ECCPInputLenInvalid,       // input length invalid (e.g. length is 0)
	ECCPPointHeadNot04,        // point encoding does not start with 0x04 (uncompressed form)
	ECCPPubKeyError,           // invalid public key
	ECCPNotInCurve,            // point is not on the curve
	ECCPIntegerTooBig,         // integer is out of range (too big)
	ECCPZeroALL,               // all zeros
	ECCPDecryVerifyFailed,     // decryption integrity check failed
	ECCPVerifyFailed,          // signature verification failed
	ECCPExchangeRoleInvalid,   // invalid key-exchange role
	ECCPZeroPoint,             // point at infinity (zero point)
	ECCPInOutSameBuffer        // input and output share the same buffer
};


typedef struct
{
    U32 *x; 
    U32 *y; 
}ECCP_POINT;

typedef struct
{
	U32 *x; 
	U32 *y;
	U32 *z;
}ECCP_JACOBIPOINT;

void ECCP_Init(U32 *p, U32 *a, U32 *b, U32 *Gx, U32 *Gy, U32 *n, U32 PBitLen, U32 NBitLen, U32 COMB_n, U32 *COMB_iGR, U32 *COMB_iG);

void ECCP_Init_1(ECCP_CURVE *curve);

void ECCP_Point2Char(ECCP_POINT *P, U8 *c);

void ECCP_Char2Point(U8 *c, ECCP_POINT *P);

U8 ECCP_IntegerCheck(U32 k[], U32 kWORDLEN, U32 n[], U32 nWORDLEN);

U32 ECCP_ModAdd(U32 *a, U32 aWordLen, U32 *b, U32 bWordLen, U32 *N);

U32 ECCP_ModSub(U32 *a, U32 aWordLen, U32 *b, U32 bWordLen, U32 *N);

U8 ECCP_PointMul(U32 *k, U32 kWordLen, ECCP_POINT *P, ECCP_POINT *Q);

U8 ECCP_PointMul_G(U32 *k, U32 kWordLen, ECCP_POINT *Q);

U8 ECCP_PointMul_Comb(U32 *k, U32 kWordLen, U8 n, U32 *iG, ECCP_POINT *Q);

U8 ECCP_PointMul_Comb_Residue(U32 *k, U32 kWordLen, U8 n, U32 *iGR, ECCP_POINT *Q);

U8 ECCP_PointAdd(ECCP_POINT *Pin, ECCP_POINT *Pin2, ECCP_POINT *Pout);

U8 ECCP_TestPoint(ECCP_POINT *P);

void ECCP_Close();

#endif
