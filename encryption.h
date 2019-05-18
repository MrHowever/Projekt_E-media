#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <math.h>
#include <stdint.h>

struct public_key
{
  int64_t n;
  int32_t e;
};

struct private_key
{
  int64_t n;
  int64_t d;
};

int32_t gen_32bit();
int32_t generate_prime();
void generate_keys(struct public_key*, struct private_key*);

uint64_t mod_pow(uint64_t,uint64_t,uint64_t);
uint64_t modmult(uint64_t,uint64_t,uint64_t);
int64_t mul_mod_inv(int64_t,int64_t);

int check_witness(uint32_t,uint32_t,uint32_t);
int miller_rabin_test(uint32_t);

uint64_t gcd(uint64_t,uint64_t);

uint64_t encrypt(uint64_t, struct public_key*);
uint64_t decrypt(uint64_t, struct private_key*);

#endif
