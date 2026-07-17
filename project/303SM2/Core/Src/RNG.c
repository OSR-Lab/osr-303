/*******************************************************************************
 ******     Copyright (c) 2014--2020 OSR.Co.Ltd. All rights reserved.     ****** 
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>          //rand and srand function
#include <time.h>

#if defined(__linux)
    #include <unistd.h>
#elif defined(_WIN32)
    #include <process.h>
#endif

#include "RNG.h"

static uint32_t state=0x77777777;

uint32_t genrand_uint32() {
	state = ((state * 1103515245) + 12345) & 0x7fffffff;
	return state;
}


/* Function: get U32 random number
 * Parameters:
 *     random ------ pointer to U32 random number
 *     wordLen ----- word length of random number
 * Return: 
 * Caution:
 */
void GetRandU32(U32 random[], U32 wordLen)
{
	static U32 count=0;
	U32 i;

	// set RNG seed
#if defined(__linux)
    srand(((U32)time(NULL) | getpid()) + count + rand());
	//srand(count);
#elif defined(_WIN32)
    srand(((U32)time(NULL) | _getpid()) + count + rand());
#endif
	for(i=0; i<wordLen; i++)
	{
		// random[i] = (rand()<<17) | (rand()<<2) | (rand()&3);    // for rand() < 0x7FFF
		random[i] = genrand_uint32();
	}

	count++;
}

/* Function: get U8 random number
 * Parameters:
 *     random ------ pointer to U8 random number
 *     byteLen ----- byte length of random number
 * Return: 
 * Caution:
 */
void GetRandU8(U8 random[], U32 byteLen)
{
	U32 rand[1];

	while(byteLen>=4)
	{
		GetRandU32(rand, 1);
		memcpy(random, (U8 *)rand, 4);
		random+=4;
		byteLen-=4;
	}

	if(byteLen)
	{
		GetRandU32(rand, 1);
		memcpy(random, (U8 *)rand, byteLen);
	}
}


/* Function: get U8 random number
 * Parameters:
 *     random ------ pointer to U8 random number
 *     byteLen ----- byte length of random number
 * Return: 
 * Caution:must be called after srand();
 */
void GetRandU8_2(U8 random[], U32 byteLen)
{
	U32 i;

	for(i=0; i<byteLen; i++)
	{
		random[i] = (U8)rand();
	}
}
