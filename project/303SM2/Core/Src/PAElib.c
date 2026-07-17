/*******************************************************************************
 ******     Copyright (c) 2014--2020 OSR.Co.Ltd. All rights reserved.     ****** 
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
//#include "utility.h"
#include "PAElib.h"
#include <stdint.h>


//#include "utility.h"


/* Function: convert 0x1122334455667788 to 0x4433221188776655
 * Parameters:
 *      in --------- source address
 *     out --------- destination address
 *     wordLen ----- word length of in/out
 * Caution: 
 */
void U32_endian_convert(U8 *in, U8 *out, U32 wordLen)
{
	U8 tmp;

	if(in == out)
	{
		while(wordLen>0)
		{
			tmp=*in; 
			*in=*(in+3);
			*(in+3)=tmp;
			in+=1;
			tmp=*in; 
			*in=*(in+1);
			*(in+1)=tmp;
			wordLen--;
			in+=3;
		}
	}
	else
	{
		while(wordLen>0)
		{
			*(out)   = *(in+3);
			*(out+1) = *(in+2);
			*(out+2) = *(in+1);
			*(out+3) = *(in);
			wordLen--;
			in += 4;
			out += 4;
		}
	}
}

/* Function: convert 0x1122334455667788 to 0x5566778811223344
 * Parameters:
 *      in --------- source address
 *     out --------- destination address
 *     wordLen ----- word length of in/out
 * Caution: 
 *     1. wordLen must > 1
 */
void U32_BigInt_reverse(U32 *in, U32 *out, U32 wordLen)
{
	U32 i, tmp, halfWordLen=wordLen>>1;

	if(in == out)
	{
		for(i=0; i<halfWordLen; i++)
		{
			tmp = in[i];
			in[i] = in[wordLen-1-i];
			in[wordLen-1-i] = tmp;
		}
	}
	else
	{
		for(i=0; i<wordLen; i++)
		{
			out[i] = in[wordLen-1-i];
		}
	}
}

/* Function: convert 0x1122334455667788 to 0x8877665544332211
 * Parameters:
 *      in --------- source address
 *     out --------- destination address
 *     byteLen ----- byte length of in/out
 * Caution: 
 *     1. byteLen must > 1
 */
void U8_BigInt_reverse(U8 *in, U8 *out, U32 byteLen)
{
	U8 tmp;
	U32 i, halfByteLen=byteLen>>1;

	if(in == out)
	{
		for(i=0; i<halfByteLen; i++)
		{
			tmp = in[i];
			in[i] = in[byteLen-1-i];
			in[byteLen-1-i] = tmp;
		}
	}
	else
	{
		for(i=0; i<byteLen; i++)
		{
			out[i] = in[byteLen-1-i];
		}
	}
}

/* Function: convert 0x1122334455 to 0x5544332211000000
 * Parameters:
 *      in --------- source address
 *     out --------- destination address
 *     byteLen ----- byte length of in/out
 * Caution: 
 *     1. byteLen must > 1
 */
void U8_BigInt_reverse_with_padding0(U8 *in, U8 *out, U32 byteLen)
{
	U8_BigInt_reverse(in, out, byteLen);
	memset(out+byteLen, 0, (((byteLen+3)/4)<<2)-byteLen);
}

/* Function: copy U32 data
 * Parameters:
 *     dst --------- destination address
 *     src --------- source address
 *     wordLen ----- word length of source data
 */
void U32_Copy(U32 dst[], U32 src[], U32 wordLen)
{
	U32 i;

	for(i=0; i<wordLen; i++)
		dst[i] = src[i];
}

/* Function: clear U32 data
 * Parameters:
 *     a ----------- data address
 *     wordLen ----- word length of data
 */
void U32_Clear(U32 a[], U32 wordLen)
{
#if 0
	memset(a, 0, wordLen<<2);
#else
	while(wordLen)
		a[--wordLen]=0;
#endif
}

/* Function: check a is 1 or not
 * Parameters:
 *     a ------------ pointer to U32 big integer a
 *     aWordLen ----- word length of big integer a
 * Return: 1(a is 1), 0(a is not 1)
 * Caution:
 */
U8 Bigint_Check_1(U32 a[], U32 aWordLen)
{
	U32 i;

	if(!aWordLen)
	{
		return 0;
	}

	if(a[0] != 1)
	{
		return 0;
	}

	for(i=1; i<aWordLen; i++)
	{
		if(a[i])
			return 0;
	}

	return 1;
}

/* Function: check a equal to p-1 or not
 * Parameters:
 *     a ------------ pointer to U32 big integer a
 *     p ------------ pointer to U32 big integer p, p must be odd
 *     wordLen ------ word length of a and p
 * Return: 1(a is 1), 0(a is not 1)
 * Caution:
 *     1. make sure p is odd
 */
U8 Bigint_Check_p_1(U32 a[], U32 p[], U32 wordLen)
{
	U32 i;

	if(!wordLen)
	{
		return 0;
	}

	if(a[0] != p[0] - 1)
	{
		return 0;
	}

	for(i=1; i<wordLen; i++)
	{
		if(a[i] != p[i])
			return 0;
	}

	return 1;
}

/* Function: check a is 0 or not
 * Parameters:
 *     a ------------ pointer to U32 big integer a
 *     aWordLen ----- word length of big integer a
 * Return: 1(a is 0), 0(a is not 0)
 * Caution:
 */
U8 Bigint_Check_NULL(U32 a[], U32 aWordLen)
{
	U32 i;

	if(!aWordLen)
	{
		return 1;
	}

	for(i=0; i<aWordLen; i++)
	{
		if(a[i])
			return 0;
	}

	return 1;
}

/* Function: check a is 0 or not
 * Parameters:
 *     a ------------ pointer to U8 big integer a
 *     aByteLen ----- byte length of big integer a
 * Return: 1(a is 0), 0(a is not 0)
 * Caution:
 */
U8 BigintU8_Check_NULL(U8 a[], U32 aByteLen)
{
	U32 i;

	if(!aByteLen)
	{
		return 1;
	}

	for(i=0; i<aByteLen; i++)
	{
		if(a[i])
			return 0;
	}

	return 1;
}

/* Function: get true word length of big integer a
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 * Return: 
 *     true word length of big integer a
 */
U32 Get_WordLen(U32 a[], U32 aWordLen)
{
#if 1
	while(1)
	{
		if(aWordLen)
		{
			if(!a[aWordLen-1])
				aWordLen--;
			else
				return aWordLen;
		}
		else
			return 0;
	}
#else
	while(aWordLen && !a[aWordLen-1])     // shrink aWordLen while the most significant word is 0,
		aWordLen--;
	return aWordLen;
#endif
}

/* Function: get bit length of big integer a
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 * Return:
 *     bit length of big integer a
 */
U32 Get_BitLen(U32 a[], U32 aWordLen)
{
	S32 bitLen;

	aWordLen = Get_WordLen(a, aWordLen);
	
	if(aWordLen==0)
	{
		return 0;
	}

#if 0

	bitLen=(aWordLen<<5)-1;     //make (bitLen>>5) be the top indicator of big integer array a
	while(bitLen>=0 && !(a[bitLen>>5]&(1<<(bitLen&31))))
		bitLen--;

	return bitLen+1;

#else

	bitLen = 0;
	while(!(a[aWordLen-1] & ((uint32_t)1 << (31-bitLen))))
	{
		bitLen++;
	}

	return (aWordLen<<5)-bitLen;

#endif
}

/* Function: get aimed bit value of big integer a
 * Parameters:
 *     a ----------- big integer a
 *     bitLen ------ aimed bit location
 * Return: 
 *     bit value of aimed bit
 * Caution:
 *     1. make sure bitLen > 0
 */
U8 Get_BitValue(U32 a[], U32 bitLen)
{
	bitLen--;
	if(a[(bitLen)>>5]&((uint32_t)1<<(bitLen&31)))
	{
		return 1;
	}
	return 0;
}

/* Function: for a = b*2^t, b is odd, get t
 * Parameters:
 *     a ----------- big integer a
 * Return: 
 *     number of multiple by 2, for a
 * Caution:
 *     1. make sure a != 0
 */
U32 Get_Multiple2_Number(U32 a[])
{
	U32 i=0,j=0;

	while((a[i]&(1<<j))==0)
	{
		j++;
		if(j==32)
		{
			j=0;
			i++;
		}
	}

	return (i<<5)+j;
}

/* Function: compare big integer a and b
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 * Return: 
 *     0:a=b,   1:a>b,   2: a<b
 */
U8 Big_Compare(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen)
{
	S32 i;	

	aWordLen = Get_WordLen(a, aWordLen);
	bWordLen = Get_WordLen(b, bWordLen);

	if(aWordLen>bWordLen)
		return 1;
	if(aWordLen<bWordLen)
		return 2;

	for(i=(aWordLen-1);i>=0;i--)
	{
		if(a[i]>b[i])
			return 1;
		if(a[i]<b[i])
			return 2;
	}
	return 0;
}

/* Function: a = a/2
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 * Return: 
 *     word length of a = a/2
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 *     2. a may be 0, then aWordLen is 0, to make sure aWordLen-1 available, so data type of aWordLen is S32, not U32
 */
U32 Big_Div2(U32 a[], S32 aWordLen)
{
	S32 i;

	//aWordLen = Get_WordLen(a, aWordLen);

	if(!aWordLen)
		return 0;

	for(i=0; i<aWordLen-1; i++)
	{
		a[i] >>= 1;
		a[i] |= (a[i+1]<<31);
	}
	a[i] >>= 1;

	if(!a[i])
		return i;
	return aWordLen;
}

U32 Big_Div2_(U32 a[], S32 aWordLen, U32 out[])
{
	S32 i;

	//aWordLen = Get_WordLen(a, aWordLen);

	if(!aWordLen)
		return 0;

	for(i=0; i<aWordLen-1; i++)
	{
		out[i] = a[i]>>1;
		out[i] = out[i] | (a[i+1]<<31);
	}
	out[i] >>= 1;

	if(!out[i])
		return i;
	return aWordLen;
}

/* Function: a = a/(2^n)
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     n ----------- exponent of 2^n
 * Return: 
 *     word length of a = a/(2^n)
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 *     2. a may be 0, then aWordLen is 0, to make sure aWordLen-1 available, so data type of aWordLen is S32, not U32
 */
U32 Big_Div2n(U32 a[], S32 aWordLen, U32 n)
{
	S32 i;
	U32 j;

	//aWordLen = Get_WordLen(a, aWordLen);

	if(!aWordLen)
		return 0;

	if(n<=32)
	{
		for(i=0; i<aWordLen-1; i++)
		{
			a[i] >>= n;
			a[i] |= (a[i+1]<<(32-n));
		}
		a[i] >>= n;

		if(!a[i])
			return i;
		return aWordLen;
	}
	else        // otherwise, shift by n bits
	{
		j=n>>5; //j=n/32;
		n&=31;  //n=n%32;
		for(i=0; i<aWordLen-(S32)j-1; i++)
		{
			a[i] = a[i+j]>>n;
			a[i] |= (a[i+j+1]<<(32-n));
		}
		a[i] = a[i+j]>>n;
		U32_Clear(a+aWordLen-j, j);

		if(!a[i])
			return i;
		return aWordLen-j;
	}
}

