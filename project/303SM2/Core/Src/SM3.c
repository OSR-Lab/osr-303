/*******************************************************************************
 ******     Copyright (c) 2014--2020 OSR.Co.Ltd. All rights reserved.     ****** 
 *******************************************************************************/

/***************************************************************
**      SM3 Source Code
**
**      File Name:  SM3.c		
**      Version:    V0.1
**      Author:     Liu Feng
**      Date:       2017.06.02
**
****************************************************************/

//#include <stdio.h>
#include <string.h>
#include "SM3.h"

//extern void print_buf_U8(U8 buf[], U32 byteLen, char name[]);

// SM3 initial vector (IV) and round constants T
U32 const SM3_IV[8]= {0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e};
U32 const SM3_T[2] = {0x79cc4519, 0x7a879d8a};
U32 SM3_abcdefgh[8];     // store the 8 intermediate temporary variables

// byte-swap each U32 word to convert between big-endian data and CPU byte order
// bytelen must be a multiple of 4 (whole U32 words)
void convert(U32 *a, U32 bytelen)
{
    U8 tmp;
    U8 *p=(U8 *)a;

    while(bytelen>0)
    {
        tmp=*p; 
        *p=*(p+3);
        *(p+3)=tmp;
        p+=1;
        tmp=*p; 
        *p=*(p+1);
        *(p+1)=tmp;
        bytelen-=4;
        p+=3;
    }
}

// 32-bit left rotation of x by n bits
U32 SM3_ROTL(U32 const x[1], U8 n)
{
    return ((*x)<<n)|((*x)>>(32-n));
}

// SM3 permutation P: x ^ ROTL(x,m) ^ ROTL(x,n) (P0/P1)
U32 SM3_P(U32 x[1], U8 m, U8 n)
{
    return (*x) ^ SM3_ROTL(x,m) ^ SM3_ROTL(x,n);
}

// 64-bit add a = a + bitLen, where a is a U64 held as two U32 words (a[0] high, a[1] low)
// assumes bitLen <= 512, so at most one carry into the high word
U8 SM3_bitLen_add(U32 a[2], U32 bitLen)
{
    a[1] += bitLen;
    if(a[1] < bitLen)
    {
        a[0] += 1;
        if(a[0] < 1)
        {
            return SM3_INPUT_TOO_LONG;
        }
    }

    return SM3_SUCCESS;
}

// return the current bit length buffered within the block
U32 SM3_block_bitLen(SM3_CTX * ctx)
{
    return ctx->count[1] & 0x1FF;    // equivalent to ctx->count[1] % 512
}

