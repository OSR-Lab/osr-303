/**
 * \file config-mini-tls1_1.h
 *
 * \brief Minimal configuration for TLS 1.1 (RFC 4346)
 */
 /*
  *  Copyright The Mbed TLS Contributors
  *  SPDX-License-Identifier: Apache-2.0
  *
  *  Licensed under the Apache License, Version 2.0 (the "License"); you may
  *  not use this file except in compliance with the License.
  *  You may obtain a copy of the License at
  *
  *  http://www.apache.org/licenses/LICENSE-2.0
  *
  *  Unless required by applicable law or agreed to in writing, software
  *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  *  See the License for the specific language governing permissions and
  *  limitations under the License.
  */
  /*
   * Minimal configuration for TLS 1.1 (RFC 4346), implementing only the
   * required ciphersuite: MBEDTLS_TLS_RSA_WITH_3DES_EDE_CBC_SHA
   *
   * See README.txt for usage instructions.
   */

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#include <limits.h>

#define MBEDTLS_ERROR_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_OID_C
#define MBEDTLS_RSA_C
#define MBEDTLS_AES_C
#define MBEDTLS_MD_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_GENPRIME
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_PK_C
#define MBEDTLS_SHA256_C


//read PEM certificate
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
//#define MBEDTLS_BASE64_C
#define MBEDTLS_MD2_C
#define MBEDTLS_FS_IO

//padding mode; you must choose one of the two modes, V15 or V21
#define MBEDTLS_PKCS1_V21
//#define MBEDTLS_PKCS1_V15

//do not use the platform default entropy source; mbedtls already implements an entropy source on Windows and Linux
#define MBEDTLS_NO_PLATFORM_ENTROPY
//#include "check_config.h"

#endif