/* Function: a = a*2
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 * Return: 
 *     word length of a = a*2
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 *     2. if word length of a*2 is aWordLen+1, make sure a[aWordLen] available,
 */
U32 Big_Mul2(U32 a[], S32 aWordLen)
{
	U8 flag=0;
	S32 i;

	//aWordLen = Get_WordLen(a, aWordLen);

	if(!aWordLen)
		return 0;

	if(a[aWordLen-1]&0x80000000)
	{
		flag=1;
	}

	for(i=aWordLen-1; i>0; i--)
	{
		a[i] <<= 1;
		a[i] |= (a[i-1]>>31);
	}
	a[i] <<= 1;

	if(flag)
	{
		a[aWordLen]=1;
		return aWordLen+1;
	}

	return aWordLen;
}

/* Function: out = a*2
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     out --------- big integer out
 * Return: 
 *     word length of out = a*2
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 *     2. if word length of a*2 is aWordLen+1, make sure out[aWordLen] available,
 *     3. out must have enough buffer space
 */
U32 Big_Mul2_(U32 a[], S32 aWordLen, U32 out[])
{
	U8 flag=0;
	S32 i;

	//aWordLen = Get_WordLen(a, aWordLen);

	if(!aWordLen)
		return 0;

	if(a[aWordLen-1]&0x80000000)
	{
		flag=1;
	}

	for(i=aWordLen-1; i>0; i--)
	{
		out[i] = a[i]<<1;
		out[i] |= (a[i-1]>>31);
	}
	out[i] = a[i]<<1;

	if(flag)
	{
		out[aWordLen]=1;
		return aWordLen+1;
	}

	return aWordLen;
}

/* Function: a = a*(2^n)
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     n ----------- exponent of 2^n
 * Return: 
 *     word length of a = a*(2^n)
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 *     2. if word length of a*(2^n) is aWordLen+1, make sure a[aWordLen] available,
 *     3. make sure 0<=n<=32
 */
U32 Big_Mul2n(U32 a[], S32 aWordLen, U8 n)
{
	U8 flag=0;
	S32 i;

	//aWordLen = Get_WordLen(a, aWordLen);

	if(!aWordLen)
		return 0;

	if(a[aWordLen-1]&(0xFFFFFFFF<<(32-n)))    // if the top n bits are set, the result needs one more word
	{
		a[aWordLen]=a[aWordLen-1]>>(32-n);
		flag=1;
	}

	for(i=aWordLen-1; i>0; i--)
	{
		a[i] <<= n;
		a[i] |= (a[i-1]>>(32-n));
	}
	a[i] <<= n;

	if(flag)
	{
		return aWordLen+1;
	}

	return aWordLen;
}

U32 maxWordLen, minWordLen;
U32 temp_U32, carry;

/* Function: a = a + b
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 * Return: 
 *     word length of a = a + b
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 *     2. make sure a[maxWordLen] available, here maxWordLen =max(aWordLen,bWordLen)
 *     3. aWordLen can be 0, bWordLen can be 0 too, and the both can be 0 at the same time
 */
U32 Big_Add(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen)
{
	U32 i;

	//aWordLen = Get_WordLen(a, aWordLen);
	//bWordLen = Get_WordLen(b, bWordLen);

	if(!bWordLen)
		return aWordLen;

	if(aWordLen > bWordLen)
	{
		maxWordLen = aWordLen;
		minWordLen = bWordLen;
	}
	else
	{
		maxWordLen = bWordLen;
		minWordLen = aWordLen;
	}

	carry = 0;
	for(i=0; i<minWordLen; i++)
	{
		temp_U32 = a[i] + b[i];
		a[i] = temp_U32 + carry;
		if((temp_U32 < b[i]) || (a[i] < carry))
		{
			carry = 1;
		}
		else
		{
			carry = 0;
		}
	}

	if(aWordLen < bWordLen)
	{
		U32_Copy(a+aWordLen, b+aWordLen, bWordLen - aWordLen);
	}

	for(; i<maxWordLen; i++)
	{
		a[i] += carry;
		if(a[i] < carry)
		{
			carry = 1;
		}
		else
		{
			carry = 0;
			break;
		}
	}

	if(carry)
	{
		a[maxWordLen] = 1;
		return maxWordLen+1;
	}

	return maxWordLen;
}

/* Function: a = a - b
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 * Return: 
 *     word length of a = a - b
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 *     2. make sure a >= b
 *     3. bWordLen can be 0
 */
U32 Big_Sub(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen)
{
	U32 i, temp_a;

	//aWordLen = Get_WordLen(a, aWordLen);
	//bWordLen = Get_WordLen(b, bWordLen);

	if(!bWordLen)
		return aWordLen;

	carry = 0;
	for(i=0; i<bWordLen; i++)
	{
		temp_a = a[i];
		temp_U32 = a[i] - b[i];
		a[i] = temp_U32 - carry;
		if((temp_a < temp_U32) || (temp_U32 < carry))
		{
			carry = 1;
		}
		else
		{
			carry = 0;
		}
	}

	for(; i<aWordLen; i++)
	{
		temp_a = a[i];
		a[i] -= carry;
		if(temp_a < carry)
		{
			carry = 1;
		}
		else
		{
			//carry = 0;
			break;
		}
	}

	return Get_WordLen(a, aWordLen);
}


/* Function: b = a - b
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 * Return: 
 *     word length of b = a - b
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 *     2. make sure a >= b
 *     3. bWordLen can be 0
 */
U32 Big_Subb(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen)
{
	U32 i;

	//aWordLen = Get_WordLen(a, aWordLen);
	//bWordLen = Get_WordLen(b, bWordLen);

	carry = 0;
	for(i=0; i<bWordLen; i++)
	{
		temp_U32 = a[i] - b[i];
		b[i] = temp_U32 - carry;
		if((a[i] < temp_U32) || (temp_U32 < carry))
		{
			carry = 1;
		}
		else
		{
			carry = 0;
		}
	}

	U32_Copy(b+bWordLen, a+bWordLen, aWordLen - bWordLen);

	for(; i<aWordLen; i++)
	{
		b[i] -= carry;
		if(a[i] < carry)
		{
			carry = 1;
		}
		else
		{
			//carry = 0;
			break;
		}
	}

	return Get_WordLen(b, aWordLen);
}

/* Function: a = a + b (with signed flag)
 * Parameters:
 *     a ----------- big integer a
 *     flag_a ------ signed flag of a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     flag_b ------ signed flag of b
 *     bWordLen ---- word length of b
 * Return: 
 *     signed flag of a = a + b
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 */
U8 Big_Sign_Add(U32 a[], U8 flag_a, U32 *aWordLen, U32 b[], U8 flag_b, U32 bWordLen)
{
	if(flag_a == flag_b)    // a and b same signed
	{
		*aWordLen = Big_Add(a,*aWordLen,b,bWordLen);
	}
	else
	{
		if(Big_Compare(a,*aWordLen,b,bWordLen)<2)    // |a|>=|b|
		{
			*aWordLen = Big_Sub(a,*aWordLen,b,bWordLen);
		}
		else    // |a|<|b|
		{
			*aWordLen = Big_Subb(b,bWordLen,a,*aWordLen);
			return flag_b;
		}
	}
	return flag_a;
}

/* Function: a = a - b (with signed flag)
 * Parameters:
 *     a ----------- big integer a
 *     flag_a ------ signed flag of a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     flag_b ------ signed flag of b
 *     bWordLen ---- word length of b
 * Return: 
 *     signed flag of a = a - b
 * Caution:
 *     1. make sure aWordLen is the actual word length of a (most significant word is not 0)
 */
U8 Big_Sign_Sub(U32 a[], U8 flag_a, U32 *aWordLen, U32 b[], U8 flag_b, U32 bWordLen)
{
	if(flag_a^flag_b) // flag_a=0,flag_b=1 or flag_a=1,flag_b=0
	{
		*aWordLen = Big_Add(a,*aWordLen,b,bWordLen);
	}
	else
	{
		if(Big_Compare(a,*aWordLen,b,bWordLen)<2) // |a|>=|b|
		{
			*aWordLen = Big_Sub(a,*aWordLen,b,bWordLen);
		}
		else // |a|<|b|
		{
			*aWordLen = Big_Subb(b,bWordLen,a,*aWordLen);	
			return !flag_b;
		}
	}
	return flag_a;
}

/* Function: xinv = x^(-1) mod n
 * Parameters:
 *     x ----------- big integer x
 *     xWordLen ---- word length of x
 *     n ----------- big integer n
 *     nWordLen ---- word length of n
 *     xinv -------- big integer xinv = x^(-1) mod n
 *     xinvWordLen - word length of xinv
 * Return: 
 *     0: xinv exists, 1: xinv doesn't exist.
 * Caution:
 *     1. x and n can not both be even
 *     2. x can not be equal to n
 *     3. eithor x or n can not be 1
 *     4. make sure xinv buffer word length >= nWordLen
 *     5. to save time, you had better make sure x < n.
 */