// process each 64-byte message block; abcdefgh is obtained after processing
void SM3_block(SM3_CTX * ctx, U8 byteLen)
{
    U8 i;
    U32 *W, *W4, SS, TT1, TT2;

    //initialize abcdefgh
    memcpy(SM3_abcdefgh, ctx->hash, 32);

    //print_buf_U8((U8 *)ctx->wbuf , 64, "\r\n block");
    //print_buf((U8 *)SM3_abcdefgh , 32, "\r\n abcdefgh 000");

    //convert data
    convert(ctx->wbuf, byteLen);  

    for(i=0; i<64; i++)
    {    
        W = ctx->wbuf + (i&15);
        W4 = ctx->wbuf + ((i+4)&15);

        if(i>11)                 //(i%16) == (i&15)
        {
            *W4 = (*W4) ^ ctx->wbuf[(i-5)&15] ^ SM3_ROTL(ctx->wbuf + ((i+1)&15), 15);    //j=((i+4)&15), (j-16)&15=((i-12)&15)=((i+4)&15)=j;
            *W4 = SM3_P(W4, 15, 23) ^ SM3_ROTL(ctx->wbuf + ((i-9)&15), 7) ^ ctx->wbuf[(i-2)&15];
        } 

        if(i<16)
        {
            SS = SM3_ROTL(SM3_T , i);
            TT1 = SM3_abcdefgh[0] ^ SM3_abcdefgh[1] ^ SM3_abcdefgh[2];
            TT2 = SM3_abcdefgh[4] ^ SM3_abcdefgh[5] ^ SM3_abcdefgh[6];
        }
        else
        {
            SS = SM3_ROTL(SM3_T + 1, (i<32) ? i : (i-32));
            TT1 = ((SM3_abcdefgh[0] & SM3_abcdefgh[1]) | (SM3_abcdefgh[0] & SM3_abcdefgh[2]) | (SM3_abcdefgh[1] & SM3_abcdefgh[2]));
            TT2 = (SM3_abcdefgh[4] & SM3_abcdefgh[5]) | ((~SM3_abcdefgh[4]) & SM3_abcdefgh[6]);
        }

        SS += (SM3_abcdefgh[4] + SM3_ROTL(SM3_abcdefgh, 12));
        SS = SM3_ROTL(&SS, 7);
        TT2 += (SS + SM3_abcdefgh[7] + (*W));

        SS = SS ^ SM3_ROTL(SM3_abcdefgh, 12);
        TT1 += ((*W) ^ (*W4)) + SS + SM3_abcdefgh[3];

        // rotate the working registers a..h for this round
        SS=SM3_abcdefgh[0];
        SM3_abcdefgh[0]=TT1;
        SM3_abcdefgh[7]=SM3_abcdefgh[6];
        SM3_abcdefgh[6]=SM3_ROTL(SM3_abcdefgh+5, 19);
        SM3_abcdefgh[5]=SM3_abcdefgh[4];
        SM3_abcdefgh[4]=SM3_P(&TT2, 9, 17);
        SM3_abcdefgh[3]=SM3_abcdefgh[2];
        SM3_abcdefgh[2]=SM3_ROTL(SM3_abcdefgh+1, 9);
        SM3_abcdefgh[1]=SS;

        //if(i<30)
        //print_buf((U8 *)SM3_abcdefgh , 32, "\r\n abcdefgh");
    }

    //get ctx->hash 
    for(i=0; i<8; i++)
        ctx->hash[i] ^= SM3_abcdefgh[i];
}

// initialize the SM3 context: reset counters and load the IV
U8 SM3_Init(SM3_CTX * ctx)
{
    if(NULL == ctx)
    {
        return SM3_BUFFER_NULL;
    }

    ctx->count[0] = ctx->count[1] = 0;
    memcpy(ctx->hash, SM3_IV, 32);

    return SM3_SUCCESS;
}

// process a message given in bits (declared here, defined below)
extern U8 SM3_Process_Bit(SM3_CTX * ctx, U8 * message, U32 bitLen);

#define SM3_Process_Bytes_Simply

// process a byte-aligned message (its bit length is byteLen*8)
U8 SM3_Process(SM3_CTX * ctx, U8 * message, U32 byteLen)
{
    U8 ret = SM3_SUCCESS;   // return value

#ifdef SM3_Process_Bytes_Simply
    U8 filllen, leftlen, rightlen;
#endif

    if(NULL == ctx || NULL == message)
    {
        return SM3_BUFFER_NULL;
    }

#ifdef SM3_Process_Bytes_Simply
    if(0 == (SM3_block_bitLen(ctx)&7)) // the buffered bit length is a multiple of 8 (byte-aligned)
    {
        while(byteLen > 0)
        {
            leftlen = (SM3_block_bitLen(ctx))>>3;                        // bytes already buffered in the current block
            rightlen = 64 - leftlen;                                     // remaining free bytes in the current block
            filllen = byteLen<rightlen?byteLen:rightlen;                 // number of bytes to copy this iteration
            memcpy(((U8 *)ctx->wbuf)+leftlen, message, filllen);         // append the bytes to the block buffer
            ret = SM3_bitLen_add(ctx->count, ((U32)filllen)<<3);         // update the processed bit-length counter
            if(ret)
            {
                return ret;
            }
		
            message += filllen;
            byteLen -= filllen;
            if(!SM3_block_bitLen(ctx))
            {
                SM3_block(ctx, 64);
            }
        }
    }
    else // buffered length is not byte-aligned; fall back to the bit-level path
    {
        ret = SM3_Process_Bit(ctx, message, byteLen<<3);
    }
#else
    ret = SM3_Process_Bit(ctx, message, byteLen<<3);    // process byteLen*8 bits
#endif

    return ret;
}

