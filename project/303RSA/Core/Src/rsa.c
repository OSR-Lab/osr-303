#include "rsa.h"


int rsa(char* message, char* exponent, char* modulus, char* out)
{
  mpi M, E, N;
  mpi OUT;
  int len = 256;
  int res;
  mpi_init( &M, &E, &N, &OUT, NULL);
  
  res = mpi_read_string( &M, 16, message); 
  res = mpi_read_string( &E, 16, exponent);
  
  res = mpi_read_string( &N, 16, modulus);
  
  mpi_exp_mod(&OUT, &M, &E, &N, NULL );
  mpi_write_string(&OUT, 16, out, &len);
  
  mpi_free(&OUT, &M, &E, &N, NULL);
  return len;
}

int rsa_crt(char* message, char* p, char* q, char* dp, char* dq, char* q_inv, char* out)
{
  mpi M, P, Q, DP, DQ, QINV;
  mpi MP, MQ, H;
  mpi TMP;
  mpi OUT;
  int slen = 256;
  
  mpi_init( &M, &P, &Q, &DP, &DQ, &QINV, NULL);
  mpi_init( &MP, &MQ, &H, NULL);
  mpi_init(&TMP, NULL);
  
  mpi_read_string( &M, 16, message); 
  mpi_read_string( &P, 16, p);
  mpi_read_string( &Q, 16, q);
  mpi_read_string( &DP, 16, dp);
  mpi_read_string( &DQ, 16, dq);
  
  
  // MP = M^DP mod P
  mpi_exp_mod( &MP, &M, &DP, &P, NULL );
  
  
  // MQ = M^DQ mod q
  mpi_exp_mod( &MQ, &M, &DQ, &Q, NULL );

  
  mpi_read_string( &QINV, 16, q_inv);
  
  // H = QINV * (MP - MQ) mod P;
  
  mpi_sub_mpi( &TMP, &MP, &MQ );
  mpi_mul_mpi( &TMP, &TMP, &QINV );
  mpi_mod_mpi( &H, &TMP, &P);

 
  mpi_init(&OUT, NULL);
  mpi_mul_mpi(&TMP, &H, &Q);
  mpi_add_mpi(&OUT, &TMP, &MQ);
// 
  mpi_write_string(&OUT, 16, out, &slen); 
  mpi_free(&M,&DP,&DQ, NULL );
  mpi_free(&P,&Q,&QINV, &MP, &MQ, &H, &TMP, &OUT, NULL );
  return slen;
}
