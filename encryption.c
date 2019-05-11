#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "encryption.h"
#include <limits.h>

#define LOOPS 5;

int32_t gen_32bit()
{
  int32_t number = 0;

  for(int i = 24; i >= 0; i -= 8)
    number |= ((int32_t)(rand() % 255)) << i;

  return abs(number);
}

uint64_t modmult(uint64_t a,uint64_t b,uint64_t mod)
{
    uint64_t sum = 0;

    //    printf("\nMultiplicate %lu * %lu mod %lu= ",a,b,mod);
    
    if (a == 0 || b < mod / a)
        return (a*b)%mod;
    
    while(b>0)
    {
        if(b&1)
            sum = (sum + a) % mod;
        a = (2*a) % mod;
        b>>=1;
    }
    //printf("%lu",sum);
    
    return sum;
}

uint64_t mod_pow(uint64_t number, uint64_t power, uint64_t mod)
{
  /*
  //number = 8, power = 4283024522089987, mod = 6424537105385257
  uint64_t result = 1;

  if(mod == 1)
    return 0;

  //result = 1    number = 64    power = 212141512261044993
  //result = 1    number = 4096  power = 1070756130522496
  //result 4096   number = 
  
  while(power > 0)
  {
    if(power % 2 == 1)
      result = (result*number) % mod;

    power = power >> 1;
    number = (number*number) % mod;
  }
 
  return result;
  */
    uint64_t  product,pseq;
    product=1;
    pseq=number%mod;
    while(power>0)
    {
        if(power&1)
            product=modmult(product,pseq,mod);
        pseq=modmult(pseq,pseq,mod);
        power>>=1;
    }
    
    return product;
}

int check_witness(uint32_t odd_comp, uint32_t number, uint32_t power)
{
  uint32_t random_num = (gen_32bit()%(number-4))+2;
  uint32_t temp = mod_pow(random_num,odd_comp,number);
    
   if(temp == 1 || temp == number-1)
     return 1;
  
   while( temp != 1 && temp != (number-1) && odd_comp != (number-1))
   {
     temp = (temp*temp) % number;
     odd_comp *= 2;
   }

   if(temp != number-1 && odd_comp % 2 == 0)
       return 0;

   return 1;
}

int miller_rabin_test(uint32_t number)
{
  uint32_t decomp = 0;
  uint32_t power = 0, odd_comp = 0;
  int test;
  
  if(number % 2 == 0 && number != 2)
    return 0;

  decomp = number - 1;
  odd_comp = decomp;

  while(odd_comp % 2 == 0)   //We re looking for a number satisfying 2^u*r = n1
  {
    power++;
    odd_comp /= 2;
  }

  for(int i = 0; i < 10; i++)
  {
    if(!check_witness(odd_comp,number,power))
      return 0;
  }
  
  return 1;
}

int32_t generate_prime()
{
  int32_t number = gen_32bit();

  do
  {
    number++;
  } while(!miller_rabin_test(number));

  printf("Prime = %u\n",number);

  return number;
}

uint64_t gcd(uint64_t first, uint64_t second)
{
  uint64_t temp;

  while(second != 0)
  {
    temp = first % second;

    first = second;
    second = temp;
  }

  return first;
}

uint64_t lcm(uint64_t first, uint64_t second)
{
  return (first*second)/gcd(first,second);
}

void generate_keys(struct public_key* pubk, struct private_key* privk)
{
  int32_t first_prime = generate_prime();
  int32_t second_prime = generate_prime();
  int64_t totient_n = (int64_t) (first_prime-1)*(second_prime-1);
  int64_t temp1,temp2;
  
  printf("First: %u, Second: %u, Totient: %lu\n",first_prime,second_prime,totient_n);
  
  privk->n = (int64_t) first_prime*second_prime; 
  pubk->n = privk->n;

  printf("N: %lu\n",privk->n);
  
  pubk->e = 2;
  while(gcd(pubk->e,totient_n) != 1)        //Find e such as 1 < e < totient_n and gcd(e,totient_n) == 1
  {
    pubk->e++;
  };

  printf("E: %u\n",pubk->e);

  temp1 = (1+2*totient_n);
  temp2 = temp1/pubk->e;

  printf("Temp1: %lu, Temp2: %lu\n",temp1,temp2);
  
  privk->d = mul_mod_inv(pubk->e,totient_n);       //Calculate the private key component using extended euclidean algorithm

  printf("%lu * %u mod %lu = 1\n",privk->d,pubk->e,totient_n);
}

void encrypt(uint64_t* values, int size, struct public_key* key)
{
  for(int i = 0; i < size; i++)
  {
  //  printf("\n%lu^%u mod %lu = ",values[i],key->e,key->n);
    values[i] = mod_pow(values[i],key->e,key->n);

    //printf("%lu",values[i]);
  }
}

void decrypt(uint64_t* values, int size, struct private_key* key)
{
  for(int i = 0; i < size; i++)
    values[i] = mod_pow(values[i],key->d,key->n);
}

int64_t mul_mod_inv(int64_t e, int64_t totient)
{
  int64_t totient0 = totient;
  int64_t y = 0, x = 1;
  int64_t q,t;
  
  if(totient == 1)
    return 0;

  while(e > 1)
  {
    q = e/totient;
    t = totient;

    totient = e % totient;
    e = t;
    t = y;

    y = x - q*y;
    x = t;
  }

  if(x < 0)
    x += totient0;

  return x;
}
