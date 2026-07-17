# OSR-303 Projects
To make it easy to quickly use the OSR-303 for side-channel evaluation of cryptographic algorithms, we will progressively develop and open-source `STM32CubeIDE` projects for different cryptographic algorithms. The plan is as follows:

| Algorithm          | Status | Reference implementation |
| :----------------- | :---:|  :---:  | 
| AES                |  ✓  | [Dani Huertas](https://github.com/dhuertas/AES) |
| AES T-table        |  ✓  | [Rijndael](https://github.com/zakird/zdlibc/blob/master/rijndael-alg-fst.c) |
| Masked AES         |  ✓  | [MELITY project](https://github.com/CENSUS/masked-aes-c) |
| DES                |  ✓  | OSR |
| ASCON              |  ✓  |  [Southern Storm](https://github.com/rweather/lwc-finalists/tree/master/src/individual/ASCON) |
| SM4                |  ✓  | OSR | 
| HMAC-SM3           |  ✓  | [GmSSL Project](https://github.com/ChainSQL/GmSSL)| 
| SM2                |  ✓  | OSR | 
| HMAC-SHA256        |  ✓  | OSR | 
| ECDSA              |  ✓  | [Manuel Pégourié-Gonnard](https://github.com/mpg/p256-m)| 
| RSA                |  ✓  | [PolarSSL](https://github.com/dsommers/polarssl/blob/master/library/rsa.c) | 
| ML-KEM             |  ✓  | [PQM4](https://github.com/mupq/pqm4) | 
| ML-DSA             |  ✓  | [PQM4](https://github.com/mupq/pqm4) | 
| NTRU               |  ✓  | [ntru-crypto](https://github.com/yedino/ntru-crypto) |
| mbedTLS-AES        |  ✓  | mbedTLS v2.26 |
| mbedTLS-RSA        |  ✓  | mbedTLS v2.26 |
