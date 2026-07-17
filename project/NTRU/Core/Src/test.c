/******************************************************************************
 * NTRU Cryptography Reference Source Code
 *
 * Copyright (C) 2009-2016  Security Innovation (SI)
 *
 * SI has dedicated the work to the public domain by waiving all of its rights
 * to the work worldwide under copyright law, including all related and
 * neighboring rights, to the extent allowed by law.
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * You can copy, modify, distribute and perform the work, even for commercial
 * purposes, all without asking permission. You should have received a copy of
 * the creative commons license (CC0 1.0 universal) along with this program.
 * See the license file for more information.
 *
 *
 *********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ntru_crypto.h"
#include "test.h"

 /* entropy function
  *
  * THIS IS AN EXAMPLE FOR WORKING SAMPLE CODE ONLY.
  * IT DOES NOT SUPPLY REAL ENTROPY BECAUSE THE RANDOM SEED IS FIXED.
  *
  * IT SHOULD BE CHANGED SO THAT EACH COMMAND THAT REQUESTS A BYTE
  * OF ENTROPY RECEIVES A RANDOM BYTE.
  *
  * Returns 1 for success, 0 for failure.
  */
uint8_t
	get_entropy(
	    ENTROPY_CMD  cmd,
	    uint8_t* out)
	{
	    /* 2k/8 bytes of entropy are needed to instantiate a DRBG with a
	     * security strength of k bits. Here k = 112.
	     */
	    static uint8_t seed[28] = {
	        'P','l','e','a','s','e',' ','u','s','e',' ','a',' ',
	        'd','i','f','f','e','r','e','n','t',' ','s','e','e','d','!'
	    };
	    static size_t index;

	    if (cmd == INIT) {
	        /* Any initialization for a real entropy source goes here. */
	        index = 0;
	        return 1;
	    }

	    if (out == NULL)
	        return 0;

	    if (cmd == GET_NUM_BYTES_PER_BYTE_OF_ENTROPY) {
	        /* Here we return the number of bytes needed from the entropy
	         * source to obtain 8 bits of entropy.  Maximum is 8.
	         */
	        *out = 1;                       /* this is a perfectly random source */
	        return 1;
	    }

	    if (cmd == GET_BYTE_OF_ENTROPY) {
	        if (index == sizeof(seed))
	            return 0;                   /* used up all our entropy */

	        *out = seed[index++];           /* deliver an entropy byte */
	        return 1;
	    }
	    return 0;
	}

/* Dumps a buffer in hex to the screen for debugging */
void
DumpHex(
    const unsigned char* buf,
    int len)
{
    int i;
    for (i = 0;i < len;i++)
    {
        if (i & 0x1f) printf(":");
        printf("%02X", buf[i]);
        if ((i & 0x1f) == 0x1f) printf("\n");
    }
    printf("\n");
}


/* main
 *
 * This sample code will:
 *   1) generate a public-key pair for the EES401EP2 parameter set
 *   2) DER-encode the public key for storage in a certificate
 *   3) DER-decode the public key from a certificate for use
 *   4) encrypt a 128-bit AES key
 *   5) decrypt the 128-bit AES key
 */