// shift the message left by bitLen bits (bitLen in 1..7, byteLen in 1..63)
void message_shift_left(U8 *message, U8 byteLen, U8 bitLen)
{
    U8 i;

    for(i=0; i<byteLen-1; i++)
    {
        message[i] = (message[i]<<bitLen) | (message[i+1]>>(8-bitLen));
    }
    message[i] = message[i]<<bitLen; 
}

// process a message given by its pointer and bit length bitLen
U8 SM3_Process_Bit(SM3_CTX * ctx, U8 * message, U32 bitLen)
{
    U32 byteLen;      
    U32 block_bitLen;
    U32 filllen, leftlen, rightlen;
    U32 tmpLen;
    U8 ret;

    if(NULL == ctx || NULL == message)
    {
        return SM3_BUFFER_NULL;
    }

    // consume the message until all bitLen bits are processed
    while(bitLen > 0)
    {
        byteLen = (bitLen+7)>>3;                                     // bytes needed to hold bitLen bits
        block_bitLen = SM3_block_bitLen(ctx);                        // current bit length within the block (0..511)
        leftlen = (block_bitLen+7)>>3;                               // bytes already occupied in the current block
        block_bitLen &= 7;                                           // block_bitLen = block_bitLen % 8; (0..7)
        rightlen = 64 - leftlen;                                     // remaining free bytes in the current block
        filllen = byteLen<rightlen?byteLen:rightlen;                 // number of bytes to fill this iteration

        if(block_bitLen)   // the current block ends on a partial (non-byte-aligned) bit
        {
            // merge the incoming bits into the partial trailing byte
            *(((U8 *)ctx->wbuf)+leftlen-1) &= 0xFF<<(8-block_bitLen);
            *(((U8 *)ctx->wbuf)+leftlen-1) |= (*message)>>block_bitLen;

            if(8-block_bitLen < bitLen)   // more bits remain than fit in the current partial byte
            {
                if(filllen>0)  // the current block still has space, so more message can be appended
                {
                    memcpy(((U8 *)ctx->wbuf)+leftlen, message, filllen); 
                    message_shift_left(((U8 *)ctx->wbuf)+leftlen, filllen, 8-block_bitLen); // shift the filllen bytes left by (8-block_bitLen) bits
                    tmpLen = 8*filllen<bitLen?8*filllen:bitLen;
                    ret = SM3_bitLen_add(ctx->count, tmpLen);
                    if(ret)
                    {
                        return ret;
                    }

                    // a full 512-bit block is processed once the buffer fills
                    bitLen -= tmpLen;	
                    message += filllen;
                }
                else        // no free space left in the current block
                {
                    // finish the current partial byte, then process the full block
                    ret = SM3_bitLen_add(ctx->count, 8-block_bitLen);
                    if(ret)
                    {
                        return ret;
                    }
                    bitLen -= (8-block_bitLen);	
                    SM3_block(ctx, 64);

                    // start filling the next block with the remaining bits
                    *((U8 *)ctx->wbuf) = (*message)<<(8-block_bitLen); 
                    tmpLen = block_bitLen<bitLen?block_bitLen:bitLen;
                    ret = SM3_bitLen_add(ctx->count, tmpLen);
                    if(ret)
                    {
                        return ret;
                    }

                    // account for the bits just consumed
                    bitLen -= tmpLen;	
                    message += 1;
                }
            }
            else                         // all remaining bits fit in the current partial byte
            {
                ret = SM3_bitLen_add(ctx->count, bitLen);  
                if(ret)
                {
                    return ret;
                }

                // process the block if it is now full
                if(!SM3_block_bitLen(ctx))
                {
                    SM3_block(ctx, 64);
                }

                // all bits consumed
                bitLen=0;	
            }
        }
        else   // the current block is byte-aligned
        {
            memcpy(((U8 *)ctx->wbuf)+leftlen, message, filllen);
            tmpLen = filllen*8>bitLen?bitLen:filllen*8;
            ret = SM3_bitLen_add(ctx->count, tmpLen);    
            if(ret)
            {
                return ret;
            }
		
            bitLen -= tmpLen;
            message += filllen;

            // process the block if it is now full
            if(!SM3_block_bitLen(ctx))
            {
                SM3_block(ctx, 64);
            }
        }
    }

    return SM3_SUCCESS;
}