U8 Big_ModInv(U32 x[], U32 xWordLen, U32 n[], U32 nWordLen, U32 xinv[], U32 *xinvWordLen)
{
	U32 *u, *v, *x1, *x2, *y1, *y2;
	U32 uWordLen, vWordLen, x1WordLen, x2WordLen, y1WordLen, y2WordLen;
	U8 flag_x1, flag_x2, flag_y1, flag_y2, flag;

	if(!(x[0]&1) && !(n[0]&1))               //if x,n both even, then xinv doesn't exist
	{
		return 1;
	}

	xWordLen = Get_WordLen(x, xWordLen);
	nWordLen = Get_WordLen(n, nWordLen);
	if(0 == xWordLen || 0 == nWordLen)
	{
		return 2;
	}
	
	u = (U32 *)malloc((nWordLen+1)*4);
	v = (U32 *)malloc((nWordLen+1)*4);
	x1= (U32 *)malloc((nWordLen+1)*4);
	x2= (U32 *)malloc((nWordLen+1)*4);
	y1= (U32 *)malloc((nWordLen+1)*4);
	y2= (U32 *)malloc((nWordLen+1)*4);

	U32_Clear(u, nWordLen+1);
	U32_Clear(v, nWordLen+1);
	U32_Clear(x1, nWordLen+1);
	U32_Clear(x2, nWordLen+1);
	U32_Clear(y1, nWordLen+1);
	U32_Clear(y2, nWordLen+1);

	// assume: 0 marks a positive value (including 0), 1 marks a negative value
	flag_x1=0; 
	flag_x2=0; 
	flag_y1=0; 
	flag_y2=0;

	//x1*x+y1*n=u; x2*x+y2*n=v;
	U32_Copy(u, x, xWordLen);
	U32_Copy(v, n, nWordLen);
	x1[0] = 1;
	y2[0] = 1;

	uWordLen=xWordLen;
	vWordLen=nWordLen;
	x1WordLen=1;
	x2WordLen=0;
	y1WordLen=0;
	y2WordLen=1;

STEIN_1:
	while(!(u[0]&1))
	{
		// u=u/2;
		uWordLen=Big_Div2(u,uWordLen);		

		if((x1[0]&1) || (y1[0]&1))
		{
			// x1=x1+n; sign   
			flag_x1 = Big_Sign_Add(x1,flag_x1,&x1WordLen,n,0,nWordLen);
			// y1=y1-x; sign
			flag_y1 = Big_Sign_Sub(y1,flag_y1,&y1WordLen,x,0,xWordLen);
		}
		// x1=x1/2; 
		x1WordLen=Big_Div2(x1,x1WordLen);

		// y1=y1/2;
		y1WordLen=Big_Div2(y1,y1WordLen);
		//printf("\r\n AA x1=%lx y1=%lx u=%lx",x1[0],y1[0],u[0]);
	}

	while(!(v[0]&1))
	{
		// u=u/2;
		vWordLen=Big_Div2(v,vWordLen);	

		if((x2[0]&1) || (y2[0]&1))
		{
			// x2=x2+n; sign   
			flag_x2 = Big_Sign_Add(x2,flag_x2,&x2WordLen,n,0,nWordLen);
			// y2=y2-x; sign
			flag_y2 = Big_Sign_Sub(y2,flag_y2,&y2WordLen,x,0,xWordLen);
		}
		// x2=x2/2; 
		x2WordLen=Big_Div2(x2,x2WordLen);

		// y2=y2/2;
		y2WordLen=Big_Div2(y2,y2WordLen);
		//printf("\r\n BB x2=%lx y2=%lx v=%lx",x2[0],y2[0],v[0]);
	}

	flag = Big_Compare(u,uWordLen,v,vWordLen);
	if(flag == 0) // u==v
	{
		goto STEIN_2;
	}
	if(flag == 1) // u>v
	{
		// u=u-v;
		uWordLen = Big_Sub(u,uWordLen,v,vWordLen);
		// x1=x1-x2; sign
		flag_x1 = Big_Sign_Sub(x1,flag_x1,&x1WordLen,x2,flag_x2,x2WordLen);
		// y1=y1-y2; sign
		flag_y1 = Big_Sign_Sub(y1,flag_y1,&y1WordLen,y2,flag_y2,y2WordLen);
		//printf("\r\n AAAx1=%lx y1=%lx u=%lx",x1[0],y1[0],u[0]);
	}
	else // u<v
	{    // v=v-u;
		vWordLen = Big_Sub(v,vWordLen,u,uWordLen);
		// x2=x2-x1; sign
		flag_x2 = Big_Sign_Sub(x2,flag_x2,&x2WordLen,x1,flag_x1,x1WordLen);
		// y2=y2-y1; sign
		flag_y2 = Big_Sign_Sub(y2,flag_y2,&y2WordLen,y1,flag_y1,y1WordLen);
		//printf("\r\n BBBx2=%lx y2=%lx v=%lx",x2[0],y2[0],v[0]);
	}

	goto STEIN_1;

STEIN_2: 
	while(Big_Compare(x1, x1WordLen, n, nWordLen)==1)
	{
		x1WordLen = Big_Sub(x1,x1WordLen,n,nWordLen);
	}

	if(flag_x1)
	{
		x1WordLen = Big_Subb(n,nWordLen,x1,x1WordLen); 
	}
		
	if(uWordLen==1 && u[0]==1)
	{
		*xinvWordLen = x1WordLen;
		U32_Copy(xinv, x1, nWordLen);  //not U32_Copy(xinv, x1, x1WordLen); to avoid the case that high part all zero
		flag = 0;
	}
	else
	{
		*xinvWordLen = 0;
		flag = 1;
	}

	free(u);
	free(v);
	free(x1);
	free(y1);
	free(x2);
	free(y2);

	return flag;
}

/* Function: xinv = x^(-1) mod n (for n odd)
 * Parameters:
 *     x ----------- big integer x
 *     xWordLen ---- word length of x
 *     n ----------- big integer n
 *     nWordLen ---- word length of n
 *     xinv -------- big integer xinv = x^(-1) mod n
 *     xinvWordLen - word length of xinv
 * Return: 
 *     0: xinv exists, 1: xinv doesn't exist, 2: n is even.
 * Caution:
 *     1. n can not be even
 *     2. x can not be equal to n
 *     3. eithor x or n can not be 1
 *     4. to save time, you had better make sure x < n.
 */
U8 Big_ModInv_Odd_1(U32 x[], U32 xWordLen, U32 n[], U32 nWordLen, U32 xinv[], U32 *xinvWordLen)
{
	U32 *u, *v, *x1, *x2;
	U32 uWordLen, vWordLen, x1WordLen, x2WordLen;
	U8 flag_x1, flag_x2, flag;

	if(!(n[0]&1))
	{
		return 2;
	}

	xWordLen = Get_WordLen(x, xWordLen);
	nWordLen = Get_WordLen(n, nWordLen);
	if(0 == xWordLen || 0 == nWordLen)
	{
		return 2;
	}

	u = (U32 *)malloc((nWordLen+1)*4);
	v = (U32 *)malloc((nWordLen+1)*4);
	x1= (U32 *)malloc((nWordLen+1)*4);
	x2= (U32 *)malloc((nWordLen+1)*4);

	U32_Clear(u, nWordLen+1);
	U32_Clear(v, nWordLen+1);
	U32_Clear(x1, nWordLen+1);
	U32_Clear(x2, nWordLen+1);

	// assume: 0 marks a positive value (including 0), 1 marks a negative value
	flag_x1=0; 
	flag_x2=0; 

	//x1*x+y1*n=u; x2*x+y2*n=v;
	U32_Copy(u, x, xWordLen);
	U32_Copy(v, n, nWordLen);
	x1[0] = 1;

	uWordLen=xWordLen;
	vWordLen=nWordLen;
	x1WordLen=1;
	x2WordLen=0;

STEIN_1:
	while(!(u[0]&1))
	{
		// u=u/2;
		uWordLen=Big_Div2(u,uWordLen);		

		if(x1[0]&1)
		{
			// x1=x1+n; sign   
			flag_x1 = Big_Sign_Add(x1,flag_x1,&x1WordLen,n,0,nWordLen);
		}
		// x1=x1/2; 
		x1WordLen=Big_Div2(x1,x1WordLen);
	}

	while(!(v[0]&1))
	{
		// u=u/2;
		vWordLen=Big_Div2(v,vWordLen);		

		if(x2[0]&1)
		{
			// x2=x2+n; sign   
			flag_x2 = Big_Sign_Add(x2,flag_x2,&x2WordLen,n,0,nWordLen);
		}
		// x2=x2/2; 
		x2WordLen=Big_Div2(x2,x2WordLen);
	}

	flag = Big_Compare(u,uWordLen,v,vWordLen);
	if(flag == 0) // u==v
	{
		goto STEIN_2;
	}
	if(flag == 1) // u>v
	{
		// u=u-v;
		uWordLen = Big_Sub(u,uWordLen,v,vWordLen);
		// x1=x1-x2; sign
		flag_x1 = Big_Sign_Sub(x1,flag_x1,&x1WordLen,x2,flag_x2,x2WordLen);
	}
	else          // u<v
	{	// v=v-u;
		vWordLen = Big_Sub(v,vWordLen,u,uWordLen);
		// x2=x2-x1; sign
		flag_x2 = Big_Sign_Sub(x2,flag_x2,&x2WordLen,x1,flag_x1,x1WordLen);
	}

	goto STEIN_1;

STEIN_2:
	while(Big_Compare(x1, x1WordLen, n, nWordLen)==1)
	{
		x1WordLen = Big_Sub(x1,x1WordLen,n,nWordLen);
	}

	if(flag_x1)
	{
		x1WordLen = Big_Subb(n,nWordLen,x1,x1WordLen); 
	}
	
	if(uWordLen==1 && u[0]==1)
	{
		*xinvWordLen = x1WordLen;
		U32_Copy(xinv, x1, nWordLen);  //not U32_Copy(xinv, x1, x1WordLen); to avoid the case that high part all zero
		flag = 0;
	}
	else
	{
		flag = 1;
	}

	free(u);
	free(v);
	free(x1);
	free(x2);

	return flag;
}

/* Function: get Montgomery part inverse xinv = x^(-1) * 2^k mod n (for n odd)
 * Parameters:
 *     x ----------- big integer x
 *     xWordLen ---- word length of x
 *     n ----------- big integer n
 *     nWordLen ---- word length of n
 *     xinv -------- big integer xinv = x^(-1) * 2^k mod n
 *     xinvWordLen - word length of xinv
 * Return: 
 *     0: xinv exists, 1: xinv doesn't exist, 2: n is even.
 * Caution:
 *     1. n can not be even
 *     2. x can not be equal to n
 *     3. eithor x or n can not be 1
 *     4. to save time, you had better make sure x < n.
 */
U8 Big_MontPartInv(U32 x[], U32 xWordLen, U32 n[], U32 nWordLen, U32 xinv[], U32 *xinvWordLen, U32 *k)
{
	U32 *u, *v, *x1, *x2;
	U32 uWordLen, vWordLen, x1WordLen, x2WordLen;


	if(!(n[0]&1))
	{
		return 2;
	}

	xWordLen = Get_WordLen(x, xWordLen);
	//nWordLen = Get_WordLen(n, nWordLen);

	u = (U32 *)malloc((nWordLen+1)*4);
	v = (U32 *)malloc((nWordLen+1)*4);
	x1= (U32 *)malloc((nWordLen+1)*4);
	x2= (U32 *)malloc((nWordLen+1)*4);

	U32_Clear(u, nWordLen+1);
	U32_Clear(v, nWordLen+1);
	U32_Clear(x1, nWordLen+1);
	U32_Clear(x2, nWordLen+1);

	//x1*v+x2*u=n
	U32_Copy(u, x, xWordLen);
	U32_Copy(v, n, nWordLen);
	x1[0] = 1;

	uWordLen=xWordLen;
	vWordLen=nWordLen;
	x1WordLen=1;
	x2WordLen=0;

	*k = 0;

	while(vWordLen>0)
	{
		if(!(v[0]&1))
		{
			vWordLen = Big_Div2(v, vWordLen);
			x1WordLen = Big_Mul2(x1, x1WordLen);
		}
		else if(!(u[0]&1))
		{
			uWordLen = Big_Div2(u, uWordLen);
			x2WordLen = Big_Mul2(x2, x2WordLen);
		}
		else if(Big_Compare(v, vWordLen, u, uWordLen) != 2)
		{
			vWordLen = Big_Sub(v, vWordLen, u, uWordLen);
			vWordLen = Big_Div2(v, vWordLen);
			x2WordLen = Big_Add(x2, x2WordLen, x1, x1WordLen);
			x1WordLen = Big_Mul2(x1, x1WordLen);
		}	
		else
		{
			uWordLen = Big_Sub(u, uWordLen, v, vWordLen);
			uWordLen = Big_Div2(u, uWordLen);
			x1WordLen = Big_Add(x1, x1WordLen, x2, x2WordLen);
			x2WordLen = Big_Mul2(x2, x2WordLen);
		}

		(*k)++;
	}

	if(uWordLen!=1 || u[0]!=1)
	{
		free(u);
		free(v);
		free(x1);
		free(x2);

		return 1;
	}

	if(Big_Compare(x1, x1WordLen, n, nWordLen) == 1)
	{
		x1WordLen = Big_Sub(x1, x1WordLen, n, nWordLen);
	}

	U32_Copy(xinv, x1, nWordLen);  //not U32_Copy(xinv, x1, x1WordLen); to avoid the case that high part all zero
	*xinvWordLen = x1WordLen;

	free(u);
	free(v);
	free(x1);
	free(x2);

	return 0;
}

