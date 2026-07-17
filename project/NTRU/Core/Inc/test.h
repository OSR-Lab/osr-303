/*
 * test.h
 *
 *  Created on: Dec 9, 2024
 *      Author: osr
 */

#ifndef INC_TEST_H_
#define INC_TEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ntru_crypto.h"

uint8_t get_entropy(ENTROPY_CMD  cmd, uint8_t* out);
void DumpHex(const unsigned char* buf, int len);

#endif /* INC_TEST_H_ */
