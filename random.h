/* random.c -- Random number generator 
   Copyright (C) 2003 Robin Hogan <r.j.hogan@reading.ac.uk> */
#include <stdio.h>

/* Seed the pseudo-random number generator */
void seed_random_number_generator(long seed);

/* Return a random float between 0.0 and 1.0. 714025 possible values
   can be returned. The function prototype should be treated as "float
   uniform_deviate()" */
extern float (*uniform_deviate) ();

/* Return a float drawn from a Gaussian probability distribution with
   a mean of 0 and a standard deviation of 1. */
float gaussian_deviate();

/* Return 32 pseudo-random bits */
unsigned int bitfield32_deviate();

/* Use file_name (usually "/dev/random" or "/dev/urandom" on linux) as
   a source of high-quality random bits in subsequent calls to
   kernel_int_seed(), uniform_deviate() and gaussian_deviate(). If the
   file cannot be opened NULL is returned and calls revert to the
   pseudo-random number generator. Note that the kernel slow at
   generating random numbers so it is often worth using the kernel to
   produce a seed for the pseudo-random number generator. */
FILE *open_kernel_random_file(char *file_name);

/* Seed the pseudo-random number generator using the kernel random
   file, and return the seed. */
int kernel_int_seed();

/* Close the kernel file and use the pseudo-random number generator
   for all subsequent random number calls. */
void close_kernel_random_file();

/* Fowler/Noll/Vo hash functions */
#define FNV_32_PRIME 16777619
#define FNV_32_INIT 2166136261

#define FNV_32_HASH(hash, octet) \
  (((unsigned int)(hash) ^ (unsigned char)(octet)) * FNV_32_PRIME)

#define FNV_64_PRIME 1099511628211
#define FNV_64_INIT 14695981039346656037

#define FNV_64_HASH(hash, octet) \
  (((unsigned long long int)(hash) ^ (unsigned char)(octet)) * FNV_64_PRIME)

/* Fast hash function using the FNV algorithm */
unsigned int hash_32(int n, unsigned char *sequence, unsigned int init);

/* Fill target with n uniform deviates, the seed typically taken from
   hash_32(), returning the new seed for further calls */
unsigned int seeded_uniform_deviates(int n, float *target, unsigned int seed);

