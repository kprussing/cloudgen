/* random.c -- Random number generator 
   Copyright (C) 2003 Robin Hogan <r.j.hogan@reading.ac.uk> */
#include <stdlib.h>
#include <math.h>
#include "random.h"

static FILE *dev_random = NULL;

/* This is essentially the "ran2" function from "Numerical Recipies"
   (Press et al. 1988). It has been split into two functions, one to
   seed and the other to fetch the next value */

#define M 714025
#define IA 1366
#define IC 150889

static long iy, ir[98];
static int iff = 0;
static long idum;

void
seed_random_number_generator(long seed)
{
  int j;
  iff = 1;
  if ((seed=(IC-seed) % M) < 0)
    seed = -seed;
  for (j = 1; j <= 97; j++) {
    seed = (IA*seed+IC) % M;
    ir[j] = seed;
  }
  iy = (IA*seed+IC) % M;
  idum = seed;
}

static
float
nr_uniform_deviate(void)
{
  int j;
  if (iff == 0) {
    seed_random_number_generator(-1);
  }
  j = 1 + 97.0*iy/M;
  if (j > 97 || j < 1) {
    fprintf(stderr, "Error in random number generator\n");
    exit(1);
  }
  iy = ir[j];
  idum = (IA*idum+IC) % M;
  ir[j] = idum;
  return (float) iy/M;
}

/* By default we use the nr_uniform_deviate function */
float (*uniform_deviate) (void) = *nr_uniform_deviate;

/* Recent versions of linux provide two devices, /dev/random and
   /dev/urandom. The first provides a source of high quality random
   numbers derived from interrupt timings.  The second provides lower
   quality random numbers using interrupt timings when available and a
   more standard algorithm if not - this is much faster.  Use the
   set_kernel_random_file() function to set the file to use. */

static
float
kernel_uniform_deviate(void)
{
  unsigned short value;
  fread(&value, 2, 1, dev_random);
  return ((float) value)/65536.0;
}

int
kernel_int_seed(void)
{
  int value;
  if (dev_random) {
    fread(&value, sizeof(int), 1, dev_random);
    return value;
  }
  else {
    return 1;
  }
}

FILE *
open_kernel_random_file(char *file_name)
{
  dev_random = fopen(file_name, "r");
  if (dev_random) {
    uniform_deviate = kernel_uniform_deviate;
  }
  return dev_random;
}

void
close_kernel_random_file(void)
{
  if (dev_random) {
    fclose(dev_random);
  }
  uniform_deviate = nr_uniform_deviate;
}

/* Calculate Gaussian deviates from uniform deviates - this is the
   "gasdev" function from Numerical Recipies */
float
gaussian_deviate(void)
{
  static int iset = 0;
  static float gset;
  float fac, r, v1, v2;

  if (iset == 0) {
    do {
      v1 = 2.0 * uniform_deviate() - 1.0;
      v2 = 2.0 * uniform_deviate() - 1.0;
      r = v1*v1 + v2*v2;
    }
    while (r >= 1.0 || r == 0.0);

    fac = sqrt(-2.0*log(r)/r);
    gset = v1*fac;
    iset = 1;
    return v2*fac;
  }
  else {
    iset = 0;
    return gset;
  }
}

/* A function for generating 32 random bits */
unsigned int
bitfield32_deviate(void)
{
  static long ix1 = 1;
  static long ix2 = 1;
  unsigned long r;
  ix1 = (3877*ix1 + 29573) % 139968;
  ix2 = (8121*ix2 + 28411) % 134456;
  r = ix1*(65535.0/(139968-1.0));
  return r*65535;/* + ix2*(65535.0/(134456-1.0)); */
}

/* */
unsigned int
hash_32(int n, unsigned char *sequence, unsigned int hash)
{
  int i;
  for (i = 0; i < n; ++i) {
    hash = FNV_32_HASH(hash, sequence[i]);
  }
  return hash;
}

unsigned int
seeded_uniform_deviates(int n, float *target, unsigned int seed)
{
  int i;
  for (i = 0; i < n; ++i) {
    target[i] = (float) (seed = (IA*seed+IC) % M)/M;
  }
  return seed;
}