// process bitLen bits starting at bit position offset within message
// wrapper over SM3_Process_Bit that skips a leading bit offset
// then processes the remaining bits with SM3_Process_Bit
U8 SM3_Process_Bit_Offset(SM3_CTX * ctx, U8 * message, U32 offset, U32 bitLen)
{
    U32 tmpLen;
    U8 t;
    U8 ret;

    tmpLen = offset>>3;  //tmpLen = offset / 8; 
    message += tmpLen;
    offset = offset & 7; //offset = offset % 8; 

    // if offset is a multiple of 8, defer directly to SM3_Process_Bit
    // (no leading partial byte to handle)
    if(0 == offset)
    {
        return SM3_Process_Bit(ctx, message, bitLen);
    }

    t = *(message);
    t = t<<offset;
    tmpLen = (8-offset)<bitLen?(8-offset):bitLen;
    ret = SM3_Process_Bit(ctx, &t, tmpLen);
    if(ret)
    {
        return ret;
    }

    bitLen -= tmpLen;
    if(bitLen)
    {
        ret = SM3_Process_Bit(ctx, message+1, bitLen);
        if(ret)
        {
            return ret;
        }
    }

    return SM3_SUCCESS;
}

// finalize the hash: pad the message and output the digest
U8 SM3_Done(SM3_CTX * ctx, U8 digest[SM3_DIGEST_BYTE_LEN])
{
    U32 bitLen;
    U32 byteLen;
	
    if(NULL == ctx || NULL == digest)
    {
        return SM3_BUFFER_NULL;
    }

    bitLen = SM3_block_bitLen(ctx);        // bitLen = total bit length % 512 (0..511)
    byteLen = (bitLen+7)>>3;               // byte length of the final partial block (0..64)
    bitLen = bitLen & 7;                   // bitLen = bitLen % 8; (0..7)

    if(bitLen)   // message ends on a partial byte (byteLen in 1..64)
    {
        *((U8 *)(ctx->wbuf)+byteLen-1) &= (U8)(0xFF<<(8-1-bitLen));   // keep only the valid high bits of the last partial byte
        *((U8 *)(ctx->wbuf)+byteLen-1) |= (U8)(0x1<<(8-1-bitLen));    // append the mandatory '1' padding bit
    }
    else        // message ends on a byte boundary (byteLen in 0..63)
    {
        *((U8 *)(ctx->wbuf)+byteLen) = 0x80;                          // append the 0x80 padding byte (a '1' bit followed by zeros)
        byteLen += 1;
    }

    // byteLen is the byte length of the last block (1..64); if byteLen <= 56, pad only this block, otherwise (56 < byteLen <= 64) both this block and the next must be padded
    memset((U8 *)(ctx->wbuf)+byteLen, 0, byteLen<=56?56-byteLen:64-byteLen);   // zero-pad the remainder of the block
    if(byteLen>56)
    {
        SM3_block(ctx, 64); 
        memset((U8 *)ctx->wbuf, 0, 56); 
    }

    memcpy(ctx->wbuf+14, ctx->count, 8);                             // append the 64-bit (8-byte) message length
    SM3_block(ctx, 56);

    //convert result
    convert(ctx->hash, 32);

    memcpy(digest, ctx->hash, 32);

    //clean buffer
    for(byteLen=0; byteLen<2; byteLen++)
        ctx->count[byteLen] = 0;
    for(byteLen=0; byteLen<8; byteLen++)
        ctx->hash[byteLen] = 0;
    for(byteLen=0; byteLen<16; byteLen++)
        ctx->wbuf[byteLen] = 0;

    return SM3_SUCCESS;
}

