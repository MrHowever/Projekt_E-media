#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "encryption.h"
#include <limits.h>

#define LOOPS 5;

//Function that generates 32 bit value by generating 4 values using rand()
//and bit-shifting them. Bit-shifting makes sure that the value is big enough.
int32_t gen_32bit()
{
  int32_t number = 0;

  for(int i = 24; i >= 0; i -= 8)
    number |= ((int32_t)(rand() % 255)) << i;

  return abs(number);
}

//Encrypt value using RSA algorithm
uint64_t encrypt(uint64_t value, struct public_key* key)
{
    return mod_pow(value,key->e,key->n);
}

//Decrypt value that was encrypted using RSA algorithm
uint64_t decrypt(uint64_t value, struct private_key* key)
{
    return mod_pow(value,key->d,key->n);
}

//Function that returns a random prime number. It works by generating random
//32 bit value and then incrementing it until it finds a prime number.
//Primality of a number is confirmed by a Miller-Rabin test
int32_t generate_prime()
{
    int32_t number = gen_32bit();

    do
    {
        number++;
    } while(!miller_rabin_test(number));

    return number;
}

//Function that returns greatest common divisor of two numbers
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


//Function that generates private and public keys for RSA encryption.
//Private key consist of modulus n and parameter d
//Public key consists of modulus n and parameter e
void generate_keys(struct public_key* pubk, struct private_key* privk)
{
    //Generate two prime numbers
    int32_t first_prime = generate_prime();
    int32_t second_prime = generate_prime();

    //Calculate Euler's totient
    int64_t totient_n = (int64_t) (first_prime-1)*(second_prime-1);

    //Calculate the modulus
    privk->n = (int64_t) first_prime*second_prime;
    pubk->n = privk->n;

    //Find smallest value e for which condition gcd(e,n) == 1 is true
    pubk->e = 2;
    while(gcd(pubk->e,totient_n) != 1)        //Find e such as 1 < e < totient_n and gcd(e,totient_n) == 1
    {
        pubk->e++;
    };

    //Calculate the private key component using extended euclidean algorithm
    privk->d = mul_mod_inv(pubk->e,totient_n);
}

//Function calculating modular multiplicative inverse using extended
//Euclidean algorithm
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

//Function calculating modular multiplicative of two numbers
uint64_t modmult(uint64_t a,uint64_t b,uint64_t mod)
{
    uint64_t sum = 0;

    if (a == 0 || b < mod / a)
        return (a*b)%mod;
    
    while(b>0)
    {
        if(b&1)
            sum = (sum + a) % mod;
        a = (2*a) % mod;
        b>>=1;
    }

    return sum;
}

//Function that calculates a power of a number under modulus
uint64_t mod_pow(uint64_t number, uint64_t power, uint64_t mod)
{
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

//Function that checks whether odd_comp is a witness to number's primality
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

//Function that determines whether a number is prime using probabilistic
//Rabin-Miller test
int miller_rabin_test(uint32_t number)
{
  uint32_t decomp = 0;
  uint32_t power = 0, odd_comp = 0;
  int test;

  //If number is even return false
  if(number % 2 == 0 && number != 2)
    return 0;

  //Number is an odd number, decomp is an even number
  decomp = number - 1;
  odd_comp = decomp;

  //Find the biggest power of 2 that satisfies the equation 2^u*r = number
  while(odd_comp % 2 == 0)
  {
    power++;
    odd_comp /= 2;
  }

  //odd_comp is now an odd number
  //Check 10 times if odd_comp is a witness to number's primality, if it is
  //return true. This algorithm is probabilistic, more loops means more
  //accurate result
  for(int i = 0; i < 10; i++)
  {
    if(!check_witness(odd_comp,number,power))
      return 0;
  }
  
  return 1;
}