/* Function: xinv = x^(-1) mod n (for n odd)
 * Parameters:
 *     x ----------- big integer x
 *     xWordLen ---- word length of x
 *     n ----------- big integer n
 *     nWordLen ---- word length of n
 *     xinv -------- big integer xinv = x^(-1) mod n
 *     xinvWordLen - word length of xinv
 * Return: 
 *     0: xinv exists, 1: xinv doesn't exist, 2: n is even.
 * Caution:
 *     1. n can not be even
 *     2. x can not be equal to n
 *     3. eithor x or n can not be 1
 *     4. to save time, you had better make sure x < n.
 */
U8 Big_ModInv_Odd_2(U32 x[], U32 xWordLen, U32 n[], U32 nWordLen, U32 xinv[], U32 *xinvWordLen)
{
	U32 k, i;
	U32 *xx;


	if(!(n[0]&1))
	{
		return 2;
	}

	xWordLen = Get_WordLen(x, xWordLen);
	//nWordLen = Get_WordLen(n, nWordLen);

	if(Big_MontPartInv(x, xWordLen, n, nWordLen, xinv, xinvWordLen, &k))
	{
		return 1;
	}

	xx = (U32 *)malloc(((*xinvWordLen)+1)*4);         //make sure addition OK
	U32_Clear(xx, (*xinvWordLen)+1);

	U32_Copy(xx, xinv, *xinvWordLen);
	U32_Clear(xinv, nWordLen);

	for(i=0; i<k; i++)
	{
		if(xx[0]&1)
		{
			*xinvWordLen = Big_Add(xx, *xinvWordLen, n, nWordLen);
		}
		*xinvWordLen = Big_Div2(xx, *xinvWordLen);
	}

	U32_Copy(xinv, xx, nWordLen);  //not U32_Copy(xinv, xx, *xinvWordLen); to avoid the case that high part all zero

	free(xx);
	return 0;
}

/* Function: a = qb + r, and q is integer pary of a/b, r = a mod b.
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 *     q ----------- big integer q
 *     qWordLen ---- word length of q
 *     r ----------- big integer r
 *     rWordLen ---- word length of r
 * Return: 
 *     0: all OK, 1: b is 0, illegal.
 * Caution:
 *     1. b can not be 0
 */
U8 Big_Div(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 q[], U32 *qWordLen, U32 r[], U32 *rWordLen)
{
	U8 flag;
	U32 i, aaWordLen, aBitLen, bBitLen;


	aWordLen = Get_WordLen(a, aWordLen);
	bWordLen = Get_WordLen(b, bWordLen);

	if(bWordLen == 0)                       // b==0
	{
		return 1;
	}
	else if((bWordLen == 1)&&(b[0] == 1))   // b==1
	{
		U32_Copy(q, a, aWordLen); //q=a
		*qWordLen = aWordLen;
		U32_Clear(r, bWordLen);   //r=0
		*rWordLen = 0;
		return 0;
	}
	else if(aWordLen == 0)                  // a==0
	{
		*qWordLen = 0;            //q=0
		U32_Clear(r, bWordLen);   //r=0
		*rWordLen = 0;
		return 0;
	}

	flag = Big_Compare(a, aWordLen, b, bWordLen);
	if(flag == 0)                           // a==b
	{
		*qWordLen = 1;            //q=1
		q[0] = 1;
		U32_Clear(r, bWordLen);   //r=0
		*rWordLen = 0;
		return 0;
	}
	else if(flag == 2)                      // a<b
	{
		*qWordLen = 0;             //q=0
		U32_Copy(r, a, bWordLen);  //r=a   //not U32_Copy(r, a, aWordLen);   to avoid the case that high part all zero
		*rWordLen = aWordLen;
		return 0;
	}

	// now a>b>1
	aaWordLen = (b[bWordLen-1]&0x80000000)?bWordLen+1:bWordLen;
	U32 *aa = (U32 *)malloc((aaWordLen)*4);
	U32_Copy(aa, a+aWordLen-bWordLen, bWordLen);
	aBitLen = Get_BitLen(aa, bWordLen);
	bBitLen = Get_BitLen(b, bWordLen);
	if(aBitLen > bBitLen)          // |aBitLen-bBitLen|<32
	{
		Big_Div2n(aa, bWordLen, aBitLen-bBitLen);
	}
	else if(aBitLen < bBitLen)     // now aWordLen > bWordLen > 0
	{
		Big_Mul2n(aa, bWordLen, bBitLen-aBitLen);
		aa[0] |= a[aWordLen-bWordLen-1]>>(32-(bBitLen-aBitLen));
	}

	aBitLen = Get_BitLen(a, aWordLen);
	aaWordLen = bWordLen;
	q[0] = 0;
	*qWordLen = 1;
	
	if(Big_Compare(aa, aaWordLen, b, bWordLen)<2)       // aa>=b
	{
		aaWordLen = Big_Sub(aa, aaWordLen, b, bWordLen);
		q[0] |= 1;
	}
	for(i=aBitLen-bBitLen; i>0; i--)
	{
		aaWordLen = Big_Mul2(aa, aaWordLen);
		if(Get_BitValue(a, i))
		{
			aa[0] |= 1;
			if(!aaWordLen)   //for aa == 0
			{
				aaWordLen = 1;
			}
		}

		*qWordLen = Big_Mul2(q, *qWordLen);
		if(Big_Compare(aa, aaWordLen, b, bWordLen)<2)   // aa>=b
		{
			aaWordLen = Big_Sub(aa, aaWordLen, b, bWordLen);
			q[0] |= 1;
		}
	}

	U32_Copy(r, aa, bWordLen);   //not U32_Copy(r, aa, aaWordLen);  to avoid the case that high part all zero
	*rWordLen = aaWordLen;

	free(aa);
	return 0;
}

/* Function: a = qb + r, and q is integer pary of a/b, r = a mod b. here b is a U32 integer
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- U32 integer b
 *     q ----------- big integer q
 *     qWordLen ---- word length of q
 *     r ----------- U32 integer r
 * Return: 
 *     0: all OK, 1: b is 0, illegal.
 * Caution:
 *     1. b can not be 0
 */
U8 Big_Div_U32(U32 a[], U32 aWordLen, U32 b, U32 q[], U32 *qWordLen, U32 r[])
{
	U8 flag;
	S32 i;
	U32 s, r0;
	U64 carry, a1, a2, high1;

	//aWordLen = Get_WordLen(a, aWordLen);   //Not affected

	if(b == 0)                              // b==0
	{
		return 1;
	}
	else if(b == 1)                         // b==1
	{
		U32_Copy(q, a, aWordLen);
		*qWordLen = aWordLen;
		*r = 0;
		return 0;
	}
	else if(aWordLen == 0)                  // a==0
	{
		*qWordLen = 0;            //q=0
		*r = 0;                   //r=0
		return 0;
	}

	flag = Big_Compare(a, aWordLen, &b, 1);
	if(flag == 0)                           // a==b
	{
		*qWordLen = 1;
		q[0] = 1;
		*r = 0;
		return 0;
	}
	else if(flag == 2)                      // a<b
	{
		*qWordLen = 0;
		*r = a[0];
		return 0;
	}

	carry = 0;
	s = 0xFFFFFFFF/b;
	r0 = (0xFFFFFFFF%b) + 1;                  //r0 < 0x100000000
	for(i=(aWordLen-1); i>=0; i--)
	{
		a1=carry*r0+a[i];                   // fold the carry (times 2^32) into the next dividend word
		q[i]=(U32)(carry*s);                // initial quotient-word estimate q[i] = carry * floor(0xFFFFFFFF/b)
		while(a1>0xFFFFFFFF)
		{
			a2=a1&0xFFFFFFFF;
			high1=a1>> 32;
			a1=high1*r0+a2;
			q[i]+=(U32)(high1*s);                  // refine the estimate for the high part of a1
		}
		a2=(a1*s)>> 32;
		q[i]+=(U32)a2;                           // a2 approximates a1/b (either a1/b or a1/b - 1)
		carry=a1-a2*b;
		if(carry>=b)
		{
			carry-=b;
			q[i]+=1;                        // correct the quotient when the remainder still exceeds b
		}
	}

	*qWordLen = aWordLen;
	if(q[aWordLen-1]==0)
		(*qWordLen)--;

	*r = (U32)carry;

	return 0;
}

/* Function: c = a mod b, namely c = a % b. 
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 *     c ----------- big integer c
 *     cWordLen ---- as input, *cWordLen is word length of buffer c; as output, *cWordLen is word length of big integer c
 * Return: 
 *     0: all OK, 1: b is 0, illegal.
 * Caution:
 *     1. b can not be 0
 *     2. on input *cWordLen is the buffer word length of c; on output it is the word length of c
 *     3. if a>=b, please make sure sizeof(c) >= sizeof(b); and if a<b, please make sure sizeof(c) >= sizeof(a)
 */
