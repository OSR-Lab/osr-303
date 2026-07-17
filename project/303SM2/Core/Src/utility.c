/*******************************************************************************
 ******     Copyright (c) 2014--2020 OSR.Co.Ltd. All rights reserved.     ****** 
 *******************************************************************************/
#include <stdio.h>
#include <string.h>

#if defined(__linux)
#include <sys/types.h>    // POSIX system data types
#include <sys/stat.h>     // file mode / permission constants
#include <fcntl.h>        // open() and file-control flags
#include <unistd.h>       // read()/write()/close()
#endif

#include "utility.h"




void print_buf_U8_(U8 buf[], U32 byteLen, char name[])
{
	U32 i;

	printf("\r\n %s:\r\n",name);
	for(i=0; i<byteLen; i++)
	{
		//if(i%16 ==0 && i>0)
		//	printf("\r\n");
		//printf("%02x", buf[byteLen-1-i]);
		printf("%02x", buf[i]);
	}
	
	printf("\r\n");
}

void print_buf_U8(U8 buf[], U32 byteLen, char name[])
{
	return print_buf_U8_(buf, byteLen, name);
}

void print_buf(U32 buf[], U32 wordLen, char name[])
{
	U32 i;

	printf("\r\n %s:\r\n",name);
	for(i=0; i<wordLen; i++)
	{
		printf("%08lx", buf[wordLen-1-i]);
		//printf("0x%08x,", buf[i]);

		//printf("0x%08x", buf[i+1]);
		//printf("%08x,", buf[i]);
	}
	printf("\r\n");
}




#if defined(__linux)
void write_string(U32 fd, char *string)
{
	write(fd, string, strlen(string));
}


// write a byte array as hexadecimal characters to fd
void write_hex2char(U32 fd, U8 *in, U32 byteLen)
{
	char buffer[202];
	U32 len = 0;
	U32 tmpLen;
	U32 i;
	char tmp;
	
	while(len < byteLen)
	{
		tmpLen = byteLen-len<100?byteLen-len:100;
		for(i=0; i<tmpLen; i++)
		{
			tmp = (in[len+i]>>4);			
			buffer[i*2] = tmp<10?tmp+'0':tmp+55;
			tmp = (in[len+i]&0x0F);
			buffer[i*2+1] = tmp<10?tmp+'0':tmp+55;
		}
		buffer[i*2]='\0';  //printf("%s", buffer);
		write(fd, buffer, tmpLen*2);
		len += tmpLen;
	}
}
#endif


/* disabled: pseudo-random seed helper code
U8 AA_pseudo_seed[4];

void AA_pseudo_init(U8 seed[4])
{
	memcpy(AA_pseudo_seed, seed, 4);
}

void AA_rand(U8 * rand, U32 byteLen)
{
	U32 i,t;

	for(i=0; i<byteLen; i++)
	{
		t = (U32)AA_pseudo_seed[0]+(U32)AA_pseudo_seed[1]+(U32)AA_pseudo_seed[2]+(U32)AA_pseudo_seed[3];
		t += 0xa9;
		if(t&1)
			t ^= 0x93;
		else
			t ^= 0x4c;
		t *= 0x5a;
		t = (t>>3)|(t<<29);
		t += 1;

		memmove(AA_pseudo_seed, AA_pseudo_seed+1, 3);
		AA_pseudo_seed[3] = (U8)t;
		rand[i] = AA_pseudo_seed[3];
	}	
}
*/