// one-shot hash of a byte message (bit length = byteLen*8)
// message and digest may point to the same buffer, but the space pointed to by message must be large enough
U8 SM3_Hash(U8 * message, U32 byteLen, U8 digest[SM3_DIGEST_BYTE_LEN])
{
    U8 ret;
    SM3_CTX ctx[1];

    /*
    if(message == digest)
    {
        return SM3InOutSameBuffer;
    }
    */

    ret = SM3_Init(ctx); 
    if(ret)
    {
        return ret;
    }

    ret = SM3_Process(ctx, message, byteLen); 
    if(ret)
    {
        return ret;
    }

    ret = SM3_Done(ctx, digest); 
    if(ret)
    {
        return ret;
    }

    return SM3_SUCCESS;
}

// one-shot hash of a message given by pointer and bit length bitLen
// message and digest may point to the same buffer, but the space pointed to by message must be large enough
U8 SM3_Hash_Bit(U8 * message, U32 bitLen, U8 digest[SM3_DIGEST_BYTE_LEN])
{
    U8 ret;
    SM3_CTX ctx[1];

    /*
    if(message == digest)
    {
        return SM3InOutSameBuffer;
    }
    */

    ret = SM3_Init(ctx);
    if(ret)
    {
        return ret;
    }

    ret = SM3_Process_Bit(ctx, message, bitLen);
    if(ret)
    {
        return ret;
    }

    ret = SM3_Done(ctx, digest);
    if(ret)
    {
        return ret;
    }

    return SM3_SUCCESS;
}

// one-shot hash of bitLen bits starting at bit offset within message
// message and digest may point to the same buffer, but the space pointed to by message must be large enough
U8 SM3_Hash_Bit_Offset(U8 * message, U32 offset, U32 bitLen, U8 digest[SM3_DIGEST_BYTE_LEN])
{
    U8 ret;
    SM3_CTX ctx[1];

    /*
    if(message == digest)
    {
        return SM3InOutSameBuffer;
    }
    */

    ret = SM3_Init(ctx);
    if(ret)
    {
        return ret;
    }

    ret = SM3_Process_Bit_Offset(ctx, message, offset, bitLen);
    if(ret)
    {
        return ret;
    }

    ret = SM3_Done(ctx, digest);
    if(ret)
    {
        return ret;
    }

    return SM3_SUCCESS;
}






/* function: init sm3 hmac
 * parameters:
 *     ctx ------------------------ input, sm3 hmac context
 *     key ------------------------ input, key
 *     keyByteLen ----------------- input, key byte length
 * return: 0(success), other(error)
 * caution:
 */