U8 Big_Mod_1(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 c[], U32 *cWordLen)
{
	U8 flag;
	U32 i, aaWordLen, aBitLen, bBitLen;

	aWordLen = Get_WordLen(a, aWordLen);
	bWordLen = Get_WordLen(b, bWordLen);

	if(bWordLen == 0)                       // b==0
	{
		return 1;
	}
	else if((bWordLen == 1)&&(b[0] == 1))   // b==1
	{
		U32_Clear(c, *cWordLen);
		*cWordLen = 0;
		return 0;
	}
	else if(aWordLen == 0)                  // a==0
	{
		U32_Clear(c, *cWordLen);   //c=0
		*cWordLen = 0;
		return 0;
	}

	flag = Big_Compare(a, aWordLen, b, bWordLen);
	if(flag == 0)                           // a==b
	{
		U32_Clear(c, *cWordLen);
		*cWordLen = 0;
		return 0;
	}
	else if(flag == 2)                      // a<b
	{
		if(c != a)
		{
			U32_Copy(c, a, aWordLen);  //not U32_Copy(c, a, aWordLen);   to avoid the case that high part all zero
			U32_Clear(c+aWordLen,*cWordLen-aWordLen);
		}
		*cWordLen = aWordLen;
		return 0;
	}

	// now a>b>1
	aaWordLen = (b[bWordLen-1]&0x80000000)?bWordLen+1:bWordLen;
	U32 *aa = (U32 *)malloc((aaWordLen)*4);
	U32_Copy(aa, a+aWordLen-bWordLen, bWordLen);
	aBitLen = Get_BitLen(aa, bWordLen);
	bBitLen = Get_BitLen(b, bWordLen);
	if(aBitLen > bBitLen)          // |aBitLen-bBitLen|<32
	{
		Big_Div2n(aa, bWordLen, aBitLen-bBitLen);
	}
	else if(aBitLen < bBitLen)     // now aWordLen > bWordLen > 0
	{
		Big_Mul2n(aa, bWordLen, bBitLen-aBitLen);
		aa[0] |= a[aWordLen-bWordLen-1]>>(32-(bBitLen-aBitLen));
	}

	aBitLen = Get_BitLen(a, aWordLen);
	aaWordLen = bWordLen;
	
	if(Big_Compare(aa, aaWordLen, b, bWordLen)<2)       // aa>=b
	{
		aaWordLen = Big_Sub(aa, aaWordLen, b, bWordLen);
	}
	for(i=aBitLen-bBitLen; i>0; i--)
	{
		aaWordLen = Big_Mul2(aa, aaWordLen);
		if(Get_BitValue(a, i))
		{
			aa[0] |= 1;
			if(!aaWordLen)
			{
				aaWordLen = 1;
			}
		}

		if(Big_Compare(aa, aaWordLen, b, bWordLen)<2)   // aa>=b
		{
			aaWordLen = Big_Sub(aa, aaWordLen, b, bWordLen);
		}
	}

	U32_Copy(c, aa, bWordLen);   //not U32_Copy(c, aa, aaWordLen);  to avoid the case that high part all zero
	*cWordLen = aaWordLen;

	free(aa);
	return 0;
}

/* Function: c = a mod b, namely c = a % b. 
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 *     c ----------- big integer c
 *     cWordLen ---- word length of c
 * Return: 
 *     0: all OK, 1: b is 0, illegal.
 * Caution:
 *     1. b can not be 0
 */
U8 Big_Mod_2(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 c[], U32 *cWordLen)
{
	U8 flag;
	S32 i;
	U32 aaWordLen, bbWordLen, aaBitLen, bbBitLen;


	aWordLen = Get_WordLen(a, aWordLen);
	bWordLen = Get_WordLen(b, bWordLen);

	if(bWordLen == 0)                       // b==0
	{
		return 1;
	}
	else if((bWordLen == 1)&&(b[0] == 1))   // b==1
	{
		U32_Clear(c, bWordLen);   //c=0
		*cWordLen = 0;
		return 0;
	}
	else if(aWordLen == 0)                  // a==0
	{
		U32_Clear(c, bWordLen);   //c=0
		*cWordLen = 0;
		return 0;
	}

	flag = Big_Compare(a, aWordLen, b, bWordLen);
	if(flag == 0)                           // a==b
	{
		U32_Clear(c, bWordLen);   //c=0
		*cWordLen = 0;
		return 0;
	}
	else if(flag == 2)                      // a<b
	{
		U32_Copy(c, a, bWordLen); //not U32_Copy(c, a, aWordLen);  to avoid the case that high part all zero
		*cWordLen = aWordLen;
		return 0;
	}

	// now a>b>1
	U32 *aa = (U32 *)malloc((aWordLen)*4);
	U32_Copy(aa, a, aWordLen);
	U32 *bb = (U32 *)malloc((aWordLen)*4);
	U32_Clear(bb, aWordLen);
	U32_Copy(bb+aWordLen-bWordLen, b, bWordLen);

	aaBitLen = Get_BitLen(aa, aWordLen);
	bbBitLen = Get_BitLen(bb, aWordLen);
	if(aaBitLen > bbBitLen)          // |aBitLen-bBitLen|<32
	{
		Big_Mul2n(bb+aWordLen-bWordLen, bWordLen, aaBitLen-bbBitLen);
		
	}
	else if(aaBitLen < bbBitLen)     // now aWordLen > bWordLen > 0
	{
		Big_Div2n(bb+aWordLen-bWordLen-1, bWordLen+1, bbBitLen-aaBitLen);
	}

	aaWordLen=bbWordLen=aWordLen;
	bbBitLen = Get_BitLen(b, bWordLen);
	for(i=aaBitLen-bbBitLen; i>=0; i--)
	{
		if(Big_Compare(aa, aaWordLen, bb, bbWordLen)<2)       // aa>=bb
		{
			aaWordLen = Big_Sub(aa, aaWordLen, bb, bbWordLen);
		}
		bbWordLen = Big_Div2(bb, bbWordLen);
	}

	U32_Copy(c, aa, bWordLen);   //not U32_Copy(c, aa, aaWordLen);  to avoid the case that high part all zero
	*cWordLen = aaWordLen;

	free(aa);
	free(bb);
	return 0;
}

/* Function: c = a mod b, namely c = a % b. 
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- U32 integer b
 *     c ----------- U32 integer c
 * Return: 
 *     0: all OK, 1: b is 0, illegal.
 * Caution:
 *     1. b can not be 0
 *     2. make sure aWordLen > 1
 */
U8 Big_Mod_U32_1(U32 a[], U32 aWordLen, U32 b, U32 *c)
{
	S32 i;
	U64 aa;

	if(b == 0)                       // b==0
	{
		return 1;
	}
	else if(b == 1)                  // b==1
	{
		*c = 0;
		return 0;
	}

	aa = a[aWordLen - 1]%b;
	for(i=aWordLen-2; i>=0; i--)
	{
		aa <<= 32;
		aa |= a[i];
		aa %= b;
	}
	*c=(U32)aa;

	return 0;
}

/* Function: U64 a mod U16 b, namely U64 a % U16 b. 
 * Parameters:
 *     a ----------- U64 integer a
 *     b ----------- U16 integer b
 * Return: 
 *     U64 a mod U16 b.
 * Caution:
 *     1. b can not be 0
 */
U32 Mod64_16(U64 a, U64 b)
{
	S8 i;
	U8 len1, len2;
	U64 tmp, tmp1;
	
	if(a < b)
	{
		return (U32)a;
	}

	len1 = 0;
	tmp = 0x8000000000000000;
	while(!(a&tmp))
	{
		len1++;
		tmp >>= 1;
	}
	
	len2 = 48;
	tmp = 0x8000;
	while(!(b&tmp))
	{
		len2++;
		tmp >>= 1;
	}
	
	tmp = a;
	for(i = (len2-len1); i >= 0; i--)
	{
		tmp1 = b<<i;
		if(tmp >= tmp1)
		{
			tmp -= tmp1;
		}
	}
	
	return (U32)tmp;
}

/* Function: U64 a mod U32 b, namely U64 a % U32 b. 
 * Parameters:
 *     a ----------- U64 integer a
 *     b ----------- U32 integer b
 * Return: 
 *     U64 a mod U32 b.
 * Caution:
 *     1. b can not be 0
 *     2. actually b is U32 integer, set U64 for convenience.
 */
U32 Mod64_32(U64 a, U64 b)
{
	S8 i;
	U8 len1, len2;
	U64 tmp, tmp1;;
	
	if(a < b)
	{
		return (U32)a;
	}

	len1 = 0;
	tmp = 0x8000000000000000;
	while(!(a&tmp))
	{
		len1++;
		tmp >>= 1;
	}
	
	len2 = 32;
	tmp = 0x80000000;
	while(!(b&tmp))
	{
		len2++;
		tmp >>= 1;
	}	
	
	tmp = a;
	for(i = (len2-len1); i >= 0; i--)
	{
		tmp1 = b<<i;
		if(tmp >= tmp1)
		{
			tmp -= tmp1;
		}
	}
	
	return (U32)tmp;
}

/* Function: c = a mod b, namely c = a % b. 
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- U32 integer b
 *     c ----------- U32 integer c
 * Return: 
 *     0: all OK, 1: b is 0, illegal.
 * Caution:
 *     1. b can not be 0
 */
U8 Big_Mod_U32_2(U32 a[], U32 aWordLen, U32 b, U32 *c)
{
	S32 i;
	U64 aa;

	if(b == 0)                       // b==0
	{
		return 1;
	}
	else if(b == 1)                  // b==1
	{
		*c = 0;
		return 0;
	}

	aa = a[aWordLen - 1]%b;
	for(i=aWordLen-2; i>=0; i--)
	{
		aa <<= 32;
		aa |= a[i];
		aa = Mod64_32(aa, b);  //aa %= b;
	}
	*c=(U32)aa;

	return 0;
}

/* Function: c = a mod b, namely c = a % b. 
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- U32 integer b
 *     c ----------- U32 integer c
 * Return: 
 *     0: all OK, 1: b is 0, illegal.
 * Caution:
 *     1. b can not be 0
 */
U8 Big_Mod_U32_3(U32 a[], U32 aWordLen, U32 b, U32 *c)
{
	S32 i;
	U32 s, r0;
	U64 carry, a1, a2, high1;

	if(b == 0)                       // b==0
	{
		return 1;
	}
	else if(b == 1)                  // b==1
	{
		*c = 0;
		return 0;
	}

	s = 0xFFFFFFFF/b;
	r0 = (0xFFFFFFFF%b)+1;
	carry = 0;
	for (i = (aWordLen - 1); i >= 0; i--)
	{
		a1=carry*r0+a[i];           // fold the carry (times 2^32) into the next dividend word
		while(a1>0xFFFFFFFF)
		{
			a2=a1&0xFFFFFFFF;
			high1=a1>>32;
			a1=high1*r0+a2;
		}
		a2=(a1*s)>>32;
		carry=a1-a2*b;
		if(carry>=b)
		{
			carry -= b;
		}
	}
	*c=(U32)carry;

	return 0;
}

/* Function: c = a mod b, namely c = a % b. 
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- U16 integer b
 *     c ----------- U16 integer c
 * Return: 
 *     0: all OK, 1: b is 0, illegal.
 * Caution:
 *     1. b can not be 0
 */
U8 Big_Mod_U16(U32 a[], U32 aWordLen, U16 b, U16 *c)
{
	S32 i;
	U64 aa;

	if(b == 0)                       // b==0
	{
		return 1;
	}
	else if(b == 1)                  // b==1
	{
		*c = 0;
		return 0;
	}

	aa = a[aWordLen - 1]%b;
	for(i=aWordLen-2; i>=0; i--)
	{
		aa <<= 32;
		aa |= a[i];
		aa = Mod64_16(aa, b);  //aa %= b;
	}
	*c=(U16)aa;

	return 0;
}

