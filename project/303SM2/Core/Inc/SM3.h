#ifndef _SM3_H_
#define _SM3_H_

#include "Type.h"

/* data type 
typedef unsigned char  U8;
typedef char S8;
typedef unsigned short U16;
typedef unsigned int   U32;
*/

#define SM3_BLOCK_WORD_LEN (16)
#define SM3_BLOCK_BYTE_LEN (64)
#define SM3_DIGEST_WORD_LEN (8)
#define SM3_DIGEST_BYTE_LEN (32)


// define the return-value type
enum SM3_RET_CODE
{
	SM3_SUCCESS = 0, 
	SM3_BUFFER_NULL,            // input buffer pointer is null
	SM3_INPUT_TOO_LONG,          // input message is too long
//	SM3InOutSameBuffer        // input and output share the same buffer
};

/***************************************************************************************************
  *     In the following struct definition, U32 hash[8] may be replaced with U32 *hash; the SM3_Init and SM3_Done interfaces must be adjusted accordingly
 *     In that case, alias hash to a caller-supplied U8 digest[32] buffer
 *     to save 32 bytes of memory; on platforms without unaligned access, digest must be word-aligned,
 *     otherwise an access exception may occur;
 *     if this is changed, re-check the update/accumulation logic accordingly.
 ***************************************************************************************************/

/* type to hold the SM3 context */
typedef struct
{
    U32 count[2];  // store the bit length of the message already processed
    U32 hash[SM3_DIGEST_WORD_LEN];   // U32 *hash; used to hold the intermediate and final results
    U32 wbuf[SM3_BLOCK_WORD_LEN];
}SM3_CTX;

typedef SM3_CTX SM3_Ctx;

#define SM3_MAC_IPAD  (0x36363636)
#define SM3_MAC_OPAD  (0x5c5c5c5c)
#define SM3_MAC_IPAD_OPAD  (SM3_MAC_IPAD ^ SM3_MAC_OPAD)

//SM3 HMAC context
typedef struct
{
	U32 K0[SM3_BLOCK_WORD_LEN];
	SM3_CTX sm3_ctx[1];
} SM3_HMAC_CTX;

typedef SM3_HMAC_CTX SM3_HMAC_Ctx;



U8 SM3_Init(SM3_CTX * ctx);

U8 SM3_Process(SM3_CTX * ctx, U8 * message, U32 byteLen);

U8 SM3_Done(SM3_CTX * ctx, U8 digest[SM3_DIGEST_BYTE_LEN]);

U8 SM3_Hash(U8 * message, U32 byteLen, U8 digest[SM3_DIGEST_BYTE_LEN]);



U8 SM3_HMAC_Init(SM3_HMAC_CTX *ctx, const U8 *key, U32 keyByteLen);

U8 SM3_HMAC_Process(SM3_HMAC_CTX *ctx, const U8 *input, U32 byteLen);

U8 SM3_HMAC_Done(SM3_HMAC_CTX *ctx, U8 mac[SM3_DIGEST_BYTE_LEN]);

U8 SM3_HMAC(const U8 *key, U32 keyByteLen, const U8 *msg, U32 msgByteLen, U8 mac[SM3_DIGEST_BYTE_LEN]);

#endif