U8 SM3_HMAC_Init(SM3_HMAC_CTX *ctx, const U8 *key, U32 keyByteLen)
{
    U32 i;
    U8 ret;

    if((NULL == ctx) || (NULL == key))
    {
        return SM3_BUFFER_NULL;
    }

    //get K0
    if(keyByteLen <= SM3_BLOCK_BYTE_LEN)
    {
        memcpy(ctx->K0, key, keyByteLen);
        memset(((U8 *)(ctx->K0)) + keyByteLen, 0, SM3_BLOCK_BYTE_LEN - keyByteLen);
    }
    else
    {
        SM3_Hash((U8 *)key, keyByteLen, (U8 *)(ctx->K0));
        memset(((U8 *)(ctx->K0)) + SM3_DIGEST_BYTE_LEN, 0, SM3_BLOCK_BYTE_LEN - SM3_DIGEST_BYTE_LEN);
    }

    //get K0 ^ ipad
    for(i=0; i<SM3_BLOCK_WORD_LEN; i++)
    {
        ctx->K0[i] ^= SM3_MAC_IPAD;
    }

    ret = SM3_Init(ctx->sm3_ctx);
    if(ret)
    {
        return ret;
    }

    return SM3_Process(ctx->sm3_ctx, (U8 *)(ctx->K0), SM3_BLOCK_BYTE_LEN);
}



/* function: sm3 hmac update message
 * parametrs:
 *     ctx ------------------------ input, sm3 hmac context
 *     input ---------------------- input, message
 *     byteLen -------------------- input, byte length of the input message
 * return: 0(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
U8 SM3_HMAC_Process(SM3_HMAC_CTX *ctx, const U8 *input, U32 byteLen)
{
    return SM3_Process(ctx->sm3_ctx, (U8 *)input, byteLen);
}


/* function: message update done, get the mac
 * parametrs:
 *     ctx ------------------------ input, sm3 hmac context
 *     mac ------------------------ output, mac
 * return: 0(success), other(error)
 * caution:
 *     1. please make sure the ctx is valid and initialized
 */
U8 SM3_HMAC_Done(SM3_HMAC_CTX *ctx, U8 mac[SM3_DIGEST_BYTE_LEN])
{
    U32 i;
    U8 ret;

    if((NULL == ctx) || (NULL == mac))
    {
        return SM3_BUFFER_NULL;
    }

    //set mac as SM3((K0^ipad)||message)
    ret = SM3_Done(ctx->sm3_ctx, mac);
    if(ret)
    {
        return ret;
    }

    //get K0 ^ opad
    for(i=0; i<SM3_BLOCK_WORD_LEN; i++)
    {
        ctx->K0[i] ^= SM3_MAC_IPAD_OPAD;
    }

    ret = SM3_Init(ctx->sm3_ctx);
    if(ret)
    {
        return ret;
    }

    ret = SM3_Process(ctx->sm3_ctx, (U8 *)(ctx->K0), SM3_BLOCK_BYTE_LEN);
    if(ret)
    {
        return ret;
    }

    ret = SM3_Process(ctx->sm3_ctx, mac, SM3_DIGEST_BYTE_LEN);
    if(ret)
    {
        return ret;
    }

    return SM3_Done(ctx->sm3_ctx, mac);
}


/* function: input key and message, get the mac
 * parametrs:
 *     key ------------------------ input, key
 *     keyByteLen ----------------- input, key byte length
 *     msg ------------------------ input, message
 *     msgByteLen ----------------- input, byte length of the input message
 *     mac ------------------------ output, mac
 * return: 0(success), other(error)
 * caution:
 */
U8 SM3_HMAC(const U8 *key, U32 keyByteLen, const U8 *msg, U32 msgByteLen, U8 mac[SM3_DIGEST_BYTE_LEN])
{
    SM3_HMAC_CTX ctx[1];
    U8 ret;

    if((NULL == key) || (NULL == msg) || (NULL == mac))
    {
        return SM3_BUFFER_NULL;
    }

    ret = SM3_HMAC_Init(ctx, key, keyByteLen);
    if(ret)
    {
        return ret;
    }

    ret = SM3_Process(ctx->sm3_ctx, (U8 *)msg, msgByteLen);
    if(ret)
    {
        return ret;
    }

    return SM3_HMAC_Done(ctx, mac);
}