/* Function: c = a * b
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 *     c ----------- big integer c
 *     cWordLen ---- word length of c
 * Return: 
 * Caution:
 *     1. a and b can be the same buffer
 */
void Big_Mul_1(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 c[], U32 *cWordLen)
{
	U64 UV;
	U32 i,j,*U,*V;

	U32_Clear(c, aWordLen+bWordLen);

	//aWordLen = Get_WordLen(a, aWordLen);
	//bWordLen = Get_WordLen(b, bWordLen);

	if(!aWordLen || !bWordLen)
	{
		*cWordLen = 0;
		return ;
	}

	U32_Clear(c, bWordLen);

	V = (U32 *)(&UV);
	U = V+1;
	for(i=0; i<aWordLen; i++)
	{
		*U = 0;
		for(j=0; j<bWordLen; j++)
		{
			UV = ((U64)a[i])*b[j]+c[i+j]+(*U);
			c[i+j] = (*V);
		}
		c[i+j] = (*U);
	}

	*cWordLen = Get_WordLen(c, aWordLen+bWordLen);

	return ;
}

/* Function: c = a * b
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 *     c ----------- big integer c
 *     cWordLen ---- word length of c
 * Return: 
 * Caution:
 *     1. a and b can be the same buffer
 */
void Big_Mul_2(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 c[], U32 *cWordLen)
{
	U64 UV, *RR;
	U32 R[3]={0}, *U;
	U32 k, i, wordLen;

	U32_Clear(c, aWordLen+bWordLen);

	//aWordLen = Get_WordLen(a, aWordLen);
	//bWordLen = Get_WordLen(b, bWordLen);

	if(!aWordLen || !bWordLen)
	{
		*cWordLen = 0;
		return ;
	}

	RR = (U64 *)R;
	U = (U32 *)(&UV)+1;

	wordLen = aWordLen+bWordLen-1;
	for(k=0; k<wordLen; k++)
	{
		for(i=0; (i<=k)&&(i<aWordLen); i++)   // accumulate the partial products a[i]*b[k-i] on diagonal k
		{                                     // i is bounded by i<=k and i<aWordLen
			if((k-i)>=bWordLen)               // skip when the b index (k-i) is out of range
			{                                 // here j=k-i is beyond b, so there is no b[k-i]
				continue;
			}

			UV = ((U64)a[i])*b[k-i];
			(*RR) += UV;
			if((*RR) < UV)
			{
				R[2]++;
			}
		}
		c[k] = R[0];
		R[0] = R[1];
		R[1] = R[2];
		R[2] = 0;
	}
	c[k] = R[0];

	*cWordLen = Get_WordLen(c, aWordLen+bWordLen);

	return ;
}

/* Function: c = a * a
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     c ----------- big integer c
 *     cWordLen ---- word length of c
 * Return: 
 * Caution:
 *     1. a and c can not be the same buffer
 */
void Big_Square(U32 a[], U32 aWordLen, U32 c[], U32 *cWordLen)
{
	U64 UV, *RR;
	U32 R[3]={0}, *U;
	U32 k, i, wordLen;

	U32_Clear(c, aWordLen>>1);

	//aWordLen = Get_WordLen(a, aWordLen);

	if(!aWordLen)
	{
		*cWordLen = 0;
		return ;
	}

	RR = (U64 *)R;
	U = (U32 *)(&UV)+1;

	wordLen = (aWordLen<<1)-1;
	for(k=0; k<wordLen; k++)
	{
		for(i=0; (i<=k)&&(i<aWordLen)&&(i<=k-i); i++)
		{
			if((k-i)>=aWordLen)
			{
				continue;
			}

			UV = ((U64)a[i])*a[k-i];

			if(i<k-i)
			{
				if((*U)&0x80000000)
				{
					R[2]++;
				}
				UV <<= 1; 
			}

			(*RR) += UV;
			if((*RR) < UV)
			{
				R[2]++;
			}
		}
		c[k] = R[0];
		R[0] = R[1];
		R[1] = R[2];
		R[2] = 0;
	}
	c[k] = R[0];

	*cWordLen = Get_WordLen(c, aWordLen<<1);

	return ;
}

/* Function: c = ab mod n, namely c = ab % n. 
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 *     n ----------- big integer n
 *     nWordLen ---- word length of n
 *     c ----------- big integer c
 *     cWordLen ---- word length of c
 * Return: 
 *     0: all OK, 1: n is 0
 * Caution:
 *     1. n can not be 0
 *     2. word length of buffer c must >= nWordLen
 */
U8 Big_ModMul_1(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 n[], U32 nWordLen, U32 c[], U32 *cWordLen)
{
	U32 *T;
	U32 wordLen;
	U8 flag;

	T = (U32 *)malloc((aWordLen+bWordLen)*4);

	Big_Mul_2(a, aWordLen, b, bWordLen, T,  &wordLen);
	if(wordLen == 0)           //a or b is 0
	{
		U32_Clear(c, nWordLen);
		*cWordLen = 0;
		free(T);
		return 0;
	}

	*cWordLen = nWordLen; 
	flag = Big_Mod_1(T, wordLen, n, nWordLen, c, cWordLen);

	free(T);
	return flag;
}

U32 n1=0;        //n1 = - N[0]^(-1) mod 2^32
U8 n1_flag=0;    //n1_flag is LSB 32bits of module N

/* Function: n1 = - N[0]^(-1) mod 2^32
 * Parameters:
 *     N ----------- big integer N
 * Return:
 * Caution:
 */
void MontCal_n1(U32 N[])
{
	U32 T[2]={0,1};
	U32 x[2]={0};

	n1_flag = N[0];
	Big_ModInv(N, 1, T, 2, x, T+1);      // compute x = N^(-1) mod 2^32 (T and T+1 are scratch); then n1 = -x
	n1 = 0 - x[0];
}

/* Function: when first time to use N or N changed, call this function
 * Parameters:
 * Return: 
 * Caution:
 *//*
void Change_N()
{
	n1_flag = 0;
}
*/
/* Function: A = Mont(A,A,N)
 * Parameters:
 *     A ----------- big integer A
 *     AWordLen ---- word length of A
 *     N ----------- big integer N
 *     NWordLen ---- word length of N
 * Return: 
 *     0: all OK, 1: error(N is even)
 * Caution:
 *     1. make sure the given word length is the actual word length of the operand (most significant word is not 0)
 *     2. N can not be even
 *     3. make sure A < N, and A[0], A[1], ... A[NWordLen - 1] available.
 */
U8 MontMulFIPS_AA(U32 A[], U32 *AWordLen, U32 N[], U32 NWordLen)
{
	U32 T[3] = {0};
	U32 wordLen, i, j;
	U64 CS;
	U32 *m, *C=((U32 *)&CS)+1;
	U64 * TT = (U64 *)T;


	if(!(N[0]&1))                                        //N can not be even
		return 1;

	//*AWordLen = Get_WordLen(A, *AWordLen);

	if(!(*AWordLen))                                     //A is 0
	{
		U32_Clear(A,NWordLen);
		return 0;
	}

	if(n1_flag != N[0])                                  //if n1_flag changed, then compute it
	{
		MontCal_n1(N);
	}

	m = (U32 *)malloc((NWordLen)*4);
	for(i=0; i<NWordLen; i++)
	{
		for(j=0; j<=i; j++)
		{
			if(j>(i-j) || j>=(*AWordLen) || (i-j)>=(*AWordLen))
			{
				goto _M_N_1; 
			}
			else
			{
				CS = (U64)A[j]*A[i-j];
				if(j<i-j)
				{
					if((*C) & 0x80000000)
						T[2]+=1;
					CS<<=1;
				}
				(*TT)+=CS;
				if((*TT)<CS)
				{
					T[2]+=1;
				}
			}
_M_N_1: 	
			if(j==i)
				continue;

			CS = (U64)m[j]*N[i-j];
			(*TT)+=CS;
			if((*TT)<CS)
			{
				T[2]+=1;
			}
		}

		m[i] = T[0]*n1; 
		CS = (U64)m[i]*N[0]; 
		(*TT)+=CS;
		if((*TT)<CS)
		{
			T[2]+=1;
		}

		T[0] = T[1];
		T[1] = T[2];
		T[2] = 0;
	}

	wordLen = (NWordLen<<1);
	for(i=NWordLen; i<wordLen; i++)
	{
		for(j=i-(NWordLen-1); j<NWordLen; j++)
		{
			if(j>(i-j) || j>=(*AWordLen) || (i-j)>=(*AWordLen))
			{
				goto _M_N_2;
			}
			else
			{
				CS = (U64)A[j]*A[i-j];
				if(j<i-j)
				{
					if((*C) & 0x80000000)
						T[2]+=1;
					CS<<=1;
				}
				(*TT)+=CS;
				if((*TT)<CS)
				{
					T[2]+=1;
				}
			}
_M_N_2:
			CS = (U64)m[j]*N[i-j];
			(*TT)+=CS;
			if((*TT)<CS)
			{
				T[2]+=1;
			}
		}

		A[i-NWordLen] = T[0];
		T[0] = T[1];
		T[1] = T[2];
		T[2] = 0;
	}

	if(T[0]==1 || Big_Compare(A, NWordLen, N, NWordLen) < 2)       // 2N > T >= N (2N > T > 0)
	{
		*AWordLen = Big_Sub(A, NWordLen, N, NWordLen);
	}
	else                                                           // N > T
	{
		*AWordLen = Get_WordLen(A, NWordLen);
	}

	free(m);
	return 0;
}

/* Function: A = Mont(A,B,N)
 * Parameters:
 *     A ----------- big integer A
 *     AWordLen ---- word length of A
 *     B ----------- big integer B
 *     BWordLen ---- word length of B
 *     N ----------- big integer N
 *     NWordLen ---- word length of N
 * Return: 
 *     0: all OK, 1: error(N is even)
 * Caution:
 *     1. make sure the given word length is the actual word length of the operand (most significant word is not 0)
 *     2. N can not be even
 *     3. make sure A < N, and A[0], A[1], ... A[NWordLen - 1] available.
 *     4. A and B can be the same buffer
 */
