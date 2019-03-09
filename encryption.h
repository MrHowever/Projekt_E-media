#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <math.h>

struct public_key
{
  uint64_t n;
  uint32_t e;
};

struct private_key
{
  uint64_t n;
  uint64_t d;
};

uint32_t gen_32bit();
uint32_t mod_pow(uint64_t,uint64_t,uint64_t);
int check_witness(uint32_t,uint32_t,uint32_t);
int miller_rabin_test(uint32_t);
uint32_t generate_prime();

void generate_keys(struct public_key*, struct private_key*);
uint64_t gcd(uint64_t,uint64_t);
uint64_t lcm(uint64_t,uint64_t);

void encrypt(uint64_t*, int, struct public_key*);
void decrypt(uint64_t*, int, struct private_key*);

uint64_t mul_mod_inv(uint64_t,uint64_t);
uint64_t ext_euclidean(uint64_t,uint64_t,uint64_t*,uint64_t*);

#endif
