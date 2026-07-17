#include "Type.h"






void print_buf(U32 buf[], U32 wordLen, char name[]);

void print_buf_U8_(U8 buf[], U32 byteLen, char name[]);


#if defined(__linux)
void write_string(U32 fd, char *string);

// write a byte array as hexadecimal characters to fd
void write_hex2char(U32 fd, U8 *in, U32 byteLen);
#endif