U8 MontMulFIPS_AB(U32 A[], U32 *AWordLen, U32 B[], U32 BWordLen, U32 N[], U32 NWordLen)
{
	U32 T[3] = {0};
	U32 wordLen, i, j;
	U64 CS;
	U32 *m, *C=((U32 *)&CS)+1;
	U64 * TT = (U64 *)T;


	if(!(N[0]&1))                                        //N can not be even
		return 1;

	//*AWordLen = Get_WordLen(A, *AWordLen);
	//BWordLen = Get_WordLen(B, BWordLen);

	if(!(*AWordLen) || !BWordLen)                        //A or B is 0
	{
		U32_Clear(A,NWordLen);
		return 0;
	}

	if(n1_flag != N[0])                                  //if n1_flag changed, then compute it
	{
		MontCal_n1(N);
	}

	m = (U32 *)malloc((NWordLen)*4);
	for(i=0; i<NWordLen; i++)
	{
		for(j=0; j<=i; j++)
		{
			if(j>=(*AWordLen) || (i-j)>=BWordLen)
			{
				goto _M_N_1; 
			}
			else
			{
				CS = (U64)A[j]*B[i-j];
				(*TT)+=CS;
				if((*TT)<CS)
				{
					T[2]+=1;
				}
			}
_M_N_1: 	
			if(j==i)
				continue;

			CS = (U64)m[j]*N[i-j];
			(*TT)+=CS;
			if((*TT)<CS)
			{
				T[2]+=1;
			}
		}

		m[i] = T[0]*n1;
		CS = (U64)m[i]*N[0];
		(*TT)+=CS;
		if((*TT)<CS)
		{
			T[2]+=1;
		}

		T[0] = T[1];
		T[1] = T[2];
		T[2] = 0;
	}

	wordLen = (NWordLen<<1);
	for(i=NWordLen; i<wordLen; i++)
	{
		for(j=i-(NWordLen-1); j<NWordLen; j++)
		{
			if(j>=(*AWordLen) || (i-j)>=BWordLen)
			{
				goto _M_N_2;
			}
			else
			{
				CS = (U64)A[j]*B[i-j];
				(*TT)+=CS;
				if((*TT)<CS)
				{
					T[2]+=1;
				}
			}
_M_N_2:
			CS = (U64)m[j]*N[i-j];
			(*TT)+=CS;
			if((*TT)<CS)
			{
				T[2]+=1;
			}
		}

		A[i-NWordLen] = T[0];
		T[0] = T[1];
		T[1] = T[2];
		T[2] = 0;
	}

	if(T[0]==1 || Big_Compare(A, NWordLen, N, NWordLen) < 2)       // 2N > T >= N (2N > T > 0)
	{
		*AWordLen = Big_Sub(A, NWordLen, N, NWordLen);
	}
	else                                                           // N > T
	{
		*AWordLen = Get_WordLen(A, NWordLen);
	}

	free(m);
	return 0;
}

/* Function: out = Mont(A,B,N)
 * Parameters:
 *     A ----------- big integer A
 *     AWordLen ---- word length of A
 *     B ----------- big integer B
 *     BWordLen ---- word length of B
 *     N ----------- big integer N
 *     NWordLen ---- word length of N
 *     out --------- big integer out
 *     NWordLen ---- word length of out
 * Return: 
 *     0: all OK, 1: error(N is even)
 * Caution:
 *     1. make sure the given word length is the actual word length of the operand (most significant word is not 0)
 *     2. N can not be even
 *     3. make sure A < N, and A[0], A[1], ... A[NWordLen - 1] available.
 *     4. A and B can be the same buffer
 *     5. the word lengths of A and B are treated as NWordLen (pad the high words with 0)
 *     6. out must have a buffer of NWordLen words (high words set to 0)
 */
U8 MontMulFIPS_AB_(U32 A[], U32 AWordLen, U32 B[], U32 BWordLen, U32 N[], U32 NWordLen, U32 out[], U32 * outWordLen)
{
	U32 T[3] = {0};
	U32 wordLen, i, j;
	U64 CS;
	U32 *m;
	//U32 *m, *C=((U32 *)&CS)+1;
	U64 * TT = (U64 *)T;


	if(!(N[0]&1))                                        //N can not be even
		return 1;

	//*AWordLen = Get_WordLen(A, *AWordLen);
	//BWordLen = Get_WordLen(B, BWordLen);

	if(!(AWordLen) || !BWordLen)                        //A or B is 0
	{
		U32_Clear(A,NWordLen);
		return 0;
	}

	if(n1_flag != N[0])                                  //if n1_flag changed, then compute it
	{
		MontCal_n1(N);
	}

	m = (U32 *)malloc((NWordLen)*4);
	for(i=0; i<NWordLen; i++)
	{
		for(j=0; j<=i; j++)
		{
			if(j>=(AWordLen) || (i-j)>=BWordLen)
			{
				goto _M_N_1; 
			}
			else
			{
				CS = (U64)A[j]*B[i-j];
				(*TT)+=CS;
				if((*TT)<CS)
				{
					T[2]+=1;
				}
			}
_M_N_1: 	
			if(j==i)
				continue;

			CS = (U64)m[j]*N[i-j];
			(*TT)+=CS;
			if((*TT)<CS)
			{
				T[2]+=1;
			}
		}

		m[i] = T[0]*n1;
		CS = (U64)m[i]*N[0];
		(*TT)+=CS;
		if((*TT)<CS)
		{
			T[2]+=1;
		}

		T[0] = T[1];
		T[1] = T[2];
		T[2] = 0;
	}

	wordLen = (NWordLen<<1);
	for(i=NWordLen; i<wordLen; i++)
	{
		for(j=i-(NWordLen-1); j<NWordLen; j++)
		{
			if(j>=(AWordLen) || (i-j)>=BWordLen)
			{
				goto _M_N_2;
			}
			else
			{
				CS = (U64)A[j]*B[i-j];
				(*TT)+=CS;
				if((*TT)<CS)
				{
					T[2]+=1;
				}
			}
_M_N_2:
			CS = (U64)m[j]*N[i-j];
			(*TT)+=CS;
			if((*TT)<CS)
			{
				T[2]+=1;
			}
		}

		out[i-NWordLen] = T[0];
		T[0] = T[1];
		T[1] = T[2];
		T[2] = 0;
	}

	if(T[0]==1 || Big_Compare(out, NWordLen, N, NWordLen) < 2)       // 2N > T >= N (2N > T > 0)
	{
		*outWordLen = Big_Sub(out, NWordLen, N, NWordLen);
	}
	else                                                           // N > T
	{
		*outWordLen = Get_WordLen(out, NWordLen);
	}

	free(m);
	return 0;
}

/* Function: A = Mont(A,B,N)
 * Parameters:
 *     A ----------- big integer A
 *     AWordLen ---- word length of A
 *     B ----------- big integer B
 *     BWordLen ---- word length of B
 *     N ----------- big integer N
 *     NWordLen ---- word length of N
 * Return: 
 *     0: all OK, 1: error(N is even)
 * Caution:
 *     1. make sure the given word length is the actual word length of the operand (most significant word is not 0)
 *     2. N can not be even
 *     3. make sure A < N, and A[0], A[1], ... A[NWordLen - 1] available.
 *     4. A and B can be the same buffer
 */
U8 MontMulCIOS_AB(U32 A[], U32 *AWordLen, U32 B[], U32 BWordLen, U32 N[], U32 NWordLen)
{
	U32 m, i, j, TMPWordLen;
	U64 CS=0;
	U32 *TMP, *T, *S=((U32 *)&CS), *C=S+1;


	if(!(N[0]&1))                                        //N can not be even
		return 1;

	//*AWordLen = Get_WordLen(A, *AWordLen);
	//BWordLen = Get_WordLen(B, BWordLen);

	if(!(*AWordLen) || !BWordLen)                        //A or B is 0
	{
		U32_Clear(A,NWordLen);
		return 0;
	}

	if(n1_flag != N[0])                                  //if n1_flag changed, then compute it
	{
		MontCal_n1(N);
	}

	T = (U32 *)malloc((NWordLen+2)*4);
	U32_Clear(T, NWordLen+2);
	TMP = (U32 *)malloc((BWordLen+1)*4);
	for(i=0; i<NWordLen; i++)
	{
		if(i<(*AWordLen))
		{
			Big_Mul_2(A+i, 1, B, BWordLen, TMP, &TMPWordLen);
			Big_Add(T, NWordLen+2, TMP, TMPWordLen);
		}

		m = T[0]*n1;   
		CS = T[0] + (U64)m*N[0];   
		for(j=1; j<NWordLen; j++)
		{
			CS = (U64)m*N[j] + (*C) + T[j];
			T[j-1] = *S;
		}
		CS = (U64)(T[NWordLen]) + (*C);
		T[NWordLen-1] = *S;
		T[NWordLen] = T[NWordLen+1] + (*C);
		T[NWordLen+1] = 0;
	}

	U32_Copy(A, T, NWordLen);
	if(T[NWordLen]==1 || Big_Compare(A, NWordLen, N, NWordLen) < 2)       // 2N > T >= N (2N > T > 0)
	{
		*AWordLen = Big_Sub(A, NWordLen, N, NWordLen);
	}
	else                                                                  // N > T
	{
		*AWordLen = Get_WordLen(A, NWordLen);
	}

	free(T);
	free(TMP);
	return 0;
}

/* Function: a -> aR mod n 
 * Parameters:
 *     a ----------- input, big integer a mod n, namely a<n
 *     n ----------- input, must be odd
 *     aR ---------- output, aR mod n, namely aR<n
 *     nWordLen ---- input, buffer word length of a, n and aR
 * Return: none
 * Caution:
 *     1. a and aR can be the same buffer
 *     2. n must be odd
 */
void Big_a2aR_mod_n(U32 a[], U32 n[], U32 aR[], U32 nWordLen)
{
	U32 *R;
	U32 wordLen;

	nWordLen = Get_WordLen(n, nWordLen);
	R = (U32 *)malloc((nWordLen<<1)*4);
	U32_Clear(R, nWordLen);
	U32_Copy(R+nWordLen, a, nWordLen);
	Big_Mod_2(R, nWordLen<<1, n, nWordLen, aR, &wordLen);

	free(R);
}

/* Function: aR -> a mod n 
 * Parameters:
 *     aR ---------- input, big integer aR mod n, namely aR<n
 *     n ----------- input, must be odd
 *     a ----------- output, a mod n, namely a<n
 *     nWordLen ---- input, buffer word length of a, n and aR
 * Return: none
 * Caution:
 *     1. a and aR can be the same buffer
 *     2. n must be odd
 */
void Big_aR2a_mod_n(U32 aR[], U32 n[], U32 out[], U32 nWordLen)
{
	U32 b=1;
	U32 wordLen;

	nWordLen = Get_WordLen(n, nWordLen);
	wordLen = Get_WordLen(aR, nWordLen);

	MontMulFIPS_AB_(aR, wordLen, &b, 1, n, nWordLen, out, &wordLen);  //out = aR*1*R^(-1) mod n = a mod n.
}

/* Function: c = a^e mod n
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     e ----------- big integer e
 *     eWordLen ---- word length of e
 *     n ----------- big integer n
 *     nWordLen ---- word length of n
 *     c ----------- big integer c
 *     cWordLen ---- word length of c
 * Return: 
 *     0: all OK, 1: error(n==0 or n is even), 2: error(a=0 and e=0), 3: error(a > n)
 * Caution:
 *     1. n can not be 0, and can not be even
 *     2. a and e can not be 0 at the same time
 *     3. make sure A <= N, and A[0], A[1], ... A[NWordLen - 1] available.
 *     4. a and c can be the same buffer.
 */
