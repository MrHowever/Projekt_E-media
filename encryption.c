#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "encryption.h"

#define LOOPS 5;

uint32_t gen_32bit()
{
  uint32_t number = 0;

  for(int i = 24; i >= 0; i -= 8)
    number |= ((uint32_t)(rand() % 255)) << i;

  return number;
}

uint32_t mod_pow(uint64_t number, uint64_t power, uint64_t mod)
{
  uint64_t result = 1;
  uint64_t e = 0;
  uint64_t base = base % mod;
  
  if(mod == 1)
    return 0;
  
  while(power > 0)
  {
    if(power % 2 == 1)
      result = (result*number) % mod;

    power = power >> 1;
    number = (number*number) % mod;
  }
 
  return result;
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

uint32_t generate_prime()
{
  uint32_t number = gen_32bit();

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
  uint32_t first_prime = generate_prime();
  uint32_t second_prime = generate_prime();
  uint64_t totient_n = (uint64_t) (first_prime-1)*(second_prime-1);
  uint64_t temp1,temp2;
  
  printf("First: %u, Second: %u, Totient: %lu\n",first_prime,second_prime,totient_n);
  
  privk->n = (uint64_t) first_prime*second_prime; 
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
  uint64_t val;
  for(uint i = 0; i < size; i++)
  {
    printf("%lu^%u mod %lu = ",values[i],key->e,key->n);
    val = mod_pow(values[i],key->e,key->n);
    values[i] = mod_pow(values[i],key->e,key->n);

    printf("%lu or %lu\n",val,values[i]);
  }
}

void decrypt(uint64_t* values, int size, struct private_key* key)
{
  for(uint i = 0; i < size; i++)
    values[i] = mod_pow(values[i],key->d,key->n);
}

uint64_t mul_mod_inv(uint64_t e, uint64_t totient)
{
  uint64_t totient0 = totient;
  uint64_t y = 0, x = 1;
  uint64_t q,t;
  
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
