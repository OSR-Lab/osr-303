#include<stdio.h>
#include<string.h>
#include<stdint.h>

//IP permutation matrix
uint8_t IP_Table[64] = {
	58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
	57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7 };

//expansion matrix
uint8_t E_Table[48] = {
	32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
	8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1 };

// P-box
uint8_t P_Table[32] = {
	16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
	2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25 };

//inverse IP permutation matrix
uint8_t IPR_Table[64] = {
	40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
	38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
	36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
	34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25 };

//key first permutation matrix (PC-1)
uint8_t PC1_Table[56] = {
	57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
	63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4 };

//key second permutation matrix (PC-2)
uint8_t PC2_Table[48] = {
	14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32 };

//8 S-boxes (three-dimensional array)
uint8_t S_Box[8][4][16] = {
	{
		{14, 4,  13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
		{0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
		{4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
		{15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13}
	},
	{
		{15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
		{3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
		{0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
		{13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9}
	},
	{
		{10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
		{13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
		{13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
		{1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12}
	},
	{
		{7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15},
		{13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9},
		{10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4},
		{3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14}
	},
	{
		{2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9},
		{14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6},
		{4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14},
		{11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3}
	},
	{
		{12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11},
		{10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8},
		{9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6},
		{4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13}
	},
	{
		{4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1},
		{13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6},
		{1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2},
		{6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12}
	},
	{
		{13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7},
		{1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2},
		{7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8},
		{2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11}
	}
};

//convert CHAR to INT
void CharToBit(uint8_t input[], uint8_t output[])
{
	int i, j;
	for (j = 0; j<8; j++)
	{
		for (i = 0; i<8; i++)
		{
			output[7 * (j + 1) - i + j] = (input[j] >> i) & 1;
		}
	}
};

//convert INT to CHAR
void BitToChar(uint8_t intput[], uint8_t output[])
{
	int i, j;
	for (j = 0; j<8; j++)
	{
		for (i = 0; i<8; i++)
		{
			output[j] = output[j] * 2 + intput[i + 8 * j];
		}
	}
};

//XOR operation
void Xor(uint8_t *INA, uint8_t *INB, int len)
{
	int i;
	for (i = 0; i<len; i++)
	{
		*(INA + i) = *(INA + i) ^ *(INB + i);
	}
};

//initial IP permutation
void IP(uint8_t input[64], uint8_t output[64], uint8_t table[64])
{
	int i;
	for (i = 0; i<64; i++)
	{
		output[i] = input[table[i] - 1];//the minus-1 operation is essential!!
	}
};

//E expansion
void E(uint8_t input[32], uint8_t output[48], uint8_t table[48])
{
	int i;
	for (i = 0; i<48; i++)
	{
		output[i] = input[table[i] - 1];
	}
};

//P permutation
void P(uint8_t input[32], uint8_t output[32], uint8_t table[32])
{
	int i;
	for (i = 0; i<32; i++)
	{
		output[i] = input[table[i] - 1];
	}
};

//inverse IP
void IP_In(uint8_t input[64], uint8_t output[64], uint8_t table[64])
{
	int i;
	for (i = 0; i<64; i++)
	{
		output[i] = input[table[i] - 1];
	}
};

//PC_1
void PC_1(uint8_t input[64], uint8_t output[56], uint8_t table[56])
{
	int i;
	for (i = 0; i<56; i++)
	{
		output[i] = input[table[i] - 1];
	}
};

//PC_2
void PC_2(uint8_t input[56], uint8_t output[48], uint8_t table[48])
{
	int i;
	for (i = 0; i<48; i++)
	{
		output[i] = input[table[i] - 1];
	}
};

//S-box compression
void S(uint8_t input[48], uint8_t output[32], uint8_t table[8][4][16])
{
	int i = 0;
	int j = 0;
	uint8_t INT[8];
	for (; i<48; i = i + 6)
	{
		INT[j] = table[j][(input[i] << 1) + (input[i + 5])][(input[i + 1] << 3) + (input[i + 2] << 2) + (input[i + 3] << 1) + (input[i + 4])];
		j++;
	}
	for (j = 0; j<8; j++)
	{
		for (i = 0; i<4; i++)
		{
			output[3 * (j + 1) - i + j] = (INT[j] >> i) & 1;
		}
	}
};

//perform the DES round transformation
void F_func(uint8_t input[32], uint8_t output[32], uint8_t subkey[48])
{
	int len = 48;
	uint8_t temp[48] = { 0 };
	uint8_t temp_1[32] = { 0 };
	E(input, temp, E_Table);
	Xor(temp, subkey, len);
	S(temp, temp_1, S_Box);
	P(temp_1, output, P_Table);
};

//key circular left shift
void RotateL(uint8_t input[28], uint8_t output[28], uint8_t leftCount)
{
	int i;
	int len = 28;
	for (i = 0; i<len; i++)
	{
		output[i] = input[(i + leftCount) % len];
	}
};

//subkey generation
void des_subkey_gen(uint8_t keyIn[8], uint8_t Subkey[16][48])
{
	int loop = 1, loop_2 = 2;
	int i, j;
	uint8_t c[28], d[28], key[64];
	uint8_t pc_1[56] = { 0 };
	uint8_t pc_2[16][56] = { 0 };
	uint8_t rotatel_c[16][28] = { 0 };
	uint8_t rotatel_d[16][28] = { 0 };
	CharToBit(keyIn, key);

	PC_1(key, pc_1, PC1_Table);
	for (i = 0; i<28; i++)
	{
		c[i] = pc_1[i];
		d[i] = pc_1[i + 28];
	}
	int leftCount = 0;
	for (i = 1; i<17; i++)
	{
		if (i == 1 || i == 2 || i == 9 || i == 16)
		{
			leftCount += loop;
			RotateL(c, rotatel_c[i - 1], leftCount);
			RotateL(d, rotatel_d[i - 1], leftCount);
		}
		else
		{
			leftCount += loop_2;
			RotateL(c, rotatel_c[i - 1], leftCount);
			RotateL(d, rotatel_d[i - 1], leftCount);
		}
	}
	for (i = 0; i<16; i++)
	{
		for (j = 0; j<28; j++)
		{
			pc_2[i][j] = rotatel_c[i][j];
			pc_2[i][j + 28] = rotatel_d[i][j];
		}
	}
	for (i = 0; i<16; i++)
	{
		PC_2(pc_2[i], Subkey[i], PC2_Table);
	}
};

//DES encryption
void des_encrypt(uint8_t pt[8], uint8_t subkeys[16][48], uint8_t ct[8])
{
	uint8_t Ip[64] = { 0 };//store the matrix after the initial permutation
	uint8_t output[64] = { 0 };
	uint8_t plainbit[64] = { 0 };
	uint8_t cipherbit[64] = { 0 };
	uint8_t l[17][32], r[17][32];
	int i, j, k;

	CharToBit(pt, plainbit);
	IP(plainbit, Ip, IP_Table); //initial IP permutation!

	for (i = 0; i<32; i++)
	{
		l[0][i] = Ip[i];
		r[0][i] = Ip[32 + i];
	}

	//operations for the first 15 rounds
	for (j = 1; j<16; j++)
	{
		for (k = 0; k<32; k++)
		{
			l[j][k] = r[j - 1][k];
		}
		F_func(r[j - 1], r[j], subkeys[j - 1]);
		Xor(r[j], l[j - 1], 32);
	}

	//operations for the last round
	for (k = 0; k<32; k++)
	{
		r[16][k] = r[15][k];
	}
	F_func(r[15], l[16], subkeys[15]);
	Xor(l[16], l[15], 32);
	for (k = 0; k<32; k++)
	{
		output[k] = l[16][k];
		output[32 + k] = r[16][k];
	}
	IP_In(output, cipherbit, IPR_Table);
	BitToChar(cipherbit, ct);
};

//DES decryption
void des_decrypt(uint8_t ct[8], uint8_t subkeys[16][48], uint8_t pt[8])
{
	uint8_t Ip[64] = { 0 };//store the matrix after the initial permutation
	uint8_t output_1[64] = { 0 };
	uint8_t plainbit[64] = { 0 };
	uint8_t cipherbit[64] = { 0 };
	uint8_t l[17][32], r[17][32];
	int i, j, k;

	CharToBit(ct, cipherbit);
	IP(cipherbit, Ip, IP_Table);

	for (i = 0; i<32; i++)
	{
		l[0][i] = Ip[i];
		r[0][i] = Ip[32 + i];
	}

	//operations for the first 15 rounds
	for (j = 1; j<16; j++)
	{
		for (k = 0; k<32; k++)
		{
			l[j][k] = r[j - 1][k];
		}
		F_func(r[j - 1], r[j], subkeys[16 - j]);
		Xor(r[j], l[j - 1], 32);
	}

	//operations for the last round
	for (k = 0; k<32; k++)
	{
		r[16][k] = r[15][k];
	}
	F_func(r[15], l[16], subkeys[0]);
	Xor(l[16], l[15], 32);
	for (k = 0; k<32; k++)
	{
		output_1[k] = l[16][k];
		output_1[32 + k] = r[16][k];
	}
	IP_In(output_1, plainbit, IPR_Table);
	BitToChar(plainbit, pt);
};