U8 Big_ModExp(U32 a[], U32 aWordLen, U32 e[], U32 eWordLen, U32 n[], U32 nWordLen, U32 c[], U32 *cWordLen)
{
	U32 i, eBitLen, AWordLen, xWordLen;
	U32 *A=c, *R, *x;
	U8 flag;

	aWordLen = Get_WordLen(a, aWordLen);
	eWordLen = Get_WordLen(e, eWordLen);
//	nWordLen = Get_WordLen(n, nWordLen);

	if(!nWordLen || !(n[0]&1))     // n=0 or n is even
	{
		return 1;
	}
	else if(!aWordLen)
	{
		if(!eWordLen)              // a=0 and e=0
		{
			return 2;
		}
		U32_Clear(c, nWordLen);
		*cWordLen = 0;             // a=0 and e!=0
		return 0;
	}
	else if(!eWordLen)             // a!=0 and e=0
	{
		U32_Clear(c, nWordLen);
		c[0] = 1;
		*cWordLen = 1;
		return 0;
	}

	flag = Big_Compare(a, aWordLen, n, nWordLen);        //make sure A <= N
	if(flag == 0)
	{
		U32_Clear(c, nWordLen);
		*cWordLen = 0;
		return 0;
	}
	else if(flag == 1)
	{
		return 3;
	}

//	Change_N();                                                    //compute n1 = - N[0]^(-1) mod 2^32
	eBitLen = Get_BitLen(e, eWordLen);                             //get eBitLen

	R = (U32 *)malloc((nWordLen+aWordLen)*4);
	U32_Clear(R, nWordLen);
	U32_Copy(R+nWordLen, a, aWordLen);
	x = (U32 *)malloc((nWordLen)*4);
	Big_Mod_2(R, nWordLen+aWordLen, n, nWordLen, x, &xWordLen);    // x = aR mod n

#if 0

	AWordLen = nWordLen;
	U32_Clear(A, AWordLen);
	AWordLen = Big_Sub(A, AWordLen, n, nWordLen);                  // A = R mod N

	for(i=1; i<eBitLen; i++)
	{
		if(Get_BitValue(e, i))
		{
			MontMulCIOS_AB(A, &AWordLen, x, xWordLen, n, nWordLen);
		}
		MontMulFIPS_AA(x, &xWordLen, n, nWordLen);
	}
	MontMulCIOS_AB(A, &AWordLen, x, xWordLen, n, nWordLen);

#else

	U32_Copy(A, x, xWordLen);                                      // A = aR mod N
	AWordLen = xWordLen;
	for(i=eBitLen-1; i>0; i--)
	{
		MontMulFIPS_AA(A, &AWordLen, n, nWordLen);
		if(Get_BitValue(e, i))
		{
			MontMulCIOS_AB(A, &AWordLen, x, xWordLen, n, nWordLen);
		}	
	}

#endif

	x[0] = 1;
	MontMulCIOS_AB(A, &AWordLen, x, 1, n, nWordLen);

	*cWordLen = AWordLen;

	free(R);
	free(x);
	return 0;
}

/* Function: c = a^e mod n
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     e ----------- big integer e
 *     eWordLen ---- word length of e
 *     n ----------- big integer n
 *     nWordLen ---- word length of n
 *     c ----------- big integer c
 *     cWordLen ---- word length of c
 * Return: 
 *     0: all OK, 1: error(n==0 or n is even), 2: error(a=0 and e=0), 3: error(a > n)
 * Caution:
 *     1. n can not be 0, and can not be even
 *     2. a and e can not be 0 at the same time
 *     3. make sure A <= N, and A[0], A[1], ... A[NWordLen - 1] available.
 */
U8 Big_ModExp_SW(U32 a[], U32 aWordLen, U32 e[], U32 eWordLen, U32 n[], U32 nWordLen, U32 c[], U32 *cWordLen)
{
	U32 i, bitLen, eBitLen, AWordLen, xWordLen, x3WordLen, x5WordLen, x7WordLen;
	U32 *A=c, *R, *x, *x3, *x5, *x7;
	U8 flag, k;

	aWordLen = Get_WordLen(a, aWordLen);
	eWordLen = Get_WordLen(e, eWordLen);
//	nWordLen = Get_WordLen(n, nWordLen);

	if(!nWordLen || !(n[0]&1))     // n=0 or n is even
	{
		return 1;
	}
	else if(!aWordLen)
	{
		if(!eWordLen)              // a=0 and e=0
		{
			return 2;
		}
		U32_Clear(c, nWordLen);
		*cWordLen = 0;             // a=0 and e!=0
		return 0;
	}
	else if(!eWordLen)             // a!=0 and e=0
	{
		U32_Clear(c, nWordLen);
		c[0] = 1;
		*cWordLen = 1;
		return 0;
	}

	flag = Big_Compare(a, aWordLen, n, nWordLen);                   //make sure A <= N
	if(flag == 0)
	{
		U32_Clear(c, nWordLen);
		*cWordLen = 0;
		return 0;
	}
	else if(flag == 1)
	{
		return 3;
	}

//	Change_N();                                                     //compute n1 = - N[0]^(-1) mod 2^32
	eBitLen = Get_BitLen(e, eWordLen);                              //get eBitLen

	R = (U32 *)malloc((nWordLen+aWordLen)*4);
	U32_Clear(R, nWordLen);
	U32_Copy(R+nWordLen, a, aWordLen);
	x = (U32 *)malloc((nWordLen)*4);
	Big_Mod_2(R, nWordLen+aWordLen, n, nWordLen, x, &xWordLen);     // x = aR mod n

	x3 = (U32 *)malloc((nWordLen)*4);
	U32_Copy(x3, x, xWordLen);
	x3WordLen = xWordLen;
	MontMulFIPS_AA(x3, &x3WordLen, n, nWordLen);                    // (a^2)R mod n 

	x5 = (U32 *)malloc((nWordLen)*4);
	U32_Copy(x5, x3, x3WordLen);
	x5WordLen = x3WordLen;
	MontMulFIPS_AA(x5, &x5WordLen, n, nWordLen);                    // (a^4)R mod n 

	MontMulCIOS_AB(x3, &x3WordLen, x, xWordLen, n, nWordLen);       // (a^3)R mod n 
	MontMulCIOS_AB(x5, &x5WordLen, x, xWordLen, n, nWordLen);       // (a^5)R mod n 

	x7 = R;
	U32_Copy(x7, x3, x3WordLen);
	x7WordLen = x3WordLen;
	MontMulFIPS_AA(x7, &x7WordLen, n, nWordLen);                    // (a^6)R mod n 
	MontMulCIOS_AB(x7, &x7WordLen, x, xWordLen, n, nWordLen);       // (a^7)R mod n

	U32_Copy(A, x, xWordLen);
	AWordLen = xWordLen;
	U32_Clear(A+AWordLen, nWordLen-AWordLen);
	eBitLen--;
	while(eBitLen)
	{
		if(Get_BitValue(e, eBitLen))
		{
			flag = 1;
			if(eBitLen<3)
				bitLen = eBitLen-1;
			else
				bitLen = 2;

			for(i=1; i<=bitLen; i++)
			{
				flag<<=1;
				if(Get_BitValue(e, eBitLen-i))
					flag|=1;
			}
			for(i=1; i<=bitLen; i++)
			{
				if((flag&1)==0)
					flag>>=1;
				else
					break;
			}

			switch(flag)
			{
				case 1: MontMulFIPS_AA(A, &AWordLen, n, nWordLen);
						MontMulCIOS_AB(A, &AWordLen, x, xWordLen, n, nWordLen);
						eBitLen--;
					    break;

				case 3: for(k=0; k<2; k++)
							MontMulFIPS_AA(A, &AWordLen, n, nWordLen);
						MontMulCIOS_AB(A, &AWordLen, x3, x3WordLen, n, nWordLen);
						eBitLen-=2;
					    break;

				case 5: for(k=0; k<3; k++)
							MontMulFIPS_AA(A, &AWordLen, n, nWordLen);
						MontMulCIOS_AB(A, &AWordLen, x5, x5WordLen, n, nWordLen);
						eBitLen-=3;
					    break;

				case 7: for(k=0; k<3; k++)
							MontMulFIPS_AA(A, &AWordLen, n, nWordLen);
						MontMulCIOS_AB(A, &AWordLen, x7, x7WordLen, n, nWordLen);
						eBitLen-=3;
					    break;
			}
		}
		else
		{
			MontMulFIPS_AA(A, &AWordLen, n, nWordLen);
			eBitLen--;
		}
	}

	x[0] = 1;
	MontMulCIOS_AB(A, &AWordLen, x, 1, n, nWordLen);

	*cWordLen = AWordLen;

	free(R);
	free(x);
	free(x3);
	free(x5);
	return 0;
}

/* Function: c = ab mod n, namely c = ab % n. 
 * Parameters:
 *     a ----------- big integer a
 *     aWordLen ---- word length of a
 *     b ----------- big integer b
 *     bWordLen ---- word length of b
 *     n ----------- big integer n
 *     nWordLen ---- word length of n
 *     c ----------- big integer c
 *     cWordLen ---- word length of c
 * Return: 
 *     0: all OK, 1: n is 0 or n is even, 2: bWordLen>nWordLen
 * Caution:
 *     1. n can not be 0, and can not be even
 *     2. bWordLen can not be bigger than nWordLen
 *     3. a and b can be the same buffer, thus a==b, c = a^2 mod n
 *     4. a and c can be the same buffer
 *     5. b and c can not be the same buffer
 *     6. the word lengths of a and b are treated as nWordLen (pad the high words with 0)
 *     7. c must have a buffer of nWordLen words (high words set to 0)
 */
U8 Big_ModMul_2(U32 a[], U32 aWordLen, U32 b[], U32 bWordLen, U32 n[], U32 nWordLen, U32 c[], U32 *cWordLen)
{
	U32 *R;
	
	//aWordLen = Get_WordLen(a, aWordLen);
	//bWordLen = Get_WordLen(b, bWordLen);

	if(!nWordLen || !(n[0]&1))
	{
		return 1;
	}
	else if(!aWordLen || !bWordLen)
	{
		*cWordLen = 0;
		U32_Clear(c, nWordLen);
		return 0;
	}
	else if(bWordLen > nWordLen)
	{
		return 2;
	}

//	Change_N();                                          //compute n1 = - N[0]^(-1) mod 2^32

	R = (U32 *)malloc((nWordLen+aWordLen)*4);
	U32_Clear(R, nWordLen);
	U32_Copy(R+nWordLen, a, aWordLen);
	Big_Mod_2(R, nWordLen+aWordLen, n, nWordLen, c, cWordLen);    // c = aR mod n

	MontMulFIPS_AB(c, cWordLen, b, bWordLen, n, nWordLen);

	free(R);
	return 0;
}
