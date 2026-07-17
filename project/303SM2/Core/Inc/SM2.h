#ifndef _SM2_H_
#define _SM2_H_

#include "ECCP.h"


// define the return-value type
enum SM2_RET_CODE
{
	SM2Success = 0, 
	SM2BufferNull,            // input buffer pointer is null
	SM2InputLenInvalid,       // input length invalid (e.g. length is 0)
	SM2PointHeadNot04,        // point encoding does not start with 0x04 (uncompressed form)
	SM2PubKeyError,           // invalid public key
	SM2NotInCurve,            // point is not on the curve
	SM2IntegerTooBig,         // integer is out of range (too big)
	SM2ZeroALL,               // all zeros
	SM2DecryVerifyFailed,     // decryption integrity check (C3) failed
	SM2VerifyFailed,          // signature verification failed
	SM2ExchangeRoleInvalid,   // invalid key-exchange role
	SM2ZeroPoint,             // point at infinity (zero point)
	SM2InOutSameBuffer        // input and output share the same buffer
};

// SM2 key-exchange role
enum SM2_Exchange_Role
{
	SM2_Role_Receiver = 0,
	SM2_Role_Sender
};



U8 SM2_Init();

//U8 SM2_GetKey(U8 priKey[32], U8 pubKey[65]);

U8 SM2_GetPubKey_from_PriKey(U8 priKey[32], U8 pubKey[65]);

//U8 SM2_sign_U32(U32 e[8], U32 k[8], U32 dA[8], U32 r[8], U32 s[8]);

//U8 SM2_Sign(U8 E[32], U8 priKey[32], U8 signature[64]);

//U8 SM2_Verify(U8 E[32], U8 pubKey[65], U8 signature[64]);

U8 SM2_encrypt_U32(U8 *M, U32 MByteLen, U32 k[8], U8 pubKey[65], U8 tag, U8 *C, U32 *CByteLen);

U8 SM2_Encrypt(U8 *M, U32 MByteLen, U8 pubKey[65], U8 tag, U8 *C, U32 *CByteLen);

//U8 SM2_Decrypt(U8 *C, U32 CByteLen, U8 priKey[32], U8 tag, U8 *M, U32 *MByteLen);

U8 SM2_GetZ(U8 *ID, U16 byteLenofID, U8 pubKey[65], U8 Z[32]);

U8 SM2_GetE(U8 *M, U32 byteLen, U8 Z[32], U8 E[32]);

//U8 SM2_ExchangeKey(U8 role, U8 *dA, U8 *PB, U8 *rA, U8 *RA, U8 *RB, U8 *ZA, U8 *ZB,	U32 kByteLen, U8 *KA, U8 *S1, U8 *SA);

U8 SM2_Close();

#endif
